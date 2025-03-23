#-------------------------------------------------
#
# Project created by QtCreator 2014-10-13T20:46:54
#
#-------------------------------------------------

###
# Update these
VERSION = 0.4.7
DATE = 2014-2025
# Also remember to update version in README.md, known_bugs.txt and AppImageBuilder.yml
###


QT += core \
      gui \
      widgets \
      printsupport \
      svg

# Only for diff-match-patch to use deprecated QRegExp... should seriously think about replacing this obsolete library
QT += core5compat

# Uncomment the following line when building with MSys2's MinGW Qt6 static
#win32: QMAKE_LIBS += -lgraphite2 -lbz2 -lusp10 -lRpcrt4
win32: RC_ICONS = art/ico/StateS.ico

TARGET = StateS
TEMPLATE = app

QMAKE_TARGET_COPYRIGHT = copyright $$DATE Clément Foucher

DEFINES += STATES_VERSION=\\\"$$VERSION\\\"
DEFINES += STATES_YEARS=\\\"$$DATE\\\"

CONFIG += c++17
QMAKE_LFLAGS += -no-pie

SOURCES += \
	src/main.cpp \
	# Core
	src/core/states.cpp \
	src/core/basic_type/logicvalue.cpp \
	src/core/basic_type/truthtable.cpp \
	src/core/machine_manager/machinebuilder.cpp \
	src/core/machine_manager/machinemanager.cpp \
	src/core/machine_manager/machinesimulator.cpp \
	src/core/machine_manager/machinestatus.cpp \
	src/core/exceptions/statesexception.cpp \
	src/core/undo_engine/fsmundocommand.cpp \
	src/core/undo_engine/diffundocommand.cpp \
	src/core/undo_engine/machineundocommand.cpp \
	src/core/undo_engine/undoredomanager.cpp \
	src/core/xml/statesxmlanalyzer.cpp \
	# Machine
	src/machine/export/machineimageexporter.cpp \
	src/machine/export/fsm/fsmvhdlexport.cpp \
	src/machine/graphic/graphicmachine.cpp \
	src/machine/graphic/components/graphiccomponent.cpp \
	src/machine/graphic/components/subcomponents/actionbox.cpp \
	src/machine/graphic/fsm/graphicfsm.cpp \
	src/machine/graphic/fsm/components/graphicfsmstate.cpp \
	src/machine/graphic/fsm/components/graphicfsmtransition.cpp \
	src/machine/graphic/fsm/components/graphicsimulatedfsmstate.cpp \
	src/machine/graphic/fsm/components/graphicsimulatedfsmtransition.cpp \
	src/machine/graphic/fsm/components/helpers/graphicfsmtransitionneighborhood.cpp \
	src/machine/logic/machine.cpp \
	src/machine/logic/components/machinecomponent.cpp \
	src/machine/logic/components/machineactuatorcomponent.cpp \
	src/machine/logic/components/variable.cpp \
	src/machine/logic/components/subcomponents/actiononvariable.cpp \
	src/machine/logic/components/subcomponents/equation.cpp \
	src/machine/logic/components/subcomponents/operand.cpp \
	src/machine/logic/fsm/fsm.cpp \
	src/machine/logic/fsm/components/fsmstate.cpp \
	src/machine/logic/fsm/components/fsmtransition.cpp \
	src/machine/logic/fsm/verifier/fsmverifier.cpp \
	src/machine/simulated/simulatedmachine.cpp \
	src/machine/simulated/components/simulatedactuatorcomponent.cpp \
	src/machine/simulated/components/simulatedcomponent.cpp \
	src/machine/simulated/components/simulatedvariable.cpp \
	src/machine/simulated/components/subcomponents/simulatedactiononvariable.cpp \
	src/machine/simulated/components/subcomponents/simulatedequation.cpp \
	src/machine/simulated/components/subcomponents/simulatedoperand.cpp \
	src/machine/simulated/fsm/simulatedfsm.cpp \
	src/machine/simulated/fsm/components/simulatedfsmstate.cpp \
	src/machine/simulated/fsm/components/simulatedfsmtransition.cpp \
	src/machine/xml/graphicattributes.cpp \
	src/machine/xml/machinexmlparser.cpp \
	src/machine/xml/machinexmlwriter.cpp \
	src/machine/xml/xmlimportexportbuilder.cpp \
	src/machine/xml/fsm/fsmxmlparser.cpp \
	src/machine/xml/fsm/fsmxmlwriter.cpp \
	# UI
	src/ui/statesui.cpp \
	src/ui/dialogs/imageexportdialog.cpp \
	src/ui/dialogs/langselectiondialog.cpp \
	src/ui/dialogs/vhdlexportdialog.cpp \
	src/ui/dialogs/errordisplaydialog.cpp \
	src/ui/equation_editor/constanteditorwidget.cpp \
	src/ui/equation_editor/editableequation.cpp \
	src/ui/equation_editor/equationeditordialog.cpp \
	src/ui/equation_editor/equationeditorwidget.cpp \
	src/ui/equation_editor/equationpartmimedata.cpp \
	src/ui/equation_editor/inverterbarwidget.cpp \
	src/ui/equation_editor/rangeextractorwidget.cpp \
	src/ui/equation_editor/templateequationpartswidget.cpp \
	src/ui/refined_widget/contextmenu.cpp \
	src/ui/refined_widget/reactivebutton.cpp \
	src/ui/refined_widget/checkboxhtml.cpp \
	src/ui/refined_widget/labelwithclickevent.cpp \
	src/ui/refined_widget/collapsiblewidgetwithtitle.cpp \
	src/ui/refined_widget/statesgraphicsview.cpp \
	src/ui/refined_widget/editors/dynamiclineeditor.cpp \
	src/ui/refined_widget/editors/linewithupdownbuttonseditor.cpp \
	src/ui/refined_widget/editors/selfmanageddynamiclineeditor.cpp \
	src/ui/resource_bar/hinttab.cpp \
	src/ui/resource_bar/abouttab.cpp \
	src/ui/resource_bar/resourcebar.cpp \
	src/ui/resource_bar/verifiertab.cpp \
	src/ui/resource_bar/machinecomponentvisualizer.cpp \
	src/ui/resource_bar/machine_editor_tab/machineeditortab.cpp \
	src/ui/resource_bar/machine_editor_tab/variablelisteditor.cpp \
	src/ui/resource_bar/machine_editor_tab/variable_table/variabletablemodel.cpp \
	src/ui/resource_bar/machine_editor_tab/variable_table/variabletableview.cpp \
	src/ui/resource_bar/machine_editor_tab/variable_table/delegates/variabletablenamedelegate.cpp \
	src/ui/resource_bar/machine_editor_tab/variable_table/delegates/variabletablesizedelegate.cpp \
	src/ui/resource_bar/machine_editor_tab/variable_table/delegates/variabletablevaluedelegate.cpp \
	src/ui/resource_bar/component_editor/transitioneditortab.cpp \
	src/ui/resource_bar/component_editor/stateeditortab.cpp \
	src/ui/resource_bar/component_editor/actioneditor.cpp \
	src/ui/resource_bar/component_editor/componenteditortab.cpp \
	src/ui/resource_bar/component_editor/rangeeditordialog.cpp \
	src/ui/resource_bar/component_editor/conditioneditor.cpp \
	src/ui/resource_bar/simulator_tab/simulatortab.cpp \
	src/ui/resource_bar/simulator_tab/inputbitselector.cpp \
	src/ui/resource_bar/simulator_tab/inputsselector.cpp \
	src/ui/resource_bar/simulator_tab/inputvariableselector.cpp \
	src/ui/resource_bar/simulator_tab/simulatorconfigurator.cpp \
	src/ui/resource_bar/simulator_tab/simulatortimecontroller.cpp \
	src/ui/display_area/displayarea.cpp \
	src/ui/display_area/maintoolbar.cpp \
	src/ui/display_area/machine_editor_widget/scene/fsmscene.cpp \
	src/ui/display_area/machine_editor_widget/machineeditorwidget.cpp \
	src/ui/display_area/machine_editor_widget/scene/blankscene.cpp \
	src/ui/display_area/machine_editor_widget/scene/genericscene.cpp \
	src/ui/display_area/machine_editor_widget/scene/scenewidget.cpp \
	src/ui/display_area/machine_editor_widget/scene/viewconfiguration.cpp \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbar.cpp \
	src/ui/display_area/machine_editor_widget/toolbar/fsmdrawingtoolbar.cpp \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbarbuilder.cpp \
	src/ui/display_area/timeline_widget/timelinewidget.cpp \
	src/ui/display_area/timeline_widget/clocktimeline.cpp \
	src/ui/display_area/timeline_widget/graphicbittimeline.cpp \
	src/ui/display_area/timeline_widget/graphicclocktimeline.cpp \
	src/ui/display_area/timeline_widget/graphictimeline.cpp \
	src/ui/display_area/timeline_widget/graphicvectortimeline.cpp \
	src/ui/display_area/timeline_widget/statetimeline.cpp \
	src/ui/display_area/timeline_widget/variabletimeline.cpp \
	src/ui/static/pixmapgenerator.cpp \
	src/ui/truth_table/truthtabledisplay.cpp \
	src/ui/truth_table/truthtableinputtablemodel.cpp \
	src/ui/truth_table/truthtableoutputtablemodel.cpp \
	src/ui/resource_bar/component_editor/actiontypecombobox.cpp \
	src/ui/resource_bar/component_editor/actiontablemodel.cpp \
	src/ui/resource_bar/component_editor/actiontabledelegate.cpp \
	# Third party
	src/third_party/diff_match_patch/diff_match_patch.cpp

HEADERS += \
    # Core
	src/core/exceptiontypes.h \
	src/core/states.h \
	src/core/statestypes.h \
	src/core/basic_type/logicvalue.h \
	src/core/basic_type/truthtable.h \
	src/core/machine_manager/machinebuilder.h \
	src/core/machine_manager/machinemanager.h \
	src/core/machine_manager/machinesimulator.h \
	src/core/machine_manager/machinestatus.h \
	src/core/exceptions/statesexception.h \
	src/core/undo_engine/fsmundocommand.h \
	src/core/undo_engine/diffundocommand.h \
	src/core/undo_engine/machineundocommand.h \
	src/core/undo_engine/undoredomanager.h \
	src/core/xml/statesxmlanalyzer.h \
	# Machine
	src/machine/export/machineimageexporter.h \
	src/machine/export/fsm/fsmvhdlexport.h \
	src/machine/graphic/graphicmachine.h \
	src/machine/graphic/components/graphiccomponent.h \
	src/machine/graphic/components/subcomponents/actionbox.h \
	src/machine/graphic/fsm/graphicfsm.h \
	src/machine/graphic/fsm/components/graphicfsmstate.h \
	src/machine/graphic/fsm/components/graphicfsmtransition.h \
	src/machine/graphic/fsm/components/graphicsimulatedfsmstate.h \
	src/machine/graphic/fsm/components/graphicsimulatedfsmtransition.h \
	src/machine/graphic/fsm/components/helpers/graphicfsmtransitionneighborhood.h \
	src/machine/logic/machine.h \
	src/machine/logic/components/machinecomponent.h \
	src/machine/logic/components/machineactuatorcomponent.h \
	src/machine/logic/components/variable.h \
	src/machine/logic/components/subcomponents/actiononvariable.h \
	src/machine/logic/components/subcomponents/equation.h \
	src/machine/logic/components/subcomponents/operand.h \
	src/machine/logic/fsm/fsm.h \
	src/machine/logic/fsm/components/fsmstate.h \
	src/machine/logic/fsm/components/fsmtransition.h \
	src/machine/logic/fsm/verifier/fsmverifier.h \
	src/machine/simulated/simulatedmachine.h \
	src/machine/simulated/components/simulatedactuatorcomponent.h \
	src/machine/simulated/components/simulatedcomponent.h \
	src/machine/simulated/components/simulatedvariable.h \
	src/machine/simulated/components/subcomponents/simulatedactiononvariable.h \
	src/machine/simulated/components/subcomponents/simulatedequation.h \
	src/machine/simulated/components/subcomponents/simulatedoperand.h \
	src/machine/simulated/fsm/simulatedfsm.h \
	src/machine/simulated/fsm/components/simulatedfsmstate.h \
	src/machine/simulated/fsm/components/simulatedfsmtransition.h \
	src/machine/xml/graphicattributes.h \
	src/machine/xml/xmlimportexportbuilder.h \
	src/machine/xml/machinexmlparser.h \
	src/machine/xml/machinexmlwriter.h \
	src/machine/xml/fsm/fsmxmlparser.h \
	src/machine/xml/fsm/fsmxmlwriter.h \
	# UI
	src/ui/statesui.h \
	src/ui/dialogs/vhdlexportdialog.h \
	src/ui/dialogs/langselectiondialog.h \
	src/ui/dialogs/errordisplaydialog.h \
	src/ui/dialogs/imageexportdialog.h \
	src/ui/equation_editor/constanteditorwidget.h \
	src/ui/equation_editor/editableequation.h \
	src/ui/equation_editor/equationeditordialog.h \
	src/ui/equation_editor/equationeditorwidget.h \
	src/ui/equation_editor/equationpartmimedata.h \
	src/ui/equation_editor/inverterbarwidget.h \
	src/ui/equation_editor/rangeextractorwidget.h \
	src/ui/equation_editor/templateequationpartswidget.h \
	src/ui/refined_widget/contextmenu.h \
	src/ui/refined_widget/reactivebutton.h \
	src/ui/refined_widget/checkboxhtml.h \
	src/ui/refined_widget/labelwithclickevent.h \
	src/ui/refined_widget/statesgraphicsview.h \
	src/ui/refined_widget/collapsiblewidgetwithtitle.h \
	src/ui/refined_widget/editors/dynamiclineeditor.h \
	src/ui/refined_widget/editors/linewithupdownbuttonseditor.h \
	src/ui/refined_widget/editors/selfmanageddynamiclineeditor.h \
	src/ui/resource_bar/hinttab.h \
	src/ui/resource_bar/abouttab.h \
	src/ui/resource_bar/resourcebar.h \
	src/ui/resource_bar/verifiertab.h \
	src/ui/resource_bar/machinecomponentvisualizer.h \
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
	src/ui/resource_bar/machine_editor_tab/variablelisteditor.h \
	src/ui/resource_bar/machine_editor_tab/variable_table/variabletablemodel.h \
	src/ui/resource_bar/machine_editor_tab/variable_table/variabletableview.h \
	src/ui/resource_bar/machine_editor_tab/variable_table/delegates/variabletablenamedelegate.h \
	src/ui/resource_bar/machine_editor_tab/variable_table/delegates/variabletablesizedelegate.h \
	src/ui/resource_bar/machine_editor_tab/variable_table/delegates/variabletablevaluedelegate.h \
	src/ui/resource_bar/simulator_tab/simulatortab.h \
	src/ui/resource_bar/simulator_tab/inputbitselector.h \
	src/ui/resource_bar/simulator_tab/inputsselector.h \
	src/ui/resource_bar/simulator_tab/inputvariableselector.h \
	src/ui/resource_bar/simulator_tab/simulatorconfigurator.h \
	src/ui/resource_bar/simulator_tab/simulatortimecontroller.h \
	src/ui/display_area/displayarea.h \
	src/ui/display_area/maintoolbar.h \
	src/ui/display_area/machine_editor_widget/machineeditorwidget.h \
	src/ui/display_area/machine_editor_widget/scene/fsmscene.h \
	src/ui/display_area/machine_editor_widget/scene/blankscene.h \
	src/ui/display_area/machine_editor_widget/scene/genericscene.h \
	src/ui/display_area/machine_editor_widget/scene/scenewidget.h \
	src/ui/display_area/machine_editor_widget/scene/viewconfiguration.h \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbar.h \
	src/ui/display_area/machine_editor_widget/toolbar/fsmdrawingtoolbar.h \
	src/ui/display_area/machine_editor_widget/toolbar/drawingtoolbarbuilder.h \
	src/ui/display_area/timeline_widget/timelinewidget.h \
	src/ui/display_area/timeline_widget/clocktimeline.h \
	src/ui/display_area/timeline_widget/graphicbittimeline.h \
	src/ui/display_area/timeline_widget/graphicclocktimeline.h \
	src/ui/display_area/timeline_widget/graphictimeline.h \
	src/ui/display_area/timeline_widget/graphicvectortimeline.h \
	src/ui/display_area/timeline_widget/statetimeline.h \
	src/ui/display_area/timeline_widget/variabletimeline.h \
	src/ui/static/pixmapgenerator.h \
	src/ui/truth_table/truthtabledisplay.h \
	src/ui/truth_table/truthtableinputtablemodel.h \
	src/ui/truth_table/truthtableoutputtablemodel.h \
	# Third party
	src/third_party/diff_match_patch/diff_match_patch.h

INCLUDEPATH += \
    # Core
	src \
	src/core \
	src/core/basic_type \
	src/core/exceptions \
	src/core/machine_manager \
	src/core/simulation \
	src/core/undo_engine \
	src/core/xml \
	# Machine
	src/machine/export \
	src/machine/export/fsm \
	src/machine/graphic \
	src/machine/graphic/components \
	src/machine/graphic/components/subcomponents \
	src/machine/graphic/fsm \
	src/machine/graphic/fsm/components \
	src/machine/graphic/fsm/components/helpers \
	src/machine/logic \
	src/machine/logic/components \
	src/machine/logic/components/subcomponents \
	src/machine/logic/fsm \
	src/machine/logic/fsm/components \
	src/machine/logic/fsm/verifier \
	src/machine/simulated \
	src/machine/simulated/components \
	src/machine/simulated/components/subcomponents \
	src/machine/simulated/fsm \
	src/machine/simulated/fsm/components \
	src/machine/xml \
	src/machine/xml/fsm \
	# UI
	src/ui \
	src/ui/dialogs \
	src/ui/display_area \
	src/ui/display_area/machine_editor_widget \
	src/ui/display_area/machine_editor_widget/scene \
	src/ui/display_area/machine_editor_widget/toolbar \
	src/ui/display_area/timeline_widget \
	src/ui/equation_editor \
	src/ui/refined_widget \
	src/ui/refined_widget/editors \
	src/ui/truth_table \
	src/ui/resource_bar \
	src/ui/resource_bar/machine_editor_tab \
	src/ui/resource_bar/machine_editor_tab/variable_table \
	src/ui/resource_bar/machine_editor_tab/variable_table/delegates \
	src/ui/resource_bar/component_editor \
	src/ui/resource_bar/simulator_tab \
	src/ui/static \
	# Third party
	src/third_party/diff_match_patch

OTHER_FILES += \
    README.md \
    LICENSE \
    art/AUTHORS.md \
    text/known_bugs.txt \
    text/release_notes.txt \
    text/upcoming_features.txt \
    text/code_guidelines.txt

RESOURCES += \
    art/art.qrc \
    translations/translations.qrc

TRANSLATIONS += \
    translations/french.ts
