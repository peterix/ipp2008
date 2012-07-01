/*******************************************************************************
 * ICP - Dáma server 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file server_handler.cpp
 * 
 * Zpracování požadavků na serveru
 *
 * @version $Rev: 181 $
 ******************************************************************************/

#include "server_handler.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "ticpp.h"

// následující proměnné jsou globální, kvůli synchronizaci vláken

static wxMutex mutexPlayersList; /**< Mutex na kontrolu přístupu k seznamu hráčů */
std::map<std::string,bool> PlayersList;	/**< Seznam hráčů. Klíčem je přezdívka hráče a hodnotou je zda hráč s někým hraje (true/false). */
std::map<std::string,bool>::iterator PlayersListIterator; /**< Iterátor seznamu hráčů */

static wxMutex mutexRequestList;
/** Struktura obsahujicí data potřebná pro výměnu požadavků */
struct RequestListStruct {
	std::string request;	/**< Řetězec s XML požadavkem */
	std::string from;	/**< Přezdívka hráče, od kterého požadavek přichází */
} ;
std::multimap<std::string, RequestListStruct> RequestList; /**< Seznam požadavků. Klíčem je přezdívka hráče a hodnotou struktura RequestListStruct */
std::multimap<std::string, RequestListStruct>::iterator RequestListIterator; /**< Iterátor seznamu požadavků*/

static wxMutex mutexRegisteredPlayers;	/**< Mutex na přístup k souboru s registrovanými hráči */
static wxMutex mutexSavedGames;	/**< Mutex na přístup k souboru se seznamem uložených her */

static wxMutex mutexChatList;	/**< Mutex pro přístup k seznamu se zprávami chatu */
std::map<int, std::string> ChatList;	/**< Seznam se zprávami chatu */
int ChatList_last;	/**< Index poslední zprávy v chatu */

static wxMutex mutexGamesList;	/**< Mutex pro přístup k seznamu her */
/** Struktura obsahujicí data potřebná k uchovávání stavů her */
struct GamesListStruct {
	std::string whitePlayer;	/**< Jméno bílého hráče */
	std::string blackPlayer;	/**< Jméno černého hráče */
	Deska *gameDesk;	/**< Herní plocha */
	int game_id;	/**< Číslo hry */
} ;
std::map<std::string, GamesListStruct> GamesList;	/**< Seznam her. Klíčem jsou spojené přezdívky hráčů, pravidlem < jako první a hodnotou struktura GamesListStruct */
std::map<std::string, GamesListStruct>::iterator GamesListIterator;	/**< Iterátor seznamu her */

/**
 *	Práce s klientem
 *
 *	Stará se o komunikaci s klientem (je volána v rámci každého vlákna)
 *	@param	p_clientSocket	ukazatel na socket, přes který se bude komunikovat s klientem
 *	@param	p_thread	ukazatel na vlákno, ve kterém funkce běží
 *	@return 0 jestliže vše proběhlo v pořádku
 *	@return 1 jestliže nastala chyba
 */
int ServerHandler::HandleClient(void)
{
	char pomBuffer[REQUEST_BUFFER_SIZE];
	RequestListStruct pomStruct;
	std::string request;
	std::string opponent;
	
	while(true) {
		if(p_clientSocket->IsDisconnected())	// kontrola zda je klient připojen
			break;

		if(chat_enabled) {
			mutexChatList.Lock();	// zamčení přístupu k chatu

				if((chat_last+10) < ChatList_last) {	// nastavení posledního záznamu, který se má klientovi poslat
					chat_last = ChatList_last - 10;
					if(chat_last < 0)
						chat_last = 0;
				}

				if(ChatList.find(chat_last+1) != ChatList.end()) {	// nalezen záznam v seznamu chatu
					if(p_clientSocket->WaitForWrite(0,200)) {	// posláni zprávy klientovi
						p_clientSocket->WriteMsg(ChatList[chat_last+1].c_str(), ChatList[chat_last+1].size()+1);

						if(p_clientSocket->LastCount() != ChatList[chat_last+1].size()+1) {
							std::cerr << "Chyba: Problem pri odesilani zpravy chatu." << std::endl;
							mutexChatList.Unlock();
							break;
						}
					}
					chat_last++;
				}

			mutexChatList.Unlock();	// odemčení přístupu k seznamu požadavků
		}

		if(p_clientSocket->IsDisconnected())	// kontrola zda je klient připojen
			break;

		mutexRequestList.Lock();	// zamčení přístupu k seznamu požadavků

			RequestListIterator = RequestList.find(playersName);	// vyhledání záznamu v seznamu požadavků
			if(RequestListIterator != RequestList.end()) {	// nalezen záznam v seznamu požadavků
				if(p_clientSocket->WaitForWrite(0,200)) {
					p_clientSocket->WriteMsg(RequestListIterator->second.request.c_str(), RequestListIterator->second.request.size()+1);

					if(p_clientSocket->LastCount() != RequestListIterator->second.request.size()+1) {
						std::cerr << "Chyba: Problem pri odesilani requestu uzivateli." << std::endl;
						mutexRequestList.Unlock();
						break;
					}
				}
				RequestList.erase(RequestListIterator);	// smazání přeposlaného požadavku
			}

		mutexRequestList.Unlock();	// odemčení přístupu k seznamu požadavků
		
		if(p_clientSocket->IsDisconnected())	// kontrola, zda je klient stále připojen
			break;

		if(p_clientSocket->WaitForRead(0,200)) {	// ceka 200 ms na vstup

			// načtení dat z klientského socketu
			p_clientSocket->ReadMsg(pomBuffer, REQUEST_BUFFER_SIZE-1);
			if(p_clientSocket->LastCount() == 0)
				continue;
			pomBuffer[p_clientSocket->LastCount()] = 0;
			request.assign(pomBuffer);

			pomBuffer[0] = 0;

			///////// sem vložit přechroustání dat
			if(ParseRequest(request, opponent))
				continue;
			// příprava dat na vložení do seznamu požadavků
			pomStruct.request = request;
			pomStruct.from = playersName;
			
			bool isOpponentOnline = true;
			mutexPlayersList.Lock();
				if(PlayersList.find(opponent) == PlayersList.end())	// je protihráč připojen?
					isOpponentOnline = false;
			mutexPlayersList.Unlock();

			if(isOpponentOnline) {	// kontrola zda je protihráč připojen
				mutexRequestList.Lock();	// uzamčení seznamu požadavků
					RequestList.insert(std::pair<std::string,RequestListStruct>(opponent, pomStruct));	// vložení nového požadavku
				mutexRequestList.Unlock();	// odemčení seznamu požadavků
			}
		}
	}

	// úklid uživatelových dat ze sdílených seznamů
	mutexPlayersList.Lock();
		PlayersList.erase(playersName);	// odstranění hráče ze seznamu online hráčů
	mutexPlayersList.Unlock();

	mutexRequestList.Lock();
		RequestList.erase(playersName);	// vymazání požadavků pro tohoto hráče
	mutexRequestList.Unlock();

	mutexGamesList.Lock();	// vymazání her z paměti
		for(GamesListIterator = GamesList.begin(); GamesListIterator != GamesList.end(); GamesListIterator++) {
			if(GamesListIterator->second.whitePlayer == playersName || GamesListIterator->second.blackPlayer == playersName) {
				GamesList.erase(GamesListIterator);	// odstranění hry z paměti
			}
		}
	mutexGamesList.Unlock();
	return 0;	// všechno v pořádku
}

/**
 *	Parsování požadavků klienta.
 *
 *	Provede rozparsování požadavků klienta a nastaví příslušné hodnoty do sdílených seznamů.
 *	@param request požadavek přijatý od klienta
 *	@param p_thread ukazatel na vlákno v kterém funkce běží
 *	@return 0 jestliže je potřeba požadavek přeposlat druhému klientovi
 *	@return 1 jestliže není třeba požadavek posílat druhému klientovi
 */
