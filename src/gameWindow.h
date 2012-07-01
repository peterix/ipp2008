/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file gameWindow.h
 * 
 * Okno pro vlastní hru.
 *
 * @version $Rev: 180 $
 ******************************************************************************/

#include <wx/wx.h>
#include <wx/image.h>
#include "cl_util.h"

#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <wx/statline.h>
#include "client_connect.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#include "ticpp.h"
#include "cl_receiver.h"

class Deska;


class gameWindow: public wxFrame, public Receiver {
public:

	gameWindow(wxWindow* parent, int id, const wxString& title, bool netgame,const wxString& _with,const wxString& _bily ,const wxString& _cerny, std::string & stav, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);
	
	virtual int Receive (std::string zprava);

private:
	void Init();
	void set_properties();
	void do_layout();
	void getXY(int index, int &x, int &y);
	void UpdateAllFields(bool endgame);
	void SetDefaultColor(int x, int y);
	
	void logReplayTurn();
	void saveReplay();

protected:
	// okraj
	wxStaticBox* sizer_1_staticbox;
	// pole tlačítek
	wxBitmapButton* buttons[8][8];
	// pravý panel
	wxStaticBitmap* bitmap_1;
	wxStaticText* black_name;
	wxStaticText* black_pieces;
	wxStaticLine* static_line_1;
	wxPanel* panel_1;
	wxStaticText* label_2;
	wxPanel* panel_2;
	wxStaticLine* static_line_3;
	wxStaticText* white_pieces;
	wxStaticText* white_name;
	wxStaticBitmap* bitmap_1_copy;
	
	wxButton* buttonLog;

	Deska * deska;
	// end wxGlade
	wxString bilyHrac;
	wxString cernyHrac;
	wxString protiHrac;
	barvaHrace mistnihrac;
	
	/// jedná se o síťovou hru?
	bool sitovaHra;

	/// je vše plně inicializováno
	bool initOK;
	
	/// cesta k souboru s replay
	wxString replayPath;
	/// příznak zda ukládat replay
	bool logReplay;
	/// xml dokumnet s replayem
	ticpp::Document* replayXml;
	ticpp::Element* topElement;
	
	DECLARE_EVENT_TABLE();

public:
	virtual void buttonhandler(wxCommandEvent &event); // wxGlade: <event_handler>
	virtual void OnClose(wxCloseEvent &event);
}; // wxGlade: end class


#endif // GAMEWINDOW_H
