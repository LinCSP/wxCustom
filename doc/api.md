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
