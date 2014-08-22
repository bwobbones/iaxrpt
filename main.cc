//----------------------------------------------------------------------------------------
// Name:        main.cc
// Purpose:     Core application
// Author:      Michael Van Donselaar
// Modified by: Steven Henke,W9SH, XELATEC, LLC sph@xelatec.com
// Created:     2003
// Copyright:   (c) Michael Van Donselaar ( michael@vandonselaar.org )
// Licence:     GPL
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// GCC implementation
//----------------------------------------------------------------------------------------

#if defined(__GNUG__) && ! defined(__APPLE__)
    #pragma implementation "main.h"
#endif

//----------------------------------------------------------------------------------------
// Standard wxWindows headers
//----------------------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// For all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//----------------------------------------------------------------------------------------
// Header of this .cpp file
//----------------------------------------------------------------------------------------

#include "main.h"
#include "devices.h"
#include "prefs.h"
#include "accounts.h"
#include "wx/tokenzr.h"


//----------------------------------------------------------------------------------------
// Remaining headers
//----------------------------------------------------------------------------------------

#include "app.h"
#include "dial.h"
#include "frame.h"
#include "wx/dirdlg.h"

//----------------------------------------------------------------------------------------
// forward decls for callbacks
//----------------------------------------------------------------------------------------
extern "C" {
     static int iaxc_callback(iaxc_event e);
     int        doTestCall(int ac, char **av);
}

//----------------------------------------------------------------------------------------
// Static variables
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// wxWindows macro: Declare the application.
//----------------------------------------------------------------------------------------

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. theApp and
// not wxApp).

IMPLEMENT_APP( theApp )

//----------------------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------------------

