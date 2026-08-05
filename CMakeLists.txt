cmake_minimum_required(VERSION 3.16)

project(wxCustomization
    VERSION 0.1.0
    DESCRIPTION "wxWidgets styling and customization library"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# wxWidgets dependency
find_package(wxWidgets REQUIRED COMPONENTS core base)
message(STATUS "wxWidgets version: ${wxWidgets_VERSION_STRING}")

add_library(wxCustomization
    include/wxCustomization/wxCustomization.h
    include/wxCustomization/Length.h
    include/wxCustomization/Color.h
    include/wxCustomization/StyleTypes.h
    include/wxCustomization/StyleValue.h
    include/wxCustomization/Style.h
    include/wxCustomization/Lexer.h
    include/wxCustomization/StyleSelector.h
    include/wxCustomization/StyleRule.h
    include/wxCustomization/StyleSheet.h
    include/wxCustomization/StyleResolverContext.h
    include/wxCustomization/StyleResolver.h
    include/wxCustomization/Painter.h
    include/wxCustomization/StyledControl.h
    include/wxCustomization/StyledAccessible.h
    include/wxCustomization/Theme.h
    include/wxCustomization/widgets/StyledButton.h
    include/wxCustomization/widgets/StyledMessageDialog.h
    include/wxCustomization/widgets/StyledPanel.h
    include/wxCustomization/widgets/StyledLabel.h
    include/wxCustomization/widgets/StyledToggleButton.h
    include/wxCustomization/widgets/StyledCheckBox.h
    include/wxCustomization/widgets/StyledRadioButton.h
    include/wxCustomization/widgets/StyledLineEdit.h
    include/wxCustomization/widgets/StyledComboBox.h
    include/wxCustomization/widgets/StyledSlider.h
    include/wxCustomization/widgets/StyledProgressBar.h
    include/wxCustomization/widgets/StyledGroupBox.h
    include/wxCustomization/widgets/StyledTabWidget.h
    include/wxCustomization/widgets/StyledTitleBar.h
    include/wxCustomization/widgets/StyledFrame.h
    src/dummy.cpp
    src/Length.cpp
    src/Color.cpp
    src/StyleValue.cpp
    src/Style.cpp
    src/Lexer.cpp
    src/StyleSheet.cpp
    src/StyleResolver.cpp
    src/Painter.cpp
    src/StyledControl.cpp
    src/StyledAccessible.cpp
    src/Theme.cpp
    src/widgets/StyledButton.cpp
    src/widgets/StyledMessageDialog.cpp
    src/widgets/StyledPanel.cpp
    src/widgets/StyledLabel.cpp
    src/widgets/StyledToggleButton.cpp
    src/widgets/StyledCheckBox.cpp
    src/widgets/StyledRadioButton.cpp
    src/widgets/StyledLineEdit.cpp
    src/widgets/StyledComboBox.cpp
    src/widgets/StyledSlider.cpp
    src/widgets/StyledProgressBar.cpp
    src/widgets/StyledGroupBox.cpp
    src/widgets/StyledTabWidget.cpp
    src/widgets/StyledTitleBar.cpp
    src/widgets/StyledFrame.cpp
)

add_library(wxCustomization::wxCustomization ALIAS wxCustomization)

target_include_directories(wxCustomization
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${wxWidgets_INCLUDE_DIRS}
)

target_link_libraries(wxCustomization
    PUBLIC
        ${wxWidgets_LIBRARIES}
)

target_compile_definitions(wxCustomization
    PUBLIC
        ${wxWidgets_DEFINITIONS}
)

target_compile_options(wxCustomization
    PUBLIC
        ${wxWidgets_CXX_FLAGS}
)

option(WXC_BUILD_EXAMPLES "Build wxCustomization examples" ON)
option(WXC_BUILD_TESTS "Build wxCustomization tests" ON)

# The GTK port needs GTK headers for the native window move-drag used by
# StyledTitleBar (works on Wayland, where wxWindow::Move is a no-op).
if(UNIX AND NOT APPLE)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTK3 gtk+-3.0)
        if(GTK3_FOUND)
            target_include_directories(wxCustomization PRIVATE ${GTK3_INCLUDE_DIRS})
            target_link_libraries(wxCustomization PRIVATE ${GTK3_LIBRARIES})
        endif()
    endif()
endif()

if(WXC_BUILD_EXAMPLES)
    add_subdirectory(examples/demo)
endif()

if(WXC_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()
