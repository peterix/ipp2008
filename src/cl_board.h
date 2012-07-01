/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_board.h
 *
 * Třída pro práci s herní deskou.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/
 
#ifndef CL_BOARD
#define CL_BOARD

#include <list>
#include <string>

#include "cl_util.h"

class Kamen;

enum navratovaHodnota
{
	SPATNY_TAH = 0,
	SKAKANI = 1,
	HRAC_UCHOPIL_KAMEN = 2,
	BILY_PROHRAL = 3,
	CERNY_PROHRAL = 4,
	HRA_POKRACUJE = 5
};

class Deska
{
	public:
		/// vytvoří novou herní plochu
		Deska( bool init );
		
		/// zruší herní plochu
		~Deska();
		
		// GET/SET
		void SetStav(std::string stav);
		std::string GetStav( void );

		// kdo vlastní políčko xy?
		barvaHrace GetPlayer(int x, int y);
		
		// co je na políčku xy
		typPole GetField(int x, int y);
		
		// získej kámen z políčka xy
		Kamen *GetSutr(int x, int y);
		
		// kdo je zrovna na tahu?
		barvaHrace GetCurrentPlayer();
		
		/// obsahuje políčko XY aktivní (použitelný) kámen?
		bool IsActive(int x, int y);
		
		/// obsahuje políčko XY kámen, kterým se právě táhne?
		bool IsGrabbed(int x, int y);

		/// vrátí pozici chyceného kamene
		Point GetGrabbed();
		
		/// obsahuje políčko XY kámen označený ke smazání?
		bool IsMarked(int x, int y);
		
		/// jediný zvenku povolený vstup - kliknutá pozice. pokud dojde ke změně, vrací 1. jinak vrací 0
		navratovaHodnota Input(int x, int y);
		
		/// zjistí počet bílých kamenů na desce
		int GetWhite(void);
		/// zjistí počet černých kamenů na desce
		int GetBlack(void);
		
	private:
		// označí kámen pro odstranění za předpokladu, že již neni označen a pole XY je platné
		bool Mark (int x, int y);
		// nastavení hráče na tahu
		void SetCurrentPlayer( barvaHrace barva );
		// prohození obsahu dvou políček
		void Swap (int x1,int y1,int x2,int y2);
		
		friend class Kamen;
		friend class Dama;
		
		// přemaže seznam aktivních kamenů
		void ClearActive(void);
		/// zruší všechny kameny označené k odstranění, upustí kámen, povýší kameny na dámy a předá kontrolu druhému hráči
		void DokonciTah(void);
		/// získej seznam pozic použitelných kamenů
		void UpdateActive();
		/// počet bílých kamenů na desce
		int numWhite;
		/// počet černých kamenů na desce
		int numBlack;
		/// počet označených kamenů na desce
		int numMarked;
		/// deska na kameny
		Kamen * pole[8][8];
		// hráč má v 'ruce' kámen
		Kamen * grabbed; 
		barvaHrace naTahu; // který hráč je na tahu?
		std::list<Point> *active;
};
#endif
