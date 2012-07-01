/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file replayWindow.cpp
 * 
 * Implementace okna pro přehrávání záznamů her.
 *
 * @version $Rev: 180 $
 ******************************************************************************/

#include "replayWindow.h"
#include <iostream>
#include "cl_board.h"
#include "cl_util.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "ticpp.h"

replayWindow::replayWindow(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
    wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	// kvůli segfaultům
	deska = NULL;
	tahy = NULL;
	bilyHrac = NULL;
	cernyHrac = NULL;
	tah = 0;
	
	// Pokus o načtení hry.
    wxString filename =  wxFileSelector(_("Vyberte soubor s uloženým záznamem hry."), _T("replays/"),/* default file*/ _T(""),/* default ext*/ _T(".d2008") , _("*.d2008"),wxFD_OPEN|wxFD_FILE_MUST_EXIST , this ,/*pozice */ -1, -1);
	if ( !filename.empty() )
	{
		Load(filename);
/*		if (!Load(filename))
			Close();*/
	}
/*	else Close();*/
	
	// begin wxGlade: replayWindow::replayWindow
	sizer_1_copy_staticbox = new wxStaticBox(this, -1, wxEmptyString);
	//int index = 1;
	for(int y = 7;y>=0;y--)
	{
		for(int x= 0;x<8;x++)
		{
			buttons[x][y] = new wxBitmapButton(this, wxID_ANY, wxBitmap(wxT("data/nothing.png"), wxBITMAP_TYPE_ANY));
			//index++;
		}
	}

	bitmap_1_copy_1 = new wxStaticBitmap(this, wxID_ANY, wxBitmap(wxT("data/bstone.png"), wxBITMAP_TYPE_ANY));
	if(!cernyHrac)
		black_name = new wxStaticText(this, wxID_ANY, _("ČERNÝ"));
	else
		black_name = new wxStaticText(this, wxID_ANY, *cernyHrac);
	
	black_pieces = new wxStaticText(this, wxID_ANY, _("12 / 12"));

	static_line_1_copy = new wxStaticLine(this, wxID_ANY);
	panel_5 = new wxPanel(this, wxID_ANY);

	cislo_tahu_txt = new wxStaticText(this, wxID_ANY, _("Tah č.: 1/120"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	slider_tahy = new wxSlider(this, 7, 0, 0, maxtahu-1, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS);
	btn_back = new wxBitmapButton(this, 1, wxBitmap(wxT("data/back.png"), wxBITMAP_TYPE_ANY));
	btn_play = new wxBitmapButton(this, 2, wxBitmap(wxT("data/start.png"), wxBITMAP_TYPE_ANY));
	btn_pause = new wxBitmapButton(this, 3, wxBitmap(wxT("data/pause.png"), wxBITMAP_TYPE_ANY));
	btn_forw = new wxBitmapButton(this, 4, wxBitmap(wxT("data/forw.png"), wxBITMAP_TYPE_ANY));

	label_3 = new wxStaticText(this, wxID_ANY, _("Prodleva (vteřiny):"));

	prodleva = new wxSpinCtrl(this, 6, wxT("3"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 15);

	panel_4 = new wxPanel(this, wxID_ANY);
	static_line_4 = new wxStaticLine(this, wxID_ANY);

	white_pieces = new wxStaticText(this, wxID_ANY, _("12 / 12"));

	if(!bilyHrac)
		white_name = new wxStaticText(this, wxID_ANY, _("BÍLÝ"));
	else
		white_name = new wxStaticText(this, wxID_ANY, *bilyHrac);
    
    bitmap_1_copy_copy = new wxStaticBitmap(this, wxID_ANY, wxBitmap(wxT("data/wstone.png"), wxBITMAP_TYPE_ANY));

	deska = new Deska(0);
	casovac = new wxTimer(this, 5);
	
    set_properties();
    do_layout();
    if ( !filename.empty() )
	    UpdateAllFields( 0 );
}


BEGIN_EVENT_TABLE(replayWindow, wxFrame)
    EVT_BUTTON(wxID_ANY, replayWindow::lolhandler)
    EVT_BUTTON(1, replayWindow::buttanz)
    EVT_BUTTON(2, replayWindow::buttanz)
    EVT_BUTTON(3, replayWindow::buttanz)
    EVT_BUTTON(4, replayWindow::buttanz)
    EVT_TIMER(5, replayWindow::OnTimer)
    EVT_SPINCTRL(6, replayWindow::setDelay)
    EVT_CLOSE(replayWindow::OnClose) // hráč uzavírá okno
    EVT_SCROLL(replayWindow::OnSlider)
END_EVENT_TABLE();

int replayWindow::Load(wxString filename)
{
	///@TODO: otestovat se souborem s unikodem v nazvu
	ticpp::Document xml(std::string(filename.mb_str()));
	maxtahu = 0;
    try
    {
		xml.LoadFile();
		ticpp::Element* topElement = xml.FirstChildElement("replay", false);
		
		std::string bily = topElement->GetAttribute("white");
		bilyHrac = new wxString(bily.c_str(), wxConvUTF8);
		
		std::string cerny = topElement->GetAttribute("black");
		cernyHrac = new wxString(cerny.c_str(), wxConvUTF8);
		
		// projdeme xml
		ticpp::Iterator<ticpp::Element> child("state");  
		tahy = new std::vector<std::string>;
	   
		for ( child = child.begin(topElement); child != child.end(); child++ ) 
		{
		    std::string state = child->GetText(false);
		    tahy->push_back(state);
		    maxtahu ++;
		}
    }
    catch (...)
    {
        return 0;
    }
    return 1;
}

void replayWindow::OnTimer(wxTimerEvent& event)
{
	Next();
}

void replayWindow::OnSlider(wxScrollEvent &event)
{
	if(tah != slider_tahy->GetValue())
	{
		tah = slider_tahy->GetValue();
		UpdateAllFields(0);
	}
}

void replayWindow::Next(void)
{
	if(tah + 1 < maxtahu)
	{
		tah++;
		UpdateAllFields(0);
	}
	if(casovac->IsRunning() && (tah + 1 == maxtahu))
	{
		casovac->Stop();
		btn_play->SetBackgroundColour(wxNullColour);
	}
}

void replayWindow::Prev(void)
{
	if(tah > 0)
	{
		tah--;
		UpdateAllFields(0);
	}
}

void replayWindow::SetDefaultColor(int x, int y)
{
    if((x+y)%2)
    {
    	buttons[x][y]->SetBackgroundColour(wxColour(249, 248, 248));
    }
    else
    {
    	buttons[x][y]->SetBackgroundColour(wxColour(180, 180, 180));
    }
}

void replayWindow::UpdateAllFields(bool endgame)
{
	std::string stav = (*tahy)[tah];
	deska->SetStav(stav);
	int index = 1;
    for(int y = 7;y>=0;y--)
    {
    	for(int x= 0;x<8;x++)
    	{
    		typPole pole = deska->GetField( x, y );
    		switch(pole)
    		{
    			case OKRAJ:
    				break;
    			case PRAZDNE_POLE:
    				buttons[x][y]->SetBitmapLabel(wxBitmap(wxT("data/nothing.png")));
    				break;
    			case BILY_KAMEN:
    				buttons[x][y]->SetBitmapLabel(wxBitmap(wxT("data/wstone.png")));
    				break;
    			case CERNY_KAMEN:
    				buttons[x][y]->SetBitmapLabel(wxBitmap(wxT("data/bstone.png")));
    				break;
    			case BILA_DAMA:
    				buttons[x][y]->SetBitmapLabel(wxBitmap(wxT("data/wking.png")));
    				break;
    			case CERNA_DAMA:
    				buttons[x][y]->SetBitmapLabel(wxBitmap(wxT("data/bking.png")));
    				break;
    		}
    		if(deska->IsActive(x,y)) buttons[x][y]->SetBackgroundColour(wxColour(150, 150, 248));
			else if(deska->IsMarked(x,y)) buttons[x][y]->SetBackgroundColour(wxColour(249, 150, 150));
    		else if(deska->IsGrabbed(x,y)) buttons[x][y]->SetBackgroundColour(wxColour(150, 248, 150));
    		else SetDefaultColor(x, y);
    		index++;
    	}
    }
    wxString textwh;
    textwh << deska->GetWhite();
    textwh << _(" / 12");
    white_pieces->SetLabel(textwh);
    wxString textbk;
    textbk << deska->GetBlack();
    textbk << _(" / 12");
    black_pieces->SetLabel(textbk);
//    cislo_tahu_txt = new wxStaticText(this, wxID_ANY, _("Tah č.: 1/120"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
    wxString texttahy;
    texttahy << _("Tah č.:") << tah + 1 << _("/") << maxtahu;
    cislo_tahu_txt->SetLabel(texttahy);
    slider_tahy->SetValue(tah);
    
}

void replayWindow::getXY(int index, int &x, int &y)
{
	index -=1;
	y = 7 - (index / 8);
	x = index % 8;
}


void replayWindow::OnClose(wxCloseEvent &event)
{
	
	if(deska) delete deska;
	if(bilyHrac) delete bilyHrac;
	if(cernyHrac) delete cernyHrac;
	if(tahy) delete tahy;
	
	Destroy(); // zničení okna
}


void replayWindow::lolhandler(wxCommandEvent &event)
{
    event.Skip();
    wxLogDebug(wxT("Event handler (replayWindow::lolhandler) not implemented yet")); //notify the user that he hasn't implemented the event handler yet
}
/*
    btn_back = new wxBitmapButton(this, 1, wxBitmap(wxT("data/back.png"), wxBITMAP_TYPE_ANY));
    btn_play = new wxBitmapButton(this, 2, wxBitmap(wxT("data/start.png"), wxBITMAP_TYPE_ANY));
    btn_pause = new wxBitmapButton(this, 3, wxBitmap(wxT("data/pause.png"), wxBITMAP_TYPE_ANY));
    btn_forw = new wxBitmapButton(this, 4, wxBitmap(wxT("data/forw.png"), wxBITMAP_TYPE_ANY));
*/
void replayWindow::buttanz(wxCommandEvent &event)
{
	int index = event.GetId();
	switch(index)
	{
		case 1:
				Prev();
			break;
		case 2:
				casovac->Start(prodleva->GetValue() * 1000);
				btn_play->SetBackgroundColour(wxColour(150, 248, 150));
			break;
		case 3:
				casovac->Stop();
				btn_play->SetBackgroundColour(wxNullColour);
			break;
		case 4:
				Next();
			break;
	}
}


void replayWindow::setDelay(wxSpinEvent &event)
{
	if(casovac->IsRunning())
	{
		casovac->Start(prodleva->GetValue() * 1000);
	}
}


// wxGlade: add replayWindow event handlers


void replayWindow::set_properties()
{
    // begin wxGlade: replayWindow::set_properties
    SetTitle(_("Přehrávač minulých her"));
    SetSize(wxSize(663, 484));
    
    for(int y = 7;y>=0;y--)
    {
    	for(int x= 0;x<8;x++)
    	{
    		buttons[x][y]->SetMinSize(wxSize(60, 60));
    		SetDefaultColor(x,y);
    	}
    }
    
    black_name->SetFont(wxFont(16, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
    black_pieces->SetFont(wxFont(20, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
    btn_back->SetSize(btn_back->GetBestSize());
    btn_play->SetSize(btn_play->GetBestSize());
    btn_pause->SetSize(btn_pause->GetBestSize());
    btn_forw->SetSize(btn_forw->GetBestSize());
    white_pieces->SetFont(wxFont(20, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
    white_name->SetFont(wxFont(16, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
    // end wxGlade
}


void replayWindow::do_layout()
{
    // begin wxGlade: replayWindow::do_layout
    wxStaticBoxSizer* sizer_1_copy = new wxStaticBoxSizer(sizer_1_copy_staticbox, wxHORIZONTAL);
    wxBoxSizer* sizer_2_copy = new wxBoxSizer(wxVERTICAL);
    wxGridSizer* grid_sizer_3 = new wxGridSizer(2, 4, 0, 0);
    wxGridSizer* grid_sizer_1_copy = new wxGridSizer(8, 8, 0, 0);
    for(int y = 7;y>=0;y--)
    {
    	for(int x= 0;x<8;x++)
    	{
    		grid_sizer_1_copy->Add(buttons[x][y], 0, wxEXPAND, 0);
    	}
    }
    sizer_1_copy->Add(grid_sizer_1_copy, 3, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    sizer_2_copy->Add(bitmap_1_copy_1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 6);
    sizer_2_copy->Add(black_name, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    sizer_2_copy->Add(black_pieces, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    sizer_2_copy->Add(static_line_1_copy, 0, wxEXPAND, 0);
    sizer_2_copy->Add(panel_5, 1, wxEXPAND, 0);
    sizer_2_copy->Add(cislo_tahu_txt, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    sizer_2_copy->Add(slider_tahy, 0, wxEXPAND, 0);
    grid_sizer_3->Add(btn_back, 0, wxEXPAND, 0);
    grid_sizer_3->Add(btn_play, 0, wxEXPAND, 0);
    grid_sizer_3->Add(btn_pause, 0, wxEXPAND, 0);
    grid_sizer_3->Add(btn_forw, 0, wxEXPAND, 0);
    sizer_2_copy->Add(grid_sizer_3, 1, wxEXPAND, 0);
    sizer_2_copy->Add(label_3, 0, 0, 0);
    sizer_2_copy->Add(prodleva, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    sizer_2_copy->Add(panel_4, 1, wxEXPAND, 0);
    sizer_2_copy->Add(static_line_4, 0, wxEXPAND, 0);
    sizer_2_copy->Add(white_pieces, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    sizer_2_copy->Add(white_name, 0, wxALIGN_CENTER_HORIZONTAL, 0);
    sizer_2_copy->Add(bitmap_1_copy_copy, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 6);
    sizer_1_copy->Add(sizer_2_copy, 1, wxALL|wxEXPAND, 2);
    SetSizer(sizer_1_copy);
    sizer_1_copy->SetSizeHints(this);
    Layout();
    // end wxGlade
}

