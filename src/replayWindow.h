/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file replayWindow.h
 * 
 * Okno pro přehrávání záznamů her.
 *
 * @version $Rev: 180 $
 ******************************************************************************/

#include <wx/wx.h>
#include <wx/image.h>
#include "cl_util.h"

#ifndef REPLAYWINDOW_H
#define REPLAYWINDOW_H

#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <vector>
#include <string>

class Deska;

class replayWindow: public wxFrame
{
public:

    replayWindow(wxWindow* parent, int id, const wxString& title, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);

private:
    void set_properties();
    void do_layout();
    void getXY(int index, int &x, int &y);
	void UpdateAllFields(bool endgame);
	void SetDefaultColor(int x, int y);
	void Prev(void);
	void Next(void);
	int Load(wxString filename);
protected:
    wxStaticBox* sizer_1_copy_staticbox;
    // pole tlačítek
	wxBitmapButton* buttons[8][8];
    
    wxStaticBitmap* bitmap_1_copy_1;
    
    wxStaticText* black_name;
    wxStaticText* black_pieces;
    
    wxStaticLine* static_line_1_copy;
    
    wxPanel* panel_5;
    wxStaticText* cislo_tahu_txt;
    
    wxSlider* slider_tahy;
    wxBitmapButton* btn_back;
    wxBitmapButton* btn_play;
    wxBitmapButton* btn_pause;
    wxBitmapButton* btn_forw;
    
    wxStaticText* label_3;
    
    wxSpinCtrl* prodleva;
    
    wxPanel* panel_4;
    wxStaticLine* static_line_4;
    
    wxStaticText* white_pieces;
    wxStaticText* white_name;
    
    wxStaticBitmap* bitmap_1_copy_copy;

	Deska * deska;
	// end wxGlade
	wxString *bilyHrac;
	wxString *cernyHrac;
	
	bool nacteno;
	int maxtahu;
	int tah;
	wxTimer * casovac;
	std::vector<std::string> *tahy;

    DECLARE_EVENT_TABLE();

public:
    virtual void lolhandler(wxCommandEvent &event); // wxGlade: <event_handler>
    virtual void buttanz(wxCommandEvent &event); // wxGlade: <event_handler>
	virtual void OnClose(wxCloseEvent &event);
    virtual void setDelay(wxSpinEvent &event); // wxGlade: <event_handler>
    virtual void OnTimer(wxTimerEvent &event);
    virtual void OnSlider(wxScrollEvent &event);

}; // wxGlade: end class


#endif // REPLAYWINDOW_H
