/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file cl_receiver.h
 * 
 * Virtuální třída, kterou dědí všechny okna která komunikují se serverem.
 * 
 * @version $Rev: 180 $
 ******************************************************************************/

#ifndef CLIENT_RECEIVER_H
#define CLIENT_RECEIVER_H
#include <iostream>

/**
 * interface pro přeposílání zpráv oknům
 *
 * @param zprava Zpráva pro příjemce
 *
 * @return informace jestli byla zpráva úspěšně přijata - true/false
 */
class Receiver
{
public:
	virtual int Receive(std::string zprava)
	{
		std::cout << "Default receiver:" << std::endl << zprava << std::endl;
		return 1;
	};
};
#endif

