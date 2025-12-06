set(text_files
    "code_guidelines.txt"
    "known_bugs.txt"
    "release_notes.txt"
    "upcoming_features.txt"
    "../README.md"
)

qt_add_library(text INTERFACE)

target_sources(text INTERFACE ${text_files})