bool theApp::OnInit()
{
    wxConfig *config = theApp::getConfig();

    wxString str;
    wxString reginfo;
	wxFileName filename;
    long     dummy;
    bool     bCont;
	int      tmpIaxPort;

#ifndef __WXMAC__
// XXX this seems broken on mac with wx-2.4.2
    m_single_instance_checker = new wxSingleInstanceChecker(_T("iaxcomm.lock"));

    // And if a copy is alreay running, then abort...
    if ( m_single_instance_checker->IsAnotherRunning() ) {
        wxMessageDialog second_instance_messagedialog( (wxWindow*)NULL,
                  _( "Another instance is already running." ),
                  _( "Startup Error" ),
                  wxOK | wxICON_INFORMATION );
        second_instance_messagedialog.ShowModal();
        // Returning FALSE from within wxApp::OnInit() will terminate the application.
        return FALSE;
    }
#endif

    config->SetPath(_T("/Prefs"));

    // Load up the XML Resource handler, to be able to load XML resources..

    wxXmlResource::Get()->InitAllHandlers();

    // Load up enough XML resources to construct a main frame

    load_xrc_resource(_T("frame.xrc"));
    load_xrc_resource(_T("menubar.xrc"));
    load_xrc_resource(_T("panel.xrc"));
    load_xrc_resource(_T("prefs.xrc"));
    load_xrc_resource(_T("directory.xrc"));
    load_xrc_resource(_T("devices.xrc"));

    extern void InitXmlResource();
    InitXmlResource();

    IncomingRing.Init( 880, 960, 16000, 48000, 10);
    RingbackTone.Init( 440, 480, 16000, 48000, 10);
    IntercomTone.Init( 440, 960,  6000,  6000,  1);

    // Read in some config info
    nCalls           = config->Read(_T("nCalls"), 2);
    DefaultAccount   = config->Read(_T("DefaultAccount"), _T(""));

    IncomingRingName = config->Read(_T("RingTone"), _T(""));
    RingBackToneName = config->Read(_T("RingBack"), _T(""));
    IntercomToneName = config->Read(_T("Intercom"), _T(""));
	HelpURL          = config->Read(_T("HelpURL"), _T(""));

    IncomingRing.LoadTone(IncomingRingName, 10);
    RingbackTone.LoadTone(RingBackToneName, 10);
    IntercomTone.LoadTone(IntercomToneName,  1);

	tmpIaxPort=0;
	config->SetPath(_T("/Prefs"));
	tmpIaxPort=config->Read(_T("Iax2Port"), 0l);

	if(tmpIaxPort<1025 || tmpIaxPort>65534 )tmpIaxPort=4569;
	/*
	{
		wxString lmess;	
	    lmess.Printf(_T("IaxPort Set:  %d"), tmpIaxPort);
		wxLogMessage( lmess );
    }
	*/
	config->SetPath(_T("/Access"));
	str=config->Read(_T("Password"), _T(""));
	if(str.IsEmpty())
	{
		AccessLevel      = 1;
	}
	else
	{
		AccessLevel      = 0; 	
	}
	config->SetPath(_T("/Prefs"));
	StartupLinks         = config->Read(_T("StartupLinks"), _T(""));
	PermaLinks           = config->Read(_T("PermaLinks"), _T(""));

    if(DefaultAccount.IsEmpty()) {
        bool     bCont;
        long     dummy;
        wxString Name;
        wxString Path;

        Path = config->GetPath();
        config->SetPath(_T("/Accounts"));
        bCont = config->GetFirstGroup(Name, dummy);
        while ( bCont ) {
            DefaultAccount = Name;
            bCont = config->GetNextGroup(Name, dummy);
        }

        config->SetPath(Path);
    }

	iaxc_set_preferred_source_udp_port(tmpIaxPort);

    // Call initialize now, as the frame constructor might read config values
    // and update settings (such as preferred codec).
    if(iaxc_initialize(nCalls)) {
        wxLogError(_("Couldn't Initialize IAX Client "));
    }

    // Create an instance of the main frame.
    // Using a pointer since behaviour will be when close the frame, it will
    // Destroy() itself, which will safely call "delete" when the time is right.
    // [The constructor is empty, since using the default value for parent, which is
    // NULL].

    theFrame = new MyFrame();

    if(DefaultAccount.IsEmpty()) {
        // If we never did find a default account, must be a new install

        AddAccountDialog dialog(theFrame, _T(""));

        dialog.SetTitle(_T("Welcome to iaxRpt!"));
      #ifdef PROVIDER
        dialog.AccountName->SetValue(_T("asterisk"));
        dialog.HostName->SetValue(_T("asterisk"));
      #endif

        dialog.ShowModal();
    }

    // This is of dubious usefulness unless an app is a dialog-only app. This
    // is the first created frame, so it is the top window already.

    SetTopWindow( theFrame );

    // Show the main frame. Unlike most controls, frames don't show immediately upon
    // construction, but instead wait for a specific Show() function.

    theFrame->Show( TRUE );
	
	/* skin it */
	config->SetPath(_T("/Prefs"));
	AppSkin = config->Read(_T("AppSkin"), _T(""));
	xippr=0;
	
	if (AppSkin == _T("xelatec") ) {
		xippr=1;
		theFrame->SetLabel(_T("Xelatec PTT Dispatcher"));
		if(HelpURL.IsEmpty())
		{
			HelpURL=_T("http://www.xelatec.com/xippr/iaxrpt_help");		
		}
	}
	else if( AppSkin == _T("idas") )
	{
		xippr=1;
		theFrame->SetLabel(_T("IDAS-IT-RPT"));
		if(HelpURL.IsEmpty())
		{
			HelpURL=_T("http://www.idas-italia.it");		
		}	
	}
	else if( AppSkin == _T("user") )
	{
		xippr=1;
		wxString AppSkinTitle;
        AppSkinTitle = config->Read(_T("AppSkinTitle"), _T(""));
		theFrame->SetLabel(_T(AppSkinTitle));
		if(HelpURL.IsEmpty())
		{
			HelpURL=_T("http://www.xelatec.com/xippr/iaxrpt_help");		
		}	
	}
	
  #ifdef __WXMSW__
    theTaskBarIcon = new MyTaskBarIcon();
    theTaskBarIcon->SetIcon(wxICON(application));
  #endif

	wxGetApp().theFrame->MicLevel = config->Read(_T("MicLevel"), 0l);
	wxGetApp().theFrame->MicBoost = config->Read(_T("MicBoost"), 0l);

    SetAudioDevices(InputDevice,
                    OutputDevice,
                    RingDevice,
                    wxGetApp().theFrame->InputSourceIndex);

	iaxc_input_level_set(wxGetApp().theFrame->MicLevel/100.0);		           //maw 0413
	//iaxc_input_level_set(25);		           //maw 0413

    iaxc_set_silence_threshold(-99);
    iaxc_set_event_callback(iaxc_callback);
    iaxc_start_processing_thread();

    // Callerid from wxConfig
    Name   = config->Read(_T("Name"),   _T("iaxRpt User"));
    Number = _T("0"); // config->Read(_T("Number"), _T("700000000"));
    SetCallerID(Name, Number);
						
	/*	Register all Accounts with their Hosts
	    not for iaxRpt please!
	*/
	#if 0
    config->SetPath(_T("/Accounts"));
    bCont = config->GetFirstGroup(str, dummy);
    while ( bCont ) {
        RegisterByName(str);
        bCont = config->GetNextGroup(str, dummy);
    }
	#endif

    wxString dest;
    if(argc > 1) {
        dest.Printf(_T("%s"), argv[1]);
        Dial(dest);
    }

    return TRUE;
}

