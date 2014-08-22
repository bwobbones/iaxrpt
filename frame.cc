//----------------------------------------------------------------------------------------
// Name:        frame.cc
// Purpose:     Main frame
// Author:      Michael Van Donselaar
// Modified by:
// Created:     2003
// Copyright:   (c) Michael Van Donselaar ( michael@vandonselaar.org )
// Licence:     GPL
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// GCC implementation
//----------------------------------------------------------------------------------------

#if defined(__GNUG__) && ! defined(__APPLE__)
    #pragma implementation "frame.h"
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

#include "frame.h"

//----------------------------------------------------------------------------------------
// Remaining headers
// ---------------------------------------------------------------------------------------

#include "app.h"
#include "main.h"
#include "prefs.h"
#include "devices.h"
#include "directory.h"
#include "accounts.h"
#include "calls.h"
#include "dial.h"
#include "wx/image.h"
#include "wx/statusbr.h"
#include "wx/filesys.h"
#include "wx/fs_zip.h"
#include "wx/html/helpctrl.h"
#include "wx/utils.h"
#include "wx/tokenzr.h"

#if !wxUSE_JOYSTICK
#   error You must set wxUSE_JOYSTICK to 1 in setup.h
#endif

static bool pttMode;      // are we in PTT mode?
static bool pttState=0;     // is the PTT button pressed?
static bool silenceMode;  // are we in silence suppression mode?

static bool wasActive;
//----------------------------------------------------------------------------------------
// Event table: connect the events to the handler functions to process them
//----------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU    (IAXCLIENT_EVENT,      MyFrame::HandleEvent)

