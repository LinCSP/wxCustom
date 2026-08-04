#include "wxCustomization/Theme.h"

#include "wxCustomization/StyledControl.h"

#include <wx/filename.h>
#include <wx/timer.h>
#include <wx/window.h>

namespace wxCustomization {

StyleSheet* Theme::s_global = nullptr;

/// Timer-based file watcher used by Theme::StartWatching.
class ThemeWatcher : public wxEvtHandler {
public:
    ThemeWatcher(Theme* theme, int intervalMs)
        : m_theme(theme)
    {
        m_timer.SetOwner(this);
        Bind(wxEVT_TIMER, &ThemeWatcher::OnTimer, this);
        m_timer.Start(intervalMs);
    }

    void Stop() { m_timer.Stop(); }

private:
    void OnTimer(wxTimerEvent& /*evt*/) { m_theme->CheckForChanges(); }

    Theme* m_theme;
    wxTimer m_timer;
};

Theme::Theme() = default;

Theme::Theme(const wxString& filePath)
{
    Load(filePath);
}

Theme::~Theme()
{
    StopWatching();
}

bool Theme::Load(const wxString& filePath)
{
    // Load into a temporary sheet first: a mid-save or broken file must not
    // clear the theme that is currently applied.
    StyleSheet newSheet;
    if (!newSheet.Load(filePath)) {
        m_lastError = newSheet.GetLastError();
        return false;
    }

    m_sheet = std::move(newSheet);
    m_filePath = filePath;
    m_fileModTime = wxFileName(filePath).GetModificationTime();
    m_lastError.clear();
    return true;
}

bool Theme::Reload()
{
    if (m_filePath.empty()) {
        m_lastError = "Theme::Reload: no theme file loaded";
        return false;
    }
    return Load(m_filePath);
}

void Theme::SetGlobal(StyleSheet* sheet)
{
    s_global = sheet;
}

void Theme::ApplyTo(wxWindow* root, StyleSheet* sheet)
{
    if (root == nullptr) {
        return;
    }
    if (sheet == nullptr) {
        sheet = s_global;
    }
    if (sheet == nullptr) {
        return;
    }

    if (StyledControl* styled = dynamic_cast<StyledControl*>(root)) {
        styled->SetStyleSheet(sheet);
    }
    for (wxWindow* child : root->GetChildren()) {
        ApplyTo(child, sheet);
    }

    root->Layout();
}

void Theme::StartWatching(wxWindow* root, int intervalMs)
{
    StopWatching();
    m_watchRoot = root;
    m_watcher.reset(new ThemeWatcher(this, intervalMs));
}

void Theme::StopWatching()
{
    if (m_watcher) {
        m_watcher->Stop();
        m_watcher.reset();
    }
    m_watchRoot = nullptr;
}

bool Theme::IsWatching() const
{
    return m_watcher != nullptr;
}

bool Theme::CheckForChanges()
{
    if (m_filePath.empty()) {
        return false;
    }

    const wxDateTime modTime = wxFileName(m_filePath).GetModificationTime();
    if (!modTime.IsValid() || modTime == m_fileModTime) {
        return false;
    }

    if (!Reload()) {
        return false;
    }
    if (m_watchRoot != nullptr) {
        ApplyTo(m_watchRoot);
    }
    return true;
}

} // namespace wxCustomization
