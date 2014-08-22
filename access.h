//----------------------------------------------------------------------------------------
// Name:        access.h
// Purpose:     Describes access dialog
// Author:      Steven Henke sph@xelatec.com
// Modified by:
// Created:     2008
// Copyright:   (C)XELATEC,LLC http://www.xelatec.com
// Licence:     GPL
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Begin single inclusion of this .h file condition
//----------------------------------------------------------------------------------------

#ifndef _ACCESS_H_
#define _ACCESS_H_

//----------------------------------------------------------------------------------------
// GCC interface
//----------------------------------------------------------------------------------------

#if defined(__GNUG__) && ! defined(__APPLE__)
    #pragma interface "access.h"
#endif

//----------------------------------------------------------------------------------------
// Headers
//----------------------------------------------------------------------------------------

#include "app.h"

class AccessDialog : public wxDialog
{
public: 
    AccessDialog( wxWindow* parent );
	wxTextCtrl   *Password;
    wxTextCtrl   *Confirm;

private:
	wxStaticText  *BlockLabel;
	wxButton     *UpdateButton;
	wxString	  strPassword;
    void          Show(void);
    void         OnUpdate(wxCommandEvent &event);
	void         OnOK(wxCommandEvent &event);
    DECLARE_EVENT_TABLE()
};
#endif  //_ACCESS_H_

