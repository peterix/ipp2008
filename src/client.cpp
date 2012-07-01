/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file client.cpp
 * 
 * Hlavní třída klientské aplikace.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/

#include <iostream>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/socket.h>
#include "MenuWindow.h"
#include "client.h"
#include "cl_config.h"

using namespace dama;
using namespace std;

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    // načtení konfigurace
    try
    {
        ClientConfig::instance();
    }
    catch (exception &e)
    {
        cerr << "Chyba pri nacitani konfigurace klienta: "; 
        cerr << e.what() << endl;
    }
        
    // hlavní smyčka
    wxInitAllImageHandlers();
    MenuWindow* HlavniMenu = new MenuWindow(NULL, wxID_ANY, wxEmptyString);
    SetTopWindow(HlavniMenu);
    HlavniMenu->Show();
    wxSocketBase::Initialize();
    
	return true;
}

int MyApp::OnExit()
{

    // uložení konfigurace
    try
    {
        ClientConfig::destroy();
    }
    catch (exception &e)
    {
        cerr << "Chyba pri ukladani konfigurace klienta: ";
        cerr << e.what() << endl;
    }
    
    return 0;
}
