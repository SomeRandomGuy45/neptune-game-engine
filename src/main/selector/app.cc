#include "app.h"

bool App::OnInit() {
    MainFrame* frame = new MainFrame();
    frame->Show(true);
    return true;
}

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, "Selector") {
    btn_2d = new wxButton(this, ID_2D, "2D", wxPoint(0, 0), wxSize(100, 100));
    Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}