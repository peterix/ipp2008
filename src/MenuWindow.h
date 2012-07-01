/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file MenuWindow.h
 * 
 * Okno s menu.
 * 
 * Obsahuje hlavní okno s tlačítky pro jednotlivé funkce aplikace.
 * 
 * Přihlášení, lokální hra, přehrávání záznamů, nápověda.
 * 
 * Po přihlášení k serveru se zneaktivní přihlašovací tlačítko.
 *
 * @version $Rev: 158 $
 ******************************************************************************/

// -*- C++ -*- generated by wxGlade 0.6.3 on Wed Apr 16 18:38:49 2008

#include <wx/wx.h>
#include <wx/image.h>

#ifndef MENUWINDOW_H
#define MENUWINDOW_H

// begin wxGlade: ::dependencies
// end wxGlade

// begin wxGlade: ::extracode

// end wxGlade


class MenuWindow: public wxFrame {
public:
    // begin wxGlade: MenuWindow::ids
    // end wxGlade

    MenuWindow(wxWindow* parent, int id, const wxString& title, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);

private:
    // begin wxGlade: MenuWindow::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: MenuWindow::attributes
    wxStaticBitmap* bitmapLogo;
    wxButton* buttonLogin;
    wxButton* buttonLocal;
    wxButton* buttonReplay;
    wxButton* buttonHelp;
    wxButton* buttonAbout;
    wxButton* buttonQuit;
    // end wxGlade

    DECLARE_EVENT_TABLE();

public:
    virtual void clickLogin(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void clickLocal(wxCommandEvent &event); 
    virtual void clickReplay(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void clickHelp(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void clickAbout(wxCommandEvent &event);
    virtual void clickQuit(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void onCloseWindow(wxCloseEvent &event);
    
    void enableLogin();
    void disableLogin();
   
}; // wxGlade: end class


#endif // MENUWINDOW_H
