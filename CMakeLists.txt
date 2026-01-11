# Old QMake project header kept for nostalgia
#-------------------------------------------------
#
# Project created by QtCreator 2014-10-13T20:46:54
#
#-------------------------------------------------

# Define project
cmake_minimum_required(VERSION 3.31)
project(StateS VERSION 0.4.9 LANGUAGES CXX)
# Also remember to update version in README.md, known_bugs.txt, AppImageBuilder.yml and WindowsMetadata.rc (x4!)

# Set variables used by the project
set(STATES_DATES "2014-2026")
# Also remember to update date in WindowsMetadata.rc

# Configure toolchain
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Aditional configuration
set(CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/ClementFoucher/StateS")

# Qt required modules
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    LinguistTools
    PrintSupport
    Svg
    Widgets
)

# Setup standard Qt configuration
qt_standard_project_setup(
    REQUIRES                  6.8
    I18N_SOURCE_LANGUAGE      en
    I18N_TRANSLATED_LANGUAGES fr
)

# Define CMake target
if(WIN32)
    qt_add_executable(StateS
        "src/main.cpp"
        "packaging/Windows/WindowsMetadata.rc"
    )
else()
    qt_add_executable(StateS
        "src/main.cpp"
    )
endif()

# Add project subdirectories
add_subdirectory("art")
add_subdirectory("src")
add_subdirectory("text")

# Link project libraries
target_link_libraries(StateS PRIVATE
    core
    machine
    ui
    dtl
    text
)

# Define C++ macros used in code
target_compile_definitions(StateS PRIVATE
    STATES_VERSION_MAJOR=\"${PROJECT_VERSION_MAJOR}\"
    STATES_VERSION_MINOR=\"${PROJECT_VERSION_MINOR}\"
    STATES_VERSION_PATCH=\"${PROJECT_VERSION_PATCH}\"
    STATES_DATE=\"${STATES_DATES}\"
)

# Link with Qt libraries
target_link_libraries(StateS PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::PrintSupport
    Qt6::Svg
    Qt6::Widgets
)

# Add translations
set_property(
    TARGET StateS
    PROPERTY QT_EXCLUDE_SOURCES_FROM_TRANSLATION
    "art/*"
    "text/*"
    "README.md"
)

qt_add_translations(StateS
    TS_FILE_DIR     "translations"
    RESOURCE_PREFIX "/translations"
    LUPDATE_TARGET  "update_translations_preserve_vanished"
)

qt_add_translations(StateS
    TS_FILE_DIR     "translations"
    RESOURCE_PREFIX "/translations"
    LUPDATE_TARGET  "update_translations_clean_vanished"
    LUPDATE_OPTIONS "-no-obsolete"
)

# Set some os-specific properties
set_target_properties(StateS PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE    ON
)
