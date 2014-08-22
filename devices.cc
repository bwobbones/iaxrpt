//----------------------------------------------------------------------------------------
// Name:        devices.cc
// Purpose:     devices dialog
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
    #pragma implementation "devices.h"
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
#include "devices.h"

//----------------------------------------------------------------------------------------
// Remaining headers
// ---------------------------------------------------------------------------------------

#include "app.h"
#include "frame.h"
#include "main.h"

//----------------------------------------------------------------------------------------
// Event table: connect the events to the handler functions to process them
//----------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(DevicesDialog, wxDialog)
    EVT_CHOICE(  XRCID("InputDevice"),        DevicesDialog::OnDirty)
	EVT_CHOICE(  XRCID("InputSource"),        DevicesDialog::OnDirty)
    EVT_CHOICE(  XRCID("OutputDevice"),       DevicesDialog::OnDirty)
    EVT_CHOICE(  XRCID("SpeakerInputDevice"), DevicesDialog::OnDirty)
    EVT_CHOICE(  XRCID("SpeakerOutputDevice"),DevicesDialog::OnDirty)
    EVT_CHOICE(  XRCID("RingDevice"),         DevicesDialog::OnDirty)
    EVT_CHECKBOX(XRCID("RingOnSpeaker"),      DevicesDialog::OnDirty)
	EVT_CHECKBOX(XRCID("CueTones"),           DevicesDialog::OnDirty)
	EVT_CHECKBOX(XRCID("JoyPtt"),             DevicesDialog::OnDirty)
    EVT_BUTTON(  wxID_SAVE,                   DevicesDialog::OnSave)
    EVT_BUTTON(  wxID_APPLY,                  DevicesDialog::OnApply)
	EVT_COMMAND_SCROLL(XRCID("MicSlider"),    DevicesDialog::OnMicSlider)
	EVT_CHECKBOX(XRCID("MicBoost"),           DevicesDialog::OnDirty)
END_EVENT_TABLE()

//----------------------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------------------

DevicesDialog::DevicesDialog(wxWindow* parent)
{    
    wxXmlResource::Get()->LoadDialog(this, parent, wxT("Devices"));

    // Reach in for our controls

    InputDevice     = XRCCTRL(*this, "InputDevice",         wxChoice);
	InputSource     = XRCCTRL(*this, "InputSource",         wxChoice);
    OutputDevice    = XRCCTRL(*this, "OutputDevice",        wxChoice);
    SpkInputDevice  = XRCCTRL(*this, "SpeakerInputDevice",  wxChoice);
    SpkOutputDevice = XRCCTRL(*this, "SpeakerOutputDevice", wxChoice);
    RingDevice      = XRCCTRL(*this, "RingDevice",          wxChoice);
    RingOnSpeaker   = XRCCTRL(*this, "RingOnSpeaker",       wxCheckBox);
	CueTones        = XRCCTRL(*this, "CueTones",            wxCheckBox);
	JoyPtt          = XRCCTRL(*this, "JoyPtt",              wxCheckBox);

	MicSlider       = XRCCTRL(*this, "MicSlider",           wxSlider);
	MicBoost        = XRCCTRL(*this, "MicBoost",            wxCheckBox);

    SaveButton      = XRCCTRL(*this, "wxID_SAVE",           wxButton);
    ApplyButton     = XRCCTRL(*this, "wxID_APPLY",          wxButton);
    CancelButton    = XRCCTRL(*this, "wxID_CANCEL",         wxButton);

    GetAudioDevices();

    if(RingOnSpeaker)
        RingOnSpeaker->SetValue(wxGetApp().theFrame->RingOnSpeaker);

    CueTones->SetValue(wxGetApp().theFrame->CueTones);

    JoyPtt->SetValue(wxGetApp().theFrame->JoyPtt);

	MicSlider->SetRange(0,100);
	MicSlider->SetValue(wxGetApp().theFrame->MicLevel);
    MicBoost->SetValue(wxGetApp().theFrame->MicBoost);

	InputSource->SetSelection(wxGetApp().theFrame->InputSourceIndex);
}

//----------------------------------------------------------------------------------------
// Private methods
//----------------------------------------------------------------------------------------

void DevicesDialog::GetAudioDevices()
{
    struct iaxc_audio_device *devices;
    int               nDevs;
    int               input, output, ring,source;
    int               i;
    long              caps;
    wxString          devname;
#if defined(__UNICODE__)
    wchar_t           wdevname[256];
    wxMBConvUTF8      utf8;
#endif

    iaxc_audio_devices_get(&devices, &nDevs, &input, &output, &ring);

    for(i=0; i<nDevs; i++) {
        caps =    devices->capabilities;
#if defined(__UNICODE__)
        utf8.MB2WC(wdevname, devices->name, 256);
        devname = wdevname;
#else
        devname = devices->name;
#endif

        if(caps & IAXC_AD_INPUT) {
            InputDevice->Append(devname);
            SpkInputDevice->Append(devname);
        }

        if(caps & IAXC_AD_OUTPUT) {
            OutputDevice->Append(devname);
            SpkOutputDevice->Append(devname);
        }

        if(caps & IAXC_AD_RING)
            RingDevice->Append(devname);

        if(i == input) {
            InputDevice->SetStringSelection(devname);
            wxGetApp().InputDevice = devname;
        }

        if(i == output) {
            OutputDevice->SetStringSelection(devname);
            wxGetApp().OutputDevice = devname;
        }

        if(i == ring)
            RingDevice->SetStringSelection(devname);

        devices++;
    }

    // Set selections based on saved config (even though they're not
    // validated).  Invalid input to SetAudioDevices selects defaults

    devname = wxGetApp().SpkInputDevice;
    if(devname) {
        SpkInputDevice->SetSelection(SpkInputDevice->FindString(devname));
    }

    devname = wxGetApp().SpkOutputDevice;
    if(devname) {
        SpkOutputDevice->SetSelection(SpkOutputDevice->FindString(devname));
    }

}

