/*******************************************************************************
 * ICP - Dáma server 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file server.h
 *
 * Hlavičkový soubor: Implementace serveru a jeho vláken
 * 
 * @version $Rev: 168 $
 ******************************************************************************/
#ifndef __DAMA_SERVER
#define __DAMA_SERVER

#include <iostream>
#include <wx/app.h>
#include <wx/socket.h>
#include <wx/thread.h>
#include "server_handler.h"

/**
 *	Třída definujicí aplikaci
 *
 *	Třída definujicí wxWidgets konzolovou aplikaci
 */
class ServerApp : public wxAppConsole
{
public:
	virtual bool OnInit();
	virtual int OnRun();
};

/**
 *	Třída definujicí server
 */
class Server
{
	wxSocketServer*   serverSocket;  /**< Socket serveru */
	int port;  /**< Port, na kterém server naslouchá */

public:
	/** Konstruktor. Lze pomocí něj nastavit port, který je jinak defaultně
	    nastaven na 8007. */
	Server(int s_port = 8007) :
		port(s_port) {}
	/** Destruktor. Uvolňuje serverový socket.*/
	~Server() { serverSocket->Destroy(); }

	// funkce resici "nastartovani" serveru
	int CreateServerSocket(void);
	void AcceptConnection(void);
};

/**
 *	Třída definujicí serverová "Detached" vlákna
 */
class ServerThread : public wxThread
{
	Server*   m_pServer;  /**< Socket serveru */
	wxSocketBase* m_pSocket;  /**< Klientský socket pro read/write operace */
	ServerHandler*	m_pSHandler;

public:
	/** Konstruktor. Nastavuje se pomocí něj server a klientský socket. Vytvoří instanci třídy ServerHandler. */
	ServerThread(Server* pServer, wxSocketBase* pSocket) :
				m_pServer(pServer), m_pSocket(pSocket) { m_pSHandler = new ServerHandler(m_pSocket); }
	/** Destruktor. Nic nedělá, prostředky se uvolňují ve funkci OnExit() */
	~ServerThread() { }

	/** 
	 *	Vstupní funkce vlákna.
	 *	Funkce je volána automaticky při spuštění vlákna funkcí Run().
	 *	Volá metodu třídy ServerHandler pro zpracování požadavků klienta
	 */
	virtual ExitCode Entry() {
		return (ExitCode) m_pSHandler->HandleClient();
	};
	
	/** 
	 *	Výstupní funkce vlákna.
	 *	Funkce je automaticky volána při ukončení vlákna, a postará se o
	 *	zrušení klientského socketu s nímž vlákno pracovalo.
	 */
	virtual void OnExit() {
		m_pSocket->Destroy();
	}
};

#endif  // __DAMA_SERVER
