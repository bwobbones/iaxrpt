//----------------------------------------------------------------------------------------
// Name:        calls.cc
// Purpose:     Call appearances listctrl
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
    #pragma implementation "calls.h"
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

#include "calls.h"  

//----------------------------------------------------------------------------------------
// Remaining headers
// ---------------------------------------------------------------------------------------

#include "app.h"
#include "main.h"
#include "prefs.h"
#include "frame.h"
#include "devices.h"
#include <wx/ffile.h>
#include <wx/textdlg.h>
#include "wx/tokenzr.h"

//----------------------------------------------------------------------------------------
// Event table: connect the events to the handler functions to process them
//----------------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CallList, wxListCtrl)
    EVT_SIZE                 (                CallList::OnSize)  
    EVT_LIST_ITEM_SELECTED   (XRCID("Calls"), CallList::OnSelect)
    EVT_LIST_ITEM_RIGHT_CLICK(XRCID("Calls"), CallList::OnRClick)
    EVT_LIST_ITEM_ACTIVATED  (XRCID("Calls"), CallList::OnDClick)
END_EVENT_TABLE()

//----------------------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------------------

CallList::CallList(wxWindow *parent, int nCalls, wxWindowID id, const wxPoint& pos, 
                    const wxSize& size, long style)
                  : wxListCtrl( parent, id, pos, size, style)
{
    wxConfig   *config = theApp::getConfig();
    long        i;
    wxListItem  item;

    m_parent = parent;

    config->SetPath(_T("/Prefs"));

    // Column Headings
    InsertColumn( 0, _(""),       wxLIST_FORMAT_LEFT,(40));
    InsertColumn( 1, _("State"),  wxLIST_FORMAT_LEFT,(72));
    InsertColumn( 2, _("Node"),   wxLIST_FORMAT_LEFT,(160));
	InsertColumn( 3, _("Connected Nodes"), wxLIST_FORMAT_LEFT,(512));	
    Hide();

#if !defined(__UNICODE__)
    wxFont font   = GetFont();
    font.SetPointSize(11);
    font.SetFamily(wxSWISS);
    SetFont(font);
#endif

    for(i=0;i<nCalls;i++) {
        InsertItem(i,wxString::Format(_T("%ld"), i + 1), 0);
		item.m_itemId=i;
        item.m_mask = wxLIST_MASK_WIDTH|wxLIST_MASK_FORMAT;
        SetItem(item);
		SetItem(i, 1, _T(""));
        SetItem(i, 2, _T(""));
		SetItem(i, 3, _T(""));
		CallInfo[i].active=0;
		CallInfo[i].rxkey=0;
		CallInfo[i].txkey=0;
		CallInfo[i].newkey=0;
		CallInfo[i].iaxkey=0;
		CallInfo[i].iaxkeysent=0;
		CallInfo[i].NewkeyTimer=0;
    }

    SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    ScanRequest=0;
	ScanSelectedPreScan=0;
	UserSelected=0;
	iaxc_select_call(0);
		  
	ItemBgColour = GetItemBackgroundColour(0);
		  
    Refresh();
    Show();
    //AutoSize();
}

int CallList::HandleConnectedNodes(wxString nodelist,int callnum)
{
	int			hit,i,count,washit;
	
	wxString dest,outstring;
	wxArrayString	ArrayNodes;
	
	wxStringTokenizer tkz(nodelist, wxT(","));
	washit=0;
	while ( tkz.HasMoreTokens() )
	{
		wxString token = tkz.GetNextToken();
		ArrayNodes.Add(token,1);
		//dest.Printf(_T("Token: %s"), token.c_str());
		//wxGetApp().theFrame->LogIt(dest);		
	}

	ArrayNodes.Sort();
	count=ArrayNodes.GetCount();
	for(i=0;i<count;i++)
	{
		outstring=outstring+ArrayNodes.Item(i);
		if(i<(count-1))outstring=outstring+_T(",");
	}
	CallInfo[callnum].ConnectedNodesTimer=CALLS_NODE_STAT_TIMEOUT;
	
	if(CallInfo[callnum].ConnectedNodes!=outstring)
	{
		CallInfo[callnum].ConnectedNodes=outstring;
		SetItem(callnum, 3, CallInfo[callnum].ConnectedNodes);
		Refresh();		
	}
	//dest.Printf(_T("Sorted: %s "), outstring.c_str());
	//wxGetApp().theFrame->LogIt(dest);
	return 0;
}

void CallList::UpdateConnectedNodes()
{
   int i;
   for(i=0;i<wxGetApp().nCalls;i++) {
        if(CallInfo[i].ConnectedNodesTimer>0)
		{
			CallInfo[i].ConnectedNodesTimer--;
			if(CallInfo[i].ConnectedNodesTimer<=0){
				CallInfo[i].ConnectedNodes=_T("");
				SetItem(i, 3, CallInfo[i].ConnectedNodes);
				Refresh();
			}
		}
    }
}
void CallList::AutoSize()
{
    SetColumnWidth(2, GetClientSize().x - 100);
}

void CallList::OnSize(wxSizeEvent &event)
{
    event.Skip();
#ifdef __WXMSW__
    // XXX FIXME: for some reason not yet investigated, this crashes Linux-GTK (for SK, at least).
    // XXX2 This causes a crash _later_ in MacOSX -- but when compiled
    // with a debugging wx library, it causes an immediate assertion
    // failure saying column 2 is out of range.  Maybe it happens before
    // the columns are added or something.  Dunno.  But, the resize
    // later when you select a call is OK.
    //AutoSize();
#endif
}

void CallList::OnSelect(wxListEvent &event)
{			
	UserSelected = event.m_itemIndex;
	if(wxGetApp().theFrame->DebugLevel)
	{
		wxString lmsg;
		lmsg.Printf(_T("OnSelect() call=%i"), UserSelected);
		wxGetApp().theFrame->LogIt(lmsg);
	}	
    iaxc_unquelch(UserSelected);
    iaxc_select_call(UserSelected);
	/* for non-radio calls unmute audio */
	if( CallInfo[UserSelected].newkey==0 && 1==0
	  )
	{
		 /* unmute speaker and mic*/
		iaxc_set_audio_output(0);        
	    iaxc_set_silence_threshold(-99);
	}
	else
	{
		/*  mute the mic audio stream for a ptt call */
		iaxc_set_silence_threshold(0);
	}	
}

void CallList::OnDClick(wxListEvent &event)
{
	int i,nCalls;
    //Don't need to select, because single click should have done it
    iaxc_dump_call();
	
	CallInfo[UserSelected].active=0;
	CallInfo[UserSelected].rxkey=0;
	CallInfo[UserSelected].txkey=0;
	CallInfo[UserSelected].newkey=0;
	CallInfo[i].NewkeyTimer=0;

	/* 
	nCalls=wxGetApp().nCalls;
	UserSelected=UserSelected-1;
	if(UserSelected<0)
		UserSelected=wxGetApp().nCalls;
	*/
	
}

void CallList::OnRClick(wxListEvent &event)
{
	/* list connected nodes */
	wxString dest,nodelist;
	nodelist=CallInfo[event.m_itemIndex].ConnectedNodes;
	if(wxGetApp().theFrame->DebugLevel>0 && !nodelist.IsEmpty() )
	{
		dest.Printf(_T("SHOW CONNECTED NODES LIST call=%i %s"), 
			event.m_itemIndex, nodelist.c_str());
		wxGetApp().theFrame->LogIt(dest);
	}
	return;			
	/*
		this might make sense if the call is not a radio call
		maybe get call stats, toggle monitor, start recording,
	
	*/
    //Transfer
    int      rselected = event.m_itemIndex;
    char     ext[256];
    wxString Title;
#if defined(__UNICODE__)
    wxMBConvUTF8 utf8;
#endif
  
    Title.Printf(_T("Transfer Call %d"), rselected);
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

        iaxc_blind_transfer_call(rselected, ext);
    }
}