void DevicesDialog::OnSave(wxCommandEvent &event)
{
    //First, apply the changes
    OnApply(event);

    wxConfig *config = theApp::getConfig();

    config->SetPath(_T("/Prefs"));

    config->Write(_T("Input Device"),          wxGetApp().InputDevice);
	config->Write(_T("Input Source"),          wxGetApp().InputSource);
    config->Write(_T("Output Device"),         wxGetApp().OutputDevice);
    config->Write(_T("Speaker Input Device"),  wxGetApp().SpkInputDevice);
    config->Write(_T("Speaker Output Device"), wxGetApp().SpkOutputDevice);
    config->Write(_T("Ring Device"),           wxGetApp().RingDevice);
    config->Write(_T("RingOnSpeaker"),         wxGetApp().theFrame->RingOnSpeaker);
	config->Write(_T("JoyPtt"),                wxGetApp().theFrame->JoyPtt);
	config->Write(_T("CueTones"),              wxGetApp().theFrame->CueTones);
	config->Write(_T("MicLevel"),              wxGetApp().theFrame->MicLevel);
	config->Write(_T("MicBoost"),              wxGetApp().theFrame->MicBoost);

    delete config;
    SaveButton->Disable();
}

void DevicesDialog::OnApply(wxCommandEvent &event)
{
    // First, save to Main
    wxGetApp().InputDevice     = InputDevice->GetStringSelection();
	wxGetApp().InputSource     = InputSource->GetStringSelection();
    wxGetApp().OutputDevice    = OutputDevice->GetStringSelection();
    wxGetApp().SpkInputDevice  = SpkInputDevice->GetStringSelection();
    wxGetApp().SpkOutputDevice = SpkOutputDevice->GetStringSelection();
    wxGetApp().RingDevice      = RingDevice->GetStringSelection();
   
    if(RingOnSpeaker)
        wxGetApp().theFrame->RingOnSpeaker = RingOnSpeaker->GetValue();
	if(CueTones)
        wxGetApp().theFrame->CueTones = CueTones->GetValue();
	if(JoyPtt)
        wxGetApp().theFrame->JoyPtt = JoyPtt->GetValue();

	wxGetApp().theFrame->InputSourceIndex = InputSource->GetSelection();

	wxGetApp().theFrame->MicLevel = MicSlider->GetValue(); 
	wxGetApp().theFrame->MicBoost = MicBoost->GetValue();
																
    // Apply the changes
    SetAudioDevices(wxGetApp().InputDevice,
                    wxGetApp().OutputDevice,
                    wxGetApp().RingDevice,
                    wxGetApp().theFrame->InputSourceIndex);

    ApplyButton->Disable();
    CancelButton->SetLabel(_("Done"));
}

void DevicesDialog::OnDirtyScroll(wxScrollEvent &event)
{
    SaveButton->Enable();
    ApplyButton->Enable();
    CancelButton->SetLabel(_("Cancel"));
}

void DevicesDialog::OnDirty(wxCommandEvent &event)
{
    SaveButton->Enable();
    ApplyButton->Enable();
    CancelButton->SetLabel(_("Cancel"));
}

void SetAudioDevices(wxString inname, wxString outname, wxString ringname, int inputsource)
{
    struct iaxc_audio_device *devices;
    int                      nDevs;
    int                      i;
    int                      input  = 0;
    int                      output = 0;
    int                      ring   = 0;
	int		    			 dummy  = 0;

#if defined(__UNICODE__)
    wchar_t                  wdevname[256];
    wxMBConvUTF8             utf8;
#endif

    // Note that if we're called with an invalid devicename, the deviceID
    // stays 0, which equals default.

    iaxc_audio_devices_get(&devices, &nDevs, &input, &output, &ring);

    for(i=0; i<nDevs; i++) {
        if(devices->capabilities & IAXC_AD_INPUT) {
#if defined(__UNICODE__)
            utf8.MB2WC(wdevname, devices->name, 256);
            if(inname.Cmp(wdevname) == 0)
#else
            if(inname.Cmp(devices->name) == 0)
#endif
                input = devices->devID;
        }

        if(devices->capabilities & IAXC_AD_OUTPUT) {
#if defined(__UNICODE__)
            utf8.MB2WC(wdevname, devices->name, 256);
            if(outname.Cmp(wdevname) == 0)
#else
            if(outname.Cmp(devices->name) == 0)
#endif
                output = devices->devID;
        }

        if(devices->capabilities & IAXC_AD_RING) {
#if defined(__UNICODE__)
            utf8.MB2WC(wdevname, devices->name, 256);
            if(ringname.Cmp(wdevname) == 0)
#else
            if(ringname.Cmp(devices->name) == 0)
#endif
                ring = devices->devID;
        }
        devices++;
    }
    i=iaxc_audio_devices_set(input,output,ring);
	//iaxc_input_source_set(inputsource);

	iaxc_mic_boost_set(wxGetApp().theFrame->MicBoost);

	if(wxGetApp().theFrame->DebugLevel)
	{
		wxString lmsg;
		lmsg.Printf(_T("iaxc_audio_devices_set=%i"),i);
		wxGetApp().theFrame->LogIt(lmsg);
	}
}


void DevicesDialog::OnMicSlider(wxScrollEvent &event)
{
	wxGetApp().theFrame->MicLevel = MicSlider->GetValue();
	iaxc_input_level_set(wxGetApp().theFrame->MicLevel/100.0);
	SaveButton->Enable();
    ApplyButton->Enable();
    CancelButton->SetLabel(_("Cancel"));
}



