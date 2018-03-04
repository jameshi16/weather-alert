#include "alerter.hpp"

#include <sstream>

Alerter::Alerter(HWND eventWindow) : m_pSource(0), m_pSession(0) {
    HRESULT hr;
    if (ref_count == 0)
        hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET); //starts up MF without sockets.

    if (SUCCEEDED(hr)) {
        alerterCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL); //not inheritble, auto-reset, no signal, nameless
        if (alerterCloseEvent == NULL) {
            std::stringstream ss;
            ss << "Could not create event for use in alerter. HRESULT Error: " << HRESULT_FROM_WIN32(GetLastError());
            throw std::runtime_error(std::string(ss.str().c_str()));
        }

        eventWindow = m_eventWindow;
    }

    ref_count++;
}

Alerter::~Alerter() {
    Shutdown();
    ref_count--;
}

AlertReturnCode Alerter::getLastError() {
    return lastError;
}

void Alerter::setMode(AlerterMode am) {
    if (m_mediaState != CLOSED && m_mediaState != READY)
        lastError = CANNOT_CHANGE_MODE_WHILE_PLAYING;
    else lastError = SUCCESS;

    a_mode = am;
}

void Alerter::playAudio() {
    switch (a_mode) {
        case BACKGROUND: {
            HRESULT result = PlayAudioStream();
            if (FAILED(result))
                lastError = FAILED_TO_PLAY;
            else lastError = SUCCESS;
            break;
        }
        case DUCKING: {
            lastError = UNSUPPORTED_OPERATION;
            break;
        }
        case EXCLUSIVE: {
            lastError = UNSUPPORTED_OPERATION;
            //HRESULT result = PlayExclusiveStream();
            break;
        }
        default:
            throw std::logic_error("Unexpected case reached. Contact the application owner.");
    }
}

void Alerter::stopAudio() {
    if (m_mediaState != PLAYING) {
        lastError = FAILED_TO_STOP;
        return;
    }

    if (m_mediaState == NULL) {
        lastError = FAILED_TO_STOP;
        return;
    }

    HRESULT hr = m_pSession->Stop();
    if (SUCCEEDED(hr))
        m_medaiaState = STOP;

    lastError = SUCCESS;
    return;
}

HRESULT Alerter::Shutdown() {
    HRESULT hr = StopSession(); //closes the session
    
    if (ref_count == 1)
        MFShutdown(); //shuts down the platform

    if (alerterCloseEvent) {
        CloseHandle(alerterCloseEvent); //closes the handle
        alerterCloseEvent = NULL;
    }

    return hr;
}

HRESULT Alerter::Invoke(IMFAsyncResult *pResult) {
    MediaEventType meType = MEUnknown; //the event type
    IMFMediaEventPtr pEvent(0); //the event itself

    HRESULT hr = m_pSession->EndGetEvent(pResult, &pEvent); //obtains the event
    if (FAILED(hr))
        return hr;

    hr = pEvent->GetType(&meType); //obtains the event type
    if (FAILED(hr))
        return hr;

    switch (meType) {
        case MESessionClosed:
            SetEvent(alerterCloseEvent); //sets the close event to signalled
            break;
        default:
            hr = m_pSession->BeginGetEvent(reinterpret_cast<IMFAsyncCallback*>(this), NULL); //get the next event
            if (FAILED(hr))
                return hr;
            break;
    }

    //Above, if the message type was to close the event, we won't need to post a message to our event window.
    //Hence, when the state is closing, the event handler is probably waiting for alerterCloseEvent to signal.

    if (m_mediaState != CLOSING) {
        pEvent->AddRef(); //adds a reference to the event

        PostMessage(m_eventWindow, WM_APP_PLAYER_EVENT, 
                    static_cast<WPARAM>(*pEvent), 
                    static_cast<LPARAM>(*meType)); //posts a message to the message window
    }

    return S_OK;
}

HRESULT Alerter::PlayAudioStream() {
    if (m_mediaState != STOP)
        return -1; //it's an error, and I'm lazy to find a suitable error code

    if (m_pSession == NULL || m_pSource == NULL) 
        return E_UNEXPECTED;

    return StartPlayback();
}

HRESULT Alerter::CreatePlaybackTopology(IMFMediaSource* pSource, IMFPresentationDescriptor* pPD, IMFTopology **ppTopology) {
    IMFTopologyPtr pTopology(0); //the topology we're generating
    unsigned long cSourceStreams = 0;

    //Create topology
    HRESULT hr = MFCreateTopology(&pTopology); 
    if (FAILED(hr))
        return hr;
    
    //Get number of streams in the media source
    hr = pPD->GetStreamDescriptorCount(&cSourceStreams);
    if (FAILED(hr))
        return hr;

    // Create topology nodes for each stream, then add to the topology
    for (unsigned long i = 0; i < cSourceStreams; i++) {
        hr = AddBranchToPartialTopology(pTopology, pSource, pPD, i); //adds a new branch to the topology
        if (FAILED(hr))
            return hr;
    }

    *ppTopology = pTopology;
    (*ppTopology)->AddRef();
    return hr;
}

