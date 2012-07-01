/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_board.cpp
 * 
 * Implementace třídy pro práci s herní deskou.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/
#include <list>
#include <iostream>
#include "cl_board.h"
#include "cl_pieces.h"
#include <sstream>

using namespace std;

/**
 *	Konstruktor hrací desky
 *
 *	@param	init	určuje, jestli se mají na desku rozestavit kameny
 */
Deska::Deska( bool init )
{
	for(int i = 0;i<BOARDSIZE;i+=1)
		for(int j = 0;j<BOARDSIZE;j+=1)
		{
			pole[i][j] = NULL;
		}
	if(init)
	{
		for(int i = 0;i<BOARDSIZE;i+=2)
		{
			pole[i]  [0] = new Kamen(i  ,0,this,BILY);
			pole[i+1][1] = new Kamen(i+1,1,this,BILY);
			pole[i]  [2] = new Kamen(i  ,2,this,BILY);
			pole[i+1][5] = new Kamen(i+1,5,this,CERNY);
			pole[i]  [6] = new Kamen(i  ,6,this,CERNY);
			pole[i+1][7] = new Kamen(i+1,7,this,CERNY);
		}
		numWhite = 12;
		numBlack = 12;
	}
	else
	{
		numWhite = 0;
		numBlack = 0;
	}
	grabbed = NULL;
	naTahu = BILY;
	numMarked = 0;
	active = NULL;
	if(init) UpdateActive();
}

/**
 *	Nastavení stavu desky z řetězce
 *
 *	@param	stav	řetězec se stavem desky (předpokládá se, že v korektním formátu)
 */
void Deska::SetStav(std::string stav)
{
	istringstream in;
	in.str(stav);
	int tp;
	numBlack = 0;
	numWhite = 0;
	
	
	if(grabbed) grabbed = NULL;
	for(int y = 0;y<BOARDSIZE;y+=1)
	{
		for(int x = 0;x<BOARDSIZE;x+=1)
		{
			if(pole[x][y] != NULL) delete pole[x][y];
			in >> tp;
			bool mark = 0;
			if(tp > 100)
			{
				mark = 1;
				tp -= 100;
			}
			switch(tp)
			{
				case OKRAJ:
					// kvůli výpisům z překladače ...
				break;
				case PRAZDNE_POLE:
					pole[x][y] = NULL;
				break;
				case BILY_KAMEN:
					pole[x][y] = new Kamen(x,y,this,BILY);
					numWhite++;
				break;
				case BILA_DAMA:
					pole[x][y] = new Dama(x,y,this,BILY);
					numWhite++;
				break;
				case CERNY_KAMEN:
					pole[x][y] = new Kamen(x,y,this,CERNY);
					numBlack++;
				break;
				case CERNA_DAMA:
					pole[x][y] = new Dama(x,y,this,CERNY);
					numBlack++;
				break;
			}
			if(mark)
			{
				Mark(x,y);
			}
		}
	}
	int nt;
	in >> nt;
	naTahu = (barvaHrace) nt;
	
	int x,y;
	in >> x;
	in >> y;
	grabbed = GetSutr(x,y);
	if(grabbed != NULL)
		ClearActive(); // nejsou aktivní kameny (je jeden chycený)
	else
		UpdateActive();
}

/**
 *	Získání řetězce se stavem desky
 *
 *	@return		řetězec se stavem desky
 */
std::string  Deska::GetStav( void )
{
	std::string  s;
	std::stringstream out;
	for(int y = 0;y<BOARDSIZE;y+=1)
	{
		for(int x = 0;x<BOARDSIZE;x+=1)
		{
			int marked = 0;
			if(pole[x][y] != NULL)
			{
				if(pole[x][y]->IsMarked())
				{
					marked = 1;
				}
			}
			if(marked)
				out << (int)GetField(x,y) + 100 << " ";
			else
				out << (int)GetField(x,y) << " ";
		}
	}
	out << (int)naTahu << " ";
	Point bod = GetGrabbed();
	out << bod.x << " ";
	out << bod.y << " ";
	s = out.str();
	return s;
}

/**
 *	Destruktor hrací desky
 *
 *	Zruší všechny použité objekty
 *
 */
Deska::~Deska()
{
	for(int i = 0;i<BOARDSIZE;i+=1)
		for(int j = 0;j<BOARDSIZE;j+=1)
		{
			if(pole[i][j] != NULL ) delete pole[i][j];
		}
	if(active != NULL) delete active;
}

/**
 *	Vstup hrací desky
 *
 *	@param	x	x-ová souřadnice vstupu
 *	@param	y	y-ová souřadnice vstupu
 *
 *	@return	výsledek tahu
 */
