#-------------------------------------------------
#
# Project created by QtCreator 2014-10-13T20:46:54
#
#-------------------------------------------------

QT += \
      core \
      gui \
      printsupport \
      xml \
      svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StateS
TEMPLATE = app


SOURCES += \
    core/machine/fsm/fsm.cpp \
    core/main.cpp \
    core/states.cpp \
    core/machine/fsm/fsmstate.cpp \
    core/machine/fsm/fsmtransition.cpp \
    ui/scene/fsm/fsmgraphicalstate.cpp \
    ui/scene/fsm/fsmgraphicaltransition.cpp \
    ui/scene/scenewidget.cpp \
    ui/scene/genericscene.cpp \
    ui/scene/fsm/fsmgraphicaltransitionneighborhood.cpp \
    core/signal/inout.cpp \
    core/signal/input.cpp \
    core/signal/io.cpp \
    core/simulation/clock.cpp \
    ui/simulation/graphictimeline.cpp \
    ui/simulation/graphicclocktimeline.cpp \
    ui/simulation/clocktimeline.cpp \
    ui/resource_bar/component_editor/transitioneditortab.cpp \
    ui/resource_bar/component_editor/stateeditortab.cpp \
    ui/equation_editor/equationeditor.cpp \
    ui/equation_editor/graphicequation.cpp \
    ui/equation_editor/equationmimedata.cpp \
    ui/resource_bar/abouttab.cpp \
    core/signal/output.cpp \
    ui/scene/fsm/fsmscene.cpp \
    ui/resource_bar/simulator_tab/simulatortab.cpp \
    ui/equation_editor/inverterbar.cpp \
    core/signal/signal.cpp \
    core/signal/equation.cpp \
    core/machine/machine.cpp \
    ui/refined_widget/dynamiclineedit.cpp \
    ui/refined_widget/contextmenu.cpp \
    ui/resource_bar/component_editor/actionlisteditor.cpp \
    ui/resource_bar/component_editor/componenteditortab.cpp \
    ui/resource_bar/signal_editor/signaleditortab.cpp \
    ui/resource_bar/signal_editor/signallisteditor.cpp \
    ui/refined_widget/dynamictableitemdelegate.cpp \
    ui/resource_bar/machine_builder/machinebuildertab.cpp \
    ui/resource_bar/resourcebar.cpp \
    ui/refined_widget/reactivebutton.cpp \
    ui/statesui.cpp \
    ui/langselectiondialog.cpp \
    ui/resource_bar/component_editor/actioneditor.cpp \
    core/machine/machinecomponent.cpp \
    core/machine/machineactuatorcomponent.cpp \
    core/machine/fsm/fsmcomponent.cpp \
    core/basic_type/logicvalue.cpp \
    ui/simulation/signaltimeline.cpp \
    ui/resource_bar/simulator_tab/inputbitselector.cpp \
    ui/resource_bar/simulator_tab/inputsselector.cpp \
    ui/resource_bar/simulator_tab/inputsignalselector.cpp \
    core/machine/fsm/fsmvhdlexport.cpp \
    core/basic_type/truthtable.cpp \
    ui/truthtabledisplay.cpp \
    ui/resource_bar/component_editor/conditioneditor.cpp \
    core/machine/fsm/fsmverifier.cpp \
    ui/resource_bar/verifiertab.cpp \
    core/simulation/fsmsimulator.cpp \
    ui/simulation/simulationwidget.cpp \
    ui/resource_bar/machinecomponentvisualizer.cpp \
    ui/refined_widget/tablewidgetwithresizeevent.cpp \
    ui/refined_widget/checkboxhtml.cpp \
    ui/refined_widget/labelwithclickevent.cpp \
    core/machine/machinebuilder.cpp \
    ui/resource_bar/machine_builder/fsmtoolspanel.cpp \
    ui/resource_bar/machine_builder/machinetoolspanel.cpp \
    ui/refined_widget/collapsiblewidgetwithtitle.cpp \
    ui/displayarea.cpp \
    core/simulation/machinesimulator.cpp \
    ui/scene/blankscene.cpp \
    ui/scene/machineimageexporter.cpp \
    ui/imageexportdialog.cpp \
    ui/vhdlexportdialog.cpp


