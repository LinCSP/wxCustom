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
| `StyledSlider` | `::groove` | Дорожка слайдера |
| `StyledSlider` | `::sub-page` | Заполненная часть дорожки |
| `StyledSlider` | `::handle` | Ручка слайдера |
| `StyledProgressBar` | `::groove` | Дорожка прогресс-бара |
| `StyledProgressBar` | `::chunk` | Заполненная часть прогресс-бара |
| `StyledGroupBox` | `::title` | Заголовок группы |

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

> **Примечание:** `border-radius` поддерживает проценты (`50%`), которые интерпретируются относительно меньшей стороны элемента. Для квадратного элемента `border-radius: 50%` превращает его в круг.

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

### Стилизация вкладок StyledTabWidget

Вкладки `StyledTabWidget` стилизуются через под-контролы `::tab-bar`, `::tab` и `::pane` с псевдо-состояниями `:hover`, `:pressed`, `:selected`, `:disabled`. Особенности отрисовки вкладок: `border-radius` применяется только к верхним углам, рамка рисуется сверху и по бокам (низ не обводится), а выбранная вкладка перекрывает нижнюю рамку `::tab-bar` — получается вид в духе Bootstrap 5:

```css
StyledTabWidget::tab-bar {
    border-bottom-width: 1dip;
    border-color: #dee2e6;
    border-style: solid;
}

StyledTabWidget::tab {
    color: #0d6efd;
    padding: 8dip 16dip;
    border-width: 1dip;
    border-color: transparent;
    border-style: solid;
    border-radius: 6dip;
    text-align: center;
    min-width: 80dip;
}

StyledTabWidget::tab:hover {
    color: #0a58ca;
    background-color: #f1f3f5;
}

StyledTabWidget::tab:pressed {
    color: #0a58ca;
    background-color: #e9ecef;
}

StyledTabWidget::tab:selected {
    background-color: #ffffff;
    color: #495057;
    border-color: #dee2e6;
}

StyledTabWidget:focused {
    outline-width: 2dip;
    outline-color: #85c1e9;
    outline-offset: 2dip;
}

StyledTabWidget::pane {
    background-color: #ffffff;
    padding: 0dip;
}
```

Псевдо-состояние `:selected` выделяет активную вкладку; при совпадении нескольких состояний (например, активная вкладка под курсором) `:selected` имеет приоритет над `:hover` и `:pressed`. Держите `border-width` у `::tab` одинаковым во всех состояниях (невидимая рамка через `border-color: transparent`), чтобы вкладки не «прыгали» при наведении.

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
