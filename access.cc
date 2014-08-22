//----------------------------------------------------------------------------------------
// Name:        access.cc
// Purpose:     dialog box manage access
// Author:      Steven Henke sph@xelatec.com
// Modified by:
// Created:     2008
// Copyright:   (C)XELATEC,LLC http://www.xelatec.com
// Licence:     GPL
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// GCC implementation
//----------------------------------------------------------------------------------------

#if defined(__GNUG__) && ! defined(__APPLE__)
    #pragma implementation "access.h"
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

#include "access.h"  

//----------------------------------------------------------------------------------------
// Remaining headers
// ---------------------------------------------------------------------------------------

#include "app.h"
#include "frame.h"
#include "main.h"

//----------------------------------------------------------------------------------------
// Event table: connect the events to the handler functions to process them
//----------------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(AccessDialog, wxDialog)
    EVT_BUTTON(XRCID("wxID_UPDATE"),     AccessDialog::OnUpdate)
	EVT_BUTTON(XRCID("wxID_OK"),         AccessDialog::OnOK)
END_EVENT_TABLE()

//----------------------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------------------

AccessDialog::AccessDialog( wxWindow* parent )
{
    wxConfig  *config = theApp::getConfig();
    wxXmlResource::Get()->LoadDialog(this, parent, wxT("Access"));

    //----Reach in for our controls-----------------------------------------------------
    Password     		= XRCCTRL(*this, "Password",     wxTextCtrl);
    Confirm      		= XRCCTRL(*this, "Confirm",      wxTextCtrl);
	UpdateButton      	= XRCCTRL(*this, "wxID_UPDATE",  wxButton);
	BlockLabel			= XRCCTRL(*this, "wxID_LABEL",   wxStaticText);

	config->SetPath(_T("/Access"));
	strPassword = config->Read(_T("Password"), _T(""));

	//Password->SetValue(strPassword);

	if(wxGetApp().AccessLevel)
	{
		Confirm->Enable(1);
		UpdateButton->Enable(1); 
		BlockLabel->SetLabel(_("Access Enabled. You may change the password."));
	}
	else
	{
		Confirm->Enable(0);
		UpdateButton->Enable(0);
		BlockLabel->SetLabel(_("Enter the Access Password."));
	} 
}

//----------------------------------------------------------------------------------------
// Private methods
//----------------------------------------------------------------------------------------

void AccessDialog::OnOK(wxCommandEvent &event)
{
	if(!Password->GetValue().IsSameAs(strPassword)) {
        wxMessageBox(_("Try Again"),
                     _("Password Mismatch"),
                       wxICON_INFORMATION);
        return;
    }
	else
	{
		wxGetApp().AccessLevel=1;
		Close(TRUE);
	}
}

void AccessDialog::OnUpdate(wxCommandEvent &event)
{
#if defined(__UNICODE__)
    wxMBConvUTF8 utf8;
#endif
    wxConfig  *config = theApp::getConfig();

    if(!Password->GetValue().IsSameAs(Confirm->GetValue())) {
        wxMessageBox(_("Try Again"),
                     _("Password Mismatch"),
                       wxICON_INFORMATION);
        return;
    }
    config->SetPath(_T("/Access"));
    config->Write(_T("Password"), Password->GetValue());
	wxGetApp().AccessLevel=1;
	wxMessageBox(_("Entries Match."),
                     _("Password Changed."),
                       wxICON_INFORMATION);
    Close(TRUE);
}
/*	end of file		*/
