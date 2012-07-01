/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file LoginWindow.cpp
 * 
 * Implementace přihlašovacího okna.
 *
 * @version $Rev: 171 $
 ******************************************************************************/

#include "LoginWindow.h"
#include "cl_config.h"
#include "LobbyWindow.h"
#include "MenuWindow.h"
#include "md5.h"
#include <iostream>

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "ticpp.h"

/**
 * Konstruktor
 * 
 * @param parent
 * @param id
 * @param title
 * @param pos
 * @param size
 * @param style
 * @return
 */
LoginWindow::LoginWindow(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
    wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
    // begin wxGlade: LoginWindow::LoginWindow
    labelServer = new wxStaticText(this, wxID_ANY, _("Server"));
    textServer = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    labelUser = new wxStaticText(this, wxID_ANY, _("Uživatel"));
    textUser = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    labelPassword = new wxStaticText(this, wxID_ANY, _("Heslo"));
    textPassword = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    buttonRegister = new wxButton(this, wxID_ANY, _("Registrovat"));
    buttonLogin = new wxButton(this, wxID_ANY, _("Přihlásit"));
    
    Connect(buttonRegister->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LoginWindow::clickRegister));
    Connect(buttonLogin->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LoginWindow::clickLogin));
    
    Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LoginWindow::onCloseWindow));
    //Connect(SOCKET_ID,wxEVT_SOCKET, wxSocketEventHandler(LoginWindow::onSocketEvent));

    set_properties();
    do_layout();
    // end wxGlade
    haveToEnableLogin = true;
    
    // registrace pro příjem zpráv týkajících se sítě
    clSocket * sock = clSocket::Instance();
    sock->RegisterHandler(this);
}


void LoginWindow::set_properties()
{
    // begin wxGlade: LoginWindow::set_properties
    SetTitle(_("Přihlášení"));
    labelServer->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, 0, wxT("")));
    labelUser->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, 0, wxT("")));
    labelPassword->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD, 0, wxT("")));
    buttonLogin->SetFocus();
    // end wxGlade
    
    dama::ClientConfig* config = dama::ClientConfig::instance();
    
    textServer->ChangeValue(wxString(config->getServer().c_str(), wxConvUTF8));
    textUser->ChangeValue(wxString(config->getUser().c_str(), wxConvUTF8));
}


void LoginWindow::do_layout()
{
    // begin wxGlade: LoginWindow::do_layout
    wxBoxSizer* sizer_4 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizer_9 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
    sizer_9->Add(labelServer, 0, 0, 0);
    sizer_9->Add(textServer, 0, wxEXPAND, 0);
    sizer_9->Add(labelUser, 0, wxTOP, 10);
    sizer_9->Add(textUser, 0, wxEXPAND, 0);
    sizer_9->Add(labelPassword, 0, wxTOP, 10);
    sizer_9->Add(textPassword, 0, wxEXPAND, 0);
    sizer_8->Add(buttonRegister, 0, wxLEFT|wxRIGHT|wxALIGN_BOTTOM, 10);
    sizer_8->Add(buttonLogin, 0, wxALIGN_BOTTOM, 0);
    sizer_9->Add(sizer_8, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL, 10);
    sizer_4->Add(sizer_9, 1, wxALL|wxEXPAND, 10);
    SetSizer(sizer_4);
    sizer_4->Fit(this);
    sizer_4->SetSizeHints(this);
    Layout();
    Centre();
    // end wxGlade
}

int LoginWindow::Receive(std::string zprava)
{
	std::string pom_str;
	ticpp::Document doc;
	ticpp::Element *root_element;
	
	try 
	{
		doc.Parse(zprava);
	}
	catch( ticpp::Exception& ex )
	{
		return 0;
	}
	
	root_element = doc.FirstChildElement();	// ziskani prvniho elementu
	pom_str = root_element->Value();	// ziskani hodnoty prvniho elementu
	if(pom_str == "register")
	{
		pom_str = root_element->GetAttribute("success");
		if(pom_str == "true")
		{
			wxMessageBox(_("Registrace proběhla úspěšně."), _("Registrace"), wxICON_INFORMATION);
		}
		else
		{
			wxMessageBox(_("Vaše zvolené jméno je již zaregistrováno někým jiným."), _("Registrace"), wxICON_EXCLAMATION);
		}
		return 1;
	}
	else if (pom_str == "login")
	{
		pom_str = root_element->GetAttribute("success");
		if(pom_str == "true")
		{
			// skrytí aktuálního okna
			Hide();
			
			// vytvoření okna serveru
			LobbyWindow* lobby = new LobbyWindow(GetParent(), wxID_ANY, wxEmptyString);
			lobby->Show();
			
			// zavření tohoto okna
			haveToEnableLogin = false;
			Close();
			return 1;
		}
		else
		{
			pom_str = root_element->GetAttribute("bugnumber");
			wxMessageBox(_("Jméno nebo heslo nesouhlasí."), _("Přihlášení"), wxICON_EXCLAMATION);
			return 1;
		}
	}
	return 0;
}


