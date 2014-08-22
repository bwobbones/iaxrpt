//----------------------------------------------------------------------------------------
// Name:        devices,h
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

#ifndef _DEVICES_H_
#define _DEVICES_H_

//----------------------------------------------------------------------------------------
// GCC interface
//----------------------------------------------------------------------------------------

#if defined(__GNUG__) && ! defined(__APPLE__)
    #pragma interface "devices.h"
#endif

//----------------------------------------------------------------------------------------
// Headers
//----------------------------------------------------------------------------------------

#include "app.h"

void         SetAudioDevices(wxString in, wxString out, wxString ring, int inputsource);

class DevicesDialog : public wxDialog
{
public: 
    DevicesDialog( wxWindow* parent );
        
private:

    wxChoice    *InputDevice;
	wxChoice    *InputSource;
    wxChoice    *OutputDevice;
    wxChoice    *SpkInputDevice;
    wxChoice    *SpkOutputDevice;
    wxChoice    *RingDevice;
    wxCheckBox  *RingOnSpeaker;
	wxCheckBox  *CueTones;
	wxCheckBox  *JoyPtt;
	wxCheckBox  *MicBoost;
	wxSlider	*MicSlider;

    wxButton    *SaveButton;
    wxButton    *ApplyButton;
    wxButton    *CancelButton;

    void         GetAudioDevices();
    void         OnSave(wxCommandEvent &event);
    void         OnApply(wxCommandEvent &event);
    void         OnDirty(wxCommandEvent &event);
	void         OnDirtyScroll(wxScrollEvent &event);
	void		 OnMicSlider(wxScrollEvent &event);

    DECLARE_EVENT_TABLE()

};

#endif  //_DEVICES_H_
