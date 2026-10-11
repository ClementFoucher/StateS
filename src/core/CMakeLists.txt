set(core_header_files
    "componentid.h"
    "states.h"
    "equations/equationcomputer.h"
    "equations/truthtable.h"
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
    "values/bitvectorvalue.h"
    "values/booleanvalue.h"
    "values/machinevalue.h"
    "values/naturalvalue.h"
    "values/relativevalue.h"
    "xml/statesxmlanalyzer.h"
)

set(core_source_files
    "states.cpp"
    "equations/equationcomputer.cpp"
    "equations/truthtable.cpp"
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
    "values/bitvectorvalue.cpp"
    "values/booleanvalue.cpp"
    "values/machinevalue.cpp"
    "values/naturalvalue.cpp"
    "values/relativevalue.cpp"
    "xml/statesxmlanalyzer.cpp"
)

set(core_include_directories
    "."
    "equations"
    "exceptions"
    "machine_manager"
    "simulation"
    "undo_engine"
    "undo_engine/undo_commands"
    "values"
    "xml"
)

qt_add_library(core INTERFACE)

target_sources(core INTERFACE ${core_header_files} ${core_source_files})

target_include_directories(core INTERFACE ${core_include_directories})
