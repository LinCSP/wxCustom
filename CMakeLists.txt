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
    src/dummy.cpp
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

if(WXC_BUILD_EXAMPLES)
    add_subdirectory(examples/demo)
endif()
