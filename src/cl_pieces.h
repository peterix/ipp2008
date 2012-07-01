/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_pieces.h
 * 
 * Třída pro práci s herními kameny.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/
 ///TODO: otestovat
 ///TODO: masivní refaktoring
 
#ifndef CL_PIECES
#define CL_PIECES

#include "cl_util.h"
#include "cl_board.h"

// obyčejný kámen
class Kamen
{
	private:
		Deska * board; // deska - pro zjišťování možných tahů
		int x;
		int y;
		barvaHrace barva;
		bool marked; /// označen pro odstranění na konci tahu?
		friend class Dama;
		bool GoJump(int dx,int dy); /// pomocná metoda pro skákání
	public:
		Kamen(int _x, int _y, Deska * _board,barvaHrace _barva, bool _marked = 0);
		
		void GetPozice(int & _x,int & _y);
		Point GetPozice();
		
		virtual typPole GetType();/// vrátí typ kamene
		virtual bool IsStone();/// vrací 1, když se jedná o kámen
		
		barvaHrace GetBarva();/// vrátí barvu kamene
		bool Enemy(barvaHrace _barva);
		bool IsBlack();/// vrací 1, pokud je kámen černý
		
		
		virtual bool CanJump();/// může kámen skákat?
		virtual bool CanMove();/// může se kámen pohybovat?
		
		bool IsMarked ();/// je označen pro odstranění (tzn. už byl jednou přeskočen)?
		void Mark (); /// označit pro odstranění
		
		virtual bool Go(int _x,int _y);/// pohyb kamenem (při neúspěchu vrátí 0, při úspěchu 1)
};

class Dama : public Kamen
{
	public:
		Dama(int x, int y, Deska * _board,barvaHrace barva) : Kamen(x,y,_board,barva){};
		typPole GetType();
		bool IsStone();/// vrací 0, když se jedná o dámu
		bool CanJump();
		bool CanMove();
		bool Go(int _x,int _y);
	private:
		bool CanJumpTrace(int dx, int dy);
		bool GoTrace(int dx, int dy, int _x, int _y);
};

#endif
