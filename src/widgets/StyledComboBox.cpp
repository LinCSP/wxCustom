#include "wxCustomization/widgets/StyledComboBox.h"

#include "wxCustomization/Painter.h"
#include "wxCustomization/Style.h"

#include <wx/dc.h>
#include <wx/popupwin.h>
#include <wx/settings.h>

#include <algorithm>

namespace wxCustomization {

namespace {

constexpr int DEFAULT_DROPDOWN_WIDTH_DIP = 24;
constexpr int MAX_POPUP_HEIGHT_DIP = 200;
constexpr int ITEM_MIN_HEIGHT_DIP = 24;

wxColour AdjustColour(const wxColour& base, int delta)
{
    const auto clamp = [](int v) {
        if (v < 0) return 0;
        if (v > 255) return 255;
        return v;
    };
    return wxColour(
        clamp(static_cast<int>(base.Red()) + delta),
        clamp(static_cast<int>(base.Green()) + delta),
        clamp(static_cast<int>(base.Blue()) + delta),
        base.Alpha());
}

} // namespace

// ---------------------------------------------------------------------------
// StyledComboBoxPopup
// ---------------------------------------------------------------------------

class StyledComboBoxPopup : public wxPopupTransientWindow {
public:
    explicit StyledComboBoxPopup(StyledComboBox* combo);

    void SetChoices(const wxArrayString& choices);
    void SetSelection(int selection);
    int GetSelection() const;

    void PopupList();
    void DismissList();

protected:
    void OnDismiss() override;

private:
    class ListWindow : public wxWindow {
    public:
        ListWindow(StyledComboBoxPopup* popup, wxWindow* parent);

        void SetChoices(const wxArrayString& choices);
        void SetSelection(int selection);
        int GetSelection() const { return m_selection; }
        int GetHovered() const { return m_hovered; }
        size_t GetCount() const { return m_choices.GetCount(); }

        void SetHoveredFromPoint(const wxPoint& pt);
        void MoveHover(int delta);
        void ConfirmSelection();

        int GetTotalHeight() const;

    private:
        void OnPaint(wxPaintEvent& evt);
        void OnMouseMove(wxMouseEvent& evt);
        void OnLeftUp(wxMouseEvent& evt);
        void OnKeyDown(wxKeyEvent& evt);
        void OnKillFocus(wxFocusEvent& evt);
        void OnLeaveWindow(wxMouseEvent& evt);

        int HitTest(const wxPoint& pt) const;
        wxRect GetItemRect(int index) const;
        int GetItemHeight() const;
        Style GetItemStyle(const wxString& state) const;

        StyledComboBoxPopup* m_popup;
        wxArrayString m_choices;
        int m_selection = -1;
        int m_hovered = -1;

        wxDECLARE_EVENT_TABLE();
    };

    StyledComboBox* m_combo;
    ListWindow* m_list;
};

// ---------------------------------------------------------------------------
// StyledComboBoxPopup::ListWindow
// ---------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(StyledComboBoxPopup::ListWindow, wxWindow)
    EVT_PAINT(StyledComboBoxPopup::ListWindow::OnPaint)
    EVT_MOTION(StyledComboBoxPopup::ListWindow::OnMouseMove)
    EVT_LEFT_UP(StyledComboBoxPopup::ListWindow::OnLeftUp)
    EVT_KEY_DOWN(StyledComboBoxPopup::ListWindow::OnKeyDown)
    EVT_KILL_FOCUS(StyledComboBoxPopup::ListWindow::OnKillFocus)
    EVT_LEAVE_WINDOW(StyledComboBoxPopup::ListWindow::OnLeaveWindow)
wxEND_EVENT_TABLE()

StyledComboBoxPopup::ListWindow::ListWindow(StyledComboBoxPopup* popup, wxWindow* parent)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
               wxBORDER_SIMPLE | wxWANTS_CHARS)
    , m_popup(popup)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void StyledComboBoxPopup::ListWindow::SetChoices(const wxArrayString& choices)
{
    m_choices = choices;
    if (m_selection >= static_cast<int>(m_choices.GetCount())) {
        m_selection = static_cast<int>(m_choices.GetCount()) - 1;
    }
    if (m_hovered >= static_cast<int>(m_choices.GetCount())) {
        m_hovered = -1;
    }
    Refresh();
}

