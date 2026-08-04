#include <gtest/gtest.h>

#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/frame.h>
#include <wx/sizer.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "wxCustomization/Theme.h"
#include "wxCustomization/widgets/StyledButton.h"
#include "wxCustomization/widgets/StyledGroupBox.h"
#include "wxCustomization/widgets/StyledPanel.h"

extern wxFrame* gTestFrame;

using wxCustomization::StyleSheet;
using wxCustomization::StyledButton;
using wxCustomization::StyledGroupBox;
using wxCustomization::StyledPanel;
using wxCustomization::Theme;

namespace {

wxString WriteTempFile(const wxString& name, const wxString& content)
{
    const wxString path = wxString::Format("/tmp/wxc_theme_test_%s.qss", name);
    FILE* f = fopen(path.mb_str(), "wb");
    if (f != nullptr) {
        const std::string utf8 = content.utf8_string();
        fwrite(utf8.data(), 1, utf8.size(), f);
        fclose(f);
    }
    return path;
}

} // namespace

TEST(Theme, LoadValidFile)
{
    const wxString path = WriteTempFile(
        "valid", "StyledButton { background-color: #ff0000; }\n");

    Theme theme;
    ASSERT_TRUE(theme.Load(path));
    EXPECT_EQ(theme.GetFilePath(), path);
    EXPECT_TRUE(theme.GetLastError().empty());
    EXPECT_EQ(theme.GetSheet().GetRules().size(), 1u);
}

TEST(Theme, LoadMissingFileFailsAndKeepsSheet)
{
    Theme theme;
    ASSERT_TRUE(theme.Load(WriteTempFile(
        "kept", "StyledButton { color: #00ff00; }\n")));

    EXPECT_FALSE(theme.Load("/tmp/wxc_theme_test_does_not_exist.qss"));
    EXPECT_FALSE(theme.GetLastError().empty());
    // The previously loaded sheet is still intact.
    EXPECT_EQ(theme.GetSheet().GetRules().size(), 1u);
}

TEST(Theme, ReloadReadsFileAgain)
{
    const wxString path = WriteTempFile(
        "reload", "StyledButton { color: #ff0000; }\n");

    Theme theme;
    ASSERT_TRUE(theme.Load(path));
    EXPECT_EQ(theme.GetSheet().GetRules().size(), 1u);

    WriteTempFile("reload",
                  "StyledButton { color: #ff0000; }\n"
                  "StyledPanel { background-color: #ffffff; }\n");
    ASSERT_TRUE(theme.Reload());
    EXPECT_EQ(theme.GetSheet().GetRules().size(), 2u);
}

TEST(Theme, ReloadWithoutLoadFails)
{
    Theme theme;
    EXPECT_FALSE(theme.Reload());
    EXPECT_FALSE(theme.GetLastError().empty());
}

TEST(Theme, GlobalSheetRoundtrip)
{
    StyleSheet sheet;
    Theme::SetGlobal(&sheet);
    EXPECT_EQ(Theme::GetGlobal(), &sheet);
    Theme::SetGlobal(nullptr);
    EXPECT_EQ(Theme::GetGlobal(), nullptr);
}

TEST(Theme, ApplyToAppliesSheetRecursively)
{
    StyleSheet sheet;
    ASSERT_TRUE(sheet.LoadFromString("StyledButton { color: #ff0000; }"));

    StyledPanel* panel = new StyledPanel(gTestFrame, wxID_ANY);
    StyledGroupBox* group = new StyledGroupBox(panel, wxID_ANY, "Group");
    StyledButton* button = new StyledButton(group, wxID_ANY, "OK");
    wxWindow* plainChild = new wxWindow(panel, wxID_ANY); // not styled: skipped

    Theme::ApplyTo(gTestFrame, &sheet);

    EXPECT_EQ(panel->GetStyleSheet(), &sheet);
    EXPECT_EQ(group->GetStyleSheet(), &sheet);
    EXPECT_EQ(button->GetStyleSheet(), &sheet);
    EXPECT_EQ(plainChild->GetChildren().size(), 0u);

    // Clean up the branch created by this test.
    gTestFrame->RemoveChild(panel);
    panel->Destroy();
}

TEST(Theme, ApplyToUsesGlobalSheetByDefault)
{
    StyleSheet global;
    ASSERT_TRUE(global.LoadFromString("StyledPanel { background-color: #ffffff; }"));
    Theme::SetGlobal(&global);

    StyledPanel* panel = new StyledPanel(gTestFrame, wxID_ANY);
    Theme::ApplyTo(gTestFrame);
    EXPECT_EQ(panel->GetStyleSheet(), &global);

    Theme::SetGlobal(nullptr);
    gTestFrame->RemoveChild(panel);
    panel->Destroy();
}

TEST(Theme, CheckForChangesReloadsModifiedFile)
{
    const wxString path = WriteTempFile(
        "watch", "StyledButton { color: #ff0000; }\n");

    Theme theme;
    ASSERT_TRUE(theme.Load(path));
    EXPECT_FALSE(theme.CheckForChanges()); // nothing changed yet

    // Rewrite the file and force a newer modification time (rewriting alone
    // may keep the same mtime second, which would make the test flaky).
    WriteTempFile("watch",
                  "StyledButton { color: #ff0000; }\n"
                  "StyledButton:hover { color: #00ff00; }\n");
    const wxDateTime future = wxDateTime::Now() + wxTimeSpan::Seconds(5);
    wxFileName(path).SetTimes(nullptr, &future, nullptr);

    EXPECT_TRUE(theme.CheckForChanges());
    EXPECT_EQ(theme.GetSheet().GetRules().size(), 2u);
    EXPECT_FALSE(theme.CheckForChanges()); // already up to date
}

TEST(Theme, WatchingLifecycle)
{
    Theme theme;
    ASSERT_TRUE(theme.Load(WriteTempFile("lifecycle", "StyledButton { color: red; }\n")));

    EXPECT_FALSE(theme.IsWatching());
    theme.StartWatching(gTestFrame, 100);
    EXPECT_TRUE(theme.IsWatching());
    theme.StopWatching();
    EXPECT_FALSE(theme.IsWatching());
}
