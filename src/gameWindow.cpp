/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file gameWindow.cpp
 *
 * Implementace okna pro vlastní hru.
 *
 * @version $Rev: 180 $
 ******************************************************************************/

#include "gameWindow.h"
#include <iostream>
#include "cl_board.h"
#include "cl_util.h"
#include <sstream>

/// ID tlačítka pro logování
#define LOG_BUTTON_ID 500
gameWindow::gameWindow
(
	wxWindow* parent,
	int id,
	const wxString& title,
	bool netgame,
	const wxString& _with,
	const wxString& _bily,
	const wxString& _cerny,
	std::string & stav,
	const wxPoint& pos,
	const wxSize& size,
	long style
):wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
    sizer_1_staticbox = new wxStaticBox(this, -1, wxEmptyString);
    int index = 1;
    for(int y = 7;y>=0;y--)
    {
    	for(int x= 0;x<8;x++)
    	{
    		buttons[x][y] = new wxBitmapButton(this, index, wxBitmap(wxT("data/nothing.png"), wxBITMAP_TYPE_ANY));
    		index++;
    	}
    }
    
    
    /// @todo jména hráčů
    bilyHrac = _bily;
    cernyHrac = _cerny;
    protiHrac = _with;
    sitovaHra = netgame;
    if(bilyHrac == protiHrac) mistnihrac = CERNY;
    else mistnihrac = BILY;
    
    bitmap_1 = new wxStaticBitmap(this, wxID_ANY, wxBitmap(wxT("data/bstone.png"), wxBITMAP_TYPE_ANY));
    //black_name = new wxStaticText(this, wxID_ANY, _("BLACK NAME"));
    black_name = new wxStaticText(this, wxID_ANY, cernyHrac);
    black_pieces = new wxStaticText(this, wxID_ANY, _("12 / 12"));
    static_line_1 = new wxStaticLine(this, wxID_ANY);
    panel_1 = new wxPanel(this, wxID_ANY);
    label_2 = new wxStaticText(this, wxID_ANY, _("Na řadě je:\nWHITE NAME"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
    panel_2 = new wxPanel(this, wxID_ANY);
    static_line_3 = new wxStaticLine(this, wxID_ANY);
    white_pieces = new wxStaticText(this, wxID_ANY, _("12 / 12"));
    // white_name = new wxStaticText(this, wxID_ANY, _("WHITE NAME"));
    white_name = new wxStaticText(this, wxID_ANY, bilyHrac);
    bitmap_1_copy = new wxStaticBitmap(this, wxID_ANY, wxBitmap(wxT("data/wstone.png"), wxBITMAP_TYPE_ANY));
    //static_line_2 = new wxStaticLine(this, wxID_ANY);
	
	if(sitovaHra)
	{
		deska = new Deska(0); // 1 = rozmístit kameny, 0 = nerozmisťovat
		deska->SetStav(stav);
	    // registrace pro příjem zpráv týkajících se sítě
	    clSocket * sock = clSocket::Instance();
	    sock->RegisterHandler(this);
	}
	else
	{
		deska = new Deska(1); // 1 = rozmístit kameny, 0 = nerozmisťovat
	}
    
    set_properties();
    do_layout();
    UpdateAllFields( 0 );
    
    // zeptáme se, zda ukládat replay
    wxMessageDialog* dialog = new wxMessageDialog(this,
              _("Chcete ukládat průběh hry?"), 
              _("Dáma 2008"), wxYES|wxNO);
    int answer = dialog->ShowModal();
    dialog->Destroy();
        
    if (answer == wxID_NO)
    {
        logReplay = false;
    }
    else
    {
        logReplay = true;
        // otevřeme soubor
        wxString filename =  wxFileSelector(_("Vyberte soubor pro zázname hry."), _T("replays/"),/* default file*/ _T(""),/* default ext*/ _T(".d2008") , _("*.d2008"),wxFD_SAVE|wxFD_OVERWRITE_PROMPT , this ,/*pozice */ -1, -1);
        // pokud soubor nemá spravnou příponu, doplníme ji
        if (!filename.EndsWith(_(".d2008")))
        {
            filename += _(".d2008");
        }
        
        // zkusíme ho načíst
        replayXml = new ticpp::Document(std::string(filename.mb_str()));
        try
        {
            replayXml->LoadFile();
        }
        catch (ticpp::Exception& e) {}
        
        // má se tvořit nová struktura?
        bool createNew = false;
        
        topElement = replayXml->FirstChildElement("replay", false);
        if (topElement != 0)
        {
            if (topElement->GetAttribute("white") != std::string(bilyHrac.mb_str()) ||
                    topElement->GetAttribute("black") != std::string(cernyHrac.mb_str()))
            {
                delete replayXml;
                replayXml = new ticpp::Document(std::string(filename.mb_str()));
                
                createNew = true;
            }
        }
        else
        {
            createNew = true;
        }
        
        if (createNew == true)
        {
            ticpp::Declaration* declaration = new ticpp::Declaration("1.0", "utf-8", "");
            replayXml->LinkEndChild(declaration);
                        
            topElement = new ticpp::Element("replay");            
            topElement->SetAttribute("white", std::string(bilyHrac.mb_str()));
            topElement->SetAttribute("black", std::string(cernyHrac.mb_str()));
            replayXml->LinkEndChild(topElement);
        }
        /// @todo Doplňování tahů offline
    }
}


BEGIN_EVENT_TABLE(gameWindow, wxFrame)
	EVT_BUTTON(wxID_ANY, gameWindow::buttonhandler) // události z hracího pole (klikání na tlačítka)
	EVT_CLOSE(gameWindow::OnClose) // hráč uzavírá okno
END_EVENT_TABLE();

int gameWindow::Receive (std::string zprava)
{
	std::string pom_str;
	
	ticpp::Document doc;
	ticpp::Element *root_element;
	
	if(sitovaHra) {
		try 
		{
			doc.Parse(zprava);
			root_element = doc.FirstChildElement();	// ziskáni prvniho elementu
			pom_str = root_element->Value();	// ziskáni hodnoty prvního elementu
		}
		catch( ticpp::Exception& ex )
		{
			return false;
		}
		if(pom_str == "move") {
			try {
				pom_str = root_element->GetAttribute("player");	// získání jména protihráče
				wxString mystring(pom_str.c_str(), wxConvUTF8); // konverze řetězců ... 
				if(mystring != protiHrac.c_str())
					return 0;
				int x=-1, y=-1;
				root_element->GetAttribute("x", &x);	// načtení x-ové souřadnice
				root_element->GetAttribute("y", &y);	// načtení y-ové souřadnice

				navratovaHodnota vysledek = deska->Input(x,y);
			
				switch(vysledek) // tady se dá hodit reakce na výsledky z Board::Input() -- třeba posílání zpráv do sítě
				{
					case SPATNY_TAH:
						break;
					case SKAKANI:
					case HRAC_UCHOPIL_KAMEN:
					case HRA_POKRACUJE:
						UpdateAllFields( 0 );
						if (logReplay)
						{
					    logReplayTurn();
						}
						break;
					case BILY_PROHRAL:
						UpdateAllFields( 1 );
						wxMessageBox(_("Vyhrál černý hráč."), _("Konec hry"), wxICON_INFORMATION);
						if (logReplay)
						{
					    logReplayTurn();
					    saveReplay();
						}
						break;
					case CERNY_PROHRAL:
						UpdateAllFields( 1 );
						wxMessageBox(_("Vyhrál bílý hráč."), _("Konec hry"), wxICON_INFORMATION);
						if (logReplay)
						{
					    logReplayTurn();
					    saveReplay();
						}
						break;
					
				}
				return 1;
			}
			catch (ticpp::Exception& ex)
			{
				return 0;
			}
		}
	}
	return 0; // zatím neumí zpracovávat...
}

void gameWindow::SetDefaultColor(int x, int y)
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

void gameWindow::UpdateAllFields(bool endgame)
{
	int index = 1;
    for(int y = 7;y>=0;y--)
    {
    	for(int x= 0;x<8;x++)
    	{
    		typPole pole = deska->GetField( x, y );
    		switch(pole)
    		{
    			case OKRAJ:
    				std::cerr << "FUCK FUCK FUCK" << std::endl;
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
    wxString text;
    if(!endgame)
    {
		text = _("Na tahu je:\n");
		barvaHrace naRade = deska->GetCurrentPlayer();
		if(naRade == BILY)
		{
			text += bilyHrac;
		}
		else
		{
			text += cernyHrac;
		}
    }
    else
    {
		text = _("Konec hry.");
    }
    label_2->SetLabel(text);
    wxString textwh;
    textwh << deska->GetWhite();
    textwh << _(" / 12");
    white_pieces->SetLabel(textwh);
    wxString textbk;
    textbk << deska->GetBlack();
    textbk << _(" / 12");
    black_pieces->SetLabel(textbk);
}

void gameWindow::getXY(int index, int &x, int &y)
{
	index -=1;
	y = 7 - (index / 8);
	x = index % 8;
}

/*
enum navratovaHodnota
{
	SPATNY_TAH = 0,
	SKAKANI = 1,
	HRAC_UCHOPIL_KAMEN = 2,
	BILY_PROHRAL = 3,
	CERNY_PROHRAL = 4,
	HRA_POKRACUJE = 5
};
*/

void gameWindow::buttonhandler(wxCommandEvent &event)
{
	if(sitovaHra)
		if(deska->GetCurrentPlayer() != mistnihrac)
			return;
	int index = event.GetId();
	int x,y;
	getXY(index, x, y);
	
	navratovaHodnota vysledek = deska->Input(x,y);
	
	
	if(sitovaHra)
	{
	    clSocket * sock = clSocket::Instance();
	    std::stringstream out;
		out << "<move x=\"" << x << "\" y=\"" << y << "\" player=\"" << protiHrac.mb_str()  <<"\" />";
		std::string s = out.str();
	    sock->SendRequest(s);
    }
	switch(vysledek) // tady se dá hodit reakce na výsledky z Board::Input() -- třeba posílání zpráv do sítě
	{
		case SPATNY_TAH:
		break;
		case SKAKANI:
		case HRAC_UCHOPIL_KAMEN:
		case HRA_POKRACUJE:
			UpdateAllFields( 0 );
			if (logReplay)
			{
			    logReplayTurn();
			}
		break;
		case BILY_PROHRAL:
			UpdateAllFields( 1 );
			wxMessageBox(_("Vyhrál černý hráč."), _("Konec hry"), wxICON_INFORMATION);
			if (logReplay)
			{
			    logReplayTurn();
			    saveReplay();
			}
		break;
		case CERNY_PROHRAL:
			UpdateAllFields( 1 );
			wxMessageBox(_("Vyhrál bílý hráč."), _("Konec hry"), wxICON_INFORMATION);
			if (logReplay)
			{
			    logReplayTurn();
			    saveReplay();
			}
		break;
	}
}
void gameWindow::OnClose(wxCloseEvent &event)
{
	std::cout << "Zaviram okno se hrou!" << std::endl;
	
    // odregistrování příjemu zpráv týkajících se sítě
    if(sitovaHra)
    {
	    clSocket * sock = clSocket::Instance();
	    sock->UnregisterHandler(this);
	}
    delete deska;
	Destroy(); // zničení okna
}

void gameWindow::set_properties()
{
    SetTitle(_("Hra"));
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
    //black_pieces->SetForegroundColour(wxColour(35, 35, 142));
    black_pieces->SetFont(wxFont(20, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
    label_2->SetFont(wxFont(15, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
    //white_pieces->SetForegroundColour(wxColour(35, 35, 142));
    white_pieces->SetFont(wxFont(20, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
    white_name->SetFont(wxFont(16, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("Liberation Sans")));
}


void gameWindow::do_layout()
{
    wxStaticBoxSizer* sizer_1 = new wxStaticBoxSizer(sizer_1_staticbox, wxHORIZONTAL);
    wxBoxSizer* sizer_2 = new wxBoxSizer(wxVERTICAL);
    wxGridSizer* grid_sizer_1 = new wxGridSizer(8, 8, 0, 0);
    for(int y = 7;y>=0;y--)
    {
    	for(int x= 0;x<8;x++)
    	{
    		grid_sizer_1->Add(buttons[x][y], 0, wxEXPAND, 0);
    	}
    }
    sizer_1->Add(grid_sizer_1, 3, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    sizer_2->Add(bitmap_1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 6);
    sizer_2->Add(black_name, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL/*wxEXPAND|wxALIGN_CENTRE|wxST_NO_AUTORESIZE*/, 0);
    sizer_2->Add(black_pieces, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL/*wxEXPAND|wxALIGN_CENTRE|wxST_NO_AUTORESIZE*/, 0);
    sizer_2->Add(static_line_1, 0, wxEXPAND, 0);
    sizer_2->Add(panel_1, 1, wxEXPAND, 0);
    sizer_2->Add(label_2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    sizer_2->Add(panel_2, 1, wxEXPAND, 0);
    sizer_2->Add(static_line_3, 0, wxEXPAND, 0);
    sizer_2->Add(white_pieces, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL/*wxEXPAND|wxALIGN_CENTRE|wxST_NO_AUTORESIZE*/, 0);
    sizer_2->Add(white_name, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL/*wxEXPAND|wxALIGN_CENTRE|wxST_NO_AUTORESIZE*/, 0);
    sizer_2->Add(bitmap_1_copy, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 6);
    //sizer_2->Add(static_line_2, 0, wxEXPAND, 0);
    sizer_1->Add(sizer_2, 1, wxALL|wxEXPAND, 2);
    SetSizer(sizer_1);
    sizer_1->SetSizeHints(this);
    Layout();
}

/**
 * Vloží do xml dokumentu se záznamem hry aktuální tah
 */
void gameWindow::logReplayTurn()
{
    ticpp::Element* element = new ticpp::Element("state");
    element->SetText(deska->GetStav());
    topElement->LinkEndChild(element);
}

/**
 * Uloží xml dokument se záznamem hry do souboru
 */
void gameWindow::saveReplay()
{
    try
    {
        replayXml->SaveFile();
    }
    catch (ticpp::Exception& e) {}
}
