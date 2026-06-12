#include <wx/wx.h>
#include <gtest/gtest.h>
#include <iostream>

wxFrame* gTestFrame = nullptr;

int main(int argc, char** argv)
{
    wxApp::SetInstance(new wxApp());
    int wxArgc = argc;
    if (!wxEntryStart(wxArgc, argv)) {
        std::cerr << "wxEntryStart failed\n";
        return 1;
    }

    gTestFrame = new wxFrame(nullptr, wxID_ANY, "wxCustomization test frame");

    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();

    delete gTestFrame;
    gTestFrame = nullptr;

    wxEntryCleanup();
    return result;
}