void StyledComboBoxPopup::ListWindow::SetSelection(int selection)
{
    if (selection < -1 || selection >= static_cast<int>(m_choices.GetCount())) {
        selection = -1;
    }
    m_selection = selection;
    if (m_hovered == -1 && m_selection != -1) {
        m_hovered = m_selection;
    }
    Refresh();
}

void StyledComboBoxPopup::ListWindow::SetHoveredFromPoint(const wxPoint& pt)
{
    const int hovered = HitTest(pt);
    if (hovered != m_hovered) {
        m_hovered = hovered;
        Refresh();
    }
}

void StyledComboBoxPopup::ListWindow::MoveHover(int delta)
{
    const int count = static_cast<int>(m_choices.GetCount());
    if (count == 0) {
        return;
    }

    int hovered = m_hovered;
    if (hovered < 0 || hovered >= count) {
        hovered = (delta > 0) ? -1 : count;
    }

    hovered += delta;
    hovered = std::clamp(hovered, 0, count - 1);

    if (hovered != m_hovered) {
        m_hovered = hovered;
        Refresh();
    }
}

void StyledComboBoxPopup::ListWindow::ConfirmSelection()
{
    if (m_hovered >= 0 && m_hovered < static_cast<int>(m_choices.GetCount())) {
        m_selection = m_hovered;
        m_popup->m_combo->OnPopupSelection(m_selection);
    }
    m_popup->DismissList();
}

void StyledComboBoxPopup::ListWindow::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    const wxRect rect = GetClientRect();

    const Style baseStyle = m_popup->m_combo->GetSubControlStyle("item");
    const wxColour bgColor = baseStyle.backgroundColor.IsOk()
                                 ? baseStyle.backgroundColor
                                 : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const wxColour textColor = baseStyle.color.IsOk() ? baseStyle.color
                                                      : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    dc.SetBrush(wxBrush(bgColor));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(rect);

    const wxFont font = baseStyle.font.IsOk()
                            ? baseStyle.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    dc.SetFont(font);

    const int itemHeight = GetItemHeight();
    const int count = static_cast<int>(m_choices.GetCount());

    for (int i = 0; i < count; ++i) {
        const wxRect itemRect = GetItemRect(i);
        if (!itemRect.Intersects(rect)) {
            continue;
        }

        wxColour itemBg = bgColor;
        wxColour itemText = textColor;

        if (i == m_selection) {
            const Style selectedStyle = GetItemStyle("selected");
            itemBg = selectedStyle.backgroundColor.IsOk() ? selectedStyle.backgroundColor
                                                          : wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
            itemText = selectedStyle.color.IsOk() ? selectedStyle.color
                                                  : wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
        } else if (i == m_hovered) {
            const Style hoverStyle = GetItemStyle("hover");
            itemBg = hoverStyle.backgroundColor.IsOk() ? hoverStyle.backgroundColor
                                                       : AdjustColour(bgColor, 16);
            itemText = hoverStyle.color.IsOk() ? hoverStyle.color : textColor;
        }

        dc.SetBrush(wxBrush(itemBg));
        dc.DrawRectangle(itemRect);

        dc.SetTextForeground(itemText);
        const wxString& text = m_choices[i];

        int textWidth = 0;
        int textHeight = 0;
        dc.GetTextExtent(text, &textWidth, &textHeight);

        int x = itemRect.x + baseStyle.paddingLeft;
        switch (baseStyle.textAlign) {
            case TextAlign::Center:
            case TextAlign::Justify:
                x = itemRect.x + (itemRect.width - textWidth) / 2;
                break;
            case TextAlign::Right:
                x = itemRect.x + itemRect.width - textWidth - baseStyle.paddingRight;
                break;
            case TextAlign::Left:
            default:
                break;
        }

        const int y = itemRect.y + (itemHeight - textHeight) / 2;
        dc.DrawText(text, x, y);
    }
}

void StyledComboBoxPopup::ListWindow::OnMouseMove(wxMouseEvent& evt)
{
    SetHoveredFromPoint(evt.GetPosition());
    evt.Skip();
}

void StyledComboBoxPopup::ListWindow::OnLeftUp(wxMouseEvent& evt)
{
    SetHoveredFromPoint(evt.GetPosition());
    ConfirmSelection();
}

