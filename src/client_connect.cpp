/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file client_connect.cpp
 * 
 * Implementace třídy pro komunikaci se serverem.
 * 
 * @version $Rev: 183 $
 ******************************************************************************/

#include "client_connect.h"
// event table

BEGIN_EVENT_TABLE(clSocket, wxEvtHandler)
  EVT_TIMER(666, clSocket::OnTimer)
END_EVENT_TABLE()


// fronta obsahujicí požadavky, které klient přijal
static wxMutex mutexClientRead;
std::queue<std::string> ClientReadQueue;

//fronta obsahujicí požadavky klienta k odeslání
static wxMutex mutexClientWrite;
std::queue<std::string> ClientWriteQueue;

//stav "živosti" vlákna
static wxMutex mutexClientKill;
bool ClientKill;

/**
 *	Úvodní nastavení ukazatele na instanci
 */
clSocket* clSocket::pinstance = 0;

/**
 * Registrace odběratele síťových zpráv
 */
void clSocket::RegisterHandler(Receiver * handler)
{
	std::cout << "HANDLER "<< handler << " ADDED\n";
	handlers.push_back(handler);
	handlers.unique();
}

/**
 * Odregistrování odběratele síťových zpráv
 */
void clSocket::UnregisterHandler(Receiver * handler)
{
	std::cout << "HANDLER "<< handler << " REMOVED\n";
	handlers.remove(handler);
}

/**
 *	Vytvoření / vrácení instance singletonu clSocket
 *
 *	Vrátí vždy tu stejnou instanci
 */
clSocket* clSocket::Instance () 
{
	if (pinstance == 0)  // je to první volání?
	{  
		pinstance = new clSocket; // vytvoří sole instanci
	}
	return pinstance; // adresa sole instance
}

/**
 *	Konstruktor.
 *	
 *	 Vytvoří časovač a zamezí ´spadnutí´ druhého vlákna
 */
clSocket::clSocket()
{
	casovac = new wxTimer(this, 666);
	ClientKill = false;
}

/**
 *	Destruktor
 *
 *	Zruší socket
 */
clSocket::~clSocket( void )
{
	ClientKill = true;
	//TODO: uvolnit pameť
}

/**
 *	ConnectClient.
 *
 *	Slouží k připojení klientského socketu k serveru
 *	@return true pokud se připojení zdařilo
 *	@return false jinak
 */
bool clSocket::ConnectClient(wxString c_hostname)
{
	hostname = c_hostname;
	ClientKill = false;
	ClientThread *cl_thread = new ClientThread(hostname);
	cl_thread->Create();
	cl_thread->Run();
	casovac->Start(10);
	return true;
}

/**
 *	SendRequest.
 *
 *	Odešle požadavek na server
 *	@param request požadavek, co se bude posílat na server
 *	@return true pokud se podařilo požadavek odeslat
 *	@return false jinak
 */
bool clSocket::SendRequest(std::string request)
{
	std::cout << "Odesílání zprávy:" << std::endl << request << std::endl;
	if(ClientKill)
	{
		std::cout << "CLIENT KILL ENGAGED" << std::endl;
	}
	// přístup ke sdíleným datům
	mutexClientWrite.Lock();
		ClientWriteQueue.push(request);
	mutexClientWrite.Unlock();
	return true;
}
/**
 *	ProcessRequests.
 *
 *	Zpracovaní události ze serveru, přeposílá události zaregistrovaným oknům
 *	@param event událost
 */
void clSocket::ProcessRequests()
{
	std::string zprava;
	// je zprava prijata?
	int isok = 0;
	int received= 0;

	// přístup ke sdíleným datům
	mutexClientRead.Lock();
		if(!ClientReadQueue.empty())
		{
			zprava = ClientReadQueue.front();
			ClientReadQueue.pop();
			received = 1;
		}
	mutexClientRead.Unlock();
	if(received)
	{
		std::cout << "Přijímání zprávy: " << std::endl << zprava << std::endl;
		
		// kopie seznamu registrovaných odběratelů zpráv (seznam se může měnit během Receive() )
		std::list<Receiver *> handlers2 (handlers.begin(),handlers.end());
		// všechna zaregistrovaná okna musí dostat síťovou zprávu
		std::list<Receiver *>::iterator it;
		for ( it=handlers2.begin() ; it != handlers2.end(); it++ )
		{
			if((*it)->Receive(zprava))
			{
				isok = 1;
			}
		}
		if(!isok)
		{
			std::cout << "Zpráva nebyla doručena.";
		}
	}
	if(handlers.empty())
	{
		casovac->Stop();
		ClientKill = 1;
	}
}

void clSocket::OnTimer(wxTimerEvent &event)
{
	ProcessRequests();
}

//
//	Druhé vlákno
//

/**
 *	ConnectClient.
 *
 *	Slouží k připojení klientského socketu k serveru
 *	@return true pokud se připojení zdařilo
 *	@return false jinak
 */
bool ClientThread::ConnectClient(void)
{
	wxIPV4address addr;
	sock = new wxSocketClient;//Client
	
	addr.Hostname(c_hostname);
	addr.Service(8007);	// port 8007
	if(sock->Connect(addr, false))	// neblokujicí připojení
		return true;
	if(!sock->WaitOnConnect(10))
		return false;
	return true;
}

/**
 *	HandleConnection.
 *
 *	Načítá a posílá data na server z a do sdílených seznamů
 *	@return 0 vždy
 */
int ClientThread::HandleConnection(void)
{
	char pomBuffer[REQUEST_BUFFER_SIZE];
	std::string request;
	
	while(true) {
		if(sock->IsDisconnected())
			break;
		
		mutexClientKill.Lock();
			if(ClientKill)
				break;
		mutexClientKill.Unlock();
		
		mutexClientWrite.Lock();
			if(ClientWriteQueue.size() != 0)
			{
				request = ClientWriteQueue.front();
				if(sock->WaitForWrite(0,200))
				{
					std::cout << "Thread: " << std::endl<< request << std::endl;
					sock->WriteMsg(request.c_str(), request.size()+1);
					if(sock->Error())
						std::cout << "Socket ERROR " << sock->LastError() << std::endl;
					if(sock->LastCount() != request.size()+1)
						break;
					ClientWriteQueue.pop();	// smazání požadavku z fronty
				}
			}
		mutexClientWrite.Unlock();
		
		if(sock->WaitForRead(0,200))
		{
			sock->ReadMsg(pomBuffer, REQUEST_BUFFER_SIZE-1);
			if(sock->LastCount() == 0)	// nejsou data -> přeskočit
				continue;
			pomBuffer[sock->LastCount()] = 0;
			request.assign(pomBuffer);
			
			mutexClientRead.Lock();
				ClientReadQueue.push(request);	// vložení načtených dat do sdíleného seznamu
			mutexClientRead.Unlock();
		}
	}
	
	mutexClientKill.Lock();	// nastavení flagu o zabití vlákna
		ClientKill = true;
	mutexClientKill.Unlock();

	mutexClientWrite.Lock();	// vymazání fronty požadavků
		while (!ClientWriteQueue.empty())
		{
	  	ClientWriteQueue.pop();
		}
	mutexClientWrite.Unlock();
	return 0;
}
