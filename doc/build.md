# Сборка и запуск

## Базовая сборка

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

После сборки в каталоге `build/` появятся:

- `libwxCustomization.a` — статическая библиотека.
- `examples/demo/demo` — демонстрационное приложение.
- `tests/wxCustomization_tests` — исполняемый файл тестов.

## Опции CMake

| Опция | По умолчанию | Описание |
|-------|--------------|----------|
| `WXC_BUILD_EXAMPLES` | `ON` | Собирать примеры |
| `WXC_BUILD_TESTS` | `ON` | Собирать unit-тесты |

Пример отключения тестов:

```bash
cmake .. -DWXC_BUILD_TESTS=OFF
```

## Запуск тестов

```bash
ctest --output-on-failure
```

Или напрямую:

```bash
./tests/wxCustomization_tests
```

## Запуск demo

Собираются два demo-приложения из одного `demo.cpp` — по одному на тему:

```bash
./examples/demo/demo        # светлая тема (themes/default.qss)
./examples/demo/demo_dark   # тёмная тема (themes/dark.qss)
```

Библиотечные темы лежат в `themes/` (`default.qss`, `dark.qss`); демонстрационные классы (`.title`, `.section-header`, `.tab-page`) — в `examples/demo/demo.qss`. Все файлы копируются рядом с бинарниками при сборке и загружаются относительно исполняемого файла; базовая тема выбирается compile-определением `DEMO_THEME_FILE`.

Demo разделено на три вкладки:

- **Buttons** — `StyledButton`, `StyledToggleButton`, `StyledCheckBox`, `StyledRadioButton` (внутри `StyledGroupBox`).
- **Input** — `StyledLineEdit`, `StyledComboBox`, `StyledSlider`.
- **Progress** — `StyledProgressBar`.

Вкладки реализованы виджетом `StyledTabWidget`: переключение — нажатием мыши на вкладку, стрелками Left/Right или Ctrl+Tab / Ctrl+Shift+Tab.

## Сборка с отладкой

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

## Сборка с clangd / LSP

Проект генерирует `compile_commands.json` при использовании CMake с Ninja или Unix Makefiles:

```bash
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Файл `compile_commands.json` будет создан в корне проекта (или в `build/`, в зависимости от генератора).

## Интеграция в свой проект

### Через add_subdirectory

```cmake
add_subdirectory(wxCustomization)
target_link_libraries(your_app PRIVATE wxCustomization::wxCustomization)
```

### Через find_package

Если библиотека установлена в систему:

```cmake
find_package(wxCustomization REQUIRED)
target_link_libraries(your_app PRIVATE wxCustomization::wxCustomization)
```

### Включение заголовков

```cpp
#include "wxCustomization/StyleSheet.h"
#include "wxCustomization/widgets/StyledButton.h"
```

## Устранение неполадок

### `wxWidgets not found`

Укажите путь к `wx-config`:

```bash
cmake .. -DwxWidgets_CONFIG_EXECUTABLE=/usr/local/bin/wx-config
```

### `GTest not found`

Установите GoogleTest или отключите тесты:

```bash
cmake .. -DWXC_BUILD_TESTS=OFF
```

### Стили в demo не обновляются после правок theme.qss

Файл `theme.qss` копируется в `build/examples/demo/` через `add_custom_command`. Если изменения не применились, перезапустите `make` — файл будет скопирован автоматически.
