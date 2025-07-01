#include <iostream>

#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

enum {
    ID_2D = 0,
    ID_SETTINGS = 1
 };

class App : public wxApp{
public:
    virtual bool OnInit();
};

class MainFrame : public wxFrame {
public:
    MainFrame();
private:
    void OnExit(wxCommandEvent& event);
    
    wxButton* btn_2d;
};