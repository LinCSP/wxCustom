# Установка и зависимости

## Системные требования

- **Операционная система:** Linux, Windows, macOS.
- **Компилятор:** GCC, Clang, MSVC с поддержкой C++17.
- **CMake:** версия 3.16 или новее.
- **wxWidgets:** версия 3.2 или новее (компоненты `core` и `base`).
- **GoogleTest:** требуется только для сборки тестов.

## Установка зависимостей

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential cmake libwxgtk3.2-dev libgtest-dev
```

### Fedora

```bash
sudo dnf install gcc-c++ cmake wxGTK-devel gtest-devel
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake wxwidgets-gtk3 gtest
```

### macOS (Homebrew)

```bash
brew install cmake wxwidgets googletest
```

### Windows (vcpkg)

```bash
vcpkg install wxwidgets gtest
```

## Проверка установки wxWidgets

После установки убедитесь, что wxWidgets найдётся CMake:

```bash
wx-config --version
```

Если `wx-config` не найден, укажите путь вручную при конфигурации:

```bash
cmake .. -DwxWidgets_CONFIG_EXECUTABLE=/path/to/wx-config
```

## Варианты сборки wxWidgets

Библиотека использует `wxControl` и функции рисования, доступные в минимальной сборке wxWidgets (`core` + `base`). Если вы собираете wxWidgets вручную, достаточно:

```bash
./configure --with-gtk --disable-shared --enable-monolithic
make
sudo make install
```

На Windows с MSVC используйте проекты из `build/msw` или соберите через CMake.
