#ifndef ALERTER_H
#define ALERTER_H

#include <string> //string and wstring
#include <iterator> //std::begin, std::end
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
    return hr;
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
    Alerter(const Alerter&)=delete;
    Alerter(Alerter&&);
    ~Alerter();

    /* Error Handling */
    AlertReturnCode getLastError(); //allows the user to obtain the last error

    /* Mode setting */
    void setMode(AlerterMode am); //sets the mode of the alerter

    /* Sound file */
    template <typename S>
    bool setSoundFile(S path);
    template <typename S>
    bool setSoundFile_Async(); //TODO: Actually do this later

    /* Unified Function to play audio */
    void playAudio(); //plays the audio
    void stopAudio(); //stops the audio

    /* Callback for BeginGetMethod */
    HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult *pResult);

    /* Event Handling*/
    HRESULT HandleEvent(UINT_PTR pEventPtr); //handles events when the app recieves WM_APP

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
    HRESULT OnTopologyStatus(IMFMediaEvent *pEvent); //handles for events that signals chance in topology statuses
    HRESULT OnPresentationEnded(IMFMediaEvent *pEvent); //handles end of presentation (eof of music file)
    HRESULT OnNewPresentation(IMFMediaEvent *pEvent); //handles new presentation (new music file)
    HRESULT OnSessionEvent(IMFMediaEvent *pEvent, MediaEventType meType) {return S_OK;}; //Handle any other session events

    /* Media Controls */
    HRESULT StartPlayback();

    /* The Shutdown function */
    HRESULT Shutdown();

    /* Virtual Functions that I *need* to implement. */
    ULONG STDMETHODCALLTYPE AddRef() {return 0;}
    ULONG STDMETHODCALLTYPE Release() {return 0;}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) {return E_NOINTERFACE;}
    HRESULT STDMETHODCALLTYPE GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) {return S_OK;}

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

#endif