# Каталог виджетов

## StyledPanel

Контейнер с фоном и рамкой.

```cpp
auto* panel = new wxCustomization::StyledPanel(parent, wxID_ANY);
panel->SetStyleSheet(&sheet);
```

```css
StyledPanel {
    background-color: #ffffff;
    border: 1px solid #bdc3c7;
    border-radius: 8dip;
    padding: 16dip;
}
```

## StyledLabel

Текстовая метка.

```cpp
auto* label = new wxCustomization::StyledLabel(parent, wxID_ANY, "Hello");
label->SetStyleSheet(&sheet);
```

```css
StyledLabel {
    color: #2c3e50;
    font-size: 14dip;
    text-align: center;
}
```

## StyledButton

Кнопка с поддержкой hover, pressed, focused, disabled.

```cpp
auto* button = new wxCustomization::StyledButton(parent, wxID_ANY, "Click me");
button->SetStyleSheet(&sheet);
button->Bind(wxEVT_BUTTON, &MyFrame::OnClick, this);
```

```css
StyledButton {
    background-color: #3498db;
    color: white;
    border: 1px solid #2980b9;
    border-radius: 4dip;
    padding: 8dip 16dip;
    text-align: center;
}

StyledButton:hover { background-color: #2980b9; }
StyledButton:pressed { background-color: #1c6ea4; }
StyledButton:focused { outline: 2dip solid #85c1e9; outline-offset: 2dip; }
StyledButton:disabled { background-color: #bdc3c7; }
```

## StyledToggleButton

Кнопка с фиксируемым состоянием.

```cpp
auto* toggle = new wxCustomization::StyledToggleButton(parent, wxID_ANY, "Toggle");
toggle->Bind(wxEVT_TOGGLEBUTTON, &MyFrame::OnToggle, this);
```

```css
StyledToggleButton:checked {
    background-color: #3498db;
    color: white;
}
```

## StyledCheckBox

Чекбокс с кастомным индикатором.

```cpp
auto* check = new wxCustomization::StyledCheckBox(parent, wxID_ANY, "Check me");
check->SetValue(true);
check->Bind(wxEVT_CHECKBOX, &MyFrame::OnCheck, this);
```

```css
StyledCheckBox {
    color: #2c3e50;
    spacing: 8dip;
}

StyledCheckBox::indicator {
    width: 16dip;
    height: 16dip;
    border: 1px solid #7f8c8d;
    border-radius: 3dip;
    background-color: white;
}

StyledCheckBox::indicator:checked {
    background-color: #3498db;
    border-color: #3498db;
}
```

## StyledRadioButton

Радиокнопка с круглым индикатором.

```cpp
auto* radio1 = new wxCustomization::StyledRadioButton(parent, wxID_ANY, "Option 1",
                                                       wxDefaultPosition, wxDefaultSize,
                                                       wxRB_GROUP);
auto* radio2 = new wxCustomization::StyledRadioButton(parent, wxID_ANY, "Option 2");
```

```css
StyledRadioButton::indicator {
    width: 16dip;
    height: 16dip;
    border: 1px solid #7f8c8d;
    border-radius: 50%;
    background-color: white;
}

StyledRadioButton::indicator:checked {
    border-color: #3498db;
    background-color: #3498db;
}
```

## StyledLineEdit

Однострочное поле ввода.

```cpp
auto* edit = new wxCustomization::StyledLineEdit(parent, wxID_ANY, "Type here...");
edit->Bind(wxEVT_TEXT, &MyFrame::OnText, this);
edit->Bind(wxEVT_TEXT_ENTER, &MyFrame::OnEnter, this);
```

```css
StyledLineEdit {
    background-color: #ffffff;
    color: #2c3e50;
    border: 1px solid #bdc3c7;
    border-radius: 4dip;
    padding: 6dip 8dip;
}

StyledLineEdit:focused {
    border-color: #3498db;
    outline: 2dip solid #85c1e9;
}
```

## StyledComboBox

Выпадающий список.

```cpp
wxArrayString choices;
choices.Add("Option 1");
choices.Add("Option 2");
auto* combo = new wxCustomization::StyledComboBox(parent, wxID_ANY, choices);
combo->SetSelection(0);
combo->Bind(wxEVT_COMBOBOX, &MyFrame::OnSelect, this);
```

```css
StyledComboBox {
    background-color: #ffffff;
    border: 1px solid #bdc3c7;
    border-radius: 4dip;
    padding: 6dip 4dip 6dip 8dip;
}

StyledComboBox::drop-down {
    width: 20dip;
    background-color: transparent;
}

StyledComboBox::down-arrow {
    color: #7f8c8d;
    border-width: 2dip;
}
```

## StyledMessageDialog

Стилизованное диалоговое окно.

```cpp
wxCustomization::StyledMessageDialog::Show(
    parent, "Message", "Title", wxOK | wxICON_INFORMATION, styleSheet);
```
