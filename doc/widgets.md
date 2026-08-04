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

## StyledSlider

Слайдер с настраиваемой дорожкой (`::groove`), заполненной областью (`::sub-page`) и ручкой (`::handle`).

```cpp
auto* slider = new wxCustomization::StyledSlider(parent, wxID_ANY, 25, 0, 100);
slider->Bind(wxEVT_SLIDER, &MyFrame::OnSlider, this);
```

```css
StyledSlider::groove {
    background-color: #bdc3c7;
    border-radius: 2dip;
    height: 4dip;
}

StyledSlider::sub-page {
    background-color: #3498db;
    border-radius: 2dip;
}

StyledSlider::handle {
    width: 18dip;
    height: 18dip;
    background-color: #0d6efd;
    border-radius: 50%;
}

StyledSlider::handle:hover { background-color: #0b5ed7; }
StyledSlider::handle:pressed { background-color: #0a58ca; }
```

## StyledProgressBar

Индикатор прогресса с настраиваемой дорожкой (`::groove`) и заполненной областью (`::chunk`).

```cpp
auto* bar = new wxCustomization::StyledProgressBar(parent, wxID_ANY, 25, 0, 100);
bar->SetValue(50);
bar->SetIndeterminate(true);
```

```css
StyledProgressBar {
    min-height: 24dip;
    color: #2c3e50;
    font-size: 10dip;
    text-align: center;
}

StyledProgressBar::groove {
    background-color: #e9ecef;
    border-radius: 4dip;
    height: 16dip;
}

StyledProgressBar::chunk {
    background-color: #0d6efd;
    border-radius: 4dip;
}
```

## StyledGroupBox

Группа с заголовком и рамкой. Верхняя грань рамки проходит через середину заголовка и вырезается под ним. Работает как контейнер для других виджетов. Заголовок стилизуется через под-контрол `::title`.

Дочерние виджеты размещаются от начала клиентской области (как в `StyledPanel`), поэтому в sizer группы нужно зарезервировать место под заголовок, например спейсером высотой `GetTitleHeight()`.

```cpp
auto* box = new wxCustomization::StyledGroupBox(parent, wxID_ANY, "Options");
box->SetStyleSheet(sheet);

auto* sizer = new wxBoxSizer(wxVERTICAL);
sizer->AddSpacer(box->GetTitleHeight() + box->FromDIP(4));
sizer->Add(new wxCustomization::StyledRadioButton(box, wxID_ANY, "Radio 1"), 0, wxALL, 8);
sizer->Add(new wxCustomization::StyledRadioButton(box, wxID_ANY, "Radio 2"), 0, wxALL, 8);
box->SetSizer(sizer);
```

```css
StyledGroupBox {
    border-width: 1dip;
    border-color: #bdc3c7;
    border-style: solid;
    border-radius: 4dip;
    padding: 0dip 12dip 12dip 12dip;
}

StyledGroupBox::title {
    color: var(--primary);
    font-size: 12dip;
    font-weight: bold;
    padding: 0dip 6dip;
}
```

## StyledMessageDialog

Стилизованное диалоговое окно.

```cpp
wxCustomization::StyledMessageDialog::Show(
    parent, "Message", "Title", wxOK | wxICON_INFORMATION, styleSheet);
```

## Пример компоновки: кастомный tab bar

Пока `StyledTabWidget` находится в разработке, demo использует кастомный tab bar на базе `StyledButton` и `StyledPanel`:

```cpp
auto* tabBar = new wxCustomization::StyledPanel(parent, wxID_ANY);
tabBar->AddStyleClass("tab-bar");

auto* buttonsTab = new wxCustomization::StyledButton(tabBar, wxID_ANY, "Buttons");
buttonsTab->AddStyleClass("tab");
buttonsTab->AddStyleClass("tab-active");

auto* inputTab = new wxCustomization::StyledButton(tabBar, wxID_ANY, "Input");
inputTab->AddStyleClass("tab");
```

Переключение вкладок выполняется через `wxSizer::Show()`:

```cpp
contentSizer->Show(buttonsPage, true);
contentSizer->Show(inputPage, false);
contentSizer->Layout();
```

Стили вкладок задаются через CSS-классы:

```css
StyledButton.tab {
    background-color: #f8f9fa;
    color: #2c3e50;
    border: 1dip solid #dee2e6;
    border-radius: 4dip;
    padding: 8dip 16dip;
}

StyledButton.tab.tab-active {
    background-color: var(--primary);
    color: #ffffff;
    border-color: var(--hover);
}
```

## Планируемые виджеты

| Виджет | Описание | Статус |
|--------|----------|--------|
| `StyledTabWidget` | Полноценный виджет вкладок с sub-control'ами `::tab`, `::tab-bar`, `::pane` | запланирован |