void theApp::RegisterByName(wxString RegName)
{
    wxConfig    *config = theApp::getConfig();
    wxChar      KeyPath[256];
    wxListItem  item;
#if defined(__UNICODE__)
    wxMBConvUTF8 utf8;
#endif
		 
	if(wxGetApp().theFrame->DebugLevel)
	{
		wxString msg;
		msg.Printf(_T("RegisterByName %s"),RegName.c_str());
		wxGetApp().theFrame->LogIt(msg);
	}

    wxStringTokenizer tok(RegName, _T(":@"));
    char user[256], pass[256], host[256];

    if(wxStrlen(RegName) == 0)
        return;

    if(tok.CountTokens() == 3) {

#if defined(__UNICODE__)
        utf8.WC2MB(user, tok.GetNextToken().c_str(), 256);
        utf8.WC2MB(pass, tok.GetNextToken().c_str(), 256);
        utf8.WC2MB(host, tok.GetNextToken().c_str(), 256);
#else
        strncpy(user, tok.GetNextToken().c_str(), 256);
        strncpy(pass, tok.GetNextToken().c_str(), 256);
        strncpy(host, tok.GetNextToken().c_str(), 256);
#endif
    } else {
        // Check if it's a Speed Dial
        wxStrcpy(KeyPath,     _T("/Accounts/"));
        wxStrcat(KeyPath,     RegName);
        config->SetPath(KeyPath);
        if(!config->Exists(KeyPath)) {
            theFrame->SetStatusText(_T("Register format error"));
            return;
        }
#if defined(__UNICODE__)
        utf8.WC2MB(user, config->Read(_T("Username"), _T("")).c_str(), 256);
        utf8.WC2MB(pass, config->Read(_T("Password"), _T("")).c_str(), 256);
        utf8.WC2MB(host, config->Read(_T("Host"), _T("")).c_str(), 256);
#else
        wxStrcpy(user, config->Read(_T("Username"), _T("")));
        wxStrcpy(pass, config->Read(_T("Password"), _T("")));
        wxStrcpy(host, config->Read(_T("Host"), _T("")));
#endif
    }
    iaxc_register(user, pass, host);
}

int theApp::OnExit(void)
{
    // Delete the single instance checker
#ifndef __WXMAC__
    delete m_single_instance_checker;
#endif
    return 0;
}

//----------------------------------------------------------------------------------------
// Private methods
//----------------------------------------------------------------------------------------

void theApp::load_xrc_resource( const wxString& xrc_filename )
{
    wxConfig        *config = theApp::getConfig();
    wxString         xrc_fullname;
    static wxString  xrc_subdirectory = _T("");

#ifdef __WXMAC__
    if(xrc_subdirectory.IsEmpty()) {
        CFBundleRef mainBundle = CFBundleGetMainBundle ();
        CFURLRef resDir = CFBundleCopyResourcesDirectoryURL (mainBundle);
        CFURLRef resDirAbs = CFURLCopyAbsoluteURL (resDir);
        CFStringRef resPath = CFURLCopyFileSystemPath(resDirAbs, kCFURLPOSIXPathStyle);
        char path[1024];
        CFStringGetCString(resPath, path, 1024, kCFStringEncodingASCII);
        xrc_subdirectory = wxString(path) + wxFILE_SEP_PATH + _T("rc");
    }
#endif

    // First, look where we got the last xrc
    if(!xrc_subdirectory.IsEmpty()) {
        xrc_fullname = xrc_subdirectory + wxFILE_SEP_PATH + xrc_filename;
        if ( ::wxFileExists( xrc_fullname ) ) {
            wxXmlResource::Get()->Load( xrc_fullname );
            return;
        }
    }

    // Next, check where config points
    if(config->Exists(_T("/Prefs/XRCDirectory"))) {
        xrc_fullname = config->Read(_T("/Prefs/XRCDirectory"), _T("")) + wxFILE_SEP_PATH + xrc_filename;
        if ( ::wxFileExists( xrc_fullname ) ) {
            wxXmlResource::Get()->Load( xrc_fullname );
            return;
        }
    }

    // Third, check in cwd
    xrc_fullname = wxGetCwd() + wxFILE_SEP_PATH + _T("rc") + wxFILE_SEP_PATH + xrc_filename;
    if ( ::wxFileExists( xrc_fullname ) ) {
        wxXmlResource::Get()->Load( xrc_fullname );
        return;
    }

    wxString dirHome;
    wxGetHomeDir(&dirHome);

    // Last, look in ~/rc
    xrc_fullname = dirHome + wxFILE_SEP_PATH + xrc_filename;
    if ( ::wxFileExists( xrc_fullname ) ) {
        wxXmlResource::Get()->Load( xrc_fullname );
        return;
    }
}

