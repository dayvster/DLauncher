
cmake_minimum_required(VERSION 3.16)

project(DLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Collect sources and headers only from source tree
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/*.cpp
)
# Remove duplicate sources from utils
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/utils/readApps.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/utils/theme.cpp")

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/*.h
)

# Exclude build dir junk
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")
list(FILTER HEADERS EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/components
    ${CMAKE_SOURCE_DIR}/src/utils
)

