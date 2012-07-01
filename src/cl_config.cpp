/*******************************************************************************
 * ICP - Dáma Klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_config.cpp
 * 
 * Implementace třídy pro konfiguraci klienta.
 * 
 * @version $Rev: 153 $
 ******************************************************************************/

#include <iostream>

#include "cl_config.h"
#include "cl_util.h"


#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "ticpp.h"

using namespace std;

namespace dama
{

// jediná instance této třídy
ClientConfig* ClientConfig::_theInstance = 0;

ClientConfig::ClientConfig()
{
    _fileName = "cl_config.xml";
    
    // výchozí hodnoty
    setUser("");
    setServer("localhost");
    setHelpPath("./help/index.html");
    
    try
    {
        load();
    }
    catch (exception& e)
    {
        cerr << "Chyba při načítání konfigurace: " << e.what() << endl;
    }
}

ClientConfig::~ClientConfig()
{
    try
    {
        save();
    }
    catch (exception& e)
    {
        cerr << "Chyba při ukládání konfigurace: " << e.what() << endl;
    }
    
}

ClientConfig* ClientConfig::instance()
{
    
    if (0 == _theInstance)
    {
        _theInstance = new ClientConfig();        
    }
    
    return _theInstance;
}

void ClientConfig::destroy()
{
    delete _theInstance;
    _theInstance = 0;
}

void ClientConfig::load()
{
    ticpp::Document xml(_fileName);
    
    try
    {
        xml.LoadFile();
    }
    catch (ticpp::Exception& e)
    {
        
    }
    
    ticpp::Element* topElement = xml.FirstChildElement("config", false);
    if (topElement == 0)
    {
        topElement = new ticpp::Element("config");
    }
    
    ticpp::Element* element = topElement->FirstChildElement("user", false);
    if (element != 0)
    {
        setUser(element->GetTextOrDefault(getUser()));
    }
    
    element = topElement->FirstChildElement("server", false);
    if (element != 0)
    {
        setServer(element->GetTextOrDefault(getServer()));
    }
    
    element = topElement->FirstChildElement("helpPath", false);
    if (element != 0)
    {
        setHelpPath(element->GetTextOrDefault(getHelpPath()));
    }
    
}

void ClientConfig::save()
{
    ticpp::Document xml(_fileName);
    
    ticpp::Declaration* declaration = new ticpp::Declaration("1.0", "utf-8", "");
    xml.LinkEndChild(declaration);
    
    ticpp::Element* topElement = new ticpp::Element("config");
    xml.LinkEndChild(topElement);
    
    ticpp::Element* element = new ticpp::Element("user");
    element->SetText(getUser());
    topElement->InsertEndChild(*element);
    
    element = new ticpp::Element("server");
    element->SetText(getServer());
    topElement->InsertEndChild(*element);
    
    element = new ticpp::Element("helpPath");
    element->SetText(getHelpPath());
    topElement->InsertEndChild(*element);
    
    xml.SaveFile();
}

}
    

