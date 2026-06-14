# Руководство по стилям

wxCustomization использует подмножество Qt Style Sheets (QSS) с wx-специфичными расширениями.

## Базовый синтаксис

```css
/* Селектор по типу */
StyledButton {
    background-color: #3498db;
    color: white;
    border: 1px solid #2980b9;
    border-radius: 4dip;
    padding: 8dip 16dip;
}

/* Псевдо-состояния */
StyledButton:hover { background-color: #2980b9; }
StyledButton:pressed { background-color: #1c6ea4; }
StyledButton:disabled { background-color: #95a5a6; }

/* Селектор по ID */
StyledButton#okButton { background-color: #2ecc71; }

/* Селектор по классу */
StyledButton.danger { background-color: #e74c3c; }

/* Динамическое свойство */
StyledButton[role="danger"] { background-color: #e74c3c; }
```

## Селекторы

| Тип | Пример | Специфичность |
|-----|--------|---------------|
| Универсальный | `*` | 0 |
| Тип | `StyledButton` | 1 |
| Класс | `.primary` | 10 |
| ID | `StyledButton#ok` | 100 |
| Свойство | `StyledButton[role="ok"]` | 10 |
| Псевдо-состояние | `StyledButton:hover` | +10 |
| Sub-control | `StyledCheckBox::indicator` | отдельная ветвь |

## Псевдо-состояния

| Состояние | Описание |
|-----------|----------|
| `:normal` | Базовое состояние |
| `:hover` | Курсор над виджетом |
| `:pressed` | Кнопка мыши нажата |
| `:focused` | Виджет в фокусе |
| `:disabled` | Виджет отключён |
| `:checked` | Установлено состояние (тогглеры, чекбоксы, радио) |
| `:unchecked` | Снято состояние |
| `:indeterminate` | Неопределённое состояние чекбокса |
| `:read-only` | Поле только для чтения (LineEdit) |

## Sub-control'ы

Sub-control'ы позволяют стилизовать отдельные части виджета:

```css
StyledCheckBox::indicator {
    width: 16dip;
    height: 16dip;
    border: 1px solid #7f8c8d;
    border-radius: 3px;
    background-color: white;
}

StyledCheckBox::indicator:checked {
    background-color: #3498db;
    border-color: #3498db;
}
```

| Виджет | Sub-control | Назначение |
|--------|-------------|------------|
| `StyledCheckBox` | `::indicator` | Индикатор состояния |
| `StyledRadioButton` | `::indicator` | Круглый индикатор |
| `StyledComboBox` | `::drop-down` | Кнопка раскрытия списка |
| `StyledComboBox` | `::down-arrow` | Стрелка в кнопке |
| `StyledComboBox` | `::item` | Элемент popup-списка |

## CSS-переменные

```css
:root {
    --primary: #3498db;
    --primary-hover: #2980b9;
    --text: #2c3e50;
}

StyledButton {
    background-color: var(--primary);
}

StyledButton:hover {
    background-color: var(--primary-hover);
}
```

## Поддерживаемые свойства

### Цвет и фон

- `color`
- `background-color`
- `background` (сокращённая запись)
- `background-image`
- `background-repeat`
- `background-position`
- `background-gradient`
- `opacity`

### Рамка и outline

- `border`, `border-width`, `border-style`, `border-color`, `border-radius`
- `border-top-width`, `border-right-width`, `border-bottom-width`, `border-left-width`
- `outline-width`, `outline-color`, `outline-offset`

### Отступы и размеры

- `padding`, `padding-top`, `padding-right`, `padding-bottom`, `padding-left`
- `margin`, `margin-top`, `margin-right`, `margin-bottom`, `margin-left`
- `width`, `height`
- `min-width`, `max-width`, `min-height`, `max-height`

### Шрифт и текст

- `font-size`
- `text-align` (`left`, `center`, `right`, `justify`)
- `text-decoration` (`none`, `underline`, `overline`, `line-through`)

### Содержимое

- `icon` / `image`
- `icon-size`
- `spacing`

## Единицы измерения

| Единица | Описание |
|---------|----------|
| `px` | Физические пиксели |
| `dip` / `dp` | Независимые от плотности пиксели (рекомендуется) |
| `pt` | Типографские пункты (для шрифтов) |
| `em` | Относительно текущего размера шрифта |
| `%` | Проценты |

## Box Model

```
┌─────────────────────────────┐
│           margin            │
│  ┌───────────────────────┐  │
│  │        border         │  │
│  │  ┌─────────────────┐  │  │
│  │  │     padding     │  │  │
│  │  │  ┌───────────┐  │  │  │
│  │  │  │  content  │  │  │  │
│  │  │  └───────────┘  │  │  │
│  │  └─────────────────┘  │  │
│  └───────────────────────┘  │
└─────────────────────────────┘
```

## Примеры

### Тёмная тема

```css
:root {
    --bg-primary: #2c3e50;
    --bg-secondary: #34495e;
    --text-primary: #ecf0f1;
    --accent: #3498db;
}

StyledPanel {
    background-color: var(--bg-primary);
}

StyledLabel {
    color: var(--text-primary);
}

StyledButton {
    background-color: var(--accent);
    color: white;
    border-radius: 4dip;
    padding: 8dip 16dip;
}
```