HEADERS  += \
    core/machine/fsm/include/fsm.h \
    core/include/states.h \
    core/machine/fsm/include/fsmstate.h \
    core/machine/fsm/include/fsmtransition.h \
    ui/scene/fsm/include/fsmgraphicalstate.h \
    ui/scene/fsm/include/fsmgraphicaltransition.h \
    ui/scene/include/scenewidget.h \
    ui/scene/include/genericscene.h \
    ui/scene/fsm/include/fsmgraphicaltransitionneighborhood.h \
    core/signal/include/inout.h \
    core/signal/include/input.h \
    core/signal/include/io.h \
    core/simulation/include/clock.h \
    ui/simulation/include/graphictimeline.h \
    ui/simulation/include/graphicclocktimeline.h \
    ui/resource_bar/component_editor/include/stateeditortab.h \
    ui/resource_bar/component_editor/include/transitioneditortab.h \
    ui/equation_editor/include/equationeditor.h \
    ui/equation_editor/include/graphicequation.h \
    ui/equation_editor/include/equationmimedata.h \
    core/signal/include/output.h \
    ui/scene/fsm/include/fsmscene.h \
    ui/resource_bar/include/abouttab.h \
    ui/simulation/include/clocktimeline.h \
    ui/resource_bar/simulator_tab/include/simulatortab.h \
    ui/equation_editor/include/inverterbar.h \
    core/signal/include/signal.h \
    core/signal/include/equation.h \
    core/machine/include/machine.h \
    ui/refined_widget/include/dynamiclineedit.h \
    ui/refined_widget/include/contextmenu.h \
    ui/resource_bar/component_editor/include/actionlisteditor.h \
    ui/resource_bar/component_editor/include/componenteditortab.h \
    ui/resource_bar/signal_editor/include/signaleditortab.h \
    ui/resource_bar/signal_editor/include/signallisteditor.h \
    ui/refined_widget/include/dynamictableitemdelegate.h \
    ui/resource_bar/machine_builder/include/machinebuildertab.h \
    ui/resource_bar/include/resourcebar.h \
    ui/refined_widget/include/reactivebutton.h \
    ui/include/statesui.h \
    ui/include/langselectiondialog.h \
    ui/resource_bar/component_editor/include/actioneditor.h \
    core/machine/include/machinecomponent.h \
    core/machine/include/machineactuatorcomponent.h \
    core/machine/fsm/include/fsmcomponent.h \
    core/basic_type/include/logicvalue.h \
    ui/simulation/include/signaltimeline.h \
    ui/resource_bar/simulator_tab/include/inputsignalselector.h \
    ui/resource_bar/simulator_tab/include/inputbitselector.h \
    ui/resource_bar/simulator_tab/include/inputsselector.h \
    core/machine/fsm/include/fsmvhdlexport.h \
    core/basic_type/include/truthtable.h \
    ui/include/truthtabledisplay.h \
    ui/resource_bar/component_editor/include/conditioneditor.h \
    core/machine/fsm/include/fsmverifier.h \
    ui/resource_bar/include/verifiertab.h \
    core/simulation/include/fsmsimulator.h \
    ui/simulation/include/simulationwidget.h \
    ui/resource_bar/include/machinecomponentvisualizer.h \
    ui/refined_widget/include/tablewidgetwithresizeevent.h \
    ui/refined_widget/include/checkboxhtml.h \
    ui/refined_widget/include/labelwithclickevent.h \
    core/machine/include/machinebuilder.h \
    ui/resource_bar/machine_builder/include/fsmtoolspanel.h \
    ui/resource_bar/machine_builder/include/machinetoolspanel.h \
    ui/refined_widget/include/collapsiblewidgetwithtitle.h \
    ui/include/displayarea.h \
    core/simulation/include/machinesimulator.h \
    ui/scene/include/blankscene.h \
    ui/scene/include/machineimageexporter.h \
    ui/include/imageexportdialog.h \
    ui/include/vhdlexportdialog.h

unix:QMAKE_CXXFLAGS += \
    -std=c++11 \
    -pedantic

INCLUDEPATH += \
# Core
    core/include \
    core/basic_type/include \
    core/machine/include \
    core/machine/fsm/include \
    core/signal/include \
    core/simulation/include \
# UI
    ui/include \
    ui/equation_editor/include \
    ui/refined_widget/include \
    ui/resource_bar/include \
    ui/resource_bar/component_editor/include \
    ui/resource_bar/machine_builder/include \
    ui/resource_bar/signal_editor/include \
    ui/resource_bar/simulator_tab/include \
    ui/scene/include \
    ui/scene/fsm/include \
    ui/simulation/include

OTHER_FILES += \
    known_bugs.txt \
    README \
    README.txt \
    LICENSE \
    LICENSE.txt \
    release_notes.txt \
    core/basic_type/README.txt \
    art/AUTHORS \
    art/AUTHORS.txt \
    TODO

RESOURCES += \
    art.qrc \
    translations.qrc

TRANSLATIONS += \
    translations/français.ts

DISTFILES += \
    upcoming_features.txt

