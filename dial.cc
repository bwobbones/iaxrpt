//----------------------------------------------------------------------------------------
// Name:        dial.cc
// Purpose:     dial class
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
    #pragma implementation "dial.h"
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

#include "dial.h"  

//----------------------------------------------------------------------------------------
// Remaining headers
// ---------------------------------------------------------------------------------------

#include "app.h"
#include "frame.h"
#include "main.h"
#include "wx/tokenzr.h"

void DialEntry(wxString& EntryName)
{
    wxString Msg;
    wxString FQIN;

    wxConfig *config = theApp::getConfig();
    wxString  val;
    wxString  KeyPath;


    KeyPath = _T("/Entries/") + EntryName;
    config->SetPath(KeyPath);

    if(!config->Exists(KeyPath)) {
        EntryName << _T(" unknown");
        wxMessageBox(KeyPath, EntryName);
        return;
    }
    wxString AccountName = config->Read(_T("Account"), _T(""));
    wxString Extension   = config->Read(_T("Extension"), _T("s"));

    Dial(AccountName + _T("/") + Extension);
}

void Dial( wxString DialStr )
{
	int rval;
    wxConfig *config = theApp::getConfig();
    wxString  FQIN;
#if defined(__UNICODE__)
    wxMBConvUTF8 utf8;
    char to[256];
#endif

	if(wxGetApp().theFrame->DebugLevel)
	{
		wxString lmsg;
		lmsg.Printf(_T("Dial(%s)\n"), DialStr.c_str());
		wxGetApp().theFrame->LogIt(lmsg);
	}

    wxString  AccountInfo = DialStr.BeforeLast('/');    // Empty   if no '/'
    wxString  Extension   = DialStr.AfterLast('/');     // dialstr if no '/'

    if(DialStr.IsEmpty())
        return;

    if(AccountInfo.IsEmpty()) {
        AccountInfo = wxGetApp().DefaultAccount;

        // Dialstr has no "/" and no default server: add default extension
        if(AccountInfo.IsEmpty()) {
            AccountInfo = Extension;
            Extension  = _T("s");
        }
    }

    wxString  RegInfo    = AccountInfo.BeforeLast('@'); // Empty   if no '@'
    wxString  Host       = AccountInfo.AfterLast('@');

    wxString  Username   = RegInfo.BeforeFirst(':');
    wxString  Password   = RegInfo.AfterFirst(':');     // Empty if no ':'


    if(RegInfo.IsEmpty()) {
        config->SetPath(_T("/Accounts/") + Host);
        Host     = config->Read(_T("Host"), Host);
        Username = config->Read(_T("Username"), _T(""));
        Password = config->Read(_T("Password"), _T(""));
    }

    FQIN.Printf(_T("%s:%s@%s/%s"),
                Username.c_str(),
                Password.c_str(),
                Host.c_str(),
                Extension.c_str());

	if(wxGetApp().theFrame->DebugLevel)
	{
		wxString lmsg;
		lmsg.Printf(_T("Dialing: %s\n"), FQIN.c_str());
		wxGetApp().theFrame->LogIt(lmsg);
	}

#if defined(__UNICODE__)
    utf8.WC2MB(to, FQIN.c_str(), 256);
    rval=iaxc_call(to);
#else
    rval=iaxc_call((char *) FQIN.c_str());
#endif

    if(wxGetApp().theFrame->DebugLevel)
	{
		wxString lmsg;
		lmsg.Printf(_T("iaxc_call=%i"),rval);
		wxGetApp().theFrame->LogIt(lmsg);
	}
}

