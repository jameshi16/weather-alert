#include "alerter.hpp"

#include <sstream>

Alerter::Alerter() : m_pSource(0) {
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

HRESULT Alerter::CreatePlaybackTopology(IMFMediaSource* pSource, IMFPresentationDescriptor* pPD, IMFTopology **ppTopology) {
    IMFTopologyPtr pTopology(0); //the topology we're generating
    DWORD cSourceStreams = 0;

    //Create topology
    HRESULT hr = MFCreateTopology(&pTopology); 
    
}

HRESULT Alerter::CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource) {
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID; //object does not exist... yet
    IMFSourceResolverPtr pSourceResolver(0); //interface that can resolve media sources from URL or byte stream
    IUnknownPtr pSource(0); //base interface to most COM interfaces

    /* Create the media source */
    HRESULT hr = MFCreateSourceResolver(&pSourceResolver); //creates the source resolver

    if (FAILED(hr))
        return false;

    /* Create the object */
    hr = pSourceResolver->CreateObjectFromURL(sURL,
                                            MF_RESOLUTION_MEDIASOURCE,
                                            NULL,
                                            &ObjectType,
                                            &pSource);

    if (FAILED(hr))
        return false;

    hr = pSource->QueryInterface(IID_IMFMediaSource, reinterpret_cast<void**>(ppSource)); //Get the IMFMediaSource interface from the media source.
    return hr;
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