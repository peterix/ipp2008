/*******************************************************************************
 * ICP - Dáma server 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file server.cpp
 *
 * Implementace serveru a jeho vláken
 *
 * @version $Rev: 185 $
 ******************************************************************************/

#include "server.h"

// --------------------------------------------------------------------------
// vytvoření aplikace
// --------------------------------------------------------------------------

IMPLEMENT_APP_CONSOLE(ServerApp);

// ==========================================================================
// implementace
// ==========================================================================

// --------------------------------------------------------------------------
// třída serveru
// --------------------------------------------------------------------------

/**
 *	Vytvoření socketu pro server
 *
 *	Funkce vytvoří socket serveru na určitém portu
 *
 *	@return 0	jestliže došlo k chybě
 *	@return 1 jestliže vše proběhlo v pořádku
 */
int Server::CreateServerSocket(void)
{
	// vytvoření adresy pro port
	wxIPV4address addr;
	if(!addr.Service(port)) {
		std::cerr << "Chyba: Nepodarilo se nastavit port." << std::endl;
		return 0;
	}
	// vytvoření socketu
	serverSocket = new wxSocketServer(addr);
	if(!serverSocket->Ok()) // podařilo se vytvořit socket?
	{
		std::cerr << "Chyba: nelze vytvorit socket na portu " << port << "." << std::endl;
		serverSocket->Destroy();  // ukončení socketu
		return 0;
	}
	return 1;
}

/**
 *	Příjem přídchozích spojení
 *
 *	Funkce běží v nekonečné smyčce a čeká na příchozí spojení, pro která
 *	vytvoří samostatná vlákna, která se starají o zpracování jednotlivých
 *	klientů.
 */
void Server::AcceptConnection(void)
{
	while(true) {

		wxSocketBase * pSocket;
		// Volání Accept zablokuje provádění programu, dokud nepřijme příchozí spojení
		pSocket = serverSocket->Accept(true);

		if ( !pSocket ) { // Agent: ok, tady to po par minutách nečinnosti sletí
			std::cerr << "Oznameni: Vyprsel casovy limit pro spojeni, cekam na dalsi." << std::endl;
			continue;
		}

		// vytvoření nového vlákna pro klientský socket
		ServerThread *p_thread = new ServerThread(this, pSocket);
		p_thread->Create();
		// spuštění vlákna
		p_thread->Run();

  }
}

// --------------------------------------------------------------------------
// třída aplikace
// --------------------------------------------------------------------------

/**
 *	Inicializace aplikace
 */
bool ServerApp::OnInit()
{
	// inicializace socketů, aby s nimi bylo možné pracovat ve více vláknech
	wxSocketBase::Initialize();
	return true;
}

/**
 *	Náhrada funkce main
 */
int ServerApp::OnRun()
{
	Server *p_server = new Server();

	// vytvoření socketu pro server
	if(!p_server->CreateServerSocket())
		return 1;

	// příjem a obsluha spojeni na serverovém socketu
	p_server->AcceptConnection();

	delete p_server;
  // Success
	return 0;
}
