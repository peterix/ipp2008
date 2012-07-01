/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file client.h
 * 
 * @version $Rev: 180 $ 
 ******************************************************************************/
#ifndef __DAMA_CLIENT
#define __DAMA_CLIENT
#include <wx/wx.h>

class MyApp: public wxApp {
public:
    bool OnInit();
    int OnExit();
};

#endif	// __DAMA_CLIENT
