#pragma once

#include "wxCustomization/StyleSheet.h"

#include <wx/datetime.h>
#include <wx/string.h>

#include <memory>

class wxWindow;

namespace wxCustomization {

class ThemeWatcher;

/// Theme manager: loads a stylesheet from a file and applies it to widget
/// trees. Optionally watches the theme file and hot-reloads it on change.
///
/// Typical usage:
///
/// ```cpp
/// wxCustomization::Theme theme;
/// theme.Load("themes/default.qss");
/// wxCustomization::Theme::SetGlobal(&theme.GetSheet());
/// wxCustomization::Theme::ApplyTo(frame);   // applies the global sheet
/// theme.StartWatching(frame);               // hot-reload on file change
/// ```
class Theme {
public:
    Theme();
    explicit Theme(const wxString& filePath);
    ~Theme();

    /// Load (or replace) the theme from @p filePath. On failure the previous
    /// sheet is kept and GetLastError() describes the problem.
    bool Load(const wxString& filePath);
    /// Reload the theme from the file it was loaded from.
    bool Reload();
    const wxString& GetFilePath() const { return m_filePath; }
    wxString GetLastError() const { return m_lastError; }

    /// The loaded stylesheet. Non-const: widgets resolve styles against it.
    StyleSheet& GetSheet() { return m_sheet; }
    const StyleSheet& GetSheet() const { return m_sheet; }

    /// Global stylesheet shared by the application. Non-owning pointer: the
    /// caller (usually a Theme instance or the app object) must keep it alive.
    static void SetGlobal(StyleSheet* sheet);
    static StyleSheet* GetGlobal() { return s_global; }

    /// Apply @p sheet (or the global sheet when nullptr) to @p root itself and
    /// to every StyledControl in its subtree, then re-layout the tree.
    static void ApplyTo(wxWindow* root, StyleSheet* sheet = nullptr);

    /// Poll the theme file every @p intervalMs; on modification reload it and
    /// re-apply to @p root. The root window must outlive the watching.
    void StartWatching(wxWindow* root, int intervalMs = 500);
    void StopWatching();
    bool IsWatching() const;

    /// Single watch poll: reloads and re-applies if the file changed since the
    /// last load. Returns true when a reload happened. Called by the watcher
    /// timer; exposed for tests and manual polling.
    bool CheckForChanges();

private:
    StyleSheet m_sheet;
    wxString m_filePath;
    wxString m_lastError;
    wxDateTime m_fileModTime;

    std::unique_ptr<ThemeWatcher> m_watcher;
    wxWindow* m_watchRoot = nullptr;

    static StyleSheet* s_global;
};

} // namespace wxCustomization