void StyledComboBoxPopup::ListWindow::OnKeyDown(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();
    switch (key) {
        case WXK_UP:
        case WXK_NUMPAD_UP:
            MoveHover(-1);
            return;

        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            MoveHover(1);
            return;

        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            if (m_hovered != 0) {
                m_hovered = 0;
                Refresh();
            }
            return;

        case WXK_END:
        case WXK_NUMPAD_END:
            if (m_hovered != static_cast<int>(m_choices.GetCount()) - 1) {
                m_hovered = static_cast<int>(m_choices.GetCount()) - 1;
                Refresh();
            }
            return;

        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            ConfirmSelection();
            return;

        case WXK_ESCAPE:
            m_popup->DismissList();
            return;

        default:
            break;
    }
    evt.Skip();
}

void StyledComboBoxPopup::ListWindow::OnKillFocus(wxFocusEvent& /*evt*/)
{
    m_popup->DismissList();
}

void StyledComboBoxPopup::ListWindow::OnLeaveWindow(wxMouseEvent& /*evt*/)
{
    if (m_hovered != -1) {
        m_hovered = -1;
        Refresh();
    }
}

int StyledComboBoxPopup::ListWindow::HitTest(const wxPoint& pt) const
{
    const int index = pt.y / GetItemHeight();
    if (index < 0 || index >= static_cast<int>(m_choices.GetCount())) {
        return -1;
    }
    return index;
}

wxRect StyledComboBoxPopup::ListWindow::GetItemRect(int index) const
{
    const int itemHeight = GetItemHeight();
    return wxRect(0, index * itemHeight, GetClientSize().x, itemHeight);
}

int StyledComboBoxPopup::ListWindow::GetItemHeight() const
{
    wxClientDC dc(const_cast<ListWindow*>(this));
    const Style baseStyle = m_popup->m_combo->GetSubControlStyle("item");
    const wxFont font = baseStyle.font.IsOk()
                            ? baseStyle.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    dc.SetFont(font);

    int textHeight = 0;
    dc.GetTextExtent("Ag", nullptr, &textHeight);

    const int minHeight = m_popup->m_combo->FromDIP(ITEM_MIN_HEIGHT_DIP);
    return std::max(minHeight, textHeight + baseStyle.paddingTop + baseStyle.paddingBottom);
}

int StyledComboBoxPopup::ListWindow::GetTotalHeight() const
{
    return static_cast<int>(m_choices.GetCount()) * GetItemHeight();
}

Style StyledComboBoxPopup::ListWindow::GetItemStyle(const wxString& state) const
{
    return m_popup->m_combo->GetSubControlStyle("item", state);
}

// ---------------------------------------------------------------------------
// StyledComboBoxPopup
// ---------------------------------------------------------------------------

StyledComboBoxPopup::StyledComboBoxPopup(StyledComboBox* combo)
    : wxPopupTransientWindow(combo->GetParent(), wxBORDER_NONE)
    , m_combo(combo)
{
    m_list = new ListWindow(this, this);
}

void StyledComboBoxPopup::SetChoices(const wxArrayString& choices)
{
    m_list->SetChoices(choices);
}

void StyledComboBoxPopup::SetSelection(int selection)
{
    m_list->SetSelection(selection);
}

int StyledComboBoxPopup::GetSelection() const
{
    return m_list->GetSelection();
}

void StyledComboBoxPopup::PopupList()
{
    if (m_list->GetCount() == 0) {
        return;
    }

    const wxPoint screenPos = m_combo->GetScreenPosition();
    const wxSize comboSize = m_combo->GetSize();
    const int width = comboSize.x;

    const int totalHeight = m_list->GetTotalHeight();
    const int maxHeight = m_combo->FromDIP(MAX_POPUP_HEIGHT_DIP);
    const int height = std::min(totalHeight, maxHeight);

    SetSize(width, height);
    m_list->SetSize(width, height);

    Position(screenPos + wxPoint(0, comboSize.y), wxDefaultSize);

    wxPopupTransientWindow::Popup(m_list);
    m_list->SetFocus();
}

void StyledComboBoxPopup::DismissList()
{
    Dismiss();
}