HRESULT Alerter::CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource) {
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID; //object does not exist... yet
    IMFSourceResolverPtr pSourceResolver(0); //interface that can resolve media sources from URL or byte stream
    IUnknownPtr pSource(0); //base interface to most COM interfaces

    /* Create the media source */
    HRESULT hr = MFCreateSourceResolver(&pSourceResolver); //creates the source resolver

    if (FAILED(hr))
        return hr;

    /* Create the object */
    hr = pSourceResolver->CreateObjectFromURL(sURL,
                                            MF_RESOLUTION_MEDIASOURCE,
                                            NULL,
                                            &ObjectType,
                                            &pSource);

    if (FAILED(hr))
        return hr;

    hr = pSource->QueryInterface(IID_IMFMediaSource, reinterpret_cast<void**>(ppSource)); //Get the IMFMediaSource interface from the media source.
    return hr;
}

/**
 * pTopology - The topology interface
 * pSource - The source media. Should have been initialized with CreateMediaSource
 * Presentation Descriptor - the Descriptor for the presentation of the source. Should have been initialized with source->CreatePresentationDescripter
 * The stream number to add to the topoogy
 * 
 * Note that the topology is only partial.
 **/
HRESULT Alerter::AddBranchToPartialTopology(IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor* pPD, DWORD iStream) {
    IMFStreamDescriptorPtr pSD(0); //the stream description, describes the stream.
    IMFActivatePtr pSinkActivate(0);
    IMFTopologyNodePtr pSourceNode(0); //the source node
    IMFTopologyNodePtr pOutputNode(0); //the sink node

    BOOL fSelected = FALSE;

    HRESULT hr = pPD->GetStreamDescriptorByIndex(iStream, &fSelected, &pSD); //obtains the stream descripter. fSelected shows whether or not the stream has been selected.
    if (FAILED(hr))
        return hr;

    if (fSelected) { //if the stream is selected
        hr = CreateMediaSinkActivate(pSD, &pSinkActivate); //creates the media sink activation object
        if (FAILED(hr))
            return hr;

        hr = AddSourceNode(pTopology, pSource, pPD, pSD, &pSourceNode); //adds the source node for the stream
        if (FAILED(hr))
            return hr;

        hr = AddOutputNode(pTopology, pSinkActivate, 0, &pOutputNode); //adds the output node for the renderer
        if (FAILED(hr))
            return hr;

        hr = pSourceNode->ConnectOutput(0, pOutputNode, 0); //connects the source node to the output mode (downstream, from source to output)
    } //no select, no care
    return hr;
}

/**
 * pSourceSD - The Stream Descriptor to use
 * ppActivate - The IMFActivate obejct to write to when the function succeeds.
 **/
HRESULT Alerter::CreateMediaSinkActivate(IMFStreamDescriptor* pSourceSD, IMFActivate **ppActivate) {
    IMFMediaTypeHandler pHandler(0);
    IMFActivate pActivate(0);

    HRESULT hr = pSourceSD->GetMediaTypeHandler(&pHandler); //gets the media type handler for the source stream.
    if (FAILED(hr))
        return hr;

    GUID guidMajorType; //gets the major media type (major media type: identifies what kind of data is in the stream, audio or video)
    hr = pHandler->GetMajorType(&guidMajorType);
    if (FAILED(hr))
        return hr;

    if (MFMediaType_Audio == guidMajorType) //if the major type is audio
        hr = MFCreateAudioRendererActivate(&pActivate); //creates the audio activate
    else { //includes video too by the way
        hr = E_FAIL; //fails the thing. I can't deleselect a stream unless I have the presentation descriptor.
    }

    *ppActivate = pActivate;
    (*ppActivate)->AddRef(); //adds a ref, it's gonna get -1 later on in the deconstructor
    return hr;
}

HRESULT Alerter::AddSourceNode(IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, IMFStreamDescriptor *pSD, IMFTopologyNode **ppNode) {
    IMFTopologyNodePtr pNode(0);

    //Creates an empty node
    HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
    if (FAILED(hr))
        return hr;

    //Set attributes
    hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
    if (FAILED(hr))
        return hr;

    hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
    if (FAILED(hr))
        return hr;

    hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
    if (FAILED(hr))
        return hr;

    //Add the node to the topology
    hr = pTopology->AddNode(pNode);
    if (FAILED(hr))
        return hr;
        
    *ppNode = pNode;
    (*ppNode)->AddRef(); //adds reference, one gonna be removed on deconstruction.
    return hr;
}

HRESULT Alerter::AddOutputNode(IMFTopology *pTopology, IMFActivate *pActivate, DWORD dwId, IMFTopologyNode **ppNode) {
    IMFTopologyNodePtr pNode(0);

    //Creates an empty node
    HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
    if (FAILED(hr))
        return hr;

    //Set the object pointer
    hr = pNode->SetObject(pActivate);
    if (FAILED(hr))
        return hr;

    //Set the stream sink ID attribute
    hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId);
    if (FAILED(hr))
        return hr;

    hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
    if (FAILED(hr))
        return hr;

    //Add node to topology
    hr = pTopology->AddNode(pNode);
    if (FAILED(hr))
        return hr;

    *ppNode = pNode;
    (*ppNode)->AddRef();
    return hr;
}

HRESULT Alerter::CreateSession() {
    HRESULT hr = CloseSession(); //closes the old session
    if (FAILED(hr))
        return hr;

    hr = MFCreateMediaSession(NULL, &m_pSession); //creates the media session
    if (FAILED(hr))
        return hr;

    hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL); //begin to recieve events from the media session
    m_mediaState = READY;
    return hr;
}

HRESULT Alerter::StopSession() {
    HRESULT hr = S_OK; //by default it works

    if (m_pSession) {
        DWORD dwWaitResult = 0;
        
        m_mediaState = CLOSING;

        hr = m_pSession->Close(); //command a close
        if (SUCCEEDED(hr)) {
            dwWaitResult = WaitForSingleObject(alerterCloseEvent, 5000);  //waits for the closing event for 5000ms (5 secs)
            if (dwWaitResult == WAIT_TIMEOUT) {
                throw std::runtime_error("Media Session didn't close in time!");
                return -1; //it failed, and I'm too lazy to find proper error codes to use
            }
        }

        if (SUCCEDED(hr)) {
            if (m_pSource)
                m_pSource->Shutdown(); //shuts down the media source

            if (m_pSession)
                m_pSession->Shutdown(); //shuts downs the media session
        }

        m_pSource.Release();
        m_pSession.Release();
        m_mediaState = Closed;
        return hr;
    }
}

HRESULT HandleEvent(UINT_PTR pEventPtr) {
    HRESULT hrStatus = S_OK; //the error code so far
    MediaEventType meType = MEUnknown; //the media event type

    IMFMediaEventPtr pEvent(reinterpret_cast<IMFMediaEvent*>(pEventPtr)); //gets the event by casting it back to IMFMediaEvent*

    if (pEvent == NULL)
        return E_POINTER; //pointer error

    HRESULT hr = pEvent->GetType(&meType); //obtains the media type
    if (FAILED(hr))
        return hr;

    hr = pEvent->GetStatus(&hrStatus); //obtains the event status. This is determined by the operation that triggered the event.
    if (SUCCEEDED(hr) && FAILED(hrStatus))
        hr = hrStatus;

    if (FAILED(hr))
        return hr;

    switch (meType) {
        case MESessionTopoloyStatus:
            //hr = OnTopologyStatus(pEvent);
            break;

        case MEEndOfPresentation:
            //hr = OnPresentationEnded(pEvent);
            break;

        case MENewPresentation:
            //hr = OnNewPresentation(pEvent);
            break;

        default:
            //hr = OnSessionEvent(pEvent, meType);
            break;
    }

    return hr;
}

HRESULT Alerter::OnTopologyStatus(IMFMediaEvent *pEvent) {
    UINT32 status;

    HRESULT hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status); //obtains the value associated with the attribute
    if (SUCCEEDED(hr) && (status == MF_TOPOLOGYSTATUS_READY)) {
        //The MSDN documentation called some functions here relating to video rendering, but since:
        //1) I don't have a video to show
        //2) _uuid() doesn't work, because GNU GCC business (alternative: actually get the UUID myself, it's probably IID_IMFVideoDisplayControl)
        //so I won't be including them
        //hr = StartPlayback();
    }
    
    return hr;
}

HRESULT Alerter::OnPresentationEnded(IMFMediaEvent *pEvent) {
    m_state = Stopped; //when file finish, it's no longer playing
    return S_OK;
}

//New presentation = new topology, need to make that
//According to MSDN, this function should not be called at all, but just in case, we'll put this here.
HRESULT Alerter::OnNewPresentation(IMFMediaEvent *pEvent) {
    IMFPresentationDescripterPtr pPD(0);
    IMFTopologyPtr pTopology(0);

    HRESULT hr = GetEventObject(pEvent, &pPD); //obtains the event object (a presentation descriptor), which is called by SetObject()
    if (FAILED(hr))
        return hr;

    hr = CreatePlaybackTopology(m_pSource, pPD, &pTopology); //create a partial topology
    if (FAILED(hr))
        return hr;

    hr = m_pSession->SetTopology(0, pTopology);
    if (FAILED(hr))
        return hr;

    m_state = OPEN_PENDING; //pending the openings
    return hr;
}

HRESULT Alerter::StartPlayback() {
    if (m_pSession == NULL) {
        throw std::runtime_error("Media Session is null! Cannot start playback.");
        return ERROR_INVALID_DATA;
    }

    PROPVARIANT varStart; //prop variant is essentially: "I can be anything here"
    PropVariantInit(&varStart); //sets varStart to VT_EMPTY (a.k.a doesn't contain anything)

    HRESULT hr = m_pSession->Start(&GUID_NULL, &varStart); //starts the playback. Since varStart is initliazed to VT_EMPTY, this means that it does not have a fixed start
    if (SUCCEEDED(hr)) {
        m_mediaState = PLAYING; //can treat as started. If error occurs while starting, m_pSession will throw a event with an error code
    }

    PropVariantClear(&varStart); //clears the prop
    return hr;
}

