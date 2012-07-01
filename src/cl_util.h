/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_util.h
 * 
 * Pomocné deklarace a definice pro klienta.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/

#ifndef CL_UTIL
#define CL_UTIL

#include <string>

const int BOARDSIZE = 8;

namespace dama
{

/**
 * Třída pro výjimku
 * dama::Exception bere jeden parametr string, který funguje jako předání error message
 */
class Exception : public std::exception
{
private: 
    std::string _str;
    
public:
    Exception(std::string str) : _str(str) {};
    virtual ~Exception() throw() {}
    
    virtual const char* what() const throw()
    {
        return _str.c_str();
    }
};

}

struct Point
{
  int x, y;   // souřadnice bodu

  Point() : x(0), y(0) {}
  Point(int _x, int _y) : x(_x), y(_y) {}
  bool operator==(const Point & p)
  {
    if(x == p.x && y== p.y) return 1;
    return 0;
  }
};

enum typPole
{
	OKRAJ,
	PRAZDNE_POLE,
	BILY_KAMEN,
	BILA_DAMA,
	CERNY_KAMEN,
	CERNA_DAMA
};

enum barvaHrace
{
	BILY,
	CERNY,
	NIKDO,
	ERROR
};
#endif