wxConfig *theApp::getConfig()
{
    wxConfig  *config = new wxConfig(_T("iaxRpt"));

#ifndef __WXMSW__
    config->SetUmask(0077);	// owner only
#endif
    return config;
}

#ifdef __WXMSW__
BEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(XRCID("TaskBarRestore"), MyTaskBarIcon::OnRestore)
    EVT_MENU(XRCID("TaskBarHide"),    MyTaskBarIcon::OnHide)
    EVT_MENU(XRCID("TaskBarExit"),    MyTaskBarIcon::OnExit)
	EVT_TASKBAR_LEFT_DOWN(MyTaskBarIcon::OnLButtonDown)
	EVT_TASKBAR_RIGHT_DOWN(MyTaskBarIcon::OnRButtonDown)
END_EVENT_TABLE()

void MyTaskBarIcon::OnRestore(wxCommandEvent&)
{
    wxGetApp().theFrame->Show(TRUE);
}

void MyTaskBarIcon::OnIconize(wxIconizeEvent&)
{
	wxGetApp().theFrame->Show(FALSE);
}

void MyTaskBarIcon::OnHide(wxCommandEvent&)
{
	wxGetApp().theFrame->Show(FALSE);
}

void MyTaskBarIcon::OnExit(wxCommandEvent&)
{
    wxGetApp().theFrame->Close(TRUE);
    wxGetApp().ProcessIdle();
}

void MyTaskBarIcon::OnLButtonDClick(wxTaskBarIconEvent&)
{
    wxGetApp().theFrame->Close(TRUE);
    wxGetApp().ProcessIdle();
}

void MyTaskBarIcon::OnRButtonDown(wxTaskBarIconEvent&)		
{
	wxGetApp().theFrame->Iconize(FALSE);
	wxGetApp().theFrame->Show(TRUE);
}

void MyTaskBarIcon::OnLButtonDown(wxTaskBarIconEvent&)
{
	wxGetApp().theFrame->Iconize(FALSE);
	wxGetApp().theFrame->Show(TRUE);
}
#endif

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------

extern "C" {

   /* yes, using member variables, and polling for changes
    * isn't really a nice way to do callbacks.  BUT, I've tried the
    * "right" ways, unsuccessfully:
    *
    * 1) I've tried using wxGuiMutexEnter/Leave, but that deadlocks (and it should, I guess),
    * because sometimes these _are_ called in response to GUI events (i.e. the status callback eventually
    * gets called when you press hang up.
    *
    * 2) I've tried using events and ::wxPostEvent, but that also deadlocks (at least on GTK),
    * because it calls wxWakeUpIdle, which calls wxGuiMutexEnter, which deadlocks also, for some reason.
    *
    * So, this isn't ideal, but it works, until I can figure out a better way
    */

    // handle events via posting.
  #ifdef USE_GUILOCK
    static int iaxc_callback(iaxc_event e)
    {
        int ret;
        if (!wxThread::IsMain()) wxMutexGuiEnter();
            ret = wxGetApp().theFrame->HandleIAXEvent(&e);
        if (!wxThread::IsMain()) wxMutexGuiLeave();
            return ret;
    }
  #else
    static int iaxc_callback(iaxc_event e)
    {
        iaxc_event *copy;
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, IAXCLIENT_EVENT);	
		
		if(e.type==IAXC_EVENT_STATE && FALSE)
		{
			if(wxGetApp().theFrame->DebugLevel){
				wxString msg;
				msg.Printf(_T("IE STATE %i %i "),e.ev.call.state,9 );
				wxGetApp().theFrame->LogIt(msg);
			}
			//msg.Printf(_T("IE STATE %i %i %s"),e.ev.call.state,e.ev.call.subclass,e.ev.call.message );
			//wxGetApp().theFrame->LogIt(msg);
		}
        /*
		if(e.type==IAXC_EVENT_TEXT)
		{
			wxString msg;
			msg.Printf(_T("IE TEXT %s"),e.ev.text.message);
			wxGetApp().theFrame->LogIt(msg);
		}
		*/
        

        copy = (iaxc_event *)malloc(sizeof(iaxc_event));
        memcpy(copy,&e,sizeof(iaxc_event));

        evt.SetClientData(copy);
		
        //fprintf(stderr, "Posting Event\n");
        wxPostEvent(wxGetApp().theFrame, evt);

        // pretend we handle all events.
        return 1;
    }
  #endif

}
