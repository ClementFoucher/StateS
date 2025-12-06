# Define aliases for files
set_source_files_properties("3rd_party/29699-kleanSweep.svg"
    PROPERTIES QT_RESOURCE_ALIAS "clear"
)
set_source_files_properties("3rd_party/Oxygen15.04.1-flag-blue.svg"
    PROPERTIES QT_RESOURCE_ALIAS "add_checkpoint"
)
set_source_files_properties("svg/Active_on_state.svg"
    PROPERTIES QT_RESOURCE_ALIAS "active_on_state"
)
set_source_files_properties("svg/Active_on_state_down.svg"
    PROPERTIES QT_RESOURCE_ALIAS "active_on_state_down"
)
set_source_files_properties("svg/Assign.svg"
    PROPERTIES QT_RESOURCE_ALIAS "assign"
)
set_source_files_properties("svg/Attach_window.svg"
    PROPERTIES QT_RESOURCE_ALIAS "attach_window"
)
set_source_files_properties("svg/Create_FSM.svg"
    PROPERTIES QT_RESOURCE_ALIAS "new_FSM"
)
set_source_files_properties("svg/Decrement.svg"
    PROPERTIES QT_RESOURCE_ALIAS "decrement"
)
set_source_files_properties("svg/Detach_window.svg"
    PROPERTIES QT_RESOURCE_ALIAS "detach_window"
)
set_source_files_properties("svg/Export_PDF.svg"
    PROPERTIES QT_RESOURCE_ALIAS "export_PDF"
)
set_source_files_properties("svg/Export_VHDL.svg"
    PROPERTIES QT_RESOURCE_ALIAS "export_VHDL"
)
set_source_files_properties("svg/Export_image.svg"
    PROPERTIES QT_RESOURCE_ALIAS "export_image"
)
set_source_files_properties("svg/Falling_edge.svg"
    PROPERTIES QT_RESOURCE_ALIAS "falling_edge"
)
set_source_files_properties("svg/Increment.svg"
    PROPERTIES QT_RESOURCE_ALIAS "increment"
)
set_source_files_properties("svg/Load.svg"
    PROPERTIES QT_RESOURCE_ALIAS "load"
)
set_source_files_properties("svg/Pulse.svg"
    PROPERTIES QT_RESOURCE_ALIAS "pulse"
)
set_source_files_properties("svg/Pulse_down.svg"
    PROPERTIES QT_RESOURCE_ALIAS "pulse_down"
)
set_source_files_properties("svg/Redo.svg"
    PROPERTIES QT_RESOURCE_ALIAS "redo"
)
set_source_files_properties("svg/Rising_edge.svg"
    PROPERTIES QT_RESOURCE_ALIAS "rising_edge"
)
set_source_files_properties("svg/Save.svg"
    PROPERTIES QT_RESOURCE_ALIAS "save"
)
set_source_files_properties("svg/Save_as.svg"
    PROPERTIES QT_RESOURCE_ALIAS "save_as"
)
set_source_files_properties("svg/StateS.svg"
    PROPERTIES QT_RESOURCE_ALIAS "StateS"
)
set_source_files_properties("svg/Undo.svg"
    PROPERTIES QT_RESOURCE_ALIAS "undo"
)

# Set files lists
set(art_files
    "3rd_party/29699-kleanSweep.svg"
    "3rd_party/Oxygen15.04.1-flag-blue.svg"
    "svg/Active_on_state.svg"
    "svg/Assign.svg"
    "svg/Attach_window.svg"
    "svg/Create_FSM.svg"
    "svg/Decrement.svg"
    "svg/Detach_window.svg"
    "svg/Export_PDF.svg"
    "svg/Export_VHDL.svg"
    "svg/Export_image.svg"
    "svg/Falling_edge.svg"
    "svg/Increment.svg"
    "svg/Load.svg"
    "svg/Pulse.svg"
    "svg/Redo.svg"
    "svg/Rising_edge.svg"
    "svg/Save.svg"
    "svg/Save_as.svg"
    "svg/StateS.svg"
    "svg/Undo.svg"
)

# Embed art files as resource
qt_add_resources(StateS "art"
    PREFIX "/icons"
    FILES  ${art_files}
)