navratovaHodnota Deska::Input(int x, int y)
{
	if(!grabbed)
	{
		if(IsActive(x,y))
		{
			grabbed = GetSutr(x,y); // je jeden chycený kámen
			ClearActive(); // nejsou aktivní kameny
			return HRAC_UCHOPIL_KAMEN; // chycený kámen
		}
	}
	else
	{
		if(!grabbed->Go(x,y))
		{
			return SPATNY_TAH; // tah se nezdařil
		}
		std::cout << numMarked << std::endl;
		if(numMarked && grabbed->CanJump())
		{
			return SKAKANI; // uprostřed skákání
		}
		DokonciTah();// doskákáno nebo dokončen normální tah
		
		// někdo prohrál, nemá žádné aktivní kameny - buď je všechny ztratil, nebo jsou zablokované
		if(active->empty())
		{
			if(naTahu == BILY)
				return BILY_PROHRAL;
			else if(naTahu == CERNY)
				return CERNY_PROHRAL;
		}
		else return HRA_POKRACUJE;
		
	}
	return SPATNY_TAH; // netrefil ses
}


/**
 *	co je na políčku xy?
 *
 *	@param	x	x-ová souřadnice políčka
 *	@param	y	y-ová souřadnice políčka
 *
 *	@return	co obsahuje políčko XY
 */
typPole Deska::GetField(int x, int y)
{
	if(x<0|| x>=BOARDSIZE || y<0 || y>= BOARDSIZE)
	{
		return OKRAJ;
	}
	Kamen* k = pole[x][y];
	if(k == NULL) return PRAZDNE_POLE;
	return k->GetType();
}

/**
 *	komu 'patří' políčko xy?
 *
 *	@param	x	x-ová souřadnice políčka
 *	@param	y	y-ová souřadnice políčka
 *
 *	@return	čí je políčko XY
 */
barvaHrace Deska::GetPlayer(int x, int y)
{
	if(x<0|| x>=BOARDSIZE || y<0 || y>= BOARDSIZE)
	{
		return ERROR;
	}
	if(pole[x][y] == NULL) return NIKDO;
	return pole[x][y]->GetBarva();
}

/**
 *	Prohození obsahu dvou políček
 *
 *	@param	x1	x-ová souřadnice 1. políčka
 *	@param	y1	y-ová souřadnice 1. políčka
 *	@param	x2	x-ová souřadnice 2. políčka
 *	@param	y2	y-ová souřadnice 2. políčka
 *
 */
void Deska::Swap(int x1, int y1, int x2, int y2)
{
	Kamen * sutr = pole[x2][y2];
	pole[x2][y2] = pole[x1][y1];
	pole[x1][y1] = sutr;
}

/**
 *	Získání ukazatele na kámen na políčku x, y
 *
 *	@param	x	x-ová souřadnice políčka
 *	@param	y	y-ová souřadnice políčka
 *
 *	@return ukazatel na kámen
 */
Kamen *Deska::GetSutr(int x, int y)
{
	if(x<0|| x>=BOARDSIZE || y<0 || y>= BOARDSIZE)
	{
		return NULL;
	}
	return pole[x][y];
}

/**
 *	Obnovení seznamu políček na kterých jsou ´aktivní´ kameny - tzn., že s nimi jde hýbat
 */
void Deska::UpdateActive()
{
	bool sutr,skace,beha;
	if(active != NULL) delete active;
	list<Point> *seznamDamyHop = new list<Point>;
	list<Point> *seznamKamenyHop = new list<Point>;
	list<Point> *seznamPosun = new list<Point>;
	Kamen * k;
	for(int y = 0;y<BOARDSIZE;y++)
	{
		for(int x = 0;x<BOARDSIZE;x++)
		{
			if((k = pole[x][y]) != NULL)
			{
				sutr = k->IsStone();
				skace = k->CanJump();
				beha = k->CanMove();
				if(!k->Enemy(naTahu))
				{
					if(!sutr && skace) seznamDamyHop->push_back(k->GetPozice());
					else if(sutr && skace) seznamKamenyHop->push_back(k->GetPozice());
					else if(beha)seznamPosun->push_back(k->GetPozice());
				}
			}
		}
	}
	if(!seznamDamyHop->empty())// seznam dam které mohou skákat
	{
		delete seznamKamenyHop;
		delete seznamPosun;
		active = seznamDamyHop;
	}
	else if(!seznamKamenyHop->empty())// seznam kamenů které mohou skákat
	{
		delete seznamPosun;
		delete seznamDamyHop;
		active = seznamKamenyHop;
	}
	else // prázdný seznam nebo seznam pohyblivých kamenů
	{
		delete seznamDamyHop;
		delete seznamKamenyHop;
		active = seznamPosun;
	}
}

/**
 *	Obsahuje políčko XY aktivní (použitelný) kámen?
 *
 *	@param	x	x-ová souřadnice políčka
 *	@param	y	y-ová souřadnice políčka
 *
 *	@return	informace, jestli je políčko ´aktivní´
 */
