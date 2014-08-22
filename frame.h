//----------------------------------------------------------------------------------------
// Name:        frame,h
// Purpose:     Describes main dialog
// Author:      Michael Van Donselaar
// Modified by:
// Created:     2003
// Copyright:   (c) Michael Van Donselaar ( michael@vandonselaar.org )
// Licence:     GPL
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Begin single inclusion of this .h file condition
//----------------------------------------------------------------------------------------

#ifndef _FRAME_H_
#define _FRAME_H_

//----------------------------------------------------------------------------------------
// GCC interface
//----------------------------------------------------------------------------------------

#if defined(__GNUG__) && ! defined(__APPLE__)
    #pragma interface "frame.h"
#endif

//----------------------------------------------------------------------------------------
// Headers
//----------------------------------------------------------------------------------------

#include "app.h"
#include "calls.h"
#include "directory.h"
#include "accounts.h"
#include "access.h"
#include "ringer.h"
#include "wx/menu.h"
#include "wx/html/helpctrl.h"
#include "wx/sound.h"
#include "wx/joystick.h"

#define LEVEL_MAX -40
#define LEVEL_MIN -50
#define DEFAULT_SILENCE_THRESHOLD 1 // positive is "auto"

#define AST_CONTROL_KEY         12        /* Key Radio */
#define AST_CONTROL_UNKEY       13        /* Unkey Radio */

//----------------------------------------------------------------------------------------
// Class definition: MyTimer
//----------------------------------------------------------------------------------------

class MyTimer : public wxTimer
{
public:

    void        Notify();
};

//----------------------------------------------------------------------------------------
// Class definition: MyFrame
//----------------------------------------------------------------------------------------

class MyFrame : public wxFrame
{

public:

    MyFrame( wxWindow* parent=(wxWindow *)NULL);
   ~MyFrame();

    void        ShowDirectoryControls();
    void        OnSize(wxSizeEvent &event);
    void        OnShow();
    void        OnNotify();
    void        OnHangup(wxCommandEvent &event);
    void        OnHoldKey(wxCommandEvent &event);
    void        OnSpeakerKey(wxCommandEvent &event);
    void        OnQuit(wxEvent &event);
    void        OnPTTChange(wxCommandEvent &event);
    void        OnSilenceChange(wxCommandEvent &event);
    void        OnInputSlider(wxScrollEvent &event);
    void        OnOutputSlider(wxScrollEvent &event);
    bool        GetPTTState();
    void        CheckPTT();
    void        SetPTT(bool state);
    void        RePanel(wxString Name);
    void        ApplyCodecs(void);
    void        ApplyFilters(void);
	void        OnPttButton(wxCommandEvent &event);
	void 		OnMonitorButton(wxCommandEvent &event);
	void 		LogIt(wxString msg);

    // Handlers for library-initiated events
    void        HandleEvent(wxCommandEvent &evt);
    int         HandleIAXEvent(iaxc_event *e);
    int         HandleStatusEvent(char *msg);
    int         HandleLevelEvent(float input, float output);

	MyTimer    *maintimer;
    wxGauge    *Input;
    wxGauge    *Output;
    wxSlider   *InputSlider;
    wxSlider   *OutputSlider;
    wxChoice   *Account;
    wxComboBox *Extension;
    CallList   *Calls;
	wxButton   *HoldButton;
	wxButton   *PttButton;
	wxButton   *HangupButton;
	wxListBox  *LogList;

	wxTextCtrl *OnTheAir;
	wxColour	FrameBgColour;

    wxHtmlHelpController* help;

    wxString    IntercomPass;

	int         InputSourceIndex;

	int			PriorityCall;
	int			CallScanHeld;
 	int	        ScanaTicks;
	int			ScanbTicks;
	int			BonkTicks;
	int			MacroTicks;
	
	int			MacroCount;
	
	int			HostInformTicks;
	int         DestInformTicks;

	bool        CallScan;
	bool		CallScanPriority;

	bool        ClientPtt;		/* net ptt state on this iaxrpt client */

    bool        RingOnSpeaker;
	bool		JoyPtt;
	bool		PanelPtt;
	bool		PanelMonitor;
	bool		CueTones;

    bool        UsingSpeaker;
    bool        AGC;
    bool        AAGC;
    bool        CN;
    bool        NoiseReduce;
    bool        EchoCancel;

    bool        AllowuLawVal;
    bool        AllowaLawVal;
    bool        AllowGSMVal;
    bool        AllowSpeexVal;
    bool        AllowiLBCVal;
	
	bool		PttBonk;
	bool		PttInhibit;
	bool		pttKey;
	bool		pttHID;
	
    int         PreferredBitmap;

    bool        SPXTuneVal;
    bool        SPXEnhanceVal;
    int         SPXQualityVal;
    int         SPXBitrateVal;
    int         SPXABRVal;
    bool        SPXVBRVal;
    int         SPXComplexityVal;

	int			DebugLevel;
	int 		MicLevel;
	bool	    MicBoost;
	
	int			numStartupLinks;
	wxString	strStartupLinks[32];
	
	int			numPermaLinks;
	wxString	strPermaLinks[32];



private:

    // An icon for the corner of dialog and application's taskbar button
    wxIcon      m_icon;
    wxPanel    *aPanel;
    int         MessageTicks;

	wxSound		*m_sound_2up;
	wxSound		*m_sound_2dn;
	wxSound		*m_sound_ptt;
	wxSound     *m_sound_deny;

	wxSound		*m_sound_d0;
	wxSound		*m_sound_d1;
	wxSound		*m_sound_d2;
	wxSound		*m_sound_d3;
	wxSound		*m_sound_d4;
	wxSound		*m_sound_d5;
	wxSound		*m_sound_d6;
	wxSound		*m_sound_d7;
	wxSound		*m_sound_d8;
	wxSound		*m_sound_d9;
	wxSound		*m_sound_d10;
	wxSound		*m_sound_d11;
	
    wxJoystick	*m_stick;

    void        OnPrefs(wxCommandEvent& event);
	void        OnAccess(wxCommandEvent& event);
	int         AccessCheck(void);
    void        OnAccounts(wxCommandEvent& event);
    void        OnDevices(wxCommandEvent& event);
    void        OnDirectory(wxCommandEvent& event);
    void        OnExit(wxCommandEvent& event);
    void        OnAbout(wxCommandEvent& event);
    void        OnHelp(wxCommandEvent& event);
	void        OnSupport(wxCommandEvent& event);
    void        OnOneTouch(wxCommandEvent& event);
    void        OnKeyPad(wxCommandEvent& event);
    void        OnDialDirect(wxCommandEvent& event);
    void        OnTransfer(wxCommandEvent& event);
    void        AddPanel(wxWindow *parent, wxString Name);
    void        OnAccountChoice(wxCommandEvent &event);

	void 		OnChar(wxKeyEvent &event);
	void 		OnSetFocus(wxFocusEvent &event);
	void 		OnKillFocus(wxFocusEvent &event);

	void		KeypadPress(int KeyPress);
	void        OnSimKey(wxCommandEvent& event);

#ifdef __WXGTK__
    GdkWindow *keyStateWindow;
#endif

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------------------
// End single inclusion of this .h file condition
//----------------------------------------------------------------------------------------

#endif  // _FRAME_H_
