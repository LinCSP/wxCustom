add_executable(wxCustomization_tests
    test_main.cpp
    test_length.cpp
    test_color.cpp
    test_style_value.cpp
    test_style.cpp
)

target_link_libraries(wxCustomization_tests
    PRIVATE
        wxCustomization::wxCustomization
        GTest::gtest
)

add_test(NAME wxCustomization_tests COMMAND wxCustomization_tests)
