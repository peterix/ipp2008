/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_pieces.cpp
 * 
 * Implementace třídy pro práci s herními kameny.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/
#include "cl_pieces.h"
#include "cl_util.h"
#include <cstdlib>
#include <iostream>

/***********************************************************************************************/
//  KÁMEN
/***********************************************************************************************/

Kamen::Kamen(int _x, int _y, Deska * _board,barvaHrace _barva, bool _marked)
{
	x = _x;
	y = _y;
	board = _board;
	barva = _barva;
	marked = _marked;
};

void Kamen::GetPozice(int & _x,int & _y)
{
	_x = x; _y = y;
};

Point Kamen::GetPozice( void)
{
	return Point(x,y);
};

typPole Kamen::GetType()
{
	if(barva == BILY) return BILY_KAMEN;
	else if(barva == CERNY) return CERNY_KAMEN;
	return PRAZDNE_POLE; /// nesmysl, ale nehází to warning
}

barvaHrace Kamen::GetBarva()
{
	return barva;
}
/// vrací 1 pokud je kámen černý
bool Kamen::IsBlack()
{
	if(barva == CERNY) return 1;
	return 0;
}
/// vrací 1 pokud se jedná o kámen (dáma vrací 0)
bool Kamen::IsStone()
{
	return 1;
}

/// vrací 1 pokud je vstupní barva barvou nepřítele :)
bool Kamen::Enemy(barvaHrace _barva)
{
	if((barva == BILY && _barva == CERNY) || (barva == CERNY && _barva == BILY)) return 1;
	return 0;
}

bool Kamen::IsMarked() /// je kámen označen pro odstranění (tzn. už jednou přeskočen)?
{
	return marked;
}
void Kamen::Mark() /// označit kámen pro odstranění
{
	marked = 1;
}

bool Kamen::CanJump()
{
	if (barva == BILY)
	{
		if(board->GetPlayer(x-1, y+1) == CERNY && // je koho přeskočit
		   board->GetPlayer(x-2, y+2) == NIKDO && // a je za ním prázdné místo
		   !board->GetSutr(x-1,y+1)->IsMarked() // a přeskakovaný kámen není označen
		  ) return 1;
		if(board->GetPlayer(x+1, y+1) == CERNY &&
		   board->GetPlayer(x+2, y+2) == NIKDO &&
		   !board->GetSutr(x+1,y+1)->IsMarked()
		  ) return 1;
	}
	else if (barva == CERNY)
	{
		if(board->GetPlayer(x-1, y-1) == BILY && // je koho přeskočit
		   board->GetPlayer(x-2, y-2) == NIKDO && // a je za ním prázdné místo
		   !board->GetSutr(x-1,y-1)->IsMarked() // a přeskakovaný kámen není označen
		  ) return 1;
		if(board->GetPlayer(x+1, y-1) == BILY &&
		   board->GetPlayer(x+2, y-2) == NIKDO &&
		   !board->GetSutr(x+1,y-1)->IsMarked()
		  ) return 1;
	}
	return 0;
}
bool Kamen::CanMove()
{
	if (barva == BILY)
	{
		barvaHrace l = board->GetPlayer(x-1, y+1);
		barvaHrace p = board->GetPlayer(x+1, y+1);
		if(l == NIKDO || p == NIKDO) return 1;
	}
	else if (barva == CERNY)
	{
		barvaHrace l = board->GetPlayer(x-1, y-1);
		barvaHrace p = board->GetPlayer(x+1, y-1);
		if(l == NIKDO || p == NIKDO) return 1;
	}
	return 0;
};

bool Kamen::GoJump(int dx, int dy)
{
	Kamen *k = board->GetSutr(x+dx,y+dy);
	if(k!=NULL)
	{
		if(board->GetPlayer(x+(dx*2), y+(dy*2)) == NIKDO)
		{
			if(k->Enemy(barva))
			{
				if(!k->IsMarked())
				{
					//k->Mark();
					//board->numMarked++;
					board->Mark(x+dx,y+dy);
					board->Swap(x,y,x+(dx*2),y+(dy*2));
					x=x+(dx*2);	y=y+(dy*2);
					return 1;
				}
			}
		}
	}
	return 0;
}

bool Kamen::Go(int _x, int _y)
{
	if(barva == BILY)
	{
		if(_y == y+1 && abs(_x - x) == 1) // posun o jedno dopredu
		{
			// jde se posunout jen na prázdné pole a jen tehdy, když nejde skákat
			if(board->GetPlayer(_x, _y) == NIKDO && !CanJump())
			{
				board->Swap(x,y,_x,_y);
				x=_x; y=_y;
				return 1; // tah proběhl
			}
			return 0;
		}
		else if(_y == y+2 && abs(_x - x) == 2) // skok (dy = 2, dx = +- 2)
		{
			if((_x - x) > 0) // směr doprava nahoru
			{
				return GoJump(1,1);
			}
			else if((_x - x) < 0) // směr doleva nahoru
			{
				return GoJump(-1,1);
			}
		}
		else return 0; // není legální tah
	}
	else if(barva == CERNY)
	{
		if(_y == y-1 && abs(_x - x) == 1) // posun o jedno dopředu
		{
			// jde se posunout jen na prázdné pole a jen tehdy, když nejde skákat
			if(board->GetPlayer(_x, _y) == NIKDO && !CanJump())
			{
				board->Swap(x,y,_x,_y);
				x=_x; y=_y;
				return 1; // tah proběhl
			}
			return 0;
		}
		else if(_y == y-2 && abs(_x - x) == 2) // skok (dy = -2, dx = +- 2)
		{
			if((_x - x) > 0) // směr doprava dolu
			{
				return GoJump(1,-1);
			}
			else if((_x - x) < 0) // směr doleva dolů
			{
				return GoJump(-1,-1);
			}
		}
	}
	return 0; // není legální tah
}