int ServerHandler::ParseRequest(std::string & request, std::string & opponentsName)
{
	std::string pom_str;
	std::string pom_str2;
	
	ticpp::Document doc;
	try {
		doc.Parse(request);	// vložení requestu do parseru TinyXML++
	} catch( ticpp::Exception& ex ) {
		return 1;
	}
	
	ticpp::Element *root_element = doc.FirstChildElement();	// získání prvního elementu
	pom_str = root_element->Value();	// získání hodnoty prvního elementu
	
	// ~~~registrace~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if(pom_str == "register") {
		pom_str = root_element->GetAttribute("username");	// získání hodnoty atributu username
		pom_str2 = root_element->GetAttribute("password");	// získání hodnoty atributu password
		
		mutexRegisteredPlayers.Lock();
			int ret_code = RegisteredPlayers(pom_str, pom_str2, true);	// provedení registrace uživatele
		mutexRegisteredPlayers.Unlock();

		switch(ret_code) {
			case 0:	// poslat uživateli zprávu, že vše proběhlo v pořádku
				pom_str = "<register success=\"true\" />";
				break;
			case 1:	// uživatel již existuje poslat bug
			case 2:	// poslat bug
				pom_str = "<register success=\"false\" bugnumber=\"";
				pom_str.push_back('0'+ret_code);	//vložení čísla návratového kódu
				pom_str.append("\" />");
				break;
			default: break;	// nemělo by nastat
		}

	// ~~~přihlášení~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "login") {
		pom_str = root_element->GetAttribute("username");	// získání hodnoty atributu username
		pom_str2 = root_element->GetAttribute("password");	// získání hodnoty atributu password
		
		mutexRegisteredPlayers.Lock();
			int ret_code = RegisteredPlayers(pom_str, pom_str2, false);	// zjištění, zda je uživatel registrovaný
		mutexRegisteredPlayers.Unlock();
		mutexPlayersList.Lock();
			if(PlayersList.find(pom_str) != PlayersList.end())
				ret_code = 1;	// už je jednou přihlášen
		mutexPlayersList.Unlock();

		switch(ret_code) {
			case 0:	// poslat uživateli zprávu, že vše proběhlo v pořádku
				playersName = pom_str;	// uložení jména hráče
				// přidání hráče do seznamu online hráčů
				mutexPlayersList.Lock();
					PlayersList.insert(std::pair<std::string,bool>(pom_str, false));
				mutexPlayersList.Unlock();

				pom_str = "<login success=\"true\" />";
				break;
			case 1:	// špatné jméno nebo heslo poslat bug
			case 2:	// poslat bug
				pom_str = "<login success=\"false\" bugnumber=\"";
				pom_str.push_back('0'+ret_code);	//vložení čísla návratového kódu
				pom_str.append("\" />");
				break;
			default: break;	// nemělo by nastat
		}

	// ~~~nastartování chatu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "chatstart") {
			chat_enabled = true;	// povolení chatu
		return 1;	// požadavek neposílat dalšímu hráči

	// ~~~chat~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "chat") {
		mutexChatList.Lock();	// uzamčení seznamu s chatem

			if(ChatList.size() == 0)	// nastavení posledního indexu
				ChatList_last = 0;
			ChatList_last++;	// zvýšení indexu dalšího prvku
			// vložení záznamu do seznamu s chatem
			ChatList.insert(std::pair<int,std::string>(ChatList_last, request));

		mutexChatList.Unlock();	// uzamčení seznamu s chatem
		return 1;	// požadavek neposílat dalšímu hráči

	// ~~~seznam online hráčů~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "playerslist") {
		ticpp::Document xmlPlayersList;	// vytvoření nového xml
		ticpp::Element *top_element = new ticpp::Element("playerslist");
		xmlPlayersList.LinkEndChild(top_element);
		ticpp::Element *element;
		
		mutexPlayersList.Lock();
			// procházení seznamu hráčů
			int pl_pom = 0;
			for(PlayersListIterator = PlayersList.begin(); PlayersListIterator != PlayersList.end(); PlayersListIterator++) {
				if(PlayersListIterator->first != playersName) {	// do seznamu nepřidávat tohoto klienta
					element = new ticpp::Element("player");
					element->SetText(PlayersListIterator->first);
					top_element->LinkEndChild(element);
				}
				pl_pom++;
				if(pl_pom == 51)	// omezení délky seznamu
					break;
			}
		mutexPlayersList.Unlock();

		TiXmlPrinter printer; 
		xmlPlayersList.Accept( &printer ); 
		pom_str = printer.CStr();	// uložení výstupu TinyXML++ do řetězce

	// ~~~vyzvání ke hře~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "duel") {
		pom_str = root_element->GetAttribute("accept");
		if(pom_str.size() == 0) {	// není atribut accept -> request pouze přeposlat
			opponentsName = root_element->GetAttribute("with");
			return 0;	// request se přepošle dalšímu hráči
		} else {
			if(pom_str == "true") {	// výzva přijata
				// vytvořit novou hru
				pom_str = root_element->GetAttribute("player");
				pom_str2 = root_element->GetAttribute("with");
				
				std::string gameName;
				if(pom_str < pom_str2)	// vytvoření názvu hry ze jmen hráčů
					gameName = pom_str + pom_str2;
				else
					gameName = pom_str2 + pom_str;

				mutexSavedGames.Lock();	// zjištění, zda již neexistuje (když ne, automaticky vytvoří novou)
					int gameNumber = SavedGames(pom_str, pom_str2);

					GamesListStruct pom_glstruct;
					pom_glstruct.game_id = gameNumber;
					pom_glstruct.gameDesk = new Deska(false);
					if(!LoadGame(pom_glstruct))
						return 1;	// chyba na straně serveru, nic neposílat
				mutexSavedGames.Unlock();
				
				mutexGamesList.Lock();	// uzamčení seznamu her
					if(GamesList.find(gameName) == GamesList.end()) {	// hra neexistuje -> vytvořit
						GamesList[gameName] = pom_glstruct;
					} else {	// přepsání stávajicí hry
						delete GamesList[gameName].gameDesk;	// smazání původní hry
						GamesList[gameName] = pom_glstruct;
					}
					
				mutexGamesList.Unlock();	// odemčení seznamu her
			}
			opponentsName = root_element->GetAttribute("player");	// získání jména protihráče
			return 0;	// request se přepošle protihráči
		}

	// ~~~získání seznamu klientových her~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "getgames") {
		std::vector<std::string> client_games;
		std::vector<std::string>::iterator client_games_it;
		
		mutexSavedGames.Lock();
			client_games = GetOpponents(playersName);	// získání vektoru protihráčů
		mutexSavedGames.Unlock();
		
		ticpp::Document xmlPlayersList;	// vytvoření nového xml
		ticpp::Element *top_element = new ticpp::Element("gameslist");
		xmlPlayersList.LinkEndChild(top_element);
		ticpp::Element *element;

		for(client_games_it = client_games.begin(); client_games_it != client_games.end(); client_games_it++) {
			element = new ticpp::Element("gamewith");
			element->SetText(*client_games_it);
			top_element->LinkEndChild(element);
		}

		TiXmlPrinter printer; 
		xmlPlayersList.Accept( &printer ); 
		pom_str = printer.CStr();	// uložení výstupu TinyXML++ do řetězce

	// ~~~získání stavu hry s druhým hráčem~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "getgame") {
		pom_str = root_element->GetAttribute("with");	// s kým je hra?
		if(pom_str.size() == 0)	// špatný request od klienta
			return 1;

		mutexSavedGames.Lock();	// zjištění, zda již neexistuje (když ne, automaticky vytvoří novou)
			int gameNumber = SavedGames(playersName, pom_str);

			GamesListStruct pom_glstruct;
			pom_glstruct.game_id = gameNumber;
			pom_glstruct.gameDesk = new Deska(false);
			if(!LoadGame(pom_glstruct))
				return 1;	// chyba na straně serveru, nic neposílat
		mutexSavedGames.Unlock();

		// vytvoření xml seznamu, který se pošle klientovi
		ticpp::Document getgame_xml;
		try {
			ticpp::Element* topElement = new ticpp::Element("savedgame");	// kořenový element
			topElement->SetAttribute("with", pom_str);
			getgame_xml.LinkEndChild(topElement);
    
			ticpp::Element* element = new ticpp::Element("white");	// bílý hráč
			element->SetAttribute("nickname", pom_glstruct.whitePlayer);
			topElement->LinkEndChild(element);
    
			element = new ticpp::Element("black");	// černý hráč
			element->SetAttribute("nickname", pom_glstruct.blackPlayer);
			topElement->LinkEndChild(element);
    
			element = new ticpp::Element("gamestate");	// stav hry
			element->SetText(pom_glstruct.gameDesk->GetStav());
			topElement->LinkEndChild(element);
		} catch ( ticpp::Exception& ex ) {
			std::cerr << "XML parser error: " << ex.what() << std::endl;
			return 1;	// chyba na straně serveru, nic neposílat
		}

		TiXmlPrinter printer; 
		getgame_xml.Accept( &printer ); 
		pom_str = printer.CStr();	// uložení výstupu TinyXML++ do řetězce

	// ~~~tah kamenem~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else if(pom_str == "move") {
		pom_str = root_element->GetAttribute("player");	// získání jména protihráče
		int x=-1, y=-1;
		root_element->GetAttribute("x", &x, false);	// načtení x-ové souřadnice
		root_element->GetAttribute("y", &y, false);	// načtení y-ové souřadnice
		bool load = true;	// flag zda je potřeba natáhnout hru do paměti
		GamesListStruct glStruct;	// pomocná struktura pro získání stavu hry

		std::string gameName;
		if(pom_str < playersName)	// vytvoření názvu hry ze jmen hráčů
			gameName = pom_str + playersName;
		else
			gameName = playersName + pom_str;

		mutexGamesList.Lock();
				if(GamesList.find(gameName) != GamesList.end()) {	// hra nalezena -> nenačítat
					load = false;
				}
		mutexGamesList.Unlock();

		if(load) {	// je třeba načíst hru
			mutexSavedGames.Lock();
				int gameNumber = SavedGames(playersName, pom_str);

				GamesListStruct pom_glstruct;
				pom_glstruct.game_id = gameNumber;
				pom_glstruct.gameDesk = new Deska(false);
				if(!LoadGame(pom_glstruct))
					return 1;	// chyba na straně serveru, nic neposílat
			mutexSavedGames.Unlock();
			mutexGamesList.Lock();
				GamesList[gameName] = pom_glstruct;	// uložení hry do seznamu her
			mutexGamesList.Unlock();
		}

		mutexGamesList.Lock();
			GamesList[gameName].gameDesk->Input(x,y);	// provedení operace
			glStruct = GamesList[gameName];	// uložení hry do pomocné struktury
		mutexGamesList.Unlock();
		
		mutexSavedGames.Lock();
			if(!SaveGame(glStruct))	// uložení hry na serveru
				return 1;
		mutexSavedGames.Unlock();
		
		opponentsName = pom_str;	// nastavení jména protihráče

		std::stringstream ss;	// stream na vytvoření nového requestu pro protihráče
		ss << "<move x=\"" << x << "\" y=\"" << y << "\" player=\"" << playersName << "\" />";
		request = ss.str();
		
		return 0;
		
	// ~~~neznámý požadavek~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	} else {
		return 1;	// neznámý požadavek -> zahodit
	}

	// zapsání dat na socket (data se posílají tomuto klientovi - proto je zápis zde)
	if(p_clientSocket->WaitForWrite(0,200)) {	// čekání na zápis 200 mili-sekund potom se požadavek zahodí
		p_clientSocket->WriteMsg(pom_str.c_str(), pom_str.size()+1);

		if(p_clientSocket->LastCount() != pom_str.size()+1) {
			std::cerr << "Chyba: Pri odesilani pozadavku." << std::endl;
		}
	}
	
	return 1;
}

