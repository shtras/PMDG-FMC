#include "FMC.h"

#include <spdlog/spdlog.h>
#include <wx/wx.h>
#include <wx/hyperlink.h>

#include <array>

namespace FMC_Server
{
class MyFrame : public wxFrame
{
public:
    MyFrame()
        : wxFrame(NULL, wxID_ANY, "FMC Server")
    {
        auto panel = new wxPanel(this, wxID_ANY);

        // Set up the sizer for the panel
        auto panelSizer = new wxGridSizer(3, 2, 2);
        panel->SetSizer(panelSizer);

        portText_ = new wxTextCtrl(panel, wxID_ANY, "3456", wxDefaultPosition, wxSize(100, 25));
        panelSizer->Add(portText_);

        startBtn_ = new wxButton(panel, wxID_ANY, "Start", wxDefaultPosition, wxSize(100, 25));
        panelSizer->Add(startBtn_);
        startBtn_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            SetStatusText("Starting...");
            fmc_.Start(portText_->GetLineText(0).ToStdString());
            SetStatusText("Started");
            startBtn_->Enable(false);
            stopBtn_->Enable(true);
            dummyBtn_->Enable(true);
            url_->SetURL("http://localhost:" + portText_->GetLineText(0).ToStdString());
        });

        stopBtn_ = new wxButton(panel, wxID_ANY, "Stop", wxDefaultPosition, wxSize(100, 25));
        panelSizer->Add(stopBtn_);
        stopBtn_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            SetStatusText("Stopping...");
            fmc_.Stop();
            SetStatusText("Stopped");
            startBtn_->Enable(true);
            stopBtn_->Enable(false);
            dummyBtn_->Enable(false);
        });

        dummyBtn_ = new wxButton(panel, wxID_ANY, "Toggle dummy mode", wxDefaultPosition, wxSize(150, 25));
        panelSizer->Add(dummyBtn_);
        dummyBtn_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { fmc_.ToggleDummy(); });

        auto statusBar = CreateStatusBar(3);
        std::array<int, 3> widths = {-10, -5, -1};
        statusBar->SetStatusWidths(widths.size(), widths.data());
        SetStatusText("I'm a status bar");

        stopBtn_->Enable(false);
        dummyBtn_->Enable(false);

        // Set up the sizer for the frame and resize the frame
        // according to its contents
        auto topSizer = new wxBoxSizer(wxVERTICAL);
        topSizer->Add(panel, 1, wxEXPAND);

        url_ = new wxHyperlinkCtrl(panel, wxID_ANY, "Open in browser", "http://localhost:3456");
        panelSizer->Add(url_);

        SetSizerAndFit(topSizer);


        auto icon = new wxIcon(wxIconLocation("frontend/favicon.ico", 0));
        SetIcon(*icon);
    }

    ~MyFrame()
    {
        fmc_.Stop();
    }

private:
    FMC fmc_;
    wxTextCtrl* portText_;
    wxButton* startBtn_;
    wxButton* stopBtn_;
    wxButton* dummyBtn_;
    wxHyperlinkCtrl* url_;
};

class MyApp : public wxApp
{
public:
    bool OnInit() override
    {
#if (defined(WIN32)) && (defined(_DEBUG))
        AllocConsole();
        FILE* stream = nullptr;
        _wfreopen_s(&stream, L"CON", L"w", stdout);
#endif
        auto frame = new MyFrame();
        frame->Show(true);
        return true;
    }

    int OnExit() override
    {
#if (defined(WIN32)) && (defined(_DEBUG))
        FreeConsole();
#endif
        return 0;
    }
};
} // namespace FMC_Server

wxIMPLEMENT_APP(FMC_Server::MyApp);
