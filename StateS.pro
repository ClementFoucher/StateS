#-------------------------------------------------
#
# Project created by QtCreator 2014-10-13T20:46:54
#
#-------------------------------------------------

###
# Update these
VERSION = 0.4.4
DATE = 2014-2021
# And remember to updates version in readme and other text files
###


QT += core \
      gui \
	  widgets \
	  printsupport \
	  svg

TARGET = StateS
TEMPLATE = app

QMAKE_TARGET_COPYRIGHT = copyright $$DATE Clément Foucher

DEFINES += STATES_VERSION=\\\"$$VERSION\\\"
DEFINES += STATES_YEARS=\\\"$$DATE\\\"

CONFIG += c++14
QMAKE_LFLAGS += -no-pie

SOURCES += \
    # Core
	core/main.cpp \
	core/states.cpp \
	core/statesxmlanalyzer.cpp \
	core/basic_type/logicvalue.cpp \
	core/basic_type/truthtable.cpp \
	core/machine/machinebuilder.cpp \
	core/machine/logic/machine.cpp \
	core/machine/logic/components/machinecomponent.cpp \
	core/machine/logic/components/actiononsignal.cpp \
	core/machine/logic/components/machineactuatorcomponent.cpp \
	core/machine/logic/fsm/fsm.cpp \
	core/machine/logic/fsm/fsmcomponent.cpp \
	core/machine/logic/fsm/fsmstate.cpp \
	core/machine/logic/fsm/fsmtransition.cpp \
	core/machine/logic/fsm/fsmverifier.cpp \
	core/machine/configuration/machinestatus.cpp \
	core/machine/configuration/viewconfiguration.cpp \
	core/machine/import_export/machineimageexporter.cpp \
	core/machine/import_export/machinexmlparser.cpp \
	core/machine/import_export/machinexmlwriter.cpp \
	core/machine/import_export/fsm/fsmvhdlexport.cpp \
	core/machine/import_export/fsm/fsmxmlparser.cpp \
	core/machine/import_export/fsm/fsmxmlwriter.cpp \
	core/signal/inout.cpp \
	core/signal/input.cpp \
	core/signal/io.cpp \
	core/signal/signal.cpp \
	core/signal/equation.cpp \
	core/signal/output.cpp \
	core/signal/constant.cpp \
	core/simulation/clock.cpp \
	core/simulation/fsmsimulator.cpp \
	core/simulation/machinesimulator.cpp \
	core/exceptions/statesexception.cpp \
	core/undo_engine/fsmundocommand.cpp \
	core/undo_engine/diffundocommand.cpp \
	core/undo_engine/machineundocommand.cpp \
	core/undo_engine/undoredomanager.cpp \
	# UI
	ui/statesui.cpp \
	ui/machinecomponentvisualizer.cpp \
	ui/graphic_machine/graphicactuator.cpp \
	ui/graphic_machine/graphiccomponent.cpp \
	ui/graphic_machine/fsm/fsmgraphicstate.cpp \
	ui/graphic_machine/fsm/fsmgraphictransition.cpp \
	ui/graphic_machine/fsm/fsmgraphictransitionneighborhood.cpp \
	ui/dialogs/imageexportdialog.cpp \
	ui/dialogs/langselectiondialog.cpp \
	ui/dialogs/vhdlexportdialog.cpp \
	ui/dialogs/errordisplaydialog.cpp \
	ui/equation_editor/inverterbar.cpp \
	ui/equation_editor/equationeditor.cpp \
	ui/equation_editor/graphicequation.cpp \
	ui/equation_editor/equationmimedata.cpp \
	ui/equation_editor/constantvaluesetter.cpp \
	ui/equation_editor/editableequation.cpp \
	ui/equation_editor/rangeextractorwidget.cpp \
	ui/refined_widget/dynamiclineedit.cpp \
	ui/refined_widget/contextmenu.cpp \
	ui/refined_widget/dynamictableitemdelegate.cpp \
	ui/refined_widget/reactivebutton.cpp \
	ui/refined_widget/tablewidgetwithresizeevent.cpp \
	ui/refined_widget/checkboxhtml.cpp \
	ui/refined_widget/labelwithclickevent.cpp \
	ui/refined_widget/collapsiblewidgetwithtitle.cpp \
	ui/refined_widget/statesgraphicsview.cpp \
	ui/refined_widget/lineeditwithupdownbuttons.cpp \
	ui/resource_bar/hinttab.cpp \
	ui/resource_bar/abouttab.cpp \
	ui/resource_bar/resourcebar.cpp \
	ui/resource_bar/verifiertab.cpp \
	ui/resource_bar/machine_editor_tab/machineeditortab.cpp \
	ui/resource_bar/machine_editor_tab/signallisteditor.cpp \
	ui/resource_bar/component_editor/transitioneditortab.cpp \
	ui/resource_bar/component_editor/stateeditortab.cpp \
	ui/resource_bar/component_editor/actioneditor.cpp \
	ui/resource_bar/component_editor/componenteditortab.cpp \
	ui/resource_bar/component_editor/rangeeditordialog.cpp \
	ui/resource_bar/component_editor/conditioneditor.cpp \
	ui/resource_bar/simulator_tab/simulatortab.cpp \
	ui/resource_bar/simulator_tab/inputbitselector.cpp \
	ui/resource_bar/simulator_tab/inputsselector.cpp \
	ui/resource_bar/simulator_tab/inputsignalselector.cpp \
	ui/display_area/displayarea.cpp \
	ui/display_area/maintoolbar.cpp \
	ui/display_area/machine_editor_widget/scene/fsmscene.cpp \
	ui/display_area/machine_editor_widget/machineeditorwidget.cpp \
	ui/display_area/machine_editor_widget/scene/blankscene.cpp \
	ui/display_area/machine_editor_widget/scene/genericscene.cpp \
	ui/display_area/machine_editor_widget/scene/scenewidget.cpp \
	ui/display_area/machine_editor_widget/toolbar/drawingtoolbar.cpp \
	ui/display_area/machine_editor_widget/toolbar/fsmdrawingtoolbar.cpp \
	ui/display_area/timeline_widget/timelinewidget.cpp \
	ui/display_area/timeline_widget/clocktimeline.cpp \
	ui/display_area/timeline_widget/graphicclocktimeline.cpp \
	ui/display_area/timeline_widget/graphictimeline.cpp \
	ui/display_area/timeline_widget/signaltimeline.cpp \
	ui/static/svgimagegenerator.cpp \
	ui/truth_table/truthtabledisplay.cpp \
	ui/truth_table/truthtableinputtablemodel.cpp \
	ui/truth_table/truthtableoutputtablemodel.cpp \
	ui/resource_bar/component_editor/actiontypecombobox.cpp \
	ui/resource_bar/component_editor/actiontablemodel.cpp \
	ui/resource_bar/component_editor/actiontabledelegate.cpp \
	# Other
	third_party/diff_match_patch/diff_match_patch.cpp

HEADERS += \
    # Core
	core/include/states.h \
	core/include/statesxmlanalyzer.h \
	core/basic_type/include/logicvalue.h \
	core/basic_type/include/truthtable.h \
	core/machine/include/machinebuilder.h \
	core/machine/logic/include/machine.h \
	core/machine/logic/components/include/machinecomponent.h \
	core/machine/logic/components/include/actiononsignal.h \
	core/machine/logic/components/include/machineactuatorcomponent.h \
	core/machine/logic/fsm/include/fsm.h \
	core/machine/logic/fsm/include/fsmcomponent.h \
	core/machine/logic/fsm/include/fsmstate.h \
	core/machine/logic/fsm/include/fsmtransition.h \
	core/machine/logic/fsm/include/fsmverifier.h \
	core/machine/configuration/include/machinestatus.h \
	core/machine/configuration/include/viewconfiguration.h \
	core/machine/import_export/include/machineimageexporter.h \
	core/machine/import_export/include/machinexmlparser.h \
	core/machine/import_export/include/machinexmlwriter.h \
	core/machine/import_export/fsm/include/fsmvhdlexport.h \
	core/machine/import_export/fsm/include/fsmxmlparser.h \
	core/machine/import_export/fsm/include/fsmxmlwriter.h \
	core/signal/include/StateS_signal.h \
	core/signal/include/inout.h \
	core/signal/include/input.h \
	core/signal/include/io.h \
	core/signal/include/output.h \
	core/signal/include/constant.h \
	core/signal/include/equation.h \
	core/simulation/include/clock.h \
	core/simulation/include/fsmsimulator.h \
	core/simulation/include/machinesimulator.h \
	core/exceptions/include/statesexception.h \
	core/undo_engine/include/fsmundocommand.h \
	core/undo_engine/include/diffundocommand.h \
	core/undo_engine/include/machineundocommand.h \
	core/undo_engine/include/undoredomanager.h \
	# UI
	ui/include/statesui.h \
	ui/include/machinecomponentvisualizer.h \
	ui/graphic_machine/fsm/include/fsmgraphicstate.h \
	ui/graphic_machine/fsm/include/fsmgraphictransition.h \
	ui/graphic_machine/fsm/include/fsmgraphictransitionneighborhood.h \
	ui/graphic_machine/include/graphicactuator.h \
	ui/graphic_machine/include/graphiccomponent.h \
	ui/dialogs/include/vhdlexportdialog.h \
	ui/dialogs/include/langselectiondialog.h \
	ui/dialogs/include/errordisplaydialog.h \
	ui/dialogs/include/imageexportdialog.h \
	ui/equation_editor/include/equationeditor.h \
	ui/equation_editor/include/graphicequation.h \
	ui/equation_editor/include/equationmimedata.h \
	ui/equation_editor/include/constantvaluesetter.h \
	ui/equation_editor/include/inverterbar.h \
	ui/equation_editor/include/editableequation.h \
	ui/equation_editor/include/rangeextractorwidget.h \
	ui/refined_widget/include/dynamiclineedit.h \
	ui/refined_widget/include/contextmenu.h \
	ui/refined_widget/include/dynamictableitemdelegate.h \
	ui/refined_widget/include/reactivebutton.h \
	ui/refined_widget/include/tablewidgetwithresizeevent.h \
	ui/refined_widget/include/checkboxhtml.h \
	ui/refined_widget/include/labelwithclickevent.h \
	ui/refined_widget/include/statesgraphicsview.h \
	ui/refined_widget/include/collapsiblewidgetwithtitle.h \
	ui/refined_widget/include/lineeditwithupdownbuttons.h \
	ui/resource_bar/include/hinttab.h \
	ui/resource_bar/include/abouttab.h \
	ui/resource_bar/include/resourcebar.h \
	ui/resource_bar/include/verifiertab.h \
	ui/resource_bar/component_editor/include/stateeditortab.h \
	ui/resource_bar/component_editor/include/transitioneditortab.h \
	ui/resource_bar/component_editor/include/componenteditortab.h \
	ui/resource_bar/component_editor/include/rangeeditordialog.h \
	ui/resource_bar/component_editor/include/actioneditor.h \
	ui/resource_bar/component_editor/include/conditioneditor.h \
	ui/resource_bar/component_editor/include/actiontypecombobox.h \
	ui/resource_bar/component_editor/include/actiontablemodel.h \
	ui/resource_bar/component_editor/include/actiontabledelegate.h \
	ui/resource_bar/machine_editor_tab/include/machineeditortab.h \
	ui/resource_bar/machine_editor_tab/include/signallisteditor.h \
	ui/resource_bar/simulator_tab/include/simulatortab.h \
	ui/resource_bar/simulator_tab/include/inputsignalselector.h \
	ui/resource_bar/simulator_tab/include/inputbitselector.h \
	ui/resource_bar/simulator_tab/include/inputsselector.h \
	ui/display_area/include/displayarea.h \
	ui/display_area/include/maintoolbar.h \
	ui/display_area/machine_editor_widget/include/machineeditorwidget.h \
	ui/display_area/machine_editor_widget/scene/include/fsmscene.h \
	ui/display_area/machine_editor_widget/scene/include/blankscene.h \
	ui/display_area/machine_editor_widget/scene/include/genericscene.h \
	ui/display_area/machine_editor_widget/scene/include/scenewidget.h \
	ui/display_area/machine_editor_widget/toolbar/include/drawingtoolbar.h \
	ui/display_area/machine_editor_widget/toolbar/include/fsmdrawingtoolbar.h \
	ui/display_area/timeline_widget/include/timelinewidget.h \
	ui/display_area/timeline_widget/include/clocktimeline.h \
	ui/display_area/timeline_widget/include/graphicclocktimeline.h \
	ui/display_area/timeline_widget/include/graphictimeline.h \
	ui/display_area/timeline_widget/include/signaltimeline.h \
	ui/static/include/svgimagegenerator.h \
	ui/truth_table/include/truthtabledisplay.h \
	ui/truth_table/include/truthtableinputtablemodel.h \
	ui/truth_table/include/truthtableoutputtablemodel.h \
	# Other
	third_party/diff_match_patch/diff_match_patch.h

INCLUDEPATH += \
# Core
    core/include \
	core/basic_type/include \
	core/machine/include \
	core/machine/logic/include \
	core/machine/logic/components/include \
	core/machine/logic/fsm/include \
	core/machine/configuration/include \
	core/machine/import_export/include \
	core/machine/import_export/fsm/include \
	core/signal/include \
	core/simulation/include \
	core/exceptions/include \
	core/undo_engine/include \
# UI
    ui/include \
	ui/dialogs/include \
	ui/display_area/include \
	ui/display_area/machine_editor_widget/include \
	ui/display_area/machine_editor_widget/scene/include \
	ui/display_area/machine_editor_widget/toolbar/include \
	ui/display_area/timeline_widget/include \
	ui/graphic_machine/include \
	ui/graphic_machine/fsm/include \
	ui/equation_editor/include \
	ui/refined_widget/include \
	ui/truth_table/include \
	ui/resource_bar/include \
	ui/resource_bar/machine_editor_tab/include \
	ui/resource_bar/component_editor/include \
	ui/resource_bar/simulator_tab/include \
	ui/static/include \
# Other
    third_party/diff_match_patch

OTHER_FILES += \
    text/known_bugs.txt \
	text/Readme/README.txt \
	text/Readme/Linux/README \
	text/Readme/Windows/README.txt \
	text/release_notes.txt \
	text/upcoming_features.txt \
	text/code_guidelines.txt \
	LICENSE \
	LICENSE.txt \
	core/basic_type/README.txt \
	art/AUTHORS \
	art/AUTHORS.txt \
	TODO

RESOURCES += \
    art/art.qrc \
	translations/translations.qrc

TRANSLATIONS += \
    translations/french.ts
