set(core_header_files
    "states.h"
    "statestypes.h"
    "basic_type/logicvalue.h"
    "basic_type/truthtable.h"
    "exceptions/exceptiontypes.h"
    "exceptions/statesexception.h"
    "machine_manager/machinebuilder.h"
    "machine_manager/machinemanager.h"
    "machine_manager/machinesimulator.h"
    "machine_manager/machinestatus.h"
    "undo_engine/diffundocommand.h"
    "undo_engine/fsmundocommand.h"
    "undo_engine/machineundocommand.h"
    "undo_engine/undoredomanager.h"
    "xml/statesxmlanalyzer.h"
)

set(core_source_files
    "states.cpp"
    "basic_type/logicvalue.cpp"
    "basic_type/truthtable.cpp"
    "exceptions/statesexception.cpp"
    "machine_manager/machinebuilder.cpp"
    "machine_manager/machinemanager.cpp"
    "machine_manager/machinesimulator.cpp"
    "machine_manager/machinestatus.cpp"
    "undo_engine/diffundocommand.cpp"
    "undo_engine/fsmundocommand.cpp"
    "undo_engine/machineundocommand.cpp"
    "undo_engine/undoredomanager.cpp"
    "xml/statesxmlanalyzer.cpp"
)

set(core_include_directories
    "."
    "basic_type"
    "exceptions"
    "machine_manager"
    "simulation"
    "undo_engine"
    "xml"
)

qt_add_library(core INTERFACE)

target_sources(core INTERFACE ${core_header_files} ${core_source_files})

target_include_directories(core INTERFACE ${core_include_directories})
