#ifndef ALERTER_H
#define ALERTER_H

#include <string> //string and wstring
//#include <filesystem> //exists //enable again when GCC 8.0 is out

#include "windows.h"
#include "Mfidl.h"
#include "mfapi.h"
#include "comdef.h"
#include "comip.h"
#include "Unknwn.h"

#include "mfobjects.h"

// Some typedefs for smart pointers
_COM_SMARTPTR_TYPEDEF(IMFSourceResolver, IID_IMFSourceResolver);
_COM_SMARTPTR_TYPEDEF(IUnknown, IID_IUnknown);
_COM_SMARTPTR_TYPEDEF(IMFTopology, IID_IMFTopology);
_COM_SMARTPTR_TYPEDEF(IMFPresentationDescriptor, IID_IMFPresentationDescriptor);
_COM_SMARTPTR_TYPEDEF(IMFMediaSource, IID_IMFMediaSource);
_COM_SMARTPTR_TYPEDEF(IMFStreamDescriptor, IID_IMFStreamDescriptor);
_COM_SMARTPTR_TYPEDEF(IMFActivate, IID_IMFActivate);
_COM_SMARTPTR_TYPEDEF(IMFTopologyNode, IID_IMFTopologyNode);
_COM_SMARTPTR_TYPEDEF(IMFMediaTypeHandler, IID_IMFMediaTypeHandler);
_COM_SMARTPTR_TYPEDEF(IMFMediaSession, IID_IMFMediaSession);
_COM_SMARTPTR_TYPEDEF(IMFMediaEvent, IID_IMFMediaEvent);

enum AlerterMode : short {
    BACKGROUND,
    DUCKING,
    EXCLUSIVE
};

enum AlertReturnCode : short {
    SUCCESS,
    FILE_NOT_FOUND,
    UNSUPPORTED_OPERATION
};

/**
 * The alerter (sound) class. This class handles the audio. Inherits from IMFAsyncCallback, required for BeginGetEvent()
 * 
 * This class will eventually support 3 different modes: background, ducking, and exclusive.
 * In background mode, the sound will play like a normal application, and will not attempt to mute all other sound.
 * In ducking mode, the sound will use a communications stream to play the sound, and the behaviour is defined by the user in the Windows Sound Settings.
 * In exclusive mode, the sound will take exclusive control of the default audio device, muting all other volumes to alert the user.
 * The fallback mode will always be background mode.
**/
class Alerter : public IMFAsyncCallback {
    public:
    Alerter();
    virtual ~Alerter();

    /* Error Handling */
    AlertReturnCode getLastError(); //allows the user to obtain the last error

    /* Mode setting */
    void setMode(AlerterMode am); //sets the mode of the alerter

    /* Sound file */
    template <typename S>
    bool setSoundFile(S path) { //underlying code uses a source resolver, so this will take a while.
        return setSoundFile(std::wstring(path.begin(), path.end()));
    }
    template <typename S>
    bool setSoundFile_Async(); //TODO: Actually do this later

    /* Unified Function to play audio */
    void playAudio();

    /* Callback for BeginGetMethod */
    HRESULT Invoke(IMFAsyncResult *pResult);

    protected:
    // HRESULT PlayAudioStream(); //background
    //TODO: Ducking Mode
    // HRESULT PlayExclusiveStream(); //exclusive

    // So according to the MSDN docs, I need to create my own partial network function.
    // IGN 10/10
    HRESULT CreatePlaybackTopology(IMFMediaSource* pSource, IMFPresentationDescriptor* pPD, IMFTopology **ppTopology);
    HRESULT CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource); //creates the media source based on the input url
    HRESULT AddBranchToPartialTopology(IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, DWORD iStream); //each source starts a new branch in the topology. In normal Audio files, there should only be 1.
    HRESULT CreateMediaSinkActivate(IMFStreamDescriptor* pSourceSD, IMFActivate **ppActivate); //creates the activation object for a renderer, based on the stream media type (in this case, audio.)
    HRESULT AddSourceNode(IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, IMFStreamDescriptor *pSD, IMFTopologyNode **ppNode); //adds a node into the topology, partial or not
    HRESULT AddOutputNode(IMFTopology *pTopology, IMFActivate *pActivate, IMFTopologyNode **ppNode); //adds the sink into the topology
    HRESULT CreateSession();

    private:
    AlerterMode a_mode = BACKGROUND; //the mode of the alerter
    AlertReturnCode lastError = SUCCESS; //the last error that the object encountered
    HANDLE alerterCloseEvent = NULL; //the handle that determines if things should stop

    enum MediaState {
        PRIMED,
        PLAYING,
        STOP,
        CLOSED
    } m_mediaState = CLOSED;

    IMFMediaSourcePtr m_pSource;
    IMFMediaSessionPtr m_pSession;
};

template <>
bool Alerter::setSoundFile<std::wstring>(std::wstring path) {
    /* Check if the file exists */ //uncomment in GCC 8.0
    /*if (!std::filesystem::exists(path)) {
        lastError = FILE_NOT_FOUND; //sets the last error
        return false; //function failed
    }*/
    if (FILE *file = fopen(std::string(path.begin(), path.end()).c_str(), "r")) { //checks for the existance of the file
        fclose(file);
    } else {
        lastError = FILE_NOT_FOUND; //sets the last error
        return false;
    }

    /* Variable Declaration */
    IMFTopologyPtr pTopology(0); //the topoogy (it's a thing that describe the path of the media source to the media sink through nodes)
    IMFPresentationDescriptorPtr pSourcePD(0); //the presentation descripter

    /* Create a Media Session */
    HRESULT hr = 0;
    //HRESULT hr = CreateSession();
    if (FAILED(hr)) //failed just checks if hr < 0
        return hr;

    /* Source Resolving */
    hr = CreateMediaSource(path.c_str(), &m_pSource); //creates the source resolver

    if (FAILED(hr))
        return hr;
    
    if (FAILED(hr))
        return hr;

    /* Creating the media source from the object */
    hr = CreateMediaSource(path.c_str(), &m_pSource);
    if (FAILED(hr))
        return hr;

    /* Create the presentation descriptor */
    hr = m_pSource->CreatePresentationDescriptor(&pSourcePD);
    if (FAILED(hr))
        return hr;

    /* Create a partial topology (I don't have the complete nodes) */
    hr = CreatePlayBackTopology(m_pSource, pSourcePD, &pTopology);
    if (FAILED(hr))
        return hr;

    /* Set the topology on the media session */
    hr = m_pSession->SetTopology(0, pTopoogy); //sets the topology of the session
    if (FAILED(hr))
        return hr;

    m_mediaState = PRIMED;
    return hr;
}

#endif