/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id$
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Mferror.h>
#include <fstream>
using namespace std;
#include "colours.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "../sample.xpm"
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    MyFrame *frame = new MyFrame("Minimal wxWidgets App");

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)
{
    // set the frame icon
    SetIcon(wxICON(sample));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");

    fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit this program");

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText("Welcome to wxWidgets!");
#endif // wxUSE_STATUSBAR
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

HRESULT EnumerateTypesForStream(IMFSourceReader *pReader, DWORD dwStreamIndex, wxString &out)
{
    HRESULT hr = S_OK;
    DWORD dwMediaTypeIndex = 0;

    while (SUCCEEDED(hr))
    {
        IMFMediaType *pType = NULL;
        hr = pReader->GetNativeMediaType(dwStreamIndex, dwMediaTypeIndex, &pType);
        if (hr == MF_E_INVALIDSTREAMNUMBER)
        {
            hr = S_OK;
            break;
        }
        else if (SUCCEEDED(hr))
        {
            // Examine the media type. (Not shown.)
			GUID majorType, subtype;
			IMFMediaType *pNativeType = NULL;
			hr = pReader->GetNativeMediaType(dwStreamIndex, 0, &pNativeType);
			hr = pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
			//hr = pNativeType->GetGUID(MF_MT_SUBTYPE, &subtype);
			out = wxString::Format("%i, Vid %i, Aud %i\n", dwMediaTypeIndex, 
				majorType==MFMediaType_Video, majorType==MFMediaType_Audio);

            pType->Release();
        }
        ++dwMediaTypeIndex;
    }
    //return hr;
	return dwMediaTypeIndex;
}


HRESULT ConfigureDecoder(IMFSourceReader *pReader, DWORD dwStreamIndex, ofstream &log)
{
    IMFMediaType *pNativeType = NULL;
    IMFMediaType *pType = NULL;

    // Find the native format of the stream.
    HRESULT hr = pReader->GetNativeMediaType(dwStreamIndex, 0, &pNativeType);
    if (FAILED(hr))
    {
        return hr;
    }

    GUID majorType, subtype;

    // Find the major type.
    hr = pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
    if (FAILED(hr))
    {
		log << "Error 6\n";
        goto done;
    }

    // Define the output type.
    hr = MFCreateMediaType(&pType);
    if (FAILED(hr))
    {
		log << "Error 7\n";
        goto done;
    }

    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, majorType);
    if (FAILED(hr))
    {
		log << "Error 8\n";
        goto done;
    }

    // Select a subtype.
    if (majorType == MFMediaType_Video)
    {
        subtype= MFVideoFormat_YV12;
		//subtype= MFVideoFormat_UYVY;
    }
    else if (majorType == MFMediaType_Audio)
    {
        subtype = MFAudioFormat_PCM;
    }
    else
    {
        // Unrecognized type. Skip.
		log << "Error 9\n";
        goto done;
    }

    hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);
    if (FAILED(hr))
    {
		log << "Error 10\n";
        goto done;
    }

    // Set the uncompressed format.
    hr = pReader->SetCurrentMediaType(dwStreamIndex, NULL, pType);
	if (hr == MF_E_TOPO_CODEC_NOT_FOUND){log << "Error MF_E_TOPO_CODEC_NOT_FOUND\n";goto done;}
	if (hr == MF_E_INVALIDMEDIATYPE){log << "Error MF_E_INVALIDMEDIATYPE\n";goto done;}
	if (hr == MF_E_INVALIDREQUEST){log << "Error MF_E_INVALIDREQUEST\n";goto done;}

    if (FAILED(hr))
    {
		log << "Error 11\n";
        goto done;
    }

done:
    if(pNativeType) pNativeType->Release();
    if(pType) pType->Release();
    return hr;
}

HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride, ofstream &log)
{
    LONG lStride = 0;

    // Try to get the default stride from the media type.
    HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);

    if (FAILED(hr))
    {
        // Attribute not set. Try to calculate the default stride.

        GUID subtype = GUID_NULL;

        UINT32 width = 0;
        UINT32 height = 0;
        // Get the subtype and the image size.
        hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (FAILED(hr))
        {
            goto done;
        }
        hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
        if (FAILED(hr))
        {
            goto done;
        }
        hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &lStride);
        if (FAILED(hr))
        {
            goto done;
        }

        // Set the attribute for later reference.
        (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
    }

    if (SUCCEEDED(hr))
    {
        *plStride = lStride;
    }

done:
    return hr;
}

void GetVideoSubType(GUID subType, wxString &out)
{
	out = "Unknown";
	if(subType==MFVideoFormat_RGB8) out = "MFVideoFormat_RGB8";
	if(subType==MFVideoFormat_RGB555) out = "MFVideoFormat_RGB555";
	if(subType==MFVideoFormat_RGB565) out = "MFVideoFormat_RGB565";
	if(subType==MFVideoFormat_RGB24) out = "MFVideoFormat_RGB24";
	if(subType==MFVideoFormat_RGB32) out = "MFVideoFormat_RGB32";
	if(subType==MFVideoFormat_ARGB32) out = "MFVideoFormat_ARGB32";
	if(subType==MFVideoFormat_YV12) out = "MFVideoFormat_YV12";
	if(subType==MFVideoFormat_AYUV) out = "MFVideoFormat_AYUV";
	if(subType==MFVideoFormat_UYVY) out = "MFVideoFormat_UYVY";

	if(subType==MFAudioFormat_PCM) out = "MFAudioFormat_PCM";
}

HRESULT DecodeViewFrame(BYTE *ppbBuffer, DWORD pcbCurrentLength, UINT32 width, 
	UINT32 height, LONG plStride, wxString &subTypeStr, ofstream &log)
{
	log << "Decode "<< width << "," << height << "," << subTypeStr <<"\n";

	return S_OK;
}

HRESULT ProcessSamples(IMFSourceReader *pReader, ofstream &log)
{
    HRESULT hr = S_OK;
    IMFSample *pSample = NULL;
    size_t  cSamples = 0;
    UINT32 width = 0;
    UINT32 height = 0;	

    bool quit = false;
    while (!quit)
    {
        DWORD streamIndex, flags;
        LONGLONG llTimeStamp;

        hr = pReader->ReadSample(
            MF_SOURCE_READER_ANY_STREAM,    // Stream index.
            0,                              // Flags.
            &streamIndex,                   // Receives the actual stream index. 
            &flags,                         // Receives status flags.
            &llTimeStamp,                   // Receives the time stamp.
            &pSample                        // Receives the sample or NULL.
            );

        if (FAILED(hr))
        {
            break;
        }

        log << wxString::Format(L"Stream %d (%I64d)\n", streamIndex, llTimeStamp);
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            log << wxString::Format(L"\tEnd of stream\n");
            quit = true;
        }
        if (flags & MF_SOURCE_READERF_NEWSTREAM)
        {
           log << wxString::Format(L"\tNew stream\n");
        }
        if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
        {
           log << wxString::Format(L"\tNative type changed\n");
        }
        if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
        {
           log << wxString::Format(L"\tCurrent type changed\n");
        }
        if (flags & MF_SOURCE_READERF_STREAMTICK)
        {
            log << wxString::Format(L"\tStream tick\n");
        }

        if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
        {
            // The format changed. Reconfigure the decoder.
            hr = ConfigureDecoder(pReader, streamIndex, log);
            if (FAILED(hr))
            {
                break;
            }
        }

        if (pSample)
        {
			IMFMediaType *pCurrentType = NULL;
			LONG plStride = 0;
			GUID majorType=GUID_NULL, subType=GUID_NULL;

			HRESULT hr = pReader->GetCurrentMediaType(streamIndex, &pCurrentType);
			if(!SUCCEEDED(hr)) log << "Error 3\n";
			BOOL isComp = FALSE;
			hr = pCurrentType->IsCompressedFormat(&isComp);
			log << "iscompressed" << isComp << "\n";
			hr = pCurrentType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
			if(!SUCCEEDED(hr)) log << "Error 4\n";
			hr = pCurrentType->GetGUID(MF_MT_SUBTYPE, &subType);
			if(!SUCCEEDED(hr)) log << "Error 5\n";
			int isVideo = (majorType==MFMediaType_Video);
			if(isVideo)
			{	
				GetDefaultStride(pCurrentType, &plStride, log);
				log << "subtype" <<(subType==MFVideoFormat_RGB32)<<","<<(subType==MFAudioFormat_PCM)<<"\n";
				hr = MFGetAttributeSize(pCurrentType, MF_MT_FRAME_SIZE, &width, &height);
				if(!SUCCEEDED(hr)) log << "Error 20\n";
			}

			TCHAR szString[41]=L"";
			wxString subTypeStr;
			GetVideoSubType(subType, subTypeStr);
			::StringFromGUID2(subType, szString, 41);
			log << "subtype\t" << szString << "\t"<<subTypeStr << "\n";

			IMFMediaBuffer *ppBuffer = NULL;
			hr = pSample->ConvertToContiguousBuffer(&ppBuffer);
			log << "ConvertToContiguousBuffer=" << SUCCEEDED(hr) << "\tstride="<< plStride << "\n";

			IMF2DBuffer *m_p2DBuffer = NULL;
			ppBuffer->QueryInterface(IID_IMF2DBuffer, (void**)&m_p2DBuffer);
			log << "IMF2DBuffer=" << (m_p2DBuffer != NULL) << "\n";

			if(SUCCEEDED(hr))
			{
				BYTE *ppbBuffer;
				DWORD pcbMaxLength;
				DWORD pcbCurrentLength;
				hr = ppBuffer->Lock(&ppbBuffer, &pcbMaxLength, &pcbCurrentLength);
				log << "pcbMaxLength="<< pcbMaxLength << "\tpcbCurrentLength=" <<pcbCurrentLength << "\n";

				if(isVideo)
					DecodeViewFrame(ppbBuffer, pcbCurrentLength, width, height, plStride, subTypeStr, log);

				ppBuffer->Unlock();
			}

			if(ppBuffer) ppBuffer->Release();

            ++cSamples;
        }

        if(pSample) pSample->Release();
    }

    if (FAILED(hr))
    {
        log << wxString::Format(L"ProcessSamples FAILED, hr = 0x%x\n", hr);
    }
    else
    {
        log << wxString::Format(L"Processed %d samples\n", cSamples);
    }
	if(pSample) pSample->Release();
    return hr;
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	//http://msdn.microsoft.com/en-us/library/windows/desktop/bb530123%28v=vs.85%29.aspx
	//http://msdn.microsoft.com/en-gb/library/windows/desktop/dd940436%28v=vs.85%29.aspx
	//http://msdn.microsoft.com/en-gb/library/windows/desktop/dd389281%28v=vs.85%29.aspx#creating_source_reader
	//http://m.cplusplus.com/forum/windows/77275/
	ofstream log("log.txt");
	log << "Init\n";

    // Initialize the COM runtime.
    HRESULT hr;// = CoInitializeEx(0, COINIT_MULTITHREADED);
	int test = -1;
	wxString test2, debugStr;
	//if(hr==S_OK) test = -10;
	//if(hr==S_FALSE) test = -11;
	//if(hr==RPC_E_CHANGED_MODE) test = -12;
    if (1)//SUCCEEDED(hr)
    {
		test = -2;
        // Initialize the Media Foundation platform.
        hr = MFStartup(MF_VERSION);
        if (SUCCEEDED(hr))
        {
            // Create the source reader.
            IMFSourceReader *pReader;
            hr = MFCreateSourceReaderFromURL(L"c:\\Users\\tim\\Desktop\\Faceware\\MVI_4229.MOV", NULL, &pReader);
			//hr = MFCreateSourceReaderFromURL(L"c:\\Users\\tim\\Desktop\\Faceware\\test.avi", NULL, &pReader);
			test = SUCCEEDED(hr);
            if (SUCCEEDED(hr))
            {
				test = EnumerateTypesForStream(pReader, 0, test2);
				debugStr = debugStr + test2;
				test = EnumerateTypesForStream(pReader, 1, test2);
				debugStr = debugStr + test2;
				//test = EnumerateTypesForStream(pReader, 2, test2);
				//debugStr = debugStr + test2;
				hr = ConfigureDecoder(pReader,0,log);
				if(!SUCCEEDED(hr)) log << "Error 1\n";
				hr = ConfigureDecoder(pReader,1,log);
				if(!SUCCEEDED(hr)) log << "Error 2\n";
				
				ProcessSamples(pReader, log);
                //ReadMediaFile(pReader);
                pReader->Release();
            }
            // Shut down Media Foundation.
            MFShutdown();
        }
        //CoUninitialize();
    }
	log << std::flush;

    wxMessageBox(wxString::Format
                 (
                    "Welcome to %s!\n"
                    "\n"
                    "This is the minimal wxWidgets sample\n"
                    "running under %s. test=%i %s",
                    wxVERSION_STRING,
                    wxGetOsDescription(),
					test, debugStr
                 ),
                 "About wxWidgets minimal sample",
                 wxOK | wxICON_INFORMATION,
                 this);
}