wxString CallList::GetCodec(struct iaxc_ev_call_state c)
{
	switch (c.format) {
	case (1 << 0):	return(_T("G.723.1"));	/* G.723.1 compression */
	case (1 << 1):	return(_T("GSM"));	/* GSM compression */
	case (1 << 2):	return(_T("u-law"));	/* Raw mu-law data (G.711) */
	case (1 << 3):	return(_T("a-law"));	/* Raw A-law data (G.711) */
	case (1 << 4):	return(_T("G726"));	/* ADPCM, 32kbps  */
	case (1 << 5):	return(_T("ADPCM"));	/* ADPCM IMA */
	case (1 << 6):	return(_T("SLINEAR"));	/* Raw 16-bit Signed Linear (8000 Hz) PCM */
	case (1 << 7):	return(_T("LPC10"));	/* LPC10, 180 samples/frame */
	case (1 << 8):	return(_T("G.729a"));	/* G.729a Audio */
	case (1 << 9):	return(_T("Speex"));	/* Speex Audio */
	case (1 <<10):	return(_T("iLBC"));	/* iLBC Audio */
	default:	return(_T(""));
	}
}

int CallList::HandleStateEvent(struct iaxc_ev_call_state c)
{
    wxConfig  *config = theApp::getConfig();
    wxString   str;
	wxString   dstr;
	
    long       dummy;
    bool       bCont;
    static int selectedcall = -1;
	
#if defined(__UNICODE__)
    wchar_t ws[256];
    wxMBConvUTF8 utf8;
#endif

	static int hitnum=0;

    if(c.state & IAXC_CALL_STATE_RINGING) {
      wxGetApp().theFrame->Show();
      wxGetApp().theFrame->Raise();
    }

    int i;
    int nCalls = wxGetApp().nCalls;

	if(wxGetApp().theFrame->DebugLevel)
	{
		wxString lmsg;
		lmsg.Printf(_T("iax StateEvent %i 0x%04X"), c.callNo, c.state);
		wxGetApp().theFrame->LogIt(lmsg);
	}
    //for(i=0; i<nCalls; i++)
    //    SetItem(i, 1, _T(""));

    // first, handle inactive calls
    if(!(c.state & IAXC_CALL_STATE_ACTIVE)) {
        SetItem(c.callNo, 1, _T(""));
        SetItem(c.callNo, 2, _T(""));
		SetItem(c.callNo, 3, _T(""));
        wxGetApp().RingbackTone.Stop();
        wxGetApp().IncomingRing.Stop();
        wxGetApp().CallerIDRing.Stop();
		CallInfo[c.callNo].active=0;
		CallInfo[c.callNo].rxkey=0;
		CallInfo[c.callNo].txkey=0;
		CallInfo[c.callNo].newkey=0;
		CallInfo[c.callNo].ConnectedNodesTimer=0;
		CallInfo[c.callNo].ConnectedNodes=_T("");
		CallInfo[c.callNo].NewkeyTimer=0;
		CallInfo[c.callNo].newkeysent=0;
    } else {
	
	    wxString Info;
        wxString Codec;
        wxString RemoteName;
        wxString Remote;
		wxString AccountLabel;

        bool     outgoing = c.state & IAXC_CALL_STATE_OUTGOING;
        bool     ringing  = c.state & IAXC_CALL_STATE_RINGING;
        bool     complete = c.state & IAXC_CALL_STATE_COMPLETE;
        bool     selected = c.state & IAXC_CALL_STATE_SELECTED;

		CallInfo[c.callNo].active=1;
		CallInfo[c.callNo].selected=selected;
		CallInfo[c.callNo].complete=complete;
		CallInfo[c.callNo].ringing=ringing;		

        RemoteName.Printf(_T("%s"), c.remote_name);
        Info  = RemoteName.AfterLast('@');	// Hide username:password
        Info  = Info.BeforeFirst('/');          // Remove extension in outbound call
        					// (it will be duplicated in <>)

        Remote.Printf(_T("%s"), c.remote);
		CallInfo[c.callNo].Remote=c.remote;
        if(!Remote.IsEmpty())			// Additional info in Remote
            Info += _T(" <") + Remote + _T(">");

        Codec = GetCodec(c);			// Negotiated codec
        if(!Codec.IsEmpty())
            Info += _T(" [") + GetCodec(c) + _T("]");	// Indicate Negotiated codec

		if(wxGetApp().theFrame->DebugLevel>7)
		{
			wxString msg;
			msg.Printf(_T("Call Codec = %s"), Codec.c_str());
			wxGetApp().theFrame->LogIt(msg);
		}
	    config->SetPath(_T("/Accounts"));
		AccountLabel = config->Read(Remote + _T("/AccountLabel"),_T(""));
		Info = _T(" <") + Remote + _T("> ") + AccountLabel;	
		// maw sph find description in configuration and display it
        SetItem(c.callNo, 2, Info );

        if(selected)
            selectedcall = c.callNo;

        if(outgoing) {
            if(ringing) {
                SetItem(c.callNo, 1, _T("ring out"));
                wxGetApp().RingbackTone.Start(0);
            } else {
                if(complete) {
					if(wxGetApp().theFrame->DebugLevel>0)
						wxGetApp().theFrame->LogIt(_T("calls.c complete status"));
                    // I really need to clean up this spaghetti code
                    if(selected)
                        SetItem(c.callNo, 1, _T("ACTIVE"));
						//SetItem(c.callNo, 1, msg);
                    else
                        if(c.callNo == selectedcall)
                            SetItem(c.callNo, 1, _T(""));
                        else
                            SetItem(c.callNo, 1, _T("ACTIVE"));
							//SetItem(c.callNo, 1, msg);

                    wxGetApp().RingbackTone.Stop();
                } else {
                    // not accepted yet..
                    SetItem(c.callNo, 1, _T("-?-"));
                }
            }
        } else {
		 
            if(ringing) {
				if(wxGetApp().theFrame->DebugLevel>0)
					wxGetApp().theFrame->LogIt(_T("calls.c sees a ring in"));
                SetItem(c.callNo, 1, _T("ring in"));

                // Look for the caller in our phonebook
                config->SetPath(_T("/PhoneBook"));
                bCont = config->GetFirstGroup(str, dummy);
#if defined(__UNICODE__)
                utf8.MB2WC(ws, c.remote_name, 256);
                while ( bCont ) {
                    if(str.IsSameAs(ws))
                        break;
                    bCont = config->GetNextGroup(str, dummy);
                }

                if(!str.IsSameAs(ws)) {
#else
                while ( bCont ) {
                    if(str.IsSameAs(c.remote_name))
                        break;
                    bCont = config->GetNextGroup(str, dummy);
                }

                if(!str.IsSameAs(c.remote_name)) {
#endif
                    // Add to phone book if not there already
                    str.Printf(_T("%s/Extension"), c.remote_name);
                    config->Write(str, c.remote);
					if(wxGetApp().theFrame->DebugLevel>0)
						wxGetApp().theFrame->LogIt(_T("calls.c add to phone book."));
                } else {
                    // Since they're in the phone book, look for ringtone
                    str.Printf(_T("%s/RingTone"), c.remote_name);
                    wxGetApp().CallerIDRingName = config->Read(str, _T(""));
					if(wxGetApp().theFrame->DebugLevel>0)
						wxGetApp().theFrame->LogIt(_T("calls.c they're in the phone book."));
                }

                if(strcmp(c.local_context, "intercom") == 0) {
#if defined(__UNICODE__)
                    utf8.MB2WC(ws, c.local, 256);
                    if(config->Read(_T("/Prefs/IntercomPass"), _T("s")).IsSameAs(ws)) {
#else
                    if(config->Read(_T("/Prefs/IntercomPass"), _T("s")).IsSameAs(c.local)) {
#endif
						if(wxGetApp().theFrame->DebugLevel>0)
							wxGetApp().theFrame->LogIt(_T("calls.c intercom start."));
                        wxGetApp().IntercomTone.Start(1);
                        iaxc_millisleep(1000);
                        iaxc_unquelch(c.callNo);
                        iaxc_select_call(c.callNo);

                        wxGetApp().theFrame->UsingSpeaker = true;
                        SetAudioDevices(wxGetApp().SpkInputDevice,
                                        wxGetApp().SpkOutputDevice,
										wxGetApp().RingDevice,
										wxGetApp().theFrame->InputSourceIndex);
                    }
                } else {
					if(wxGetApp().theFrame->DebugLevel>0)
						wxGetApp().theFrame->LogIt(_T("calls.c ring start"));
                    if(wxGetApp().CallerIDRingName.IsEmpty()) {
                        wxGetApp().IncomingRing.Start(1);
                    } else {
                        wxGetApp().CallerIDRing.LoadTone(wxGetApp().CallerIDRingName, 10);
                        wxGetApp().CallerIDRing.Start(1);
                    }
                }
            } else {
                wxGetApp().IncomingRing.Stop();
                wxGetApp().CallerIDRing.Stop();
                if(complete) {
                    SetItem(c.callNo, 1, _T("ACTIVE"));
                } else { 
                    // not accepted yet..  shouldn't happen!
                    SetItem(c.callNo, 1, _T("???"));
                }
            }
        } 
    }
    
	// iaxc_answer_call

    if(c.state & IAXC_CALL_STATE_KEYED) {
	  //wxString str;
	  //str.Printf(_T("BUSY"), hitnum);
	  //hitnum++;
	  if(wxGetApp().theFrame->DebugLevel)
	  {
	    dstr.Printf(_T("BUSY SET CALL=%i"),c.callNo);
		wxGetApp().theFrame->LogIt(dstr);	  
	  }
      SetItem(c.callNo, 1, _T("BUSY"));
	  CallInfo[c.callNo].rxkey=1;
	}
    else if(c.state & IAXC_CALL_STATE_ACTIVE)
	{
	  if(wxGetApp().theFrame->DebugLevel)
	  {
	    dstr.Printf(_T("BUSY CLR CALL=%i"),c.callNo);
		wxGetApp().theFrame->LogIt(dstr);	  
	  }

	  if(c.state & IAXC_CALL_STATE_COMPLETE)
	  	SetItem(c.callNo, 1, _T("OK"));	
	  else
	  	SetItem(c.callNo, 1, _T("-?-"));
		
	  if(CallInfo[c.callNo].active && !CallInfo[c.callNo].newkeysent)
	  {
	    wxString  ymsg,zmsg;
		CallInfo[c.callNo].newkeysent=1;
		CallInfo[c.callNo].NewkeyTimer=0;
		
		if(wxGetApp().theFrame->DebugLevel)
				wxGetApp().theFrame->LogIt(_T("sent !NEWKEY1! and !IAXKEY!"));	  
		
        ymsg=_T("!NEWKEY1!");
        iaxc_send_text_call(c.callNo,ymsg.c_str());			

        zmsg=_T("!IAXKEY! 1 1 0 0");
        iaxc_send_text_call(c.callNo,zmsg.c_str());			
	  }
	  
#if 1 == 0 
	  if(!CallInfo[c.callNo].iaxkeysent)
	  {
		    wxString  ymsg;
			ymsg=_T("!IAXKEY! 1 1 0 0");
			iaxc_send_text(ymsg.c_str());
			if(wxGetApp().theFrame->DebugLevel)
				wxGetApp().theFrame->LogIt(_T("sent host !IAXKEY! 1 1 0 0"));	  
	  }
#endif
	}

	//ScanCalls();

	/*
    if((c.state & IAXC_CALL_STATE_SELECTED)) 
        SetItemState(c.callNo,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
    else
        SetItemState(c.callNo,~wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);

		
	SetItemState(iaxc_selected_call(),wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
			*/
    //AutoSize();
    Refresh();
	
	ScanRequest=1;
    return 0;
}

int CallList::ScanCalls(void)
{
    int i;
    wxString dstr;
	int nCalls;
	int selected;

	ScanAutoSelect=0;
	
	if(!wxGetApp().theFrame->CallScan) return 0;
	if(wxGetApp().theFrame->ClientPtt) return 0;
	
	if ( wxGetApp().theFrame->ScanaTicks ) return 0;
	
	
	nCalls = wxGetApp().nCalls;
	selected = iaxc_selected_call();
	//wxGetApp().theFrame->LogIt(_T("ScanCalls()"));

	for(i=0; i<nCalls; i++)
	{
	  wxListItem info;
	  wxString istr;
	  
	  info.m_itemId = i;
      info.m_col = 1;
      info.m_mask = wxLIST_MASK_TEXT;
      //info.SetId(i);
	  //info.SetColumn(1);
	  GetItem(info);
	  istr=info.GetText();

	  if( istr == _T("BUSY"))
	  {
	  	//if(wxGetApp().theFrame->CallScanPriority)istr="";
		
		if(wxGetApp().theFrame->CallScanHeld<=-1)
		{
	    	wxGetApp().theFrame->ScanaTicks=32;
			wxGetApp().theFrame->CallScanHeld=i;
			
			if(selected!=i || 1==1)
			{
				//SetItemState(selected,~wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
				iaxc_select_call(i);
				SetItemBackgroundColour(i,*wxCYAN);
				//SetItemState(i,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
				//AutoSize();
				//Refresh();
			}
			wxGetApp().theFrame->ScanaTicks=32;
			if(wxGetApp().theFrame->DebugLevel)
			{
				dstr.Printf(_T("SCAN SEL=%i"),i);
				wxGetApp().theFrame->LogIt(dstr);
			}
		} 
		else if (i==wxGetApp().theFrame->CallScanHeld) 
		{
			// still busy on this link so kick the timer
			wxGetApp().theFrame->ScanaTicks=32;	
		}
		else if (	wxGetApp().theFrame->CallScanHeld >= 0 && 
					wxGetApp().theFrame->CallScanHeld != UserSelected &&
					i == UserSelected
				)
		{
			wxGetApp().theFrame->ScanaTicks=32;
			wxGetApp().theFrame->CallScanHeld=i;
			iaxc_select_call(i);
			SetItemBackgroundColour(i,*wxCYAN);
			if(wxGetApp().theFrame->DebugLevel)
			{
				dstr.Printf(_T("SCAN PRI SEL=%i"),i);
				wxGetApp().theFrame->LogIt(dstr);
			}
		}
		else if (	wxGetApp().theFrame->CallScanHeld >= 0 && 
					wxGetApp().theFrame->CallScanHeld != i &&
					wxGetApp().theFrame->CallScanHeld != UserSelected &&
					i < wxGetApp().theFrame->CallScanHeld
				)
		{
			wxGetApp().theFrame->ScanaTicks=32;
			wxGetApp().theFrame->CallScanHeld=i;
			iaxc_select_call(i);
			SetItemBackgroundColour(i,*wxCYAN);
			if(wxGetApp().theFrame->DebugLevel)
			{
				dstr.Printf(_T("SCAN PRI HI SEL=%i"),i);
				wxGetApp().theFrame->LogIt(dstr);
			}
		}
	  } 
    }
	
	for(i=0; i<nCalls; i++)
	{
		if(i != wxGetApp().theFrame->CallScanHeld)
			SetItemBackgroundColour(i,*wxWHITE);
	}
    Refresh();
	return 0;
}


int CallList::ScanClearHeld(void)
{
	int i;
	for(i=0; i<wxGetApp().nCalls; i++)
	{
		SetItemBackgroundColour(i,*wxWHITE);
	}
    Refresh();
	return 0;
}