bool Deska::IsActive(int x, int y)
{
	if(active == NULL) return 0;
	for( list<Point>::iterator p=active->begin(); p!=active->end(); ++p)
		if(*p == Point(x,y)) return 1;
	return 0;
}

/**
 *	Obsahuje políčko XY kámen, kterým se právě táhne?
 *
 *	@param	x	x-ová souřadnice políčka
 *	@param	y	y-ová souřadnice políčka
 *
 *	@return	ano/ne
 */
bool Deska::IsGrabbed(int x, int y)
{
	if(!grabbed) return 0;
	if(grabbed->GetPozice() == Point(x,y)) return 1;
	return 0;
}

/**
 *	vrátí pozici chyceného kamene.
 *
 *	@return	pozice chyceného kamene - v případě, že žádný není chycený vrátí Point(-1,-1)
 */
Point Deska::GetGrabbed()
{
	if(grabbed)
		return grabbed->GetPozice();
	return Point(-1,-1); // nesmysl
}

/**
 *	Obsahuje políčko XY kámen označený ke smazání?
 *
 *	@param	x	x-ová souřadnice políčka
 *	@param	y	y-ová souřadnice políčka
 *
 *	@return	ano/ne
 */
bool Deska::IsMarked(int x, int y)
{
	//mimo hranice
	if(x<0|| x>=BOARDSIZE || y<0 || y>= BOARDSIZE)
	{
		return 0;
	}
	if(pole[x][y] != NULL)
		return pole[x][y]->IsMarked();
	return 0;
}

/**
 *	Označí políčko ke smazání na konci tahu
 *
 *	@param	x	x-ová souřadnice políčka
 *	@param	y	y-ová souřadnice políčka
 *
 *	@return	povedlo se políčko označit?
 */
bool Deska::Mark(int x, int y)
{
	if(x<0|| x>=BOARDSIZE || y<0 || y>= BOARDSIZE)
	{
		return 0;
	}
	if(pole[x][y] != NULL)
	{
		/*if(!pole[x][y]->IsMarked()) // jde označit jenom neoznačené
		{*/
			pole[x][y]->Mark();
			if(pole[x][y]->IsBlack())
			{
				numBlack--;
			}
			else
			{
				numWhite--;
			}
			numMarked++;
			return 1;
		//}
	}
	return 0;
}

/**
 *	Vymazání seznamu aktivních políček
 */
void Deska::ClearActive(void)
{
	if(active != NULL) delete active;
	active = new list<Point>;
}

/**
 *	Dokončení tahu
 *
 *	Odstraní označené kameny, upustí chycený kámen, přepne hráče a zaktualizuje seznam aktivních kamenů
 *
 */
void Deska::DokonciTah(void)
{
	Kamen * k;
	// odstranění označených kamenů
	for(int y = 0;y<BOARDSIZE;y++)
	{
		for(int x = 0;x<BOARDSIZE;x++)
		{
			if((k = pole[x][y]) != NULL)
			{
				if(k->IsMarked())
				{
					delete k;
					pole[x][y] = NULL;
				}
			}
		}
	}
	// puštění kamenů
	grabbed = NULL;
	// povýšení kamenů na dámy
	for(int i = 0;i<BOARDSIZE;i++)
	{
		if((k = pole[i][0]) != NULL && k->IsStone() && k->IsBlack())
		{
			delete k;
			pole[i][0] = (Kamen*) new Dama(i,0,this,CERNY);
		}
		if((k = pole[i][7]) != NULL && k->IsStone() && !k->IsBlack())
		{
			delete k;
			pole[i][7] = (Kamen*) new Dama(i,7,this,BILY);
		}
	}
	numMarked = 0;
	// změna hráče na tahu
	if(naTahu == BILY) naTahu = CERNY;
	else naTahu = BILY;
	// aktualizace seznamu aktivních kamenů
	UpdateActive();
}

/**
 *	Zjistí počet bílých kamenů na desce
 *
 *	@return počet bílých kamenů na desce
 */
int Deska::GetWhite(void)
{
	return numWhite;
}

/**
 *	Zjistí počet černých kamenů na desce
 *
 *	@return počet černých kamenů na desce
 */
int Deska::GetBlack(void)
{
	return numBlack;
}

/**
 *	Nastaví barvu aktuálního hráče
 *
 *  @param barva nová barva hráče
 */
void Deska::SetCurrentPlayer( barvaHrace barva )
{
	naTahu = barva;
	UpdateActive();
}

/**
 *	Zjistí barvu aktuálního hráče
 *
 *  @return barva hráče
 */
barvaHrace Deska::GetCurrentPlayer()
{
	return naTahu;
}
