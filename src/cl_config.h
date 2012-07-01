/*******************************************************************************
 * ICP - Dáma Klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_config.h
 * 
 * Třída pro konfiguraci klienta.
 * 
 * Tato třída je jedinečná pro celou klientskou část a obaluje přístup ke 
 * konfiguračnímu souboru klienta. 
 * 
 * @version $Rev: 153 $
 ******************************************************************************/

#ifndef CL_CONFIG_H_
#define CL_CONFIG_H_

#include <string>

using namespace std;

namespace dama
{

/**
 * Třída pro přístup ke konfiguraci klienta.
 * 
 * Tato třída implementuje pattern Singleton, tzn. v celé aplikaci je může být
 * pouze jedna jediná instance této třídy. Je vytvářena "lazy" inicializací,
 * kdy se konstruktor volá až při prvním opravdovém přístupu k instanci třídy.
 */
class ClientConfig
{
public:
    /**
     * Přístup k instanci singletonu
     */
    static ClientConfig* instance();
    
    /**
     * Zavolá destruktor
     */
    static void destroy();
    
    /**
     * Vrací uživatelské jméno
     * 
     * @return Uzivatelske jmeno
     */
    string getUser() { return _user; };
    
    /**
     * Nastavuje uživatelské jméno
     * 
     * @param user Nové uživatelské jméno
     */
    void setUser(string user) { _user = user; };
    
    /**
     * Vrací adresu serveru
     * 
     * @return Adresa serveru
     */
    string getServer() { return _server; };
    
    /**
     * Nastavuje adresu serveru
     * 
     * @param server Nová adresa serveru
     */
    void setServer(string server) { _server = server; };
    
    /**
     * Vrací cestu k nápovědě
     * 
     * @return Cesta k nápovědě
     */
    string getHelpPath() { return _helpPath; };
    
    /**
     * Nastavuje cestu k nápovědě
     * 
     * @param helpPath Nová cesta k nápovědě
     */
    void setHelpPath(string helpPath) { _helpPath = helpPath; };
    
    
private:
    ClientConfig();
    ~ClientConfig();
    
    /**
     * Načte konfiguraci ze souboru
     */
    void load();
    
    /**
     * Uloží konfiguraci do souboru
     */
    void save();
    
    /**
     * Ukazatel na jedinou instanci objektu této třídy
     */
    static ClientConfig* _theInstance;
    
    /**
     * Jméno souboru s konfigurací
     */
    string _fileName;
    
    /**
     * Adresa serveru
     */
    string _server;
    
    /**
     * Přihlašovací jméno
     */
    string _user;
    
    /**
     * Cesta k nápovědě
     */
    string _helpPath;        
};

}

#endif /* CL_CONFIG_H_ */
