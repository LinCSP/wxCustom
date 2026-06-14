# Миграция с wxWidgets

## Общий подход

Миграция выполняется постепенно: заменяйте нативные контролы на стилизованные аналоги wxCustomization и подключайте общий `StyleSheet`.

## Замена виджетов

| wxWidgets | wxCustomization |
|-----------|-----------------|
| `wxPanel` | `StyledPanel` |
| `wxStaticText` | `StyledLabel` |
| `wxButton` | `StyledButton` |
| `wxToggleButton` | `StyledToggleButton` |
| `wxCheckBox` | `StyledCheckBox` |
| `wxRadioButton` | `StyledRadioButton` |
| `wxTextCtrl` (single-line) | `StyledLineEdit` |
| `wxComboBox` | `StyledComboBox` |

## Пример замены

До:

```cpp
wxButton* button = new wxButton(panel, wxID_ANY, "Click me");
button->Bind(wxEVT_BUTTON, &MyFrame::OnClick, this);
```

После:

```cpp
wxCustomization::StyleSheet* sheet = GetStyleSheet();

auto* button = new wxCustomization::StyledButton(panel, wxID_ANY, "Click me");
button->SetStyleSheet(sheet);
button->Bind(wxEVT_BUTTON, &MyFrame::OnClick, this);
```

## Рекомендации

1. **Создайте глобальный StyleSheet.** Загружайте его один раз и передавайте виджетам через `SetStyleSheet()`.
2. **Используйте CSS-переменные.** Определите палитру в `:root` и используйте `var()`.
3. **Тестируйте навигацию.** Все виджеты wxCustomization поддерживают Tab/Shift+Tab, но убедитесь, что порядок фокуса соответствует ожиданиям.
4. **Проверяйте best size.** Поскольку виджеты рисуются вручную, размеры зависят от шрифта и padding в стилях.
5. **Accessibility.** Базовый `StyledControl` предоставляет `SetAccessibleLabel` и `SetAccessibleRole`.

## Стилизация существующих панелей

Если панель содержит нативные контролы, их фон может конфликтовать со стилизованным фоном. Рекомендуется заменить всю иерархию или обеспечить контрастный фон для нативных элементов.
