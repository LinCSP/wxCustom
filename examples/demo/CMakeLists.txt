add_executable(demo demo.cpp)

target_link_libraries(demo
    PRIVATE
        wxCustomization::wxCustomization
)

# Copy theme file next to the demo binary so it can be loaded at runtime.
# Using a custom command ensures the file is re-copied whenever it changes.
set(THEME_QSS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/theme.qss)
set(THEME_QSS_DEST ${CMAKE_CURRENT_BINARY_DIR}/theme.qss)

add_custom_command(
    OUTPUT ${THEME_QSS_DEST}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${THEME_QSS_SOURCE} ${THEME_QSS_DEST}
    DEPENDS ${THEME_QSS_SOURCE}
    COMMENT "Copying theme.qss"
)

add_custom_target(demo_theme DEPENDS ${THEME_QSS_DEST})
add_dependencies(demo demo_theme)