/***********************************************************************************************/
//  DÁMA
/***********************************************************************************************/
typPole Dama::GetType()
{
	if(barva == BILY) return BILA_DAMA;
	else if(barva == CERNY) return CERNA_DAMA;
	return PRAZDNE_POLE; /// nesmysl, ale nehází to warning
}
bool Dama::CanMove()
{
		if(board->GetPlayer(x-1, y+1) == NIKDO || 
		   board->GetPlayer(x+1, y+1) == NIKDO ||
		   board->GetPlayer(x-1, y-1) == NIKDO ||
		   board->GetPlayer(x+1, y-1) == NIKDO) return 1;
		return 0;// zablokovaná dáma
};
bool Dama::CanJump()
{
	if(CanJumpTrace(1,1) || CanJumpTrace(1,-1)|| CanJumpTrace(-1,-1) || CanJumpTrace(-1,1)) return 1;
	return 0;
};

/// vrací 1 pokud se jedná o kámen (dáma vrací 0)
bool Dama::IsStone()
{
	return 0;
}
// TODO: refactor
bool Dama::CanJumpTrace(int dx, int dy)
{
	int x0 = x+dx;
	int y0 = y+dy;
	barvaHrace b;
	while ((b = board->GetPlayer(x0, y0)) != ERROR)
	{
		if(b != NIKDO) // náraz
		{
			if(!Enemy(b)) return 0; // nelze přeskakovat kameny stejné barvy
			else// ale jiné ano :)
			{
				if(board->GetSutr(x0,y0)->IsMarked()) return 0; // už přeskočený kámen ...
				if(board->GetPlayer(x0+dx, y0+dy) == NIKDO) return 1; // testovat na volné políčko
				return 0; // a volne políčko není :(
			}
		}
		x0+=dx;y0+=dy; // testovat další políčko
	}
	return 0;
};

bool Dama::GoTrace(int dx, int dy, int _x, int _y) // dx,dy - směr hledání | _x, _y cílové políčko
{
	if(board->GetPlayer(_x, _y) != NIKDO) return 0; // cílové políčko je obsazené... nesmysl
	// nesmí to narazit samo na sebe ... nic by nefungovalo
	int x0 = x+dx;
	int y0 = y+dy;
	int found = 0;
	int foundx, foundy;
	while(!(x0 == _x && y0 == _y))
	{
		barvaHrace b = board->GetPlayer(x0, y0);
		if(!found)// dosud nenalezen žádný kámen v cestě, normální pohyb
		{
			if(Enemy(b))// nalezen soupeřův kámen v ceste
			{
				found = 1;
				foundx =x0;
				foundy =y0;
			}
			else if(b != NIKDO) // je tam něco jinýho ... přes to jít nemůžu
			{
				return 0;
			}
		}
		else // již nalezen jeden soupeřův kámen v cestě
		{
			if(b!= NIKDO)// jakýkoliv další náraz -> chyba
			{
				return 0;
			}
		}
		x0+=dx; y0+=dy;
	}
	// dá se dojít do cílového políčka
	if(found) // přeskočení
	{
		Kamen * k = board->GetSutr(foundx,foundy);
		if(k->IsMarked()) // nelze znovu přeskakovat již přeskočené kameny
		{
			return 0;
		}
		//board->GetSutr(foundx,foundy)->Mark();
		//board->numMarked++;
		board->Mark(foundx,foundy);
		board->Swap(x,y,_x,_y);
		x=_x;	y=_y;
	}
	else if(!CanJump()) // normální tah - zamezení skákání
	{
		board->Swap(x,y,_x,_y);
		x=_x; y=_y;
	}
	else
	{
		return 0; // hráč se pokusil o normální posun, když měl skákat
	}
	return 1;
}

/// pokus o provedení tahu dámou
bool Dama::Go(int _x,int _y)// _x,_y je cílové políčko
{
	// je cílové políčko na diagonále?
	int xt = _x - x;
	int yt = _y - y;
	if(abs(xt) != abs(yt)) return 0; // není
	// je
	
	if(_x < x) // pohyb doleva
	{
		if(_y < y) // pohyb doleva dolů
		{
			return GoTrace(-1,-1,_x,_y);
		}
		else if(_y > y) // pohyb doleva nahoru
		{
			return GoTrace(-1,1,_x,_y);
		}
	}
	else if(_x > x)// pohyb doprava
	{
		if(_y < y) // pohyb doprava dolů
		{
			return GoTrace(1,-1,_x,_y);
		}
		else if(_y > y) // pohyb doprava nahoru
		{
			return GoTrace(1,1,_x,_y);
		}
	}
	return 0; // tak sem by to nikdy propadnout nemělo, ale znate to... clověk nikdy neví ;)
};
