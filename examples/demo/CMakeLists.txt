add_executable(demo demo.cpp)

target_link_libraries(demo
    PRIVATE
        wxCustomization::wxCustomization
)

# Copy theme file next to the demo binary so it can be loaded at runtime.
file(COPY theme.qss DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
