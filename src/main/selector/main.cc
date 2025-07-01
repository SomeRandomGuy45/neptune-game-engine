/*
* TODO
*/

/*
* Idea... The engine.. is literally a game
* Instead of trying to write a whole new backend, just write on startup to have a choice menu (maybe add 3D???)
* It would launch the corresponding game type
* So like, if I choose to build a 2D game then it would open the exec called like "2d_base_engine"
* Cool idea.. right? (hopefully ^_^) 
*/

#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "app.h"

wxIMPLEMENT_APP(App);