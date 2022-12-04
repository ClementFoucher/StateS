#-------------------------------------------------
#
# Project created by QtCreator 2014-10-13T20:46:54
#
#-------------------------------------------------

###
# Update these
VERSION = 0.4.4
DATE = 2014-2022
# And remember to update version in readme and other text files
###


QT += core \
      gui \
	  widgets \
	  printsupport \
	  svg

# Only for diff-match-patch to use deprecated QRegExp... should seriously think about replacing this obsolete library
QT += core5compat

TARGET = StateS
TEMPLATE = app

QMAKE_TARGET_COPYRIGHT = copyright $$DATE Clément Foucher

DEFINES += STATES_VERSION=\\\"$$VERSION\\\"
DEFINES += STATES_YEARS=\\\"$$DATE\\\"

CONFIG += c++17
QMAKE_LFLAGS += -no-pie

SOURCES += \
	# Core
	src/core/main.cpp \
	src/core/states.cpp \
	src/core/statesxmlanalyzer.cpp \
	src/core/basic_type/logicvalue.cpp \
	src/core/basic_type/truthtable.cpp \
	src/core/machine/machinebuilder.cpp \
	src/core/machine/manager/machinemanager.cpp \
	src/core/machine/manager/machinestatus.cpp \
	src/core/machine/logic/machine.cpp \
	src/core/machine/logic/components/machinecomponent.cpp \
	src/core/machine/logic/components/actiononsignal.cpp \
	src/core/machine/logic/components/machineactuatorcomponent.cpp \
	src/core/machine/logic/fsm/fsm.cpp \
	src/core/machine/logic/fsm/fsmcomponent.cpp \
	src/core/machine/logic/fsm/fsmstate.cpp \
	src/core/machine/logic/fsm/fsmtransition.cpp \
	src/core/machine/logic/fsm/fsmverifier.cpp \
	src/core/machine/import_export/xmlimportexportbuilder.cpp \
	src/core/machine/import_export/machineimageexporter.cpp \
	src/core/machine/import_export/machinexmlparser.cpp \
	src/core/machine/import_export/machinexmlwriter.cpp \
	src/core/machine/import_export/fsm/fsmvhdlexport.cpp \
	src/core/machine/import_export/fsm/fsmxmlparser.cpp \
	src/core/machine/import_export/fsm/fsmxmlwriter.cpp \
	src/core/signal/signal.cpp \
	src/core/signal/input.cpp \
	src/core/signal/output.cpp \
	src/core/signal/constant.cpp \
	src/core/signal/equation.cpp \
	src/core/simulation/clock.cpp \
	src/core/simulation/fsmsimulator.cpp \
	src/core/simulation/machinesimulator.cpp \
	src/core/exceptions/statesexception.cpp \
	src/core/undo_engine/fsmundocommand.cpp \
	src/core/undo_engine/diffundocommand.cpp \
	src/core/undo_engine/machineundocommand.cpp \
	src/core/undo_engine/undoredomanager.cpp \
	# UI
	src/ui/statesui.cpp \
	src/ui/machinecomponentvisualizer.cpp \
	src/ui/graphic_machine/graphicactuator.cpp \
	src/ui/graphic_machine/graphiccomponent.cpp \
	src/ui/graphic_machine/viewconfiguration.cpp \
	src/ui/graphic_machine/fsm/fsmgraphicstate.cpp \
	src/ui/graphic_machine/fsm/fsmgraphictransition.cpp \
	src/ui/graphic_machine/fsm/fsmgraphictransitionneighborhood.cpp \
	src/ui/dialogs/imageexportdialog.cpp \
	src/ui/dialogs/langselectiondialog.cpp \
	src/ui/dialogs/vhdlexportdialog.cpp \
	src/ui/dialogs/errordisplaydialog.cpp \
	src/ui/equation_editor/inverterbar.cpp \
	src/ui/equation_editor/equationeditor.cpp \
	src/ui/equation_editor/graphicequation.cpp \
	src/ui/equation_editor/equationmimedata.cpp \
	src/ui/equation_editor/constantvaluesetter.cpp \
	src/ui/equation_editor/editableequation.cpp \
	src/ui/equation_editor/rangeextractorwidget.cpp \
	src/ui/refined_widget/dynamiclineedit.cpp \
	src/ui/refined_widget/contextmenu.cpp \
	src/ui/refined_widget/dynamictableitemdelegate.cpp \
	src/ui/refined_widget/reactivebutton.cpp \
	src/ui/refined_widget/tablewidgetwithresizeevent.cpp \
	src/ui/refined_widget/checkboxhtml.cpp \
	src/ui/refined_widget/labelwithclickevent.cpp \
	src/ui/refined_widget/collapsiblewidgetwithtitle.cpp \
	src/ui/refined_widget/statesgraphicsview.cpp \
	src/ui/refined_widget/lineeditwithupdownbuttons.cpp \
	src/ui/resource_bar/hinttab.cpp \
	src/ui/resource_bar/abouttab.cpp \
	src/ui/resource_bar/resourcebar.cpp \
	src/ui/resource_bar/verifiertab.cpp \
	src/ui/resource_bar/machine_editor_tab/machineeditortab.cpp \
	src/ui/resource_bar/machine_editor_tab/signallisteditor.cpp \
	src/ui/resource_bar/component_editor/transitioneditortab.cpp \
	src/ui/resource_bar/component_editor/stateeditortab.cpp \
	src/ui/resource_bar/component_editor/actioneditor.cpp \
	src/ui/resource_bar/component_editor/componenteditortab.cpp \
	src/ui/resource_bar/component_editor/rangeeditordialog.cpp \
	src/ui/resource_bar/component_editor/conditioneditor.cpp \
	src/ui/resource_bar/simulator_tab/simulatortab.cpp \
	src/ui/resource_bar/simulator_tab/inputbitselector.cpp \
	src/ui/resource_bar/simulator_tab/inputsselector.cpp \
	src/ui/resource_bar/simulator_tab/inputsignalselector.cpp \
	src/ui/display_area/displayarea.cpp \
	src/ui/display_area/maintoolbar.cpp \
	src/ui/display_area/machine_editor_widget/scene/fsmscene.cpp \
	src/ui/display_area/machine_editor_widget/machineeditorwidget.cpp \
	src/ui/display_area/machine_editor_widget/scene/blankscene.cpp \
	src/ui/display_area/machine_editor_widget/scene/genericscene.cpp \
	src/ui/display_area/machine_editor_widget/scene/scenewidget.cpp \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbar.cpp \
	src/ui/display_area/machine_editor_widget/toolbar/fsmdrawingtoolbar.cpp \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbarbuilder.cpp \
	src/ui/display_area/timeline_widget/timelinewidget.cpp \
	src/ui/display_area/timeline_widget/clocktimeline.cpp \
	src/ui/display_area/timeline_widget/graphicclocktimeline.cpp \
	src/ui/display_area/timeline_widget/graphictimeline.cpp \
	src/ui/display_area/timeline_widget/signaltimeline.cpp \
	src/ui/static/svgimagegenerator.cpp \
	src/ui/truth_table/truthtabledisplay.cpp \
	src/ui/truth_table/truthtableinputtablemodel.cpp \
	src/ui/truth_table/truthtableoutputtablemodel.cpp \
	src/ui/resource_bar/component_editor/actiontypecombobox.cpp \
	src/ui/resource_bar/component_editor/actiontablemodel.cpp \
	src/ui/resource_bar/component_editor/actiontabledelegate.cpp \
	# Other
	src/third_party/diff_match_patch/diff_match_patch.cpp

HEADERS += \
	# Core
	src/core/states.h \
	src/core/statesxmlanalyzer.h \
	src/core/basic_type/logicvalue.h \
	src/core/basic_type/truthtable.h \
	src/core/machine/machinebuilder.h \
	src/core/machine/manager/machinemanager.h \
	src/core/machine/manager/machinestatus.h \
	src/core/machine/logic/machine.h \
	src/core/machine/logic/components/machinecomponent.h \
	src/core/machine/logic/components/actiononsignal.h \
	src/core/machine/logic/components/machineactuatorcomponent.h \
	src/core/machine/logic/fsm/fsm.h \
	src/core/machine/logic/fsm/fsmcomponent.h \
	src/core/machine/logic/fsm/fsmstate.h \
	src/core/machine/logic/fsm/fsmtransition.h \
	src/core/machine/logic/fsm/fsmverifier.h \
	src/core/machine/import_export/xmlimportexportbuilder.h \
	src/core/machine/import_export/machineimageexporter.h \
	src/core/machine/import_export/machinexmlparser.h \
	src/core/machine/import_export/machinexmlwriter.h \
	src/core/machine/import_export/fsm/fsmvhdlexport.h \
	src/core/machine/import_export/fsm/fsmxmlparser.h \
	src/core/machine/import_export/fsm/fsmxmlwriter.h \
	src/core/signal/StateS_signal.h \
	src/core/signal/input.h \
	src/core/signal/output.h \
	src/core/signal/constant.h \
	src/core/signal/equation.h \
	src/core/simulation/clock.h \
	src/core/simulation/fsmsimulator.h \
	src/core/simulation/machinesimulator.h \
	src/core/exceptions/statesexception.h \
	src/core/undo_engine/fsmundocommand.h \
	src/core/undo_engine/diffundocommand.h \
	src/core/undo_engine/machineundocommand.h \
	src/core/undo_engine/undoredomanager.h \
	# UI
	src/ui/statesui.h \
	src/ui/machinecomponentvisualizer.h \
	src/ui/graphic_machine/graphicactuator.h \
	src/ui/graphic_machine/graphiccomponent.h \
	src/ui/graphic_machine/viewconfiguration.h \
	src/ui/graphic_machine/fsm/fsmgraphicstate.h \
	src/ui/graphic_machine/fsm/fsmgraphictransition.h \
	src/ui/graphic_machine/fsm/fsmgraphictransitionneighborhood.h \
	src/ui/dialogs/vhdlexportdialog.h \
	src/ui/dialogs/langselectiondialog.h \
	src/ui/dialogs/errordisplaydialog.h \
	src/ui/dialogs/imageexportdialog.h \
	src/ui/equation_editor/equationeditor.h \
	src/ui/equation_editor/graphicequation.h \
	src/ui/equation_editor/equationmimedata.h \
	src/ui/equation_editor/constantvaluesetter.h \
	src/ui/equation_editor/inverterbar.h \
	src/ui/equation_editor/editableequation.h \
	src/ui/equation_editor/rangeextractorwidget.h \
	src/ui/refined_widget/dynamiclineedit.h \
	src/ui/refined_widget/contextmenu.h \
	src/ui/refined_widget/dynamictableitemdelegate.h \
	src/ui/refined_widget/reactivebutton.h \
	src/ui/refined_widget/tablewidgetwithresizeevent.h \
	src/ui/refined_widget/checkboxhtml.h \
	src/ui/refined_widget/labelwithclickevent.h \
	src/ui/refined_widget/statesgraphicsview.h \
	src/ui/refined_widget/collapsiblewidgetwithtitle.h \
	src/ui/refined_widget/lineeditwithupdownbuttons.h \
	src/ui/resource_bar/hinttab.h \
	src/ui/resource_bar/abouttab.h \
	src/ui/resource_bar/resourcebar.h \
	src/ui/resource_bar/verifiertab.h \
	src/ui/resource_bar/component_editor/stateeditortab.h \
	src/ui/resource_bar/component_editor/transitioneditortab.h \
	src/ui/resource_bar/component_editor/componenteditortab.h \
	src/ui/resource_bar/component_editor/rangeeditordialog.h \
	src/ui/resource_bar/component_editor/actioneditor.h \
	src/ui/resource_bar/component_editor/conditioneditor.h \
	src/ui/resource_bar/component_editor/actiontypecombobox.h \
	src/ui/resource_bar/component_editor/actiontablemodel.h \
	src/ui/resource_bar/component_editor/actiontabledelegate.h \
	src/ui/resource_bar/machine_editor_tab/machineeditortab.h \
	src/ui/resource_bar/machine_editor_tab/signallisteditor.h \
	src/ui/resource_bar/simulator_tab/simulatortab.h \
	src/ui/resource_bar/simulator_tab/inputsignalselector.h \
	src/ui/resource_bar/simulator_tab/inputbitselector.h \
	src/ui/resource_bar/simulator_tab/inputsselector.h \
	src/ui/display_area/displayarea.h \
	src/ui/display_area/maintoolbar.h \
	src/ui/display_area/machine_editor_widget/machineeditorwidget.h \
	src/ui/display_area/machine_editor_widget/scene/fsmscene.h \
	src/ui/display_area/machine_editor_widget/scene/blankscene.h \
	src/ui/display_area/machine_editor_widget/scene/genericscene.h \
	src/ui/display_area/machine_editor_widget/scene/scenewidget.h \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbar.h \
	src/ui/display_area/machine_editor_widget/toolbar/fsmdrawingtoolbar.h \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbarbuilder.h \
	src/ui/display_area/timeline_widget/timelinewidget.h \
	src/ui/display_area/timeline_widget/clocktimeline.h \
	src/ui/display_area/timeline_widget/graphicclocktimeline.h \
	src/ui/display_area/timeline_widget/graphictimeline.h \
	src/ui/display_area/timeline_widget/signaltimeline.h \
	src/ui/static/svgimagegenerator.h \
	src/ui/truth_table/truthtabledisplay.h \
	src/ui/truth_table/truthtableinputtablemodel.h \
	src/ui/truth_table/truthtableoutputtablemodel.h \
	# Other
	src/third_party/diff_match_patch/diff_match_patch.h

INCLUDEPATH += \
	# Core
	src/core \
	src/core/basic_type \
	src/core/machine \
	src/core/machine/logic \
	src/core/machine/logic/components \
	src/core/machine/logic/fsm \
	src/core/machine/manager \
	src/core/machine/import_export \
	src/core/machine/import_export/fsm \
	src/core/signal \
	src/core/simulation \
	src/core/exceptions \
	src/core/undo_engine \
	# UI
	src/ui \
	src/ui/dialogs \
	src/ui/display_area \
	src/ui/display_area/machine_editor_widget \
	src/ui/display_area/machine_editor_widget/scene \
	src/ui/display_area/machine_editor_widget/toolbar \
	src/ui/display_area/timeline_widget \
	src/ui/graphic_machine \
	src/ui/graphic_machine/fsm \
	src/ui/equation_editor \
	src/ui/refined_widget \
	src/ui/truth_table \
	src/ui/resource_bar \
	src/ui/resource_bar/machine_editor_tab \
	src/ui/resource_bar/component_editor \
	src/ui/resource_bar/simulator_tab \
	src/ui/static \
	# Other
	src/third_party/diff_match_patch

OTHER_FILES += \
	README.md \
	text/known_bugs.txt \
	text/Readme/Linux/README \
	text/Readme/Windows/README.txt \
	text/release_notes.txt \
	text/upcoming_features.txt \
	text/code_guidelines.txt \
	LICENSE \
	LICENSE.txt \
	src/core/basic_type/README.txt \
	art/AUTHORS \
	art/AUTHORS.txt \
	TODO

RESOURCES += \
	art/art.qrc \
	translations/translations.qrc

TRANSLATIONS += \
	translations/french.ts
