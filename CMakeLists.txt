
cmake_minimum_required(VERSION 3.16)

project(DLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make compile commands available for tools (clang-tidy, language servers)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Development options
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_STRICT_WARNINGS "Enable -Wall -Wextra for developer builds" ON)

# Qt settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Collect sources and headers only from source tree
# Glob only implementation sources in src; add top-level main.cpp explicitly
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
if(EXISTS ${CMAKE_SOURCE_DIR}/main.cpp)
  list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)
endif()

# Remove duplicate sources that may exist in multiple places
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/utils/readApps.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/utils/theme.cpp")

# Exclude test sources from the main executable; we'll build tests separately
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/test/.*\.cpp$")

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/*.h
)

# Exclude build dir junk
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")
list(FILTER HEADERS EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")

# Exclude old/duplicate UI source trees to avoid duplicate definitions
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/src/components/.*")
list(FILTER HEADERS EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/src/components/.*")
# Exclude duplicate top-level listeners (we use src/core/listeners)
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/src/listeners/.*")
list(FILTER HEADERS EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/src/listeners/.*")
# Exclude legacy apps folder (core/apps contains the authoritative implementation)
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/src/apps/.*")
list(FILTER HEADERS EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/src/apps/.*")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

  target_include_directories(${PROJECT_NAME} PRIVATE
      ${CMAKE_SOURCE_DIR}/src
      ${CMAKE_SOURCE_DIR}/src/ui
      ${CMAKE_SOURCE_DIR}/src/core
      ${CMAKE_SOURCE_DIR}/src/services
      ${CMAKE_SOURCE_DIR}/src/components
      ${CMAKE_SOURCE_DIR}/src/utils
      ${CMAKE_SOURCE_DIR}/src/core
  )

if(BUILD_TESTS)
  enable_testing()
  # Test target for menu parser
  add_executable(menu_parser_test test/menu_parser_test.cpp src/utils/menu_parser.cpp)
  target_include_directories(menu_parser_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test)
  target_link_libraries(menu_parser_test PRIVATE Qt6::Widgets)
  add_test(NAME menu_parser_test COMMAND menu_parser_test)

  add_executable(discovery_test test/discovery_test.cpp src/core/apps/readApps.cpp src/core/intern.cpp src/utils/utils.cpp)
  target_include_directories(discovery_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test)
  target_link_libraries(discovery_test PRIVATE Qt6::Widgets)
  add_test(NAME discovery_test COMMAND discovery_test)
endif()

if(ENABLE_STRICT_WARNINGS)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
