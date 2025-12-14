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
    "undo_engine/statesundocommand.h"
    "undo_engine/undoredomanager.h"
    "undo_engine/undo_commands/diffundocommand.h"
    "undo_engine/undo_commands/fsmstatemoveundocommand.h"
    "undo_engine/undo_commands/fsmstaterenameundocommand.h"
    "undo_engine/undo_commands/fsmtransitionconditionsliderpositionchangeundocommand.h"
    "undo_engine/undo_commands/machinerenameundocommand.h"
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
    "undo_engine/statesundocommand.cpp"
    "undo_engine/undoredomanager.cpp"
    "undo_engine/undo_commands/diffundocommand.cpp"
    "undo_engine/undo_commands/fsmstatemoveundocommand.cpp"
    "undo_engine/undo_commands/fsmstaterenameundocommand.cpp"
    "undo_engine/undo_commands/fsmtransitionconditionsliderpositionchangeundocommand.cpp"
    "undo_engine/undo_commands/machinerenameundocommand.cpp"
    "xml/statesxmlanalyzer.cpp"
)

set(core_include_directories
    "."
    "basic_type"
    "exceptions"
    "machine_manager"
    "simulation"
    "undo_engine"
    "undo_engine/undo_commands"
    "xml"
)

qt_add_library(core INTERFACE)

target_sources(core INTERFACE ${core_header_files} ${core_source_files})

target_include_directories(core INTERFACE ${core_include_directories})
