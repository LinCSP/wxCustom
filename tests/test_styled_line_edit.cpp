#include <gtest/gtest.h>

#include <wx/frame.h>
#include <wx/textctrl.h>

#include "wxCustomization/widgets/StyledLineEdit.h"

extern wxFrame* gTestFrame;

namespace wxCustomization {

namespace {

void TypeChar(StyledLineEdit* edit, wxChar c)
{
    wxKeyEvent evt(wxEVT_CHAR);
    evt.m_keyCode = c;
    evt.m_uniChar = c;
    edit->GetEventHandler()->ProcessEvent(evt);
}

void PressKey(StyledLineEdit* edit, int keyCode)
{
    wxKeyEvent evt(wxEVT_KEY_DOWN);
    evt.m_keyCode = keyCode;
    edit->GetEventHandler()->ProcessEvent(evt);
}

} // namespace

TEST(StyledLineEdit, KeyboardInputInsertsText)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY);
    int textEvents = 0;
    edit->Bind(wxEVT_TEXT, [&textEvents](wxCommandEvent&) { ++textEvents; });

    TypeChar(edit, 'h');
    TypeChar(edit, 'i');

    EXPECT_EQ(edit->GetValue(), "hi");
    EXPECT_EQ(edit->GetInsertionPoint(), 2);
    EXPECT_EQ(textEvents, 2);
}

TEST(StyledLineEdit, KeyboardInputHandlesCyrillic)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY);

    // Regression for the non-ASCII input fix: characters must be read via
    // GetUnicodeKey(), not interpreted as Latin-1 garbage.
    TypeChar(edit, 0x043F); // п
    TypeChar(edit, 0x0440); // р
    TypeChar(edit, 0x0438); // и

    EXPECT_EQ(edit->GetValue(), wxString::FromUTF8("при"));
    EXPECT_EQ(edit->GetInsertionPoint(), 3);
}

TEST(StyledLineEdit, BackspaceAndDeleteRemoveChars)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "abc");
    EXPECT_EQ(edit->GetInsertionPoint(), 3);

    PressKey(edit, WXK_BACK);
    EXPECT_EQ(edit->GetValue(), "ab");
    EXPECT_EQ(edit->GetInsertionPoint(), 2);

    edit->SetInsertionPoint(0);
    PressKey(edit, WXK_DELETE);
    EXPECT_EQ(edit->GetValue(), "b");
}

TEST(StyledLineEdit, ReadOnlyBlocksKeyboardInput)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "abc");
    edit->SetReadOnly(true);

    TypeChar(edit, 'x');
    PressKey(edit, WXK_BACK);

    EXPECT_EQ(edit->GetValue(), "abc");
}

TEST(StyledLineEdit, SetSelectionRanges)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "hello");
    edit->SetSelection(1, 4);

    long from = -1;
    long to = -1;
    edit->GetSelection(&from, &to);
    EXPECT_EQ(from, 1);
    EXPECT_EQ(to, 4);
}

TEST(StyledLineEdit, TypingReplacesSelection)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "hello");
    edit->SetSelection(1, 4);

    TypeChar(edit, 'X');

    EXPECT_EQ(edit->GetValue(), "hXo");
    long from = -1;
    long to = -1;
    edit->GetSelection(&from, &to);
    EXPECT_EQ(from, to);
    EXPECT_EQ(edit->GetInsertionPoint(), 2);
}

TEST(StyledLineEdit, BackspaceDeletesWholeSelection)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "hello");
    edit->SetSelection(1, 4);

    PressKey(edit, WXK_BACK);

    EXPECT_EQ(edit->GetValue(), "ho");
    EXPECT_EQ(edit->GetInsertionPoint(), 1);
}

TEST(StyledLineEdit, EnterEmitsTextEnterEvent)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "abc");
    wxString entered;
    edit->Bind(wxEVT_TEXT_ENTER, [&entered](wxCommandEvent& evt) { entered = evt.GetString(); });

    PressKey(edit, WXK_RETURN);

    EXPECT_EQ(entered, "abc");
}

TEST(StyledLineEdit, GetSetValue)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "hello");
    EXPECT_EQ(edit->GetValue(), "hello");

    edit->SetValue("world");
    EXPECT_EQ(edit->GetValue(), "world");
    EXPECT_EQ(edit->GetInsertionPoint(), 5);
}

TEST(StyledLineEdit, PasswordMode)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY);
    EXPECT_FALSE(edit->GetPasswordMode());

    edit->SetPasswordMode(true);
    EXPECT_TRUE(edit->GetPasswordMode());

    edit->SetPasswordMode(false);
    EXPECT_FALSE(edit->GetPasswordMode());
}

TEST(StyledLineEdit, ReadOnlyMode)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY);
    EXPECT_FALSE(edit->GetReadOnly());

    edit->SetReadOnly(true);
    EXPECT_TRUE(edit->GetReadOnly());

    edit->SetReadOnly(false);
    EXPECT_FALSE(edit->GetReadOnly());
}

TEST(StyledLineEdit, SelectAll)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "hello");
    edit->SelectAll();
    EXPECT_EQ(edit->GetInsertionPoint(), 5);
}

TEST(StyledLineEdit, InsertionPoint)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "hello");
    edit->SetInsertionPoint(2);
    EXPECT_EQ(edit->GetInsertionPoint(), 2);

    edit->SetInsertionPoint(100);
    EXPECT_EQ(edit->GetInsertionPoint(), 5);

    edit->SetInsertionPoint(-5);
    EXPECT_EQ(edit->GetInsertionPoint(), 0);
}

TEST(StyledLineEdit, BestSizeIsNonZero)
{
    StyledLineEdit* edit = new StyledLineEdit(gTestFrame, wxID_ANY, "hello");
    const wxSize size = edit->GetBestSize();
    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
}

} // namespace wxCustomization