/**
 *	Parser souboru s registrovanými hráči.
 *
 *	Načte soubor s registrovanými hráči. Vrací zda je uživatel registrován, či ne. Popřípadě
 *	pokud je zvoleno, že jde o registraci je nový uživatel do souboru přidán.
 *
 *	@param nickname jméno (přezdívka) hráče
 *	@param password heslo hráče
 *	@param registration flag zda jde o registraci(true), či pouze přihlášení(false), defaultně false
 *	@return 0 operace proběhla v pořádku
 *	@return 1 uživatel již existuje / špatně zadané jméno či heslo
 *	@return 2	chyba při parsování xml souboru
 */
int ServerHandler::RegisteredPlayers(std::string nickname, std::string password, bool registration)
{
	ticpp::Document doc("./players.xml");
	
	try {
		doc.LoadFile();	// naloadování souboru

		ticpp::Element *root_element = doc.FirstChildElement("players");
		ticpp::Iterator< ticpp::Element > child("player");
		if(registration) {	// jde o registraci

			for ( child = child.begin( root_element ); child != child.end(); child++ ) {	// prohledávání seznamu hráčů
				ticpp::Element *nick = child->FirstChildElement("login");	// získání elementu - login
				if(nick->GetText(false) == nickname)	// nalezen záznam
					return 1;
			}
			ticpp::Element *new_player = new ticpp::Element("player");	// vytvoření elementu <player>
			new_player->SetText("");
			root_element->LinkEndChild(new_player);

			ticpp::Element *new_element = new ticpp::Element("login");	// vytvoření elementu <login>
			new_element->SetText(nickname);
			new_player->LinkEndChild(new_element);	// přilepení elementu na element <player>

			new_element = new ticpp::Element("password");	// vytvoření elementu <password>
			new_element->SetText(password);
			new_player->LinkEndChild(new_element);	// přilepení elementu na element <player>

			doc.SaveFile();	// uložení souboru
			return 0;

		} else {	// jde o login

			for ( child = child.begin( root_element ); child != child.end(); child++ ) {
				ticpp::Element *nick = child->FirstChildElement("login");	// získání elementu - login
				ticpp::Element *pass = child->FirstChildElement("password");	// získání elementu - password

				if(nick->GetText(false) == nickname) {	// stejný nick?
					if(pass->GetText(false) == password)	// stejné heslo?
						return 0;
					return 1;	// špatné heslo
				}
			}
			return 1;
		}
	} catch( ticpp::Exception& ex ) {
		std::cerr << "XML parser error: " << ex.what() << std::endl;
		return 2;
	}
	return 0;	// všechno v pořádku, i když by se to sem nikdy nemělo dostat :)
}

/**
 *	Získání čísla hry.
 *
 *	Získá číslo rozehrané hry pro zadané hráče.
 *	@param player1 přezdívka hráče
 *	@param player2 přezdívka hráče
 *	@return kladné pokud vše proběhlo v pořádku
 *	@return záporné pokud došlo k chybě
 */
int ServerHandler::SavedGames(std::string player1, std::string player2) {

	std::string pom_str;
	std::string pom_str2;
	int last_index = 0;	// index poslední hry
	int pom_index = 0;	// pomocný index
	
	if(player1 > player2) {
		pom_str = player1;
		player1 = player2;
		player2 = pom_str;
	}
	
	ticpp::Document doc("./savedgames.xml");
	
	try {
		doc.LoadFile();

		ticpp::Element *root_element = doc.FirstChildElement("games");
		ticpp::Element *index_element = root_element->FirstChildElement("index");
		index_element->GetAttribute("last", &last_index);	// získání indexu poslední hry

		ticpp::Iterator< ticpp::Element > child("game");
		for ( child = child.begin( root_element ); child != child.end(); child++ ) {	// prohledávání seznamu her
			child->GetAttribute("player1", &pom_str);	// načtení jména prvního hráče
			child->GetAttribute("player2", &pom_str2);	// načtení jména druhého hráče
			child->GetAttribute("index", &pom_index);	// načteni id hry
			
			if(player1 == pom_str && player2 == pom_str2)
				return pom_index;
		}
		// sem se dostane pouze pokud nebyl nalezen žádný záznam -> vytvoří se nový
		last_index++;	// vložíme další prvek
		ticpp::Element *new_game = new ticpp::Element("game");	// vytvoření elementu game
		// nastavení atributů
		new_game->SetAttribute("player1", player1);
		new_game->SetAttribute("player2", player2);
		new_game->SetAttribute("index", last_index);
		root_element->LinkEndChild(new_game);	// nalinkování elementu game na element games
		
		index_element->SetAttribute("last", last_index);	// přenastavení indexu poslední hry

		doc.SaveFile();

		// je třeba navíc vytvořit novou rozehranou hru a uložit ji na server
		GamesListStruct new_game_struct;
		new_game_struct.whitePlayer = player1;
		new_game_struct.blackPlayer = player2;
		new_game_struct.game_id = last_index;
		new_game_struct.gameDesk = new Deska(true);
		SaveGame(new_game_struct);
		
	} catch( ticpp::Exception& ex ) {	// výjimka xml parseru
		std::cerr << "XML parser error: " << ex.what() << std::endl;
		return -1;
	}
	return last_index;
}

/**
 *	Získání jmen protihráčů.
 *
 *	Získá jména protihráčů a vrátí je ve vektoru.
 *	@param player přezdívka hráče
 *	@return opponents vektor protihráčů, v případě chyby prázdný vektor
 */
std::vector<std::string> ServerHandler::GetOpponents(std::string player)
{
	std::vector<std::string> opponents;	// vektor protihráčů
	// pomocné řetězce pro uložení jmen
	std::string pom_str;
	std::string pom_str2;

	ticpp::Document doc("./savedgames.xml");
	
	try {
		doc.LoadFile();

		ticpp::Element *root_element = doc.FirstChildElement("games");

		ticpp::Iterator< ticpp::Element > child("game");
		for ( child = child.begin( root_element ); child != child.end(); child++ ) {	// prohledávání seznamu her
			child->GetAttribute("player1", &pom_str);	// načtení jména prvního hráče
			child->GetAttribute("player2", &pom_str2);	// načtení jména druhého hráče
			
			// vložení protihráče do vektoru
			if(player == pom_str) {
				opponents.push_back(pom_str2);
			} else if(player == pom_str2) {
				opponents.push_back(pom_str);
			}
		}
		
	} catch( ticpp::Exception& ex ) {	// výjimka xml parseru
		std::cerr << "XML parser error: " << ex.what() << std::endl;
		opponents.clear();	// vymazání vektoru
		return opponents;
	}
	
	return opponents;	// vrácení vektoru protihráčů
}

/**
 *	Načtení hry
 *
 *	Načte stav hry do struktury zadané struktury GamesListStruct
 *	@param &game struktura do které se uloží načtená data
 *	@return 1 když všechno proběhne v pořádku
 *	@return 0 jinak
 */
int ServerHandler::LoadGame(GamesListStruct & game)
{
	if(game.game_id == -1)	// není nastaveno kterou hru načíst
		return 0;

	std::stringstream ss;	// stream na poskládání jména souboru se hrou
	ss << "./game" << game.game_id << ".xml";
	std::string white_player;
	std::string black_player;
	std::string gamestate;
	
	ticpp::Document doc(ss.str());
	try {
		doc.LoadFile();
		ticpp::Element *root_element = doc.FirstChildElement("savedgame");
		ticpp::Element *element = root_element->FirstChildElement("white");
		white_player = element->GetAttribute("nickname");	// načtení jména bílého hráče

		element = root_element->FirstChildElement("black");
		black_player = element->GetAttribute("nickname");	// načtení jména černého hráče

		element = root_element->FirstChildElement("gamestate");
		gamestate = element->GetText();	// načtení stavu herní desky
		
		// uložení hodnot do herní struktury
		game.whitePlayer = white_player;
		game.blackPlayer = black_player;
		game.gameDesk->SetStav(gamestate);
		
	} catch ( ticpp::Exception& ex ) {
		std::cerr << "XML parser error: " << ex.what() << std::endl;
		return 0;
	}
	return 1;	// všechno v pořádku
}

/**
 *	Uložení hry
 *
 *	Uloží stav hry ze struktury zadané struktury GamesListStruct
 *	@param &game struktura ze které se uloží data
 *	@return 1 když všechno proběhne v pořádku
 *	@return 0 jinak
 */
int ServerHandler::SaveGame(GamesListStruct game)
{
	if(game.game_id == -1)	// není nastaveno kterou hru načíst
		return 0;

	std::stringstream ss;
	ss << "./game" << game.game_id << ".xml";
	
	ticpp::Document doc(ss.str());
	try {
		ticpp::Declaration* declaration = new ticpp::Declaration("1.0", "utf-8", "");	// deklarace
		doc.LinkEndChild(declaration);
    
		ticpp::Element* topElement = new ticpp::Element("savedgame");	// kořenový element
		doc.LinkEndChild(topElement);
    
		ticpp::Element* element = new ticpp::Element("white");	// bílý hráč
		element->SetAttribute("nickname", game.whitePlayer);
		topElement->LinkEndChild(element);
    
		element = new ticpp::Element("black");	// černý hráč
		element->SetAttribute("nickname", game.blackPlayer);
		topElement->LinkEndChild(element);
    
		element = new ticpp::Element("gamestate");	// stav hry
		element->SetText(game.gameDesk->GetStav());
		topElement->LinkEndChild(element);
    
		doc.SaveFile();
	} catch ( ticpp::Exception& ex ) {
		std::cerr << "XML parser error: " << ex.what() << std::endl;
		return 0;
	}
	return 1;	// všechno v pořádku
}
