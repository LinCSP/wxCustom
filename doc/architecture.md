# Архитектура

wxCustomization состоит из трёх основных слоёв:

1. **StyleEngine** — парсинг и разрешение стилей.
2. **StyledControl** — базовый класс для всех виджетов.
3. **Widgets** — конкретные виджеты.

```
┌─────────────────────────────────────────┐
│           Прикладное приложение          │
│                (demo, ваше приложение)   │
└───────────────────┬─────────────────────┘
                    │
┌───────────────────▼─────────────────────┐
│              Widgets                     │
│  StyledButton, StyledLineEdit, ...      │
│  StyledFrame, StyledTitleBar, StyledMenu│
└───────────────────┬─────────────────────┘
                    │
┌───────────────────▼─────────────────────┐
│           StyledControl                  │
│  фокус, мышь, клавиатура, состояния     │
└───────────────────┬─────────────────────┘
                    │
┌───────────────────▼─────────────────────┐
│              StyleEngine                 │
│  StyleSheet → StyleResolver → Style     │
│              ↓                           │
│            Painter                       │
└─────────────────────────────────────────┘
```

## StyleEngine

### StyleSheet

`StyleSheet` загружает CSS/QSS-текст из файла или строки и строит AST (абстрактное синтаксическое дерево) правил. Поддерживаются:

- селекторы по типу, классу, ID, атрибутам;
- псевдо-состояния (`:hover`, `:pressed`, `:focused` и др.);
- sub-control'ы (`::indicator`, `::drop-down`);
- CSS-переменные (`:root { --name: value; }`).

### StyleResolver

`StyleResolver` для конкретного виджета, его состояния и sub-control'а выбирает наиболее специфичные правила и строит итоговый `Style`. Специфичность считается по классическим правилам CSS: ID > класс/атрибут/псевдо > тип/sub-control.

### Style

`Style` — структура, содержащая все возможные свойства одного виджета в одном состоянии: цвета, фон, рамку, отступы, шрифт, размеры, иконки.

### Painter

`Painter` получает `wxDC`, прямоугольник и `Style`, и рисует фон, рамку, outline и содержимое.

## StyledControl

`StyledControl` — абстрактный базовый класс, наследник `wxControl`. Он:

- хранит указатель на `StyleSheet`;
- отслеживает состояния `hover`, `pressed`, `focused`, `checked`, `disabled`;
- обрабатывает события мыши и фокуса;
- предоставляет метод `GetSubControlStyle()` для разрешения стилей sub-control'ов;
- вызывает виртуальный `DrawContent()`, который наследники переопределяют для отрисовки текста, иконок и sub-control'ов.

## Widgets

Каждый виджет наследуется от `StyledControl` и реализует:

- `GetStyledControlType()` — имя для CSS-селектора;
- `DrawContent()` — отрисовку содержимого;
- `DoGetBestSize()` — preferred size для wxSizer;
- события и специфичное поведение.

## Оконный слой

Поверх виджетов стоит слой top-level окон с client-side decorations (единый стиль всего окна, как в VSCode/Bruno):

- **`StyledFrame`** — `wxFrame` без нативной шапки/рамки. Содержит `StyledTitleBar` и клиентскую панель; сам управляет drag (через `gtk_window_begin_move_drag` на GTK), maximize и resize за края/углы (через `gtk_window_begin_resize_drag` на GTK, иначе вручную).
- **`StyledTitleBar`** — наследник `StyledControl`: заголовок, caption-кнопки (min/max/close с выбором видимых через `SetCaptionButtons`), встроенные меню (`AddMenu`).
- **`StyledMenu`** — стилизованный popup-меню (`wxPopupTransientWindow`) с моделью из `wxMenu`; события выбора — обычные `wxEVT_MENU`.
- **`StyledMessageDialog`** — диалог без нативных декораций со своей шапкой (только close).

## Theme

`Theme` — менеджер тем: загружает таблицу стилей из файла, хранит глобальную тему (`SetGlobal`/`GetGlobal`), применяет её рекурсивно ко всем `StyledControl` в дереве окна (`ApplyTo`) и умеет hot-reload по таймеру (`StartWatching`).

## DPI и размеры

Все размеры в стилях задаются в `dip` (device-independent pixels) или `pt` (для шрифтов). Конвертация в физические пиксели выполняется через `wxWindow::FromDIP()`.
