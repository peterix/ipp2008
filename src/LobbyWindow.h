/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file LobbyWindow.h
 * 
 * Okno se seznamem her.
 * 
 * Obsahuje seznam online hráčů, rozehraných her a chat. Je zde funkcionalita 
 * pro chatování, vyzývání hráčů ke hře a spouštění her. Údaje se periodicky 
 * obnovují.
 *
 * @version $Rev: 160 $
 ******************************************************************************/

// -*- C++ -*- generated by wxGlade 0.6.3 on Wed Apr 16 18:38:49 2008

#ifndef LOBBYWINDOW_H
#define LOBBYWINDOW_H

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include <wx/wx.h>
#include <wx/image.h>
#include "client_connect.h"
#include "ticpp.h"
#include "cl_receiver.h"

#include <wx/listctrl.h>


class LobbyWindow: public wxFrame, public Receiver
{
public:

    LobbyWindow(wxWindow* parent, int id, const wxString& title, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);
    
	/// metoda pro příjem zpráv ze sítě
	virtual int Receive(std::string zprava);
	
private:
    // begin wxGlade: LobbyWindow::methods
    void set_properties();
    void do_layout();
    // end wxGlade
    
    void refresh();
    void setPlayerList(ticpp::Element* element);
    void invite(wxString name);
    void acceptMatch(ticpp::Element* element);
    void setGamesList(ticpp::Element* element);
    
    wxString selectedPlayer; /// aktuální vybraný hráč ze seznamu
    wxString selectedGame; /// aktualní vybraná hra ze seznamu
    
    wxTimer timer; /// časovač

protected:
    // begin wxGlade: LobbyWindow::attributes
    wxListCtrl* listGames;
    wxTextCtrl* chatBox;
    wxListCtrl* listPlayers;
    wxButton* buttonInvite;
    wxButton* buttonPlay;
    wxButton* buttonRefresh;
    wxTextCtrl* textChat;
    wxButton* buttonSend;
    // end wxGlade
    
public:
    virtual void clickInvite(wxCommandEvent &event);
    virtual void clickPlay(wxCommandEvent &event);
    virtual void clickSend(wxCommandEvent &event);
    virtual void clickRefresh(wxCommandEvent &event);
    
    virtual void onCloseWindow(wxCloseEvent &event);
    
    //virtual void onSocketEvent(wxSocketEvent &event);
    
    virtual void onPlayersActivate(wxListEvent &event);
    virtual void onPlayersSelect(wxListEvent &event);
    virtual void onGamesActivate(wxListEvent &event);
    virtual void onGamesSelect(wxListEvent &event);
    
    virtual void onTimer(wxTimerEvent &event);
    
}; // wxGlade: end class


#endif // LOBBYWINDOW_H