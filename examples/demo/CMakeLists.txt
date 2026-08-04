# Demo applications. Each theme is demonstrated by a separate binary built
# from the same demo.cpp: `demo` uses the default (light) theme, `demo_dark`
# the dark one. The base theme file is selected via DEMO_THEME_FILE.

set(DEMO_QSS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../themes/default.qss
    ${CMAKE_CURRENT_SOURCE_DIR}/../../themes/dark.qss
    ${CMAKE_CURRENT_SOURCE_DIR}/demo.qss
)

set(DEMO_QSS_OUTPUTS)
foreach(QSS_FILE ${DEMO_QSS_FILES})
    get_filename_component(QSS_NAME ${QSS_FILE} NAME)
    set(QSS_DEST ${CMAKE_CURRENT_BINARY_DIR}/${QSS_NAME})
    add_custom_command(
        OUTPUT ${QSS_DEST}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QSS_FILE} ${QSS_DEST}
        DEPENDS ${QSS_FILE}
        COMMENT "Copying ${QSS_NAME}"
    )
    list(APPEND DEMO_QSS_OUTPUTS ${QSS_DEST})
endforeach()

add_custom_target(demo_theme DEPENDS ${DEMO_QSS_OUTPUTS})

function(add_demo_executable TARGET_NAME THEME_FILE)
    add_executable(${TARGET_NAME} demo.cpp)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
            wxCustomization::wxCustomization
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE DEMO_THEME_FILE="${THEME_FILE}")
    add_dependencies(${TARGET_NAME} demo_theme)
endfunction()

add_demo_executable(demo default.qss)
add_demo_executable(demo_dark dark.qss)