void StyledComboBoxPopup::OnDismiss()
{
    wxPopupTransientWindow::OnDismiss();
    if (m_combo != nullptr && !m_combo->IsBeingDeleted()) {
        m_combo->OnPopupClosed();
    }
}

// ---------------------------------------------------------------------------
// StyledComboBox
// ---------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(StyledComboBox, StyledControl)
wxEND_EVENT_TABLE()

StyledComboBox::StyledComboBox(wxWindow* parent,
                               wxWindowID id,
                               const wxArrayString& choices,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxString& name)
    : StyledControl(parent, id, pos, size, style, name)
    , m_choices(choices)
{
    SetAccessibleRole(wxROLE_SYSTEM_CLIENT);
}

StyledComboBox::~StyledComboBox()
{
    if (m_popup != nullptr) {
        m_popup->Destroy();
        m_popup = nullptr;
    }
}

void StyledComboBox::Append(const wxString& item)
{
    m_choices.Add(item);
    if (m_popup != nullptr) {
        m_popup->SetChoices(m_choices);
    }
    InvalidateBestSize();
    Refresh();
}

void StyledComboBox::Clear()
{
    m_choices.Clear();
    m_selection = -1;
    if (m_popup != nullptr) {
        m_popup->SetChoices(m_choices);
    }
    InvalidateBestSize();
    Refresh();
}

void StyledComboBox::SetSelection(int n)
{
    SelectItem(n, false);
}

wxString StyledComboBox::GetValue() const
{
    if (m_selection >= 0 && m_selection < static_cast<int>(m_choices.GetCount())) {
        return m_choices[m_selection];
    }
    return wxString();
}

void StyledComboBox::DrawContent(wxDC& dc, const wxRect& rect)
{
    const Style& style = GetCurrentStyle();

    const wxRect dropDownRect = GetDropDownRect(rect);
    const wxRect textRect = GetTextRect(rect, dropDownRect);

    // Draw drop-down button.
    Style dropDownStyle = GetSubControlStyle("drop-down");
    if (!dropDownStyle.backgroundColor.IsOk()) {
        dropDownStyle.backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    }

    Painter painter;
    painter.Paint(dc, dropDownRect, dropDownStyle, this);

    // Draw arrow inside the drop-down button.
    DrawDropDownArrow(dc, dropDownRect);

    // Draw selected item text.
    if (textRect.width <= 0 || textRect.height <= 0) {
        return;
    }

    const wxFont font = style.font.IsOk() ? style.font
                                          : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    const wxColour textColor = style.color.IsOk() ? style.color : *wxBLACK;

    dc.SetFont(font);
    dc.SetTextForeground(textColor);

    const wxString text = GetValue();
    if (text.empty()) {
        return;
    }

    int textWidth = 0;
    int textHeight = 0;
    dc.GetTextExtent(text, &textWidth, &textHeight);

    int x = textRect.x;
    switch (style.textAlign) {
        case TextAlign::Center:
        case TextAlign::Justify:
            x = textRect.x + (textRect.width - textWidth) / 2;
            break;
        case TextAlign::Right:
            x = textRect.x + textRect.width - textWidth;
            break;
        case TextAlign::Left:
        default:
            break;
    }

    const int y = textRect.y + (textRect.height - textHeight) / 2;

    dc.SetClippingRegion(textRect);
    dc.DrawText(text, x, y);
    dc.DestroyClippingRegion();
}

wxSize StyledComboBox::DoGetBestSize() const
{
    const Style& style = GetCurrentStyle();
    const wxFont font = style.font.IsOk()
                            ? style.font
                            : wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    int maxTextWidth = 0;
    int textHeight = 0;
    {
        wxClientDC dc(const_cast<StyledComboBox*>(this));
        dc.SetFont(font);

        for (size_t i = 0; i < m_choices.GetCount(); ++i) {
            int w = 0;
            int h = 0;
            dc.GetTextExtent(m_choices[i], &w, &h);
            maxTextWidth = std::max(maxTextWidth, w);
            textHeight = std::max(textHeight, h);
        }

        if (m_choices.IsEmpty()) {
            dc.GetTextExtent("Ag", &maxTextWidth, &textHeight);
            maxTextWidth = maxTextWidth * 8;
        }
    }

    const wxRect contentRect = GetContentRect();
    const wxRect dropDownRect = GetDropDownRect(contentRect);
    const int dropDownWidth = dropDownRect.width;

    int width = maxTextWidth + dropDownWidth + style.paddingLeft + style.paddingRight;
    int height = std::max(textHeight, dropDownRect.height)
                 + style.paddingTop + style.paddingBottom;

    width += style.borderLeftWidth + style.borderRightWidth;
    height += style.borderTopWidth + style.borderBottomWidth;

    if (style.IsSet(Property::MinWidth) && width < style.minWidth) {
        width = style.minWidth;
    }
    if (style.IsSet(Property::MinHeight) && height < style.minHeight) {
        height = style.minHeight;
    }
    if (style.IsSet(Property::Width) && style.width > 0) {
        width = style.width;
    }
    if (style.IsSet(Property::Height) && style.height > 0) {
        height = style.height;
    }

    return wxSize(std::max(width, 32), std::max(height, 16));
}

