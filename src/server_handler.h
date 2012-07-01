/*******************************************************************************
 * ICP - Dama server 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file server_handler.cpp
 *
 * Hlavičkový soubor: Zpracování požadavků na serveru
 * 
 * @version $Rev: 168 $
 ******************************************************************************/
#ifndef __SERVER_HANDLER
#define __SERVER_HANDLER

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <wx/thread.h>
#include <wx/socket.h>
#include <wx/file.h>
#include "cl_board.h"

/** Makro definující velikost bufferu pro informace posílané přes sockety. */
#define REQUEST_BUFFER_SIZE	4096

struct GamesListStruct;

/**
 *	Třída definujicí zpracování klientských požadavků na serveru
 */
class ServerHandler
{
	wxSocketBase* p_clientSocket;	/**< Socket, na kterém je napojen klient. */
	std::string playersName;	/**< Jméno (přezdívka) klienta. */
	int chat_last;	/**< Index poslední zprávy chatu, kterou klient přečetl */
	bool chat_enabled;	/**< Flag zda je chat povolen */
	
public:
	/** Konstruktor. Nastavuje se pomocí něj adresa klientského socketu,
	 *	s kterým má třída komunikovat. */
	ServerHandler(wxSocketBase* p_cliSock) :
		p_clientSocket(p_cliSock) {chat_last = 0; chat_enabled = false;}
	/** Destruktor. Bez akce.*/
	~ServerHandler() { }

	// funkce pro obsluhu klienta
	int HandleClient(void);

private:
	int ParseRequest(std::string & request, std::string & opponentsName);
	int RegisteredPlayers(std::string nickname, std::string password, bool registration = false);
	int SavedGames(std::string player1, std::string player2);
	std::vector< std::string > GetOpponents(std::string player);
	int LoadGame(GamesListStruct & game);
	int SaveGame(GamesListStruct game);
};

#endif
