# wxCustomization

Библиотека поверх [wxWidgets](https://www.wxwidgets.org/) для полной кастомизации внешнего вида десктопных приложений через CSS/QSS-подобные стили.

## Что это

wxCustomization предоставляет собственные виджеты, полностью нарисованные вручную, но сохраняющие нативное поведение: фокус, клавиатурную навигацию, события мыши и accessibility. Внешний вид полностью задаётся декларативными стилями, что позволяет создавать современные интерфейсы, не зависящие от платформенного API.

## Возможности

- CSS/QSS-подобный язык стилей с переменными, псевдо-состояниями и sub-control'ами.
- Полностью кастомизируемые виджеты: Panel, Label, Button, ToggleButton, CheckBox, RadioButton, LineEdit, ComboBox, Slider, ProgressBar, GroupBox, TabWidget и другие.
- Client-side decorations: `StyledFrame` и `StyledTitleBar` — окно целиком в едином стиле (шапка, кнопки, resize, **меню в шапке**), как в VSCode/Bruno.
- Стилизованные выпадающие меню (`StyledMenu`) и диалоги (`StyledMessageDialog`) без нативных декораций.
- Готовые светлая и тёмная темы (`themes/default.qss`, `themes/dark.qss`), менеджер тем `Theme` с hot-reload.
- Поддержка фона, градиентов, рамок, скруглений, иконок, шрифтов и outline.
- DPI-независимые размеры (`dip`, `pt`).
- Unit-тесты на GoogleTest.
- Простая интеграция в существующие wxWidgets-приложения.

## Быстрый старт

```bash
git clone <repo>
cd wxCustomization
mkdir build && cd build
cmake ..
make -j$(nproc)
ctest --output-on-failure
./examples/demo/demo        # светлая тема
./examples/demo/demo_dark   # тёмная тема
```

## Системные требования

- C++17
- CMake 3.16+
- wxWidgets 3.2+ (core, base)
- GoogleTest (для тестов)

## Документация

Подробная документация находится в каталоге [`doc/`](doc/):

- [Установка и зависимости](doc/setup.md)
- [Сборка](doc/build.md)
- [Архитектура](doc/architecture.md)
- [Руководство по стилям](doc/styling.md)
- [Каталог виджетов](doc/widgets.md)
- [API](doc/api.md)
- [Миграция с wxWidgets](doc/migration.md)

## Пример использования

```cpp
#include <wx/wx.h>
#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledFrame.h"
#include "wxCustomization/widgets/StyledButton.h"

class MyApp : public wxApp {
public:
    bool OnInit() override {
        auto* frame = new wxCustomization::StyledFrame(nullptr, wxID_ANY, "Demo");

        wxCustomization::StyleSheet sheet;
        sheet.LoadFromString(R"(
            StyledButton {
                background-color: #3498db;
                color: white;
                border: 1px solid #2980b9;
                border-radius: 4dip;
                padding: 8dip 16dip;
            }
            StyledButton:hover { background-color: #2980b9; }
            StyledButton:pressed { background-color: #1c6ea4; }
        )");
        frame->SetStyleSheet(&sheet);

        auto* button = new wxCustomization::StyledButton(
            frame->GetClientPanel(), wxID_ANY, "Click me");
        button->SetStyleSheet(&sheet);

        frame->Show(true);
        return true;
    }
};
wxIMPLEMENT_APP(MyApp);
```

## Лицензия

Проект распространяется под лицензией MIT. Подробности см. в файле [LICENSE.txt](LICENSE.txt).
