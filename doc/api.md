# API

## StyleSheet

```cpp
class StyleSheet {
public:
    bool Load(const wxString& filePath);
    bool LoadFromString(const wxString& text);
    void SetVariable(const wxString& name, const wxString& value);
};
```

## StyledControl

Базовый класс всех виджетов.

```cpp
class StyledControl : public wxControl {
public:
    void SetStyleSheet(StyleSheet* sheet);
    StyleSheet* GetStyleSheet() const;

    void ApplyStyle(const wxString& state = wxEmptyString);
    virtual void UpdateStyle();

    void SetStyleProperty(const wxString& name, const wxString& value);
    wxString GetStyleProperty(const wxString& name) const;

    void AddStyleClass(const wxString& className);
    void RemoveStyleClass(const wxString& className);

    void SetAccessibleLabel(const wxString& label);
    void SetAccessibleRole(wxAccRole role);

    virtual wxString GetStyledControlType() const;
    Style GetSubControlStyle(const wxString& subControl) const;
    Style GetSubControlStyle(const wxString& subControl, const wxString& state) const;
};
```

## StyledButton

```cpp
StyledButton(wxWindow* parent, wxWindowID id, const wxString& label,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize);

void SetLabel(const wxString& label);
wxString GetLabel() const;

void SetIcon(const wxBitmap& icon);
wxBitmap GetIcon() const;
void SetIconSize(const wxSize& size);
wxSize GetIconSize() const;
```

## StyledToggleButton

```cpp
StyledToggleButton(wxWindow* parent, wxWindowID id, const wxString& label,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize);

void SetValue(bool value);
bool GetValue() const;
```

## StyledCheckBox

```cpp
StyledCheckBox(wxWindow* parent, wxWindowID id, const wxString& label,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxControlNameStr);

void SetValue(bool value);
bool GetValue() const;

void Set3StateValue(wxCheckBoxState state);
wxCheckBoxState Get3StateValue() const;
```

## StyledRadioButton

```cpp
StyledRadioButton(wxWindow* parent, wxWindowID id, const wxString& label,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = 0,
                  const wxString& name = wxControlNameStr);

void SetValue(bool value);
bool GetValue() const;
```

## StyledLineEdit

```cpp
StyledLineEdit(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxControlNameStr);

void SetValue(const wxString& value);
wxString GetValue() const;

void SetPasswordMode(bool password);
bool GetPasswordMode() const;

void SetReadOnly(bool readOnly);
bool GetReadOnly() const;

void SetSelection(long from, long to);
void SelectAll();
long GetInsertionPoint() const;
void SetInsertionPoint(long pos);
```

## StyledComboBox

```cpp
StyledComboBox(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxArrayString& choices = wxArrayString(),
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxControlNameStr);

void Append(const wxString& item);
void Clear();
size_t GetCount() const;

void SetSelection(int n);
int GetSelection() const;
wxString GetValue() const;
```

## StyledSlider

```cpp
StyledSlider(wxWindow* parent, wxWindowID id = wxID_ANY,
             int value = 0, int minValue = 0, int maxValue = 100,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxSL_HORIZONTAL,
             const wxString& name = wxControlNameStr);

void SetValue(int value);
int GetValue() const;

void SetRange(int minValue, int maxValue);
int GetMin() const;
int GetMax() const;

bool IsVertical() const;
```

## StyledProgressBar

```cpp
StyledProgressBar(wxWindow* parent, wxWindowID id = wxID_ANY,
                  int value = 0, int minValue = 0, int maxValue = 100,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxGA_HORIZONTAL,
                  const wxString& name = wxControlNameStr);

void SetValue(int value);
int GetValue() const;

void SetRange(int minValue, int maxValue);
int GetMin() const;
int GetMax() const;

bool IsVertical() const;

void SetIndeterminate(bool indeterminate);
bool IsIndeterminate() const;

void ShowText(bool show);
bool ShowsText() const;
```

## StyledGroupBox

```cpp
StyledGroupBox(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxStaticBoxNameStr);

void SetTitle(const wxString& title);
wxString GetTitle() const;

// Высота полосы заголовка в верхней части виджета (0, если заголовка нет).
int GetTitleHeight() const;
```

Контейнер: дочерние виджеты размещаются от начала клиентской области, поэтому место под заголовок резервируется в sizer'е (например, спейсером `GetTitleHeight()`). Заголовок стилизуется через под-контрол `::title`.

## StyledTabWidget

```cpp
StyledTabWidget(wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxNotebookNameStr);

// Страница должна быть создана как дочернее окно StyledTabWidget.
bool AddPage(wxWindow* page, const wxString& title, bool select = false);

size_t GetPageCount() const;
wxWindow* GetPage(size_t index) const;

wxString GetPageTitle(size_t index) const;
void SetPageTitle(size_t index, const wxString& title);

int GetSelection() const;       // -1, если страниц нет
int SetSelection(size_t index); // возвращает прежнюю выбранную вкладку
```

Контейнер вкладок: выбранная страница показывается в области `::pane`, остальные скрыты. Переключение — нажатием мыши на вкладку (сразу по нажатию), стрелками Left/Right (когда виджет в фокусе) и Ctrl+Tab / Ctrl+Shift+Tab (маршрутизируется wxWidgets через `HasMultiplePages()`, работает и при фокусе внутри страницы). Под-контролы: `::tab-bar`, `::tab` (состояния `:hover`, `:pressed`, `:selected`, `:disabled`), `::pane`.

## События

| Виджет | Событие |
|--------|---------|
| `StyledButton` | `wxEVT_BUTTON` |
| `StyledToggleButton` | `wxEVT_TOGGLEBUTTON` |
| `StyledCheckBox` | `wxEVT_CHECKBOX` |
| `StyledRadioButton` | `wxEVT_RADIOBUTTON` |
| `StyledLineEdit` | `wxEVT_TEXT`, `wxEVT_TEXT_ENTER` |
| `StyledComboBox` | `wxEVT_COMBOBOX` |
| `StyledSlider` | `wxEVT_SLIDER` |
| `StyledProgressBar` | — |
| `StyledGroupBox` | — |
| `StyledTabWidget` | `wxEVT_NOTEBOOK_PAGE_CHANGED` (класс `wxBookCtrlEvent`: `GetSelection()` — новая вкладка, `GetOldSelection()` — прежняя) |

## Theme

```cpp
Theme();
explicit Theme(const wxString& filePath);

bool Load(const wxString& filePath);   // при ошибке прежняя тема сохраняется
bool Reload();
const wxString& GetFilePath() const;
wxString GetLastError() const;
StyleSheet& GetSheet();

// Глобальная тема (невладеющий указатель).
static void SetGlobal(StyleSheet* sheet);
static StyleSheet* GetGlobal();

// Применить тему (или глобальную, если sheet == nullptr) ко всем
// StyledControl в поддереве root и пересчитать layout.
static void ApplyTo(wxWindow* root, StyleSheet* sheet = nullptr);

// Hot-reload: опрос файла по таймеру, перезагрузка и ApplyTo при изменении.
void StartWatching(wxWindow* root, int intervalMs = 500);
void StopWatching();
bool IsWatching() const;
```
