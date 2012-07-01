/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file client_connect.h
 * 
 * Třída pro komunikaci se serverem.
 * 
 * Jedná se o třídu využívající návrhový vzor Singleton.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/

#ifndef CLIENT_CONNECT_H
#define CLIENT_CONNECT_H

#include <wx/event.h>
#include <wx/timer.h>
#include <wx/socket.h>
#include <iostream>
#include <list>
#include <queue>
#include "server_handler.h"
#include "cl_receiver.h"

class ClientThread;

/**
 *	Singleton pro práci se sítí.
 *
 *	Zapouzdřuje metody pro práci s klientským socketem.
 */
class clSocket : public wxEvtHandler
{
	public:
		/**
		 *	Získání instance singletonu 
		 *
		 *	@return ukazatel na clSocket
		 */
		
		static clSocket* Instance();
		
		/**
		 *	Připojení k serveru
		 *
		 *	@return informace jestli se připojení zdařilo
		 */
		
		bool ConnectClient(wxString c_hostname);
		
		/**
		 *	Získání instance singletonu 
		 *
		 *	@param request řetězec s dotazem na server
		 *
		 *	@return podařilo se odeslat?
		 */
		bool SendRequest(std::string request);
		
		void RegisterHandler(Receiver * handler);
		void UnregisterHandler(Receiver * handler);
		
		wxString getHostname() { return hostname; };
	protected:
		clSocket();	// konstruktory
		clSocket(const clSocket&);
		clSocket& operator= (const clSocket&);
		
		~clSocket();	// destruktor
		DECLARE_EVENT_TABLE()
	private:
		void ProcessRequests(); // zprácovaní požadavků
		static clSocket* pinstance; // instance singletonu
		wxString hostname;	// adresa serveru, kam se připojit
		std::list<Receiver *> handlers; // seznam kam vsude se ma preposilat zprava o eventech
		wxTimer * casovac;
	public:
		virtual void OnTimer(wxTimerEvent &event);
};

/**
 *	Třída definujicí klientovo "Detached" vlákno
 */
class ClientThread : public wxThread
{
	wxSocketClient* sock;  /**< Socket pro read/write operace */
	wxString c_hostname;

	bool ConnectClient(void);
	int HandleConnection(void);
public:
	/** Konstruktor. */
	ClientThread(wxString phostname) { c_hostname = phostname; }
	/** Destruktor. */
	~ClientThread() { }

	/** 
	 *	Vstupní funkce vlákna.
	 *	Funkce je volána automaticky při spuštění vlákna funkcí Run().
	 */
	virtual ExitCode Entry() {
		if(!ConnectClient())	// připojení k socketu
			return (ExitCode) 1;
		
		return (ExitCode) HandleConnection();
	};
	
	/** 
	 *	Výstupní funkce vlákna.
	 *	Funkce je automaticky volána při ukončení vlákna, a postará se o
	 *	zrušení socketu s nímž vlákno pracovalo.
	 */
	virtual void OnExit() {
		sock->Destroy();
	}
};




#endif