//    EVT_MENU    (XRCID("PTT"),         MyFrame::OnPTTChange)
//    EVT_MENU    (XRCID("Silence"),     MyFrame::OnSilenceChange)

	EVT_MENU    (XRCID("Access"),      MyFrame::OnAccess)
    EVT_MENU    (XRCID("Accounts"),    MyFrame::OnAccounts)
    EVT_MENU    (XRCID("Prefs"),       MyFrame::OnPrefs)
    EVT_MENU    (XRCID("Devices"),     MyFrame::OnDevices)
    EVT_MENU    (XRCID("Directory"),   MyFrame::OnDirectory)
    EVT_MENU    (XRCID("Exit"),        MyFrame::OnExit)
    EVT_MENU    (XRCID("About"),       MyFrame::OnAbout)
    EVT_MENU    (XRCID("Help"),        MyFrame::OnHelp)
	EVT_MENU    (XRCID("Support"),     MyFrame::OnSupport)

	EVT_MENU    (XRCID("k0"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k1"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k2"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k2"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k3"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k4"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k5"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k6"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k7"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k8"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("k9"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("kq"),          MyFrame::OnSimKey)
	EVT_MENU    (XRCID("kw"),          MyFrame::OnSimKey)

    EVT_SIZE    (                      CallList::OnSize)
#ifdef __WXMSW__
    EVT_ICONIZE (                      MyTaskBarIcon::OnIconize)
#endif
    EVT_BUTTON  (XRCID("0"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("1"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("2"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("3"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("4"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("5"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("6"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("7"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("8"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("9"),           MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("10"),          MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("11"),          MyFrame::OnOneTouch)
    EVT_BUTTON  (XRCID("KP0"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP1"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP2"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP3"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP4"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP5"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP6"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP7"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP8"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KP9"),         MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KPSTAR"),      MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("KPPOUND"),     MyFrame::OnKeyPad)
    EVT_BUTTON  (XRCID("Dial"),        MyFrame::OnDialDirect)
 //   EVT_BUTTON  (XRCID("Transfer"),    MyFrame::OnTransfer)
    EVT_BUTTON  (XRCID("Hold"),        MyFrame::OnHoldKey)
    EVT_BUTTON  (XRCID("Speaker"),     MyFrame::OnSpeakerKey)
    EVT_BUTTON  (XRCID("Hangup"),      MyFrame::OnHangup)
	EVT_BUTTON  (XRCID("MON"),         MyFrame::OnMonitorButton)
	EVT_BUTTON  (XRCID("PTT"),         MyFrame::OnPttButton)

    EVT_CHOICE  (XRCID("Account"),     MyFrame::OnAccountChoice)

    EVT_COMMAND_SCROLL(XRCID("OutputSlider"), MyFrame::OnOutputSlider)
    EVT_COMMAND_SCROLL(XRCID("InputSlider"),  MyFrame::OnInputSlider)
//    EVT_TEXT_ENTER    (XRCID("Extension"),    MyFrame::OnDialDirect)


	EVT_SET_FOCUS(MyFrame::OnSetFocus)
	EVT_KILL_FOCUS(MyFrame::OnKillFocus)
	EVT_CHAR(MyFrame::OnChar)
	EVT_KEY_DOWN(MyFrame::OnChar)
	EVT_KEY_UP(MyFrame::OnChar)

END_EVENT_TABLE()

//----------------------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------------------

MyFrame::MyFrame(wxWindow *parent)
{
    wxConfig   *config = theApp::getConfig();
    wxMenuBar  *aMenuBar;
    wxString    Name;
	wxStringTokenizer	*ptoke; 	
	wxString dmsg;
	int i;
	
    // MyTimer    *timer;

    // Load up this frame from XRC. [Note, instead of making a class's
    // constructor take a wxWindow* parent with a default value of NULL,
    // we could have just had designed MyFrame class with an empty
    // constructor and then written here:
    // wxXmlResource::Get()->LoadFrame(this, (wxWindow* )NULL, "MyFrame");
    // since this frame will always be the top window, and thus parentless.
    // However, the current approach has source code that can be recycled
    // in case code to moves to having an invisible frame as the top level window.

    wxXmlResource::Get()->LoadFrame(this, parent, _T("MyFrame"));
	DebugLevel=1;
	DebugLevel=0;

    //----Set the icon------------------------------------------------------------------
#ifdef __WXMSW__
    SetIcon(wxICON(application));
#endif

    //----Add the menu------------------------------------------------------------------
    aMenuBar =  wxXmlResource::Get()->LoadMenuBar(_T("main_menubar"));
    SetMenuBar( aMenuBar);

    //----Add the statusbar-------------------------------------------------------------
    const int widths[] = {-1, 60};
    CreateStatusBar( 2 );
    SetStatusWidths(2, widths);

    //----Set some preferences ---------------------------------------------------------
    config->SetPath(_T("/Prefs"));

	numStartupLinks=0;
	#if 1==1
	Name = config->Read(_T("StartupLinks"), _T(""));
	if ( !Name.IsEmpty() )
	{
		ptoke= new wxStringTokenizer(Name,",");
		numStartupLinks=ptoke->CountTokens();
		i=0;
		while(ptoke->HasMoreTokens()){
			strStartupLinks[i]=ptoke->GetNextToken();
			i++;
		}
	}
	#endif
	
    RingOnSpeaker = config->Read(_T("RingOnSpeaker"), 0l);
	CueTones      = config->Read(_T("CueTones"), 0l);
	JoyPtt        = config->Read(_T("JoyPtt"), 0l);
    AGC           = config->Read(_T("AGC"), 0l);
    AAGC          = config->Read(_T("AAGC"), 1l);
    CN            = config->Read(_T("CN"), 1l);
    NoiseReduce   = config->Read(_T("NoiseReduce"), 1l);
    EchoCancel    = config->Read(_T("EchoCancel"), 0l);
	DebugLevel    = config->Read(_T("DebugLevel"), 0l);

    config->SetPath(_T("/Codecs"));

    AllowuLawVal    = config->Read(_T("AllowuLaw"),  0l);
    AllowaLawVal    = config->Read(_T("AllowaLaw"),  0l);
    AllowGSMVal     = config->Read(_T("AllowGSM"),   1l);
    AllowSpeexVal   = config->Read(_T("AllowSpeex"), 0l);
    AllowiLBCVal    = config->Read(_T("AllowiLBC"),  0l);
    PreferredBitmap = config->Read(_T("Preferred"),  IAXC_FORMAT_GSM);

    config->SetPath(_T("/Codecs/SpeexTune"));

    SPXTuneVal      = config->Read(_T("SPXTune"),        0l);
    SPXEnhanceVal   = config->Read(_T("SPXEnhance"),     1l);
    SPXQualityVal   = config->Read(_T("SPXQuality"),    -1l);
    SPXBitrateVal   = config->Read(_T("SPXBitrate"),     9l);
    SPXABRVal       = config->Read(_T("SPXABR"),         0l);
    SPXVBRVal       = config->Read(_T("SPXVBR"),         0l);
    SPXComplexityVal= config->Read(_T("SPXComplexity"),  3l);

    config->SetPath(_T("/Prefs"));

    //----Add the panel-----------------------------------------------------------------
    Name = config->Read(_T("UseSkin"), _T("default"));
    AddPanel(this, Name);

    pttMode = true;
	PanelPtt=0;
	pttKey=0;
	pttHID=0;

    wxGetApp().InputDevice     = config->Read(_T("Input Device"), _T(""));
	wxGetApp().InputSource     = config->Read(_T("Input Source"), _T(""));
    wxGetApp().OutputDevice    = config->Read(_T("Output Device"), _T(""));
    wxGetApp().SpkInputDevice  = config->Read(_T("Speaker Input Device"),
                                              wxGetApp().InputDevice);
    wxGetApp().SpkOutputDevice = config->Read(_T("Speaker Output Device"),
                                              wxGetApp().OutputDevice);
    wxGetApp().RingDevice      = config->Read(_T("Ring Device"), _T(""));


	if(wxGetApp().InputSource==_T("No Change"))
		InputSourceIndex=0;
	else if(wxGetApp().InputSource==_T("Line In"))
		InputSourceIndex=2;
	else if(wxGetApp().InputSource==_T("Audio Input"))
		InputSourceIndex=3;
	else if(wxGetApp().InputSource==_T("Microphone"))
	{
		InputSourceIndex=1;
	}
	else
	{
		wxGetApp().InputSource=_T("Microphone");
		config->Write(_T("Input Source"),wxGetApp().InputSource);
		InputSourceIndex=1;
	}
		
	//wxMessageBox(wxGetApp().InputSource);

	ApplyFilters();
    ApplyCodecs();
    UsingSpeaker = false;

    if(OutputSlider != NULL)
        OutputSlider->SetValue(config->Read(_T("OutputLevel"), 70));

    if(InputSlider != NULL)
        InputSlider->SetValue(config->Read(_T("InputLevel"), 70));

	MicLevel = config->Read(_T("MicLevel"), 0l);

#ifdef __WXGTK__
    // window used for getting keyboard state
    GdkWindowAttr attr;

    attr.window_type = GDK_WINDOW_CHILD;
    attr.x = 0;
    attr.y = 0;
    attr.width = 0;
    attr.height = 0;
    attr.wclass = GDK_INPUT_ONLY;
    attr.visual = NULL;
    attr.colormap = NULL;
    attr.event_mask = 0;
    keyStateWindow = gdk_window_new(NULL, &attr, 0);
#endif

    maintimer = new MyTimer();
    maintimer->Start(100);

    RePanel(Name);

    wxImage::AddHandler(new wxPNGHandler);
    wxFileSystem::AddHandler(new wxZipFSHandler);
    help = new wxHtmlHelpController;
    wxFileName filename = wxFileName(_T("iaxrpt.htb"));

	iaxc_debug_iax_set(0);
    iaxc_set_silence_threshold(0);

	m_stick=NULL;
	wxJoystick stick(wxJOYSTICK1);
    if (!stick.IsOk())
    {
        //wxMessageBox(_T("No joystick detected!"));
    }
	else
	{
	    m_stick = new wxJoystick(wxJOYSTICK1);
	}

	m_sound_2up = new wxSound;
    m_sound_2dn = new wxSound;
	m_sound_ptt = new wxSound;
	m_sound_deny = new wxSound;
	m_sound_d0  = new wxSound;
	m_sound_d1  = new wxSound;
	m_sound_d2  = new wxSound;
	m_sound_d3  = new wxSound;
	m_sound_d4  = new wxSound;
	m_sound_d5  = new wxSound;
	m_sound_d6  = new wxSound;
	m_sound_d7  = new wxSound;
	m_sound_d8  = new wxSound;
	m_sound_d9  = new wxSound;
	m_sound_d10  = new wxSound;
	m_sound_d11  = new wxSound;

	m_sound_2up->Create("2up.wav",false);
	m_sound_2dn->Create("2dn.wav",false);
	m_sound_ptt->Create("ptt.wav",false);
    m_sound_deny->Create("deny.wav",false);
	
	m_sound_d0->Create("d0.wav",false);
	m_sound_d1->Create("d1.wav",false);
	m_sound_d2->Create("d2.wav",false);
	m_sound_d3->Create("d3.wav",false);
	m_sound_d4->Create("d4.wav",false);
	m_sound_d5->Create("d5.wav",false);
	m_sound_d6->Create("d6.wav",false);
	m_sound_d7->Create("d7.wav",false);
	m_sound_d8->Create("d8.wav",false);
	m_sound_d9->Create("d9.wav",false);
	m_sound_d10->Create("d10.wav",false);
	m_sound_d11->Create("d11.wav",false);

	// wxFocusEvent dummy = NULL;
	// OnSetFocus(dummy);

    if (CueTones && m_sound_ptt->IsOk())
       	m_sound_ptt->Play(wxSOUND_ASYNC);

    OnTheAir->Show(false);

	CallScan=FALSE;
	CallScanHeld=-1;
	ScanaTicks=0;
	ScanbTicks=0;
	MacroTicks=50;
	MacroCount=0;
	ClientPtt=0;
	PttBonk=0;
	PttInhibit=0;


#ifdef NOHELP
    if (filename.FileExists()) {
	help->AddBook(filename);
	return;
    }
#endif

#ifdef DATADIR
    filename = wxFileName(wxString(DATADIR) + wxFILE_SEP_PATH +
			  _T("iaxrpt.htb"));

    if (filename.FileExists()) {
	help->AddBook(filename);
	return;
    }
#endif /* DATADIR */
}

void MyFrame::RePanel(wxString Name)
{
    aPanel->Destroy();
    AddPanel(this, Name);
    Layout();
}

void MyFrame::OnSize(wxSizeEvent &event)
{
    event.Skip();
}

void MyFrame::AddPanel(wxWindow *parent, wxString Name)
{
    wxBoxSizer *panelSizer;

    aPanel = new wxPanel(parent);
    aPanel = wxXmlResource::Get()->LoadPanel(parent, Name);
    if(aPanel == NULL)
        aPanel = wxXmlResource::Get()->LoadPanel(parent, wxT("default"));

    if(aPanel == NULL)
        wxLogError(_("Can't Load Panel in frame.cc"));

    //----Reach in for our controls-----------------------------------------------------
    Input        = XRCCTRL(*aPanel, "Input",        wxGauge);
    Output       = XRCCTRL(*aPanel, "Output",       wxGauge);
    OutputSlider = XRCCTRL(*aPanel, "OutputSlider", wxSlider);
    InputSlider  = XRCCTRL(*aPanel, "InputSlider",  wxSlider);
    Account      = XRCCTRL(*aPanel, "Account",      wxChoice);
	OnTheAir     = XRCCTRL(*aPanel, "OnTheAir",     wxTextCtrl);
	HoldButton   = XRCCTRL(*aPanel, "Hold",         wxButton);
	PttButton    = XRCCTRL(*aPanel, "PTT",          wxButton);
	HangupButton = XRCCTRL(*aPanel, "Hangup",       wxButton);
	LogList		 = XRCCTRL(*aPanel, "Log",          wxListBox);

    // Extension    = XRCCTRL(*aPanel, "Extension",    wxComboBox);

	LogList->Show(TRUE);
	LogList->Enable(TRUE);
	LogList->Set(0,NULL,NULL);

    //----Insert the Calls listctrl into it's "unknown" placeholder---------------------
    Calls = new CallList(aPanel, wxGetApp().nCalls);

    if(Calls == NULL)
        wxLogError(_("Can't Load CallList in frame.cc"));

    wxXmlResource::Get()->AttachUnknownControl(_T("Calls"), Calls);

    ShowDirectoryControls();

    panelSizer = new wxBoxSizer(wxVERTICAL);
    panelSizer->Add(aPanel,1,wxEXPAND);

    SetSizer(panelSizer);
    panelSizer->SetSizeHints(parent);

	//wxColour bbgColour;

	FrameBgColour = PttButton->GetBackgroundColour(); 
	// bbgColour.Set(wxT("#ff0000"));
	//bbgColour.Set(#ff0000);
	//bbgColour.Set(wxRED);

	//PttButton->SetBackgroundColour(bbgColour);
	//PttButton->SetBackgroundColour(*wxRED);

}

void MyFrame::ApplyFilters()
{
    // Clear these filters
    int flag = ~(IAXC_FILTER_AGC | IAXC_FILTER_AAGC | IAXC_FILTER_CN |
                 IAXC_FILTER_DENOISE | IAXC_FILTER_ECHO);
    iaxc_set_filters(iaxc_get_filters() & flag);

    flag = 0;
    //if(AGC)
    //   flag = IAXC_FILTER_AGC;

    //if(AAGC)
    //   flag = IAXC_FILTER_AAGC;

    if(CN)
       flag = IAXC_FILTER_CN;

    if(NoiseReduce)
       flag |= IAXC_FILTER_DENOISE;

    if(EchoCancel)
       flag |= IAXC_FILTER_ECHO;

    iaxc_set_filters(iaxc_get_filters() | flag);
}

void MyFrame::ApplyCodecs()
{
    int  Allowed = 0;

    if(AllowiLBCVal)
        Allowed |= IAXC_FORMAT_ILBC;

    if(AllowGSMVal)
        Allowed |= IAXC_FORMAT_GSM;

    if(AllowSpeexVal)
        Allowed |= IAXC_FORMAT_SPEEX;

    if(AllowuLawVal)
        Allowed |= IAXC_FORMAT_ULAW;

    if(AllowaLawVal)
        Allowed |= IAXC_FORMAT_ALAW;

	//Allowed = IAXC_FORMAT_G726;
	// Allowed = IAXC_FORMAT_ULAW;

	if(DebugLevel)
	{
		wxString msg;
		msg.Printf(_T("ApplyCodecs() 0x%x"), Allowed);
		LogIt(msg);
	}
		 
    iaxc_set_formats(PreferredBitmap, Allowed);

    if(SPXTuneVal) {
        iaxc_set_speex_settings(   (int)SPXEnhanceVal,
                                 (float)SPXQualityVal,
                                   (int)(SPXBitrateVal * 1000),
                                   (int)SPXVBRVal,
                                   (int)SPXABRVal,
                                   (int)SPXComplexityVal);

    }
}

MyFrame::~MyFrame()
{
#ifdef __WXMSW__
    delete wxGetApp().theTaskBarIcon;
#endif

	// so it doesn't try to play the sound after the frame is destroyed
	if (m_sound_2dn->IsOk())
	{
		//if(CueTones)m_sound_2dn->Play(wxSOUND_SYNC);
		m_sound_2dn->~wxSound();
	}

#if 1 // turned back on to eliminate errors on Windows sph
    // Getting rid of this block seems to be harmless, and also seems to get rid of the
    // frequent hang when exiting on linux.
    iaxc_dump_all_calls();
    for(int i=0;i<10;i++) {
        iaxc_millisleep(100);
    }
//    iaxc_stop_processing_thread();	 // but took this out
#endif

	 
    if(help != NULL)
        help->Quit();

    aPanel->Destroy();
}

void MyFrame::ShowDirectoryControls()
{
    wxConfig   *config = theApp::getConfig();
    wxButton   *ot;
    wxString    OTName;
    wxString    DialString;
    wxString    Name;
    wxString    Label;

    long        dummy;
    bool        bCont;

    //----Add Accounts-------------------------------------------------------------------
    if(Account != NULL) {
		wxString    tmpstr;
        Account->Clear();
        config->SetPath(_T("/Accounts"));
        bCont = config->GetFirstGroup(Name, dummy);
        while ( bCont ) {
			Label=config->Read(Name + _T("/AccountLabel"),_T(""));
			Label = _T("<") + Name + _T("> ") + Label;
            Account->Append(Label);
		    if(Name==wxGetApp().DefaultAccount)
			{
				tmpstr=Label;	
			}
            bCont = config->GetNextGroup(Name, dummy);
        }

		if(!tmpstr.IsEmpty())
		{
        	Account->SetSelection(Account->FindString(tmpstr));  	
		}
		//Account->SetSelection(Account->FindString(wxGetApp().DefaultAccount));
		if(DebugLevel)
		{
			wxString msg;
			LogIt(_T("Past Account Set Selection."));
			msg.Printf(_T("DefaultAccount=%s"),wxGetApp().DefaultAccount.c_str());
			LogIt(msg);
			msg.Printf(_T("FindString=%i"),Account->FindString(wxGetApp().DefaultAccount));
			LogIt(msg);
			msg.Printf(_T("SelectedAccount=%s"),Account->GetStringSelection().c_str());
			LogIt(msg);
		}
    }


    //----Load up One Touch Keys--------------------------------------------------------
    config->SetPath(_T("/OT"));
    bCont = config->GetFirstGroup(OTName, dummy);
    while ( bCont ) {
#if defined(__UNICODE__)
        ot = ((wxButton *)((*aPanel).FindWindow(wxXmlResource::GetXRCID(OTName))));
#else
        ot = XRCCTRL(*aPanel, OTName, wxButton);
#endif
        if(ot != NULL) {
            Name = OTName + _T("/Name");
            Label = config->Read(Name, _T(""));
            if(!Label.IsEmpty()) {
                ot->SetLabel(Label);
            } else {
                ot->SetLabel(_T("OT") + OTName);
            }
            DialString = OTName + _T("/Extension");
            Label = config->Read(DialString, _T(""));
            if(!Label.IsEmpty()) {
                ot->SetToolTip(Label);
            }
        }
        bCont = config->GetNextGroup(OTName, dummy);
    }

#if 0
    //----Load up Extension ComboBox----------------------------------------------------
    config->SetPath(_T("/PhoneBook"));
    bCont = config->GetFirstGroup(Name, dummy);
    while ( bCont ) {
        Extension->Append(Name);
        bCont = config->GetNextGroup(Name, dummy);
    }
#endif
}


/*	Every 100 ms	*/

void MyFrame::OnNotify()
{
	wxString  dmsg;
	int i;
	
    MessageTicks++;

	if(MacroTicks>0)--MacroTicks;
	
	if(MacroTicks<=0)
	{
		if(DebugLevel){
			dmsg.Printf(_T("MTicks Hit MCnt=%i numSLinks=%i"),MacroCount, numStartupLinks);
			LogIt(dmsg);
		}
				
		if(numStartupLinks>0 && MacroCount<numStartupLinks )
		{
			if(DebugLevel){
				dmsg.Printf(_T("MacroTicks Connect=%s"),strStartupLinks[MacroCount].c_str());
				LogIt(dmsg);
			}
			wxGetApp().DefaultAccount=strStartupLinks[MacroCount];
			Dial(wxGetApp().DefaultAccount);
			iaxc_set_silence_threshold(0);			
			MacroCount++;
			MacroTicks=30;
        }			
		else
		{
			MacroTicks=300000;
		}
	}
	
    if(MessageTicks > 30) {
        MessageTicks = 0;
        wxGetApp().theFrame->SetStatusText(_T(""));
    }

	if(BonkTicks>0)--BonkTicks;
	if(ScanaTicks>0)--ScanaTicks;
	if(ScanbTicks>0)
	{
		/* scan holdoff expires */
		if(--ScanbTicks==0)
			wxGetApp().theFrame->Calls->ScanRequest = 1;
	}
	
	if( pttState ) 
	{
		ScanaTicks=32;	
		ScanbTicks=32;
		CallScanHeld = -1;
		wxGetApp().theFrame->Calls->ScanRequest = 0;
	}
	else if( !pttState && wxGetApp().theFrame->Calls->ScanRequest )
	{
		wxGetApp().theFrame->Calls->ScanRequest = 0;
		wxGetApp().theFrame->Calls->ScanCalls();
	}
	else if( CallScanHeld>=0 && 
	         wxGetApp().theFrame->Calls->CallInfo[CallScanHeld].active == 1 &&
	         wxGetApp().theFrame->Calls->CallInfo[CallScanHeld].rxkey == 1 )
	{
		ScanaTicks=32;
	}
	else if( (ScanaTicks==0) && CallScanHeld>=0 ){
		if(DebugLevel)
		{	
			dmsg.Printf(_T("ScanaTicks Timeout CallScanHeld=%i %d %d"),CallScanHeld,
				wxGetApp().theFrame->Calls->CallInfo[CallScanHeld].active,
				wxGetApp().theFrame->Calls->CallInfo[CallScanHeld].rxkey
			);
			LogIt(dmsg);
		}
		CallScanHeld = -1;
		wxGetApp().theFrame->Calls->ScanCalls();
	}    
	if(pttMode) CheckPTT();
	
	
#if 1 == 0
	if(HostInformTicks>0)
	{
		--HostInformTicks;
		if(HostInformTicks<=0)
		{
		    wxString  ymsg;
			HostInformTicks=0;
			ymsg=_T("!IAXKEY! 1 1 0 0");
			iaxc_send_text(ymsg.c_str());
			if(DebugLevel)LogIt(_T("sent host !IAXKEY! 1 1 0 0"));
		}
	}
	
#endif

	for(i=0; i<wxGetApp().nCalls; i++)
	{
		if(Calls->CallInfo[i].NewkeyTimer)
		{
			Calls->CallInfo[i].NewkeyTimer--;
			if(Calls->CallInfo[i].NewkeyTimer<=0)
			{
				Calls->CallInfo[i].NewkeyTimer=0;
				Calls->CallInfo[i].newkey=0;
			}
		}
	}
	
    wxGetApp().theFrame->Calls->UpdateConnectedNodes();
}

void MyFrame::OnShow()
{
    Show(TRUE);
}

void MyFrame::OnSpeakerKey(wxCommandEvent &event)
{
    if(UsingSpeaker != true) {
        UsingSpeaker = true;
        SetAudioDevices(wxGetApp().SpkInputDevice,
                        wxGetApp().SpkOutputDevice,
                        wxGetApp().RingDevice,
                        wxGetApp().theFrame->InputSourceIndex);
    } else {
        UsingSpeaker = false;
        SetAudioDevices(wxGetApp().InputDevice,
                        wxGetApp().OutputDevice,
                        wxGetApp().RingDevice,
                        wxGetApp().theFrame->InputSourceIndex);
    }
}

void MyFrame::OnMonitorButton(wxCommandEvent &event)
{
	wxString msg;

	if(DebugLevel)
	{
		int rtt;

		struct iaxc_netstat stat_local;
		struct iaxc_netstat stat_remote;

		LogIt(_T("Monitor Pressed."));
		msg.Printf(_T("MicBoost=%i"),iaxc_mic_boost_get());
		LogIt(msg);

		msg.Printf(_T("IaxBindPort=%i"),iaxc_get_bind_port());
		LogIt(msg);
	
	 	iaxc_get_netstats(0, &rtt, &stat_local, &stat_remote);
	
		msg.Printf(_T("IaxRtt=%i"),rtt);
		LogIt(msg);
		msg.Printf(_T("IaxDelay=%i/%i"),stat_local.delay,stat_remote.delay);
		LogIt(msg);
		msg.Printf(_T("IaxJitter=%i/%i"),stat_local.jitter,stat_remote.jitter);
		LogIt(msg);
		msg.Printf(_T("IaxLossPct=%i/%i"),stat_local.losspct,stat_remote.losspct);
		LogIt(msg);
		msg.Printf(_T("IaxLossCnt=%i/%i"),stat_local.losscnt,stat_remote.losscnt);
		LogIt(msg);
		msg.Printf(_T("IaxPackets=%i/%i"),stat_local.packets,stat_remote.packets);
		LogIt(msg);
		msg.Printf(_T("IaxDropped=%i/%i"),stat_local.dropped,stat_remote.dropped);
		LogIt(msg);
		msg.Printf(_T("IaxOoo=%i/%i"),stat_local.ooo,stat_remote.ooo);
		LogIt(msg);
		msg.Printf(_T("MicLevel=%i"),wxGetApp().theFrame->MicLevel);
		LogIt(msg);
		msg.Printf(_T("MicBoost=%i"),wxGetApp().theFrame->MicBoost);
		LogIt(msg);
	}
	if(PanelMonitor)PanelMonitor=0;
	else PanelMonitor=1;
}

void MyFrame::OnPttButton(wxCommandEvent &event)
{
	if(PanelPtt)PanelPtt=0;
	else PanelPtt=1;
}

void MyFrame::OnHoldKey(wxCommandEvent &event)
{
    int i, selected = iaxc_selected_call();

	if( HoldButton->GetLabel() == _T("Scan Start") )
	{
		HoldButton->SetLabel( _T("Scan Stop"));
		CallScan=TRUE;
 	 	ScanaTicks=0;
 	 	CallScanHeld=-1;
		
		wxGetApp().theFrame->Calls->ScanSelectedPreScan=selected;
		
		wxGetApp().theFrame->Calls->ScanCalls();
		HoldButton->SetBackgroundColour(*wxCYAN);
	}
	else
	{
		HoldButton->SetLabel( _T("Scan Start")); 
		CallScan=FALSE;
		CallScanHeld=-1;
		ScanaTicks=0;
		HoldButton->SetBackgroundColour(FrameBgColour);
		for(i=0; i<wxGetApp().nCalls; i++)
		{
			if(i != wxGetApp().theFrame->CallScanHeld)
				Calls->SetItemBackgroundColour(i,*wxWHITE);
		}
		Refresh();
	}

    if(selected < 0)
        return;

    //iaxc_quelch(selected,0);
    //iaxc_select_call(-1);
}

void MyFrame::OnHangup(wxCommandEvent &event)
{
    iaxc_dump_call();
}

void MyFrame::OnQuit(wxEvent &event)
{
    Close(TRUE);
}

void MyFrame::OnOutputSlider(wxScrollEvent &event)
{
    int      pos = event.GetPosition();

    iaxc_output_level_set((double)pos/100.0);
}

void MyFrame::OnInputSlider(wxScrollEvent &event)
{
    int      pos = event.GetPosition();

    //iaxc_input_level_set((double)pos/100.0);
	//iaxc_input_level_set(0);		           //maw 0413
}

void MyFrame::OnPTTChange(wxCommandEvent &event) {

	// sphenke 20070322 this does not appear to be used
    pttMode = event.IsChecked();

    if(pttMode) {
        SetPTT(GetPTTState());
    } else {
        SetPTT(true);
        if(silenceMode) {
            iaxc_set_silence_threshold(DEFAULT_SILENCE_THRESHOLD);
            SetStatusText(_("VOX"),1);
        } else {
            iaxc_set_silence_threshold(-99);
            SetStatusText(_(""),1);
        }
        //iaxc_set_audio_output(0);  // unmute output  //maw 0413
    }
}

void MyFrame::OnSilenceChange(wxCommandEvent &event)
{
    // XXX get the actual state!
    silenceMode =  event.IsChecked();

    if(pttMode) return;

    if(silenceMode) {
        iaxc_set_silence_threshold(DEFAULT_SILENCE_THRESHOLD);
        SetStatusText(_("VOX"),1);
    } else {
        iaxc_set_silence_threshold(-99);
        SetStatusText(_(""),1);
    }
}

bool MyFrame::GetPTTState()
{
    static bool waspressed=0;
	bool pressed,keypressed;
	bool isActive = wxGetApp().theFrame->IsActive();
	
	/*	Send cue sounds if the frame focus has changed 		*/
	if(isActive!=wasActive)
	{
		if(isActive && CueTones && m_sound_2up->IsOk())
		{
			m_sound_2up->Play(wxSOUND_ASYNC);	
		}
		else if ( !isActive && CueTones && m_sound_2dn->IsOk())
		{
			m_sound_2dn->Play(wxSOUND_ASYNC);
		}
		wasActive=isActive;
	}
	
	keypressed=pressed=pttHID=pttKey=0;
	
	#ifdef __WXMAC__
	  KeyMap theKeys;
	  GetKeys(theKeys);
	  // that's the Control Key (by experimentation!)
	  keypressed = theKeys[1] & 0x08;
	  //fprintf(stderr, "%p %p %p %p\n", theKeys[0], theKeys[1], theKeys[2], theKeys[3]);
	#else
	#ifdef __WXMSW__
		keypressed = GetAsyncKeyState(VK_CONTROL)&0x8000;
	#else
	  int x, y;
	  GdkModifierType modifiers;
	  gdk_window_get_pointer(keyStateWindow, &x, &y, &modifiers);
	  keypressed = modifiers & GDK_CONTROL_MASK;
	#endif
	#endif
	
	if ( !isActive && PanelPtt ) {
		PanelPtt=0;
    }
	
	// if the frame is not active, don't activate the transmitter/ptt
	// but do allow releases, i.e. ptt off
    if ( !isActive && !keypressed ) {
		pttKey=0;
    }
	else if ( isActive && keypressed ) { 
		pttKey=1;
		PanelPtt=0;
	}
	
	if(wxGetApp().theFrame->JoyPtt && m_stick && m_stick->IsOk() && m_stick->GetButtonState())
	{
		pttHID=1;
		PanelPtt=0;
	}
	
	pressed=pttKey||pttHID||PanelPtt;

	if(waspressed!=pressed)
	{
		waspressed=pressed;
		if (pressed) 
		{
			wxGetApp().theFrame->Calls->ScanClearHeld();
			CallScanHeld=-1;
			wxGetApp().theFrame->Calls->Enable(FALSE);
			HangupButton->Enable(FALSE);
		}
		else
		{
			wxGetApp().theFrame->Calls->Enable(TRUE); 
			HangupButton->Enable(TRUE);
		}
	}
    return pressed;
}

void MyFrame::CheckPTT()
{
    bool pressed = GetPTTState();
	int i, selected = iaxc_selected_call();
	
	if( pressed && 
		wxGetApp().theFrame->Calls->CallInfo[selected].active &&
	    !wxGetApp().theFrame->Calls->CallInfo[selected].ptthit)
	{
		wxGetApp().theFrame->Calls->CallInfo[selected].ptthit=1;
	}
	if( pressed && 	
	    ! wxGetApp().theFrame->Calls->CallInfo[wxGetApp().theFrame->Calls->UserSelected].active )
	{
		PttBonk=1;
	}
	
	if( pressed &&
	    wxGetApp().theFrame->Calls->CallInfo[wxGetApp().theFrame->Calls->UserSelected].newkey==0 &&
	    !wxGetApp().theFrame->Calls->CallInfo[selected].ptthit
	  )
	{
		pressed=0;
	}
		
    if(PttInhibit && !pressed)
	{
		PttInhibit=0;
		PttButton->SetBackgroundColour(FrameBgColour);
	}
	
	if(PttInhibit && pressed && BonkTicks<=0)
	{
		if ( m_sound_deny->IsOk() )
		{
			BonkTicks=10;
			m_sound_deny->Play(wxSOUND_ASYNC);	
		}
	}
	
	if(PttBonk)
	{
		pressed=0;
		PttInhibit=1;
		if ( m_sound_deny->IsOk() )
		{
			PttBonk=0;
			BonkTicks=10;
			m_sound_deny->Play(wxSOUND_ASYNC);
			PttButton->SetBackgroundColour(*wxBLUE);			
		}
	}
	
	if(PttInhibit)pressed=0;
	
    if(pressed == pttState) return;

    SetPTT(pressed);
	
}

void MyFrame::LogIt(wxString msg)
{
	static int mc;
	static int lc;
	int i;
	wxString  tmsg;
	wxDateTime thack;
	wxString  xmsg;
	mc++;
	lc++;
	
	if(msg==_T("!NEWKEY1!") && 0)
		xmsg=_T(" ***");
	else
		xmsg=_T(" ");

	thack=wxDateTime::UNow();

	i=LogList->GetCount();
	// to do: handle list getting full, log to file, etc.

    // tmsg.Printf(_T("%4i:%s %s"), mc, thack.FormatISOTime().c_str(), msg.c_str());
	tmsg.Printf(_T("%4i:%s %s %s"), mc, thack.Format(_T("%H:%M:%S.%l")).c_str(), msg.c_str(),xmsg.c_str());
	LogList->InsertItems(1,&tmsg,i);
	if(i>4){
		LogList->SetFirstItem(i-4);
	}
}

/*
	when scanning another call may be selected
	always ptt on the user selected call

*/
void MyFrame::SetPTT(bool state)
{
	static int lastKeyed=0, lastSelected=0;
	static int pttCount=0;
    wxString pttmsg;
	
    int selected = iaxc_selected_call();

	//if(pttState)selected=lastKeyed;
	// if(pttState && (lastKeyed!=selected))state=0;

	pttCount++;

	if(DebugLevel)
	{
		wxString msg;
		msg.Printf(_T("ptt=%i cnt=%i sel=%i usel=%i"), state, pttCount, selected, wxGetApp().theFrame->Calls->UserSelected);
		LogIt(msg);
	}

    ClientPtt = pttState = state;

    if(pttState) {
        iaxc_set_silence_threshold(-99); 	//unmute input
        //iaxc_set_audio_output(1);  		// mute output
		
		if(wxGetApp().theFrame->Calls->UserSelected!=selected)
		{
			iaxc_select_call(wxGetApp().theFrame->Calls->UserSelected);
			//SetItemState(selected,~wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
			//SetItemState(wxGetApp().theFrame->Calls->UserSelected,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
			selected=wxGetApp().theFrame->Calls->UserSelected;
		}
		
	    if(selected >= 0) {
		    //iaxc_input_level_set(MicLevel/100.0);
			// ptt not available
			if(TRUE)
			{
				//SetStatusText(_("TALK "),1);
				//pttmsg=_T("103.5");
				pttmsg=_T("");
				iaxc_key_radio(selected,pttmsg.c_str());
				//iaxc_key_radio(selected,NULL);
									
				if (CueTones && m_sound_ptt->IsOk())m_sound_ptt->Play(wxSOUND_ASYNC);
				//OnTheAir->Show(true);
				PttButton->SetBackgroundColour(*wxRED);
				lastKeyed=selected;
			}
			else
			{
				if (m_sound_ptt->IsOk())m_sound_deny->Play(wxSOUND_ASYNC);
				//wasActive=0;
			}
		}
		else
		{
			if (m_sound_ptt->IsOk())m_sound_deny->Play(wxSOUND_ASYNC);
		}
    } else {
		if( wxGetApp().theFrame->Calls->CallInfo[selected].newkey==0 &&
		    wxGetApp().theFrame->Calls->CallInfo[selected].ptthit==0
		  )
		{
		    // open audio for non radio calls
			//iaxc_unquelch(UserSelected);
			//iaxc_set_audio_output(0);    // play audio
			iaxc_set_silence_threshold(-99); 	 //  mute input from mic
		}
		else
		{
			iaxc_set_silence_threshold(0); 
		}
		//iaxc_set_silence_threshold(0); 	 //  mute input from mic
        //iaxc_set_audio_output(0);  // unmute output
		//SetStatusText(_("LISTEN"),1);
        if(lastKeyed >= 0) iaxc_unkey_radio(lastKeyed,NULL);
        //OnTheAir->Show(false);
		PttButton->SetBackgroundColour(FrameBgColour);
		lastKeyed=-1;
    }
}

void MyFrame::HandleEvent(wxCommandEvent &evt)
{
    iaxc_event *e = (iaxc_event *)evt.GetClientData();
    HandleIAXEvent(e);
    free (e);
}

int MyFrame::HandleIAXEvent(iaxc_event *e)
{
    int ret = 0;
	wxString dest;

    switch(e->type) {
      case IAXC_EVENT_LEVELS:
           ret = HandleLevelEvent(e->ev.levels.input, e->ev.levels.output);
           break;
      case IAXC_EVENT_TEXT:
		   if(DebugLevel>0){
				dest.Printf(_T("RX IE TEXT call=%i %s"), e->ev.text.callNo, e->ev.text.message);
				LogIt(dest);
		   }
		   ret = HandleStatusEvent(e->ev.text.message);
		   if(e->ev.text.callNo>=0)
		   {
				wxString  ymsg,zmsg;
				ymsg=e->ev.text.message;
				if(ymsg==_T("!NEWKEY1!"))
				{
					if(DebugLevel>0){
						dest.Printf(_T("RX got !NEWKEY1! on call=%i"), e->ev.text.callNo);
						LogIt(dest);
						// dest.Printf(_T("Sending !NEWKEY1! back on call=%i"), e->ev.text.callNo);
						// LogIt(dest);
					}
					wxGetApp().theFrame->Calls->CallInfo[e->ev.text.callNo].newkey=2;
					
					// ymsg=_T("!NEWKEY1!");
					// sprintf(e->ev.text.message,"%s",ymsg.c_str());
					// iaxc_send_text_call(e->ev.text.callNo,ymsg.c_str());		
					HostInformTicks=0;
				}
				else if(ymsg==_T("!NEWKEY!"))
				{
					if(DebugLevel>0){
						dest.Printf(_T("RX got !NEWKEY! on call=%i"), e->ev.text.callNo);
						LogIt(dest);
					}
					//sprintf(e->ev.text.message,"%s",ymsg.c_str());
					// iaxc_send_text_call(e->ev.text.callNo,ymsg.c_str());		
				}
				else if(ymsg.StartsWith(_T("!IAXKEY!")))
				{
					if(DebugLevel>0){
						dest.Printf(_T("RX got !IAXKEY! on call=%i "), e->ev.text.callNo);
						LogIt(dest);
					}
					Calls->CallInfo[e->ev.text.callNo].iaxkey=1;
					
					// ymsg==_T("!IAXKEY! 1 1 0 0");
					// sprintf(e->ev.text.message,"%s",ymsg.c_str());
					// iaxc_send_text_call(e->ev.text.callNo,ymsg.c_str());			
				}
				
				else if(ymsg==_T("DENY/INHIBIT"))
				{
					// ymsg=_T("!NEWKEY!"); 
					//iaxc_send_text_call(e->ev.text.callNo,ymsg.c_str());
					ymsg=_T("KILL BONK");
					sprintf(e->ev.text.message,"%s",ymsg.c_str());
					PttBonk=1;
				}		
				else if( ymsg.StartsWith(_T("L "),&zmsg) )
				{
					wxGetApp().theFrame->Calls->HandleConnectedNodes(zmsg,e->ev.text.callNo);
					
					#if FOOP
					wxGetApp().theFrame->Calls->CallInfo[e->ev.text.callNo].ConnectedNodesTimer=120;
					wxGetApp().theFrame->Calls->CallInfo[e->ev.text.callNo].ConnectedNodes=zmsg;
					wxGetApp().theFrame->Calls->SetItem(e->ev.text.callNo, 3, zmsg);
					wxGetApp().theFrame->Calls->Refresh();
					
					if(DebugLevel>0)
					{
						dest.Printf(_T("GOT CONNECTED NODES LIST call=%i %s"), e->ev.text.callNo, zmsg.c_str());
						wxGetApp().theFrame->LogIt(dest);
					}
					#endif
				}
		   }
           break;
      case IAXC_EVENT_STATE:
	  	   if(e->ev.call.callNo>=0 && DebugLevel>0 && FALSE)
		   {
			dest.Printf(_T("IE STATE call=%i %i %d"), e->ev.call.callNo,e->ev.call.subclass,e->ev.call.message[0]);
		   	if(DebugLevel)wxGetApp().theFrame->LogIt(dest);
			if(e->ev.call.subclass==AST_CONTROL_KEY || e->ev.call.subclass==AST_CONTROL_UNKEY)
			{
				if(DebugLevel){
					dest.Printf(_T("IE STATE data %s"),e->ev.call.message);
					LogIt(dest);	
				}
			}
		   }
           ret = wxGetApp().theFrame->Calls->HandleStateEvent(e->ev.call);
           break;
      default:
           break;  // not handled
    }
     return ret;
}

int MyFrame::HandleStatusEvent(char *msg)
{
	wxString tstr;
	tstr.Printf(_T("HandleStatusEvent() %s"), msg);

#if defined(__UNICODE__)
    wchar_t ws[256];
    wxMBConvUTF8 utf8;
#endif

    MessageTicks = 0;
#if defined(__UNICODE__)
    utf8.MB2WC(ws, msg, 256);
    SetStatusText(ws);
	if(DebugLevel)LogIt(ws);
#else
    SetStatusText(msg);
	if(DebugLevel)LogIt(tstr);
	//wxGetApp().theFrame->LogIt(tstr);
#endif
    return 1;
}

int MyFrame::HandleLevelEvent(float input, float output)
{
    int inputLevel, outputLevel;

    if (input < LEVEL_MIN)
        input = LEVEL_MIN;
    else if (input > LEVEL_MAX)
        input = LEVEL_MAX;
    inputLevel = (int)input - (LEVEL_MIN);

    if (output < LEVEL_MIN)
        output = LEVEL_MIN;
    else if (input > LEVEL_MAX)
        output = LEVEL_MAX;
    outputLevel = (int)output - (LEVEL_MIN);

    static int lastInputLevel = 0;
    static int lastOutputLevel = 0;

	if(!pttState)inputLevel=0;

    if(wxGetApp().theFrame->Input != NULL) {
      if(lastInputLevel != inputLevel) {
        wxGetApp().theFrame->Input->SetValue(inputLevel*10);
        lastInputLevel = inputLevel;
      }
    }

    if(wxGetApp().theFrame->Output != NULL) {
      if(lastOutputLevel != outputLevel) {
        wxGetApp().theFrame->Output->SetValue(outputLevel*2);
        lastOutputLevel = outputLevel;
      }
    }

    return 1;
}

//----------------------------------------------------------------------------------------
// Private methods
//----------------------------------------------------------------------------------------

void MyFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	// stop the timer too!

	maintimer->Stop();
	iaxc_dump_call();
	iaxc_stop_processing_thread();
	Sleep(52);
	iaxc_shutdown();
	Sleep(52);
    Close(TRUE);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString  msg;
    char*     libver = (char *)malloc(256);

    libver = iaxc_version(libver);
    msg.Printf(_T("Program Version:\t%s\nlibiaxclient Version:\t%s\nLicense: GPL"), VERSION, libver);
    wxMessageBox(msg, _("iaxRpt"), wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnHelp(wxCommandEvent& WXUNUSED(event))
{
	wxString url;

	url=_T("http://www.xelatec.com/xipar/iaxrpthelp");
    if(!wxGetApp().HelpURL.IsEmpty())
	{
		url=wxGetApp().HelpURL;		
	}
	wxLaunchDefaultBrowser(url);
    // help->DisplayContents();
}

void MyFrame::OnSupport(wxCommandEvent& WXUNUSED(event))
{
	wxString url;
	url=_T("http://www.xelatec.com/xippr");
	wxLaunchDefaultBrowser(url);
}

void MyFrame::OnAccess(wxCommandEvent& WXUNUSED(event))
{
    AccessDialog dialog(this);
    dialog.ShowModal();
}

int MyFrame::AccessCheck(void)
{
	int rv=1;
	if(!wxGetApp().AccessLevel)
	{
		wxMessageBox(_T("Access Password Required for this Option."));
		rv=0;
	}
	return(rv);
}

void MyFrame::OnAccounts(wxCommandEvent& WXUNUSED(event))
{
	if(!AccessCheck())return;
    AccountsDialog dialog(this);
    dialog.ShowModal();
}

void MyFrame::OnPrefs(wxCommandEvent& WXUNUSED(event))
{
	if(!AccessCheck())return;
	PrefsDialog dialog(this);
	dialog.ShowModal();
}

void MyFrame::OnDevices(wxCommandEvent& WXUNUSED(event))
{
	if(!AccessCheck())return;
	DevicesDialog dialog(this);
    dialog.ShowModal();
}

void MyFrame::OnDirectory(wxCommandEvent& WXUNUSED(event))
{
	if(!AccessCheck())return;
    DirectoryDialog dialog(this);
    dialog.ShowModal();
}

void MyFrame::OnOneTouch(wxCommandEvent &event)
{
    wxConfig *config = theApp::getConfig();
    wxString  Message,msg;
    int       OTNo;
    wxString  PathName;
    wxString  DialString;
    char      digits[2048];
    char      cstr[128];
    int	      x;
    wxString  Nodestr = wxGetApp().DefaultAccount;
    OTNo = event.GetId() - XRCID("0");

    if(Nodestr.IsEmpty()) {
        return;
    }
    int n = Nodestr.First(' ');
    if (n > 0)
    {
        Nodestr.Truncate(n);
    }
    if (n == 0) return;

	if(!wxGetApp().theFrame->Calls->CallInfo[wxGetApp().theFrame->Calls->UserSelected].active)
	{
		if ( m_sound_deny->IsOk() ) m_sound_deny->Play(wxSOUND_ASYNC);
		if(DebugLevel>1){
			msg.Printf(_T("Selected is not Active DENY"));
			LogIt(msg);
		}
		return;
	}
	
	
    PathName.Printf(_T("/OT/%d"), OTNo);
    config->SetPath(PathName);
    DialString = config->Read(_T("Extension"), _T(""));

    if(DialString.IsEmpty())
        return;

    // A DialString in quotes means look up name in phone book
    if(DialString.StartsWith(_T("\""))) {
        DialString = DialString.Mid(1, DialString.Len() -2);
        DialString = config->Read(_T("/PhoneBook/") + DialString + _T("/Extension"), _T(""));
    }
    strcpy(digits,(const char *) DialString.mb_str(wxConvUTF8));
    for(x = 0; digits[x]; x++)
	{
		wxString msg;
    	sprintf(cstr, "D %s 0 1 %c", (const char *) Nodestr.mb_str(wxConvUTF8), digits[x]);
	    iaxc_send_text(cstr);
		if(DebugLevel)wxGetApp().theFrame->LogIt(cstr);
	}
}

void MyFrame::OnKeyPad(wxCommandEvent &event)
{
    char cstr[128];
	int i;
    wxString  Message;
	wxString  Extension;
	wxListItem info;
	
#if defined(__UNICODE__)
    wchar_t   digit;
#else
    char      digit;
#endif
    int       OTNo;
    wxString  Nodestr = wxGetApp().DefaultAccount;

    if(Nodestr.IsEmpty()) {
        return;
    }
    int n = Nodestr.First(' ');
    if (n > 0)
    {
        Nodestr.Truncate(n);
    }
    if (n == 0) return;

    OTNo  = event.GetId() - XRCID("KP0");
    digit = '0' + (char) OTNo;

    if(OTNo == 10)
        digit = '*';

    if(OTNo == 11)
        digit = '#';
			
	i=iaxc_selected_call();
	
	if(!Calls->CallInfo[i].active)
	{
		if ( m_sound_deny->IsOk() ) m_sound_deny->Play(wxSOUND_ASYNC);
		SetStatusText(_T("No Active Call Selected."));
		return;
	}
	
	//info.m_itemId = i;
	//info.m_col = 2;
	//info.m_mask = wxLIST_MASK_TEXT;
	//Calls->GetItem(info);
	//Extension=info.GetText();
	Extension=Calls->CallInfo[i].Remote;
	//Extension=_T("2260"));
	sprintf(cstr, "D %s 0 1 %c", Extension.c_str(), digit);
	//sprintf(cstr, "D %s 0 1 %c", (const char *) Nodestr.mb_str(wxConvUTF8), digit);
    //iaxc_send_dtmf(digit);
	//iaxc_send_text(cstr);
	iaxc_send_text_call(i,cstr);
	wxString SM;
    SM.Printf(_T("DTMF Pad %c to %s"),digit,Extension.c_str()) ;
    SetStatusText(SM);

//    Extension->SetValue(Extension->GetValue() + digit);
}

void MyFrame::OnDialDirect(wxCommandEvent &event)
{
    wxString  DialString = wxGetApp().DefaultAccount;
	int selected;
	wxString msg;

    if(DialString.IsEmpty()) {
        return;												 
    }
	
	if(1==1)
	{
		
		selected=wxGetApp().theFrame->Calls->UserSelected;
		if(DebugLevel){
			msg.Printf(_T("Selected to Dial %i"), selected);
			LogIt(msg);
		}
		
		if(wxGetApp().theFrame->Calls->CallInfo[selected].active)
		{
			if ( m_sound_deny->IsOk() ) m_sound_deny->Play(wxSOUND_ASYNC);
			if(DebugLevel){
				msg.Printf(_T("Selected is Active ABORT"));
				LogIt(msg);
			}
			return;
		}
	}
	
	iaxc_select_call(selected);
			
    int n = DialString.First(' ');
    if (n > 0)
    {
		DialString.Truncate(n);
    }
    if (n == 0) return;

    Dial(DialString);
	iaxc_set_silence_threshold(0);
}

#if 0
void MyFrame::OnTransfer(wxCommandEvent &event)
{
    //Transfer
    int      selected = iaxc_selected_call();
    char     ext[256];
    wxString Title;
#if defined(__UNICODE__)
    wxMBConvUTF8 utf8;
#endif

    Title.Printf(_T("Transfer Call %d"), selected);
    wxTextEntryDialog dialog(this,
                             _T("Target Extension:"),
                             Title,
                             _T(""),
                             wxOK | wxCANCEL);

    if(dialog.ShowModal() != wxID_CANCEL) {

#if defined(__UNICODE__)
        utf8.WC2MB(ext, dialog.GetValue().c_str(), 256);
#else
        strncpy(ext, dialog.GetValue().c_str(), 256);
#endif

        iaxc_blind_transfer_call(selected, ext);
    }
}
#endif

void MyFrame::OnAccountChoice(wxCommandEvent &event)
{
	wxString  str;

	str=Account->GetStringSelection();

	// str = str.Mid(1,4);
	str = str.AfterFirst('<');
	str = str.BeforeFirst('>');
    wxGetApp().DefaultAccount = str;
}

void MyTimer::Notify()
{
    wxGetApp().theFrame->OnNotify();
}

void MyFrame::OnSetFocus(wxFocusEvent& event)
{
    if (CueTones && m_sound_2up->IsOk())
       	m_sound_2up->Play(wxSOUND_ASYNC);

	SetStatusText( "OnSetFocus." );
}


void MyFrame::OnKillFocus(wxFocusEvent& event)
{
    if (CueTones&& m_sound_2dn->IsOk())
       	m_sound_2dn->Play(wxSOUND_ASYNC);

	SetStatusText( "OnKillFocus." );
}

void MyFrame::OnChar(wxKeyEvent& event)
{
	event.Skip();
}

/*
	KeyPress is 0-9 and 10 and 11 for *
*/
void MyFrame::KeypadPress(int KeyPress)
{
    char cstr[128];
    wxString  Message;
#if defined(__UNICODE__)
    wchar_t   digit;
#else 
    char      digit;
#endif
    int       OTNo;
    wxString  Nodestr = wxGetApp().DefaultAccount;

    if(Nodestr.IsEmpty()) {
        return;
    }
    int n = Nodestr.First(' ');
    if (n > 0)
    {
        Nodestr.Truncate(n);
    }
    if (n == 0) return;

    OTNo  = KeyPress;
    digit = '0' + (char) OTNo;

    if(OTNo == 10)
        digit = '*';

    if(OTNo == 11)
        digit = '#';
		

		
    sprintf(cstr, "D %s 0 1 %c", (const char *) Nodestr.mb_str(wxConvUTF8), digit);
    //iaxc_send_dtmf(digit);
    iaxc_send_text(cstr);
// /hack
    wxString SM;
    SM.Printf(_T("DTMF %c"), digit);
    SetStatusText(SM);

//    Extension->SetValue(Extension->GetValue() + digit);
}

void MyFrame::OnSimKey(wxCommandEvent& event)
{
	int       OTNo;
	wxString  SM;

	OTNo = event.GetId() - XRCID("k0");
	//SM.Printf(_T("DKEY %d"), OTNo);
    //SetStatusText(SM);

	switch (OTNo)
	{
		case 0: if (m_sound_d0->IsOk())m_sound_d0->Play(wxSOUND_ASYNC); break;
		case 1: if (m_sound_d1->IsOk())m_sound_d1->Play(wxSOUND_ASYNC); break;
		case 2: if (m_sound_d2->IsOk())m_sound_d2->Play(wxSOUND_ASYNC); break;
		case 3: if (m_sound_d3->IsOk())m_sound_d3->Play(wxSOUND_ASYNC); break;
		case 4: if (m_sound_d4->IsOk())m_sound_d4->Play(wxSOUND_ASYNC); break;
		case 5: if (m_sound_d5->IsOk())m_sound_d5->Play(wxSOUND_ASYNC); break;
		case 6: if (m_sound_d6->IsOk())m_sound_d6->Play(wxSOUND_ASYNC); break;
		case 7: if (m_sound_d7->IsOk())m_sound_d7->Play(wxSOUND_ASYNC); break;
		case 8: if (m_sound_d8->IsOk())m_sound_d8->Play(wxSOUND_ASYNC); break;
		case 9: if (m_sound_d9->IsOk())m_sound_d9->Play(wxSOUND_ASYNC); break;
		case 10: if (m_sound_d10->IsOk())m_sound_d10->Play(wxSOUND_ASYNC); break;
		case 11: if (m_sound_d11->IsOk())m_sound_d11->Play(wxSOUND_ASYNC); break;
		default: wxBell(); break;
	}

	KeypadPress(OTNo);
}


