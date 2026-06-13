#include <gtest/gtest.h>

#include <wx/frame.h>

#include "wxCustomization/widgets/StyledLineEdit.h"

extern wxFrame* gTestFrame;

namespace wxCustomization {

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
