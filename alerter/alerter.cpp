#include "alerter.hpp"

#include <sstream>

Alerter::Alerter() : m_pSource(0), m_pSession(0) {
    HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET); //starts up MF without sockets.
    if (SUCCEEDED(hr)) {
        alerterCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL); //not inheritble, auto-reset, no signal, nameless
        if (alerterCloseEvent == NULL) {
            std::stringstream ss;
            ss << "Could not create event for use in alerter. HRESULT Error: " << HRESULT_FROM_WIN32(GetLastError());
            throw std::runtime_error(std::string(ss.str().c_str()));
        }
    }
}

Alerter::~Alerter() {

}

AlertReturnCode Alerter::getLastError() {
    return lastError;
}

void Alerter::setMode(AlerterMode am) {
    a_mode = am;
}

void Alerter::playAudio() {
    switch (a_mode) {
        case BACKGROUND: {
            //HRESULT result = PlayAudioStream();
            break;
        }
        case DUCKING: {
            lastError = UNSUPPORTED_OPERATION;
            break;
        }
        case EXCLUSIVE: {
            //HRESULT result = PlayExclusiveStream();
            break;
        }
        default:
            throw std::logic_error("Unexpected case reached. Contact the application owner.");
    }
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
            hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL); //get the next event
            if (FAILED(hr))
                return hr;
            break;
    }
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
    (**ppTopology)->AddRef();
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
    IMFTopologyModePtr pOutputNode(0); //the sink node

    bool fSelected = false;

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
    return hr;
}