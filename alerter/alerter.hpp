#ifndef ALERTER_H
#define ALERTER_H

#include <string> //string and wstring
//#include <filesystem> //exists //enable again when GCC 8.0 is out

#include "windows.h"
#include "windowsx.h"
#include "tchar.h"
#include "Mfidl.h"
#include "mfapi.h"
#include "mferror.h"
#include "comdef.h"
#include "comip.h"
#include "Unknwn.h"

//_com_ptr_t release the pointer when operator& is called. Need to reattach it after we use it.
// Use this definition if working with pointer to pointer COM interfaces, and the function to be called has multiple arguments
#define releaseCallStoreMulti(hr, func, ptrType, smart_ptr, ...) \
        { ptrType** _ptr = &smart_ptr; \
        hr = func(__VA_ARGS__, _ptr); \
        smart_ptr.Attach(*_ptr, false); \
        _ptr = nullptr; }

// Use this definition if workign with poitner to pointer COM interfaces, and the function only accepts the pointer as an argument
#define releaseCallStoreSingle(hr, func, ptrType, smart_ptr) \
        { ptrType** _ptr = &smart_ptr; \
        hr = func(_ptr); \
        smart_ptr.Attach(*_ptr, false); \
        _ptr = nullptr; }

const GUID GUID_NULL = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}}; //my application can't find GUID_NULL, so I'll have to do it like this

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

//This is the event we are going to use. WM_APP is probably (I'm not sure) the last declaration for WM, so we take the next one
const UINT WM_APP_PLAYER_EVENT = WM_APP + 1;

template <class T>
HRESULT GetEventObject(IMFMediaEvent* pEvent, T **ppObject) {
    *ppObject = NULL; //set the output to nothing

    PROPVARIANT var;
    HRESULT hr = pEvent->GetValue(&var);
    if (SUCCEEDED(hr)) {
        if (var.vt == VT_UNKNOWN)
            hr = var.punkVal->QueryInterface(IID_IMFPresentationDescriptor, reinterpret_cast<void**>(ppObject));
        else hr = MF_E_INVALIDTYPE;
        PropVariantClear(&var);
        return hr;
    }
}

enum AlerterMode : short {
    BACKGROUND,
    DUCKING,
    EXCLUSIVE
};

enum AlertReturnCode : short {
    SUCCESS,
    FILE_NOT_FOUND,
    UNSUPPORTED_OPERATION,
    FAILED_TO_PLAY,
    FAILED_TO_STOP,
    CANNOT_CHANGE_MODE_WHILE_PLAYING
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
class Alerter final : public IMFAsyncCallback {
    public:
    Alerter(HWND eventWindow);
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
    void playAudio(); //plays the audio
    void stopAudio(); //stops the audio

    /* Callback for BeginGetMethod */
    HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult *pResult);

    protected:
    /* Media Controls (Individual) */
    HRESULT PlayAudioStream(); //background
    //TODO: Ducking Mode
    // HRESULT PlayExclusiveStream(); //exclusive

    // So according to the MSDN docs, I need to create my own partial network function.
    // IGN 10/10
    HRESULT CreatePlaybackTopology(IMFMediaSource* pSource, IMFPresentationDescriptor* pPD, IMFTopology **ppTopology);
    HRESULT CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource); //creates the media source based on the input url
    HRESULT AddBranchToPartialTopology(IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, DWORD iStream); //each source starts a new branch in the topology. In normal Audio files, there should only be 1.
    HRESULT CreateMediaSinkActivate(IMFStreamDescriptor* pSourceSD, IMFActivate **ppActivate); //creates the activation object for a renderer, based on the stream media type (in this case, audio.)
    HRESULT AddSourceNode(IMFTopology *pTopology, IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, IMFStreamDescriptor *pSD, IMFTopologyNode **ppNode); //adds a node into the topology, partial or not
    HRESULT AddOutputNode(IMFTopology *pTopology, IMFActivate *pActivate, DWORD dwId, IMFTopologyNode **ppNode); //adds the sink into the topology
    HRESULT CreateSession(); //creates a session
    HRESULT StopSession(); //stops the session

    /* Event Handling */
    HRESULT HandleEvent(UINT_PTR pEventPtr); //handles events when the app recieves WM_APP
    HRESULT OnTopologyStatus(IMFMediaEvent *pEvent); //handles for events that signals chance in topology statuses
    HRESULT OnPresentationEnded(IMFMediaEvent *pEvent); //handles end of presentation (eof of music file)
    HRESULT OnNewPresentation(IMFMediaEvent *pEvent); //handles new presentation (new music file)
    HRESULT OnSessionEvent(IMFMediaEvent *pEvent, MediaEventType meType) {return S_OK;}; //Handle any other session events

    /* Media Controls */
    HRESULT StartPlayback();

    /* The Shutdown function */
    HRESULT Shutdown();

    private:
    AlerterMode a_mode = BACKGROUND; //the mode of the alerter
    AlertReturnCode lastError = SUCCESS; //the last error that the object encountered
    HANDLE alerterCloseEvent = NULL; //the handle that determines if things should stop
    HWND m_eventWindow; //the window to post events too

    enum MediaState {
        READY,
        OPEN_PENDING,
        PLAYING,
        STOP,
        CLOSING,
        CLOSED
    } m_mediaState = CLOSED;

    IMFMediaSourcePtr m_pSource; //source media
    IMFMediaSessionPtr m_pSession; //media session

    static unsigned int ref_count; //this class is designed to not be a singleton, unlike the CPlayer class in MSDN. The ref_count is used by Shutdown() to determine if the framework should start.
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
    HRESULT hr = CreateSession();
    if (FAILED(hr)) //failed just checks if hr < 0
        return hr;

    /* Creating the media source from the object */
    releaseCallStoreMulti(hr, CreateMediaSource, IMFMediaSource, m_pSource, path.c_str());
    if (FAILED(hr))
        return hr;

    /* Create the presentation descriptor */
    releaseCallStoreSingle(hr, m_pSource->CreatePresentationDescriptor, IMFPresentationDescriptor, pSourcePD);
    if (FAILED(hr))
        return hr;

    /* Create a partial topology (I don't have the complete nodes) */
    releaseCallStoreMulti(hr, CreatePlaybackTopology, IMFTopology, pTopology, m_pSource, pSourcePD);
    if (FAILED(hr))
        return hr;

    /* Set the topology on the media session */
    hr = m_pSession->SetTopology(0, pTopology); //sets the topology of the session
    if (FAILED(hr))
        return hr;

    m_mediaState = OPEN_PENDING;
    return hr;
}
#endif