#pragma once

#include "wxCustomization/StyledControl.h"

class wxTopLevelWindow;

namespace wxCustomization {

/// A styled window title bar used by StyledFrame (client-side decorations).
///
/// Draws the window title (sub-control `::title`) and the caption buttons
/// minimize/maximize/close (sub-controls `::minimize-button`,
/// `::maximize-button`, `::close-button`, sharing the generic
/// `::caption-button`). Handles window dragging by the bar and maximize on
/// double-click. The caption buttons act on the top-level window the bar
/// belongs to (Iconize/Maximize/Close).
///
/// The bar is a plain container: custom widgets (menus, tool buttons) can be
/// added as children on the left side via a sizer (see StyledFrame).
class StyledTitleBar : public StyledControl {
public:
    StyledTitleBar(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxControlNameStr);

    void SetTitle(const wxString& title);
    wxString GetTitle() const { return m_title; }

    wxString GetStyledControlType() const override { return "StyledTitleBar"; }

    /// The bar itself is not focusable; caption buttons are mouse-driven.
    bool AcceptsFocus() const override { return false; }
    bool AcceptsFocusFromKeyboard() const override { return false; }

protected:
    /// Caption button indices; Close is the rightmost one.
    enum CaptionButton {
        BtnMinimize = 0,
        BtnMaximize = 1,
        BtnClose = 2,
        BtnCount = 3
    };

    void OnPaint(wxPaintEvent& evt) override;
    /// Renders the bar into @p rect. Separated from OnPaint so tests can
    /// paint into an arbitrary DC.
    virtual void DrawTitleBar(wxDC& dc, const wxRect& rect);

    void OnLeftDown(wxMouseEvent& evt) override;
    void OnLeftUp(wxMouseEvent& evt) override;
    void OnMotion(wxMouseEvent& evt) override;
    void OnMouseLeave(wxMouseEvent& evt) override;
    void OnLeftDClick(wxMouseEvent& evt);

    wxSize DoGetBestSize() const override;

    /// Return the caption button at @p pt (client coords) or -1.
    int HitTestCaptionButton(const wxPoint& pt) const;
    wxRect GetCaptionButtonRect(int index) const;
    Style GetCaptionButtonStyle(int index) const;

    /// Action for a caption button; virtual so tests can intercept it.
    virtual void ActivateCaptionButton(int index);

    static wxString CaptionButtonKind(int index);

protected:
    wxString m_title;
    int m_hoveredButton = -1;
    int m_pressedButton = -1;

private:
    wxTopLevelWindow* GetTopLevelWindow() const;
    wxFont GetTitleFont() const;
    int GetCaptionButtonsWidth() const;
    void DrawCaptionButton(wxDC& dc, int index);

    bool m_dragging = false;
    wxPoint m_dragOffset;

    wxDECLARE_EVENT_TABLE();
};

} // namespace wxCustomization
