add_executable(wxCustomization_tests
    test_main.cpp
    test_length.cpp
    test_color.cpp
    test_style_value.cpp
    test_style.cpp
    test_lexer.cpp
    test_stylesheet.cpp
    test_resolver.cpp
    test_painter.cpp
    test_styled_control.cpp
    test_style_engine.cpp
    test_styled_button.cpp
    test_styled_panel.cpp
    test_styled_label.cpp
    test_styled_toggle_button.cpp
    test_styled_checkbox.cpp
    test_styled_radiobutton.cpp
)

target_link_libraries(wxCustomization_tests
    PRIVATE
        wxCustomization::wxCustomization
        GTest::gtest
)

add_test(NAME wxCustomization_tests COMMAND wxCustomization_tests)