/**
 * Event handler pro kliknutí na tlačítko pro přihlášení
 * @param event
 */
void LoginWindow::clickLogin(wxCommandEvent &event)
{
    if (false == isAllFilled())
    {
        return;
    }
    
    ///@TODO provedení loginu
    
    // zobrazí se message box s cancelem - probíhá přihlašování
    // pokud se zmáčkne cancel, tak se přihlašování zruší
    // pokud se přihlašování zruší nebo nepodaří, vrátí se zpátky do tohoto okna
    // pokud se přihlašování podaří, pak se vytvoří okno s lobby, kterému se
    // nastaví parent stejný jako je parent tohoto okna, zobrazí se a toto okno
    // se zavře

    wxString enc = MD5(textPassword->GetValue());

    clSocket * sock = clSocket::Instance();
	std::string jmeno = std::string(textUser->GetValue().mb_str());
	std::string heslo = std::string(enc.mb_str());
	
    if(sock->ConnectClient(textServer->GetValue())) // máme připojení
		sock->SendRequest(std::string("<login username=\"" + jmeno + "\" password=\""+ heslo +"\" />"));
}

/**
 * Event handler pro kliknutí na tlačíko pro registraci
 * @param event
 */
void LoginWindow::clickRegister(wxCommandEvent &event)
{
    // kontrola, zda jsou povinná pole vyplněna
    if (false == isAllFilled())
    {        
        return;
    }
    
    // pokud je zadáno prázdné heslo, pak se uživatele zeptáme, 
    // jestli to tak opravdu chce
    if (textPassword->GetValue().Len() == 0)
    {
        wxMessageDialog* dialog = new wxMessageDialog(this,
              _("Nezadali jste žádné heslo.\nOpravdu chcete použít prázdné heslo?"), 
              _("Dáma 2008"), wxYES|wxNO);
        int answer = dialog->ShowModal();
        dialog->Destroy();
        
        if (answer == wxID_NO)
        {
            return;
        }
    }
    
    // vytvoříme MD5 sumu hesla, aby se neukládalo v čitelné podobě
    wxString enc = MD5(textPassword->GetValue());
    
    clSocket * sock = clSocket::Instance();
	std::string jmeno = std::string(textUser->GetValue().mb_str());
	std::string heslo = std::string(enc.mb_str());
	
    if(sock->ConnectClient(textServer->GetValue())) // máme připojení
		sock->SendRequest(std::string("<register username=\"" + jmeno + "\" password=\""+ heslo +"\" />"));
	///@TODO odebírat event o přijetí odpovědi
	///@TODO registrace uživatele

}

/**
 * Kontroluje, zda jsou všechny povinné položky vyplněny
 * @return true, pokud je vše v pořádku, jinak false
 */
bool LoginWindow::isAllFilled()
{
    // kontrola serveru
    if (textServer->GetValue().Len() == 0)
    {
        wxMessageDialog* dialog = new wxMessageDialog(this,
              _("Prosím, zadejte adresu serveru."), _("Dáma 2008"), wxOK);
        dialog->ShowModal();
        dialog->Destroy();
        return false;
    }
    
    wxString username = textUser->GetValue(); 
    
    // kontrola uživatelského jména
    if (username.Len() == 0)
    {
        wxMessageDialog* dialog = new wxMessageDialog(this,
              _("Prosím zadejte přihlašovací jméno."), _("Dáma 2008"), wxOK);
        dialog->ShowModal();
        dialog->Destroy();
        return false;
    }
    
    // uživatelské jméno nesmí obsahovat speciální znaky
    if (username.Find('<') != wxNOT_FOUND ||
            username.Find('>') != wxNOT_FOUND ||
            username.Find('=') != wxNOT_FOUND)
    {
        wxMessageDialog* dialog = new wxMessageDialog(this,
                      _("Přihlašovací jméno obsahuje nepovolené znaky"), 
                      _("Dáma 2008"), wxOK);
                dialog->ShowModal();
                dialog->Destroy();
                return false;
    }
    
    return true;
}

/**
 * Event handler pro zavření okna (a tím pádem celé aplikace)
 * @param event
 */
void LoginWindow::onCloseWindow(wxCloseEvent& event)
{
    // uložíme uživatelské jméno a adresu serveru do konfigurace
    dama::ClientConfig* config = dama::ClientConfig::instance();
    
    config->setServer(std::string(textServer->GetValue().mb_str()));
    config->setUser(std::string(textUser->GetValue().mb_str()));
    
    // pokud jsme se nepřihlásili k serveru, aktivujeme tlačítko pro příhlášení
    if (true == haveToEnableLogin)
    {
        ((MenuWindow*)GetParent())->enableLogin();
    }
    
    // odregistrování příjemu zpráv týkajících se sítě
    clSocket * sock = clSocket::Instance();
    sock->UnregisterHandler(this);
    this->Destroy();
}