void StyledComboBox::OnLeftUp(wxMouseEvent& evt)
{
    const wxRect contentRect = GetContentRect();
    const wxRect dropDownRect = GetDropDownRect(contentRect);

    if (dropDownRect.Contains(evt.GetPosition())) {
        TogglePopup();
        return;
    }

    StyledControl::OnLeftUp(evt);
}

void StyledComboBox::OnKeyDown(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();
    const bool alt = evt.AltDown();

    if (alt && (key == WXK_DOWN || key == WXK_UP)) {
        ShowPopup();
        return;
    }

    if (m_popupOpen) {
        evt.Skip();
        return;
    }

    switch (key) {
        case WXK_UP:
        case WXK_NUMPAD_UP:
            MoveSelection(-1);
            return;

        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            MoveSelection(1);
            return;

        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            if (m_selection != 0) {
                SelectItem(0, true);
            }
            return;

        case WXK_END:
        case WXK_NUMPAD_END:
            if (m_selection != static_cast<int>(m_choices.GetCount()) - 1) {
                SelectItem(static_cast<int>(m_choices.GetCount()) - 1, true);
            }
            return;

        default:
            break;
    }

    evt.Skip();
}

void StyledComboBox::OnChar(wxKeyEvent& evt)
{
    const int key = evt.GetKeyCode();

    if (key == WXK_ESCAPE && m_popupOpen) {
        HidePopup();
        return;
    }

    if (!m_popupOpen && (key == WXK_RETURN || key == WXK_SPACE || key == WXK_NUMPAD_ENTER)) {
        ShowPopup();
        return;
    }

    StyledControl::OnChar(evt);
}

void StyledComboBox::OnKillFocus(wxFocusEvent& evt)
{
    // wxPopupTransientWindow auto-dismisses on outside interaction; the popup's
    // own OnDismiss handler resets m_popupOpen. Keep focus handling simple to
    // avoid closing the popup before its ListWindow receives focus on open.
    StyledControl::OnKillFocus(evt);
}

void StyledComboBox::Activate()
{
    ShowPopup();
}

void StyledComboBox::ShowPopup()
{
    if (m_choices.IsEmpty()) {
        return;
    }

    if (m_popup == nullptr) {
        m_popup = new StyledComboBoxPopup(this);
    }

    m_popup->SetChoices(m_choices);
    m_popup->SetSelection(m_selection);
    m_popupOpen = true;
    m_popup->PopupList();
}

void StyledComboBox::HidePopup()
{
    if (m_popup != nullptr && m_popupOpen) {
        m_popup->DismissList();
    }
}

void StyledComboBox::TogglePopup()
{
    if (m_popupOpen) {
        HidePopup();
    } else {
        ShowPopup();
    }
}

void StyledComboBox::OnPopupSelection(int index)
{
    if (index >= 0 && index < static_cast<int>(m_choices.GetCount())) {
        SelectItem(index, true);
    }
}

void StyledComboBox::OnPopupClosed()
{
    m_popupOpen = false;
    SetFocus();
}

void StyledComboBox::SelectItem(int index, bool sendEvent)
{
    if (index < -1 || index >= static_cast<int>(m_choices.GetCount())) {
        index = -1;
    }

    if (index == m_selection) {
        return;
    }

    m_selection = index;
    Refresh();

    if (sendEvent) {
        wxCommandEvent event(wxEVT_COMBOBOX, GetId());
        event.SetEventObject(this);
        event.SetInt(m_selection);
        event.SetString(GetValue());
        ProcessWindowEvent(event);
    }
}

void StyledComboBox::MoveSelection(int delta)
{
    if (m_choices.IsEmpty()) {
        return;
    }

    int index = m_selection + delta;
    index = std::clamp(index, 0, static_cast<int>(m_choices.GetCount()) - 1);

    SelectItem(index, true);
}

wxRect StyledComboBox::GetDropDownRect(const wxRect& contentRect) const
{
    const Style dropDownStyle = GetSubControlStyle("drop-down");
    const wxSize size = GetDropDownSize(contentRect);

    wxRect rect;
    rect.x = contentRect.x + contentRect.width - size.x;
    rect.y = contentRect.y;
    rect.width = size.x;
    rect.height = contentRect.height;
    return rect;
}

wxRect StyledComboBox::GetTextRect(const wxRect& contentRect, const wxRect& dropDownRect) const
{
    wxRect rect = contentRect;
    rect.width = dropDownRect.x - contentRect.x;
    if (rect.width < 0) {
        rect.width = 0;
    }
    return rect;
}

wxSize StyledComboBox::GetDropDownSize(const wxRect& contentRect) const
{
    const Style dropDownStyle = GetSubControlStyle("drop-down");

    int width = FromDIP(DEFAULT_DROPDOWN_WIDTH_DIP);
    int height = contentRect.height;

    if (dropDownStyle.IsSet(Property::Width) && dropDownStyle.width > 0) {
        width = dropDownStyle.width;
    }
    if (dropDownStyle.IsSet(Property::Height) && dropDownStyle.height > 0) {
        height = dropDownStyle.height;
    }

    return wxSize(width, height);
}

void StyledComboBox::DrawDropDownArrow(wxDC& dc, const wxRect& arrowRect)
{
    Style arrowStyle = GetSubControlStyle("down-arrow");

    // If an icon/image is provided via CSS, draw it centered.
    if (arrowStyle.icon.IsOk()) {
        wxSize iconSize = arrowStyle.iconSize.IsFullySpecified()
                              ? arrowStyle.iconSize
                              : wxSize(arrowStyle.icon.GetWidth(), arrowStyle.icon.GetHeight());

        if (iconSize.x > 0 && iconSize.y > 0 &&
            (iconSize.x != arrowStyle.icon.GetWidth() || iconSize.y != arrowStyle.icon.GetHeight())) {
            wxBitmap scaled(arrowStyle.icon.ConvertToImage().Rescale(iconSize.x, iconSize.y));
            dc.DrawBitmap(scaled,
                          arrowRect.x + (arrowRect.width - iconSize.x) / 2,
                          arrowRect.y + (arrowRect.height - iconSize.y) / 2,
                          true);
        } else {
            dc.DrawBitmap(arrowStyle.icon,
                          arrowRect.x + (arrowRect.width - iconSize.x) / 2,
                          arrowRect.y + (arrowRect.height - iconSize.y) / 2,
                          true);
        }
        return;
    }

    wxColour arrowColor = arrowStyle.color.IsOk() ? arrowStyle.color : GetCurrentStyle().color;
    if (!arrowColor.IsOk()) {
        arrowColor = *wxBLACK;
    }

    const int penWidth = arrowStyle.IsSet(Property::BorderWidth) && arrowStyle.borderWidth > 0
                             ? arrowStyle.borderWidth
                             : FromDIP(2);

    // Chevron / caret pointing down, similar to Bootstrap's form-select arrow.
    const int available = std::min(arrowRect.width, arrowRect.height);
    const int size = std::max(available * 3 / 10, FromDIP(5));
    const int centerX = arrowRect.x + arrowRect.width / 2;
    const int centerY = arrowRect.y + arrowRect.height / 2;

    const wxPoint leftTop(centerX - size, centerY - size / 2 + 1);
    const wxPoint bottom(centerX, centerY + size / 2 + 1);
    const wxPoint rightTop(centerX + size, centerY - size / 2 + 1);

    dc.SetPen(wxPen(arrowColor, penWidth, wxPENSTYLE_SOLID));
    dc.DrawLine(leftTop, bottom);
    dc.DrawLine(bottom, rightTop);
}

} // namespace wxCustomization
