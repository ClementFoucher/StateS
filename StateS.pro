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
    core/main.cpp \
    core/states.cpp \
    core/basic_type/logicvalue.cpp \
    core/basic_type/truthtable.cpp \
    core/machine/machine.cpp \
    core/machine/machinecomponent.cpp \
    core/machine/machineactuatorcomponent.cpp \
    core/machine/machinebuilder.cpp \
    core/machine/fsm/fsmcomponent.cpp \
    core/machine/fsm/fsmvhdlexport.cpp \
    core/machine/fsm/fsmverifier.cpp \
    core/machine/fsm/fsm.cpp \
    core/machine/fsm/fsmstate.cpp \
    core/machine/fsm/fsmtransition.cpp \
    core/signal/inout.cpp \
    core/signal/input.cpp \
    core/signal/io.cpp \
    core/signal/signal.cpp \
    core/signal/equation.cpp \
    core/signal/output.cpp \
    core/simulation/clock.cpp \
    core/simulation/fsmsimulator.cpp \
    core/simulation/machinesimulator.cpp \
    ui/statesui.cpp \
    ui/machinecomponentvisualizer.cpp \
    ui/displayarea.cpp \
    ui/truthtabledisplay.cpp \
    ui/dialogs/imageexportdialog.cpp \
    ui/dialogs/langselectiondialog.cpp \
    ui/dialogs/vhdlexportdialog.cpp \
    ui/equation_editor/inverterbar.cpp \
    ui/equation_editor/equationeditor.cpp \
    ui/equation_editor/graphicequation.cpp \
    ui/equation_editor/equationmimedata.cpp \
    ui/refined_widget/dynamiclineedit.cpp \
    ui/refined_widget/contextmenu.cpp \
    ui/refined_widget/dynamictableitemdelegate.cpp \
    ui/refined_widget/reactivebutton.cpp \
    ui/refined_widget/tablewidgetwithresizeevent.cpp \
    ui/refined_widget/checkboxhtml.cpp \
    ui/refined_widget/labelwithclickevent.cpp \
    ui/refined_widget/collapsiblewidgetwithtitle.cpp \
    ui/resource_bar/abouttab.cpp \
    ui/resource_bar/resourcebar.cpp \
    ui/resource_bar/verifiertab.cpp \
    ui/resource_bar/component_editor/transitioneditortab.cpp \
    ui/resource_bar/component_editor/stateeditortab.cpp \
    ui/resource_bar/component_editor/actionlisteditor.cpp \
    ui/resource_bar/component_editor/actioneditor.cpp \
    ui/resource_bar/component_editor/componenteditortab.cpp \
    ui/resource_bar/component_editor/conditioneditor.cpp \
    ui/resource_bar/machine_builder/fsmtoolspanel.cpp \
    ui/resource_bar/machine_builder/machinetoolspanel.cpp \
    ui/resource_bar/machine_builder/machinebuildertab.cpp \
    ui/resource_bar/signal_editor/signaleditortab.cpp \
    ui/resource_bar/signal_editor/signallisteditor.cpp \
    ui/resource_bar/simulator_tab/simulatortab.cpp \
    ui/resource_bar/simulator_tab/inputbitselector.cpp \
    ui/resource_bar/simulator_tab/inputsselector.cpp \
    ui/resource_bar/simulator_tab/inputsignalselector.cpp \
    ui/scene/scenewidget.cpp \
    ui/scene/genericscene.cpp \
    ui/scene/blankscene.cpp \
    ui/scene/machineimageexporter.cpp \
    ui/scene/fsm/fsmgraphicalstate.cpp \
    ui/scene/fsm/fsmgraphicaltransition.cpp \
    ui/scene/fsm/fsmgraphicaltransitionneighborhood.cpp \
    ui/scene/fsm/fsmscene.cpp \
    ui/simulation/graphictimeline.cpp \
    ui/simulation/graphicclocktimeline.cpp \
    ui/simulation/clocktimeline.cpp \
    ui/simulation/signaltimeline.cpp \
    ui/simulation/simulationwidget.cpp \
    ui/static/svgimagegenerator.cpp



HEADERS  += \
    core/include/states.h \
    core/basic_type/include/logicvalue.h \
    core/basic_type/include/truthtable.h \
    core/machine/include/machine.h \
    core/machine/include/machinecomponent.h \
    core/machine/include/machineactuatorcomponent.h \
    core/machine/include/machinebuilder.h \
    core/machine/fsm/include/fsmcomponent.h \
    core/machine/fsm/include/fsmvhdlexport.h \
    core/machine/fsm/include/fsmverifier.h \
    core/machine/fsm/include/fsm.h \
    core/machine/fsm/include/fsmstate.h \
    core/machine/fsm/include/fsmtransition.h \
    core/signal/include/inout.h \
    core/signal/include/input.h \
    core/signal/include/io.h \
    core/signal/include/output.h \
    core/signal/include/signal.h \
    core/signal/include/equation.h \
    core/simulation/include/clock.h \
    core/simulation/include/fsmsimulator.h \
    core/simulation/include/machinesimulator.h \
#
    ui/include/statesui.h \
    ui/include/truthtabledisplay.h \
    ui/include/displayarea.h \
    ui/include/machinecomponentvisualizer.h \
    ui/dialogs/include/vhdlexportdialog.h \
    ui/dialogs/include/langselectiondialog.h \
    ui/dialogs/include/imageexportdialog.h \
    ui/equation_editor/include/equationeditor.h \
    ui/equation_editor/include/graphicequation.h \
    ui/equation_editor/include/equationmimedata.h \
    ui/equation_editor/include/inverterbar.h \
    ui/refined_widget/include/dynamiclineedit.h \
    ui/refined_widget/include/contextmenu.h \
    ui/refined_widget/include/dynamictableitemdelegate.h \
    ui/refined_widget/include/reactivebutton.h \
    ui/refined_widget/include/tablewidgetwithresizeevent.h \
    ui/refined_widget/include/checkboxhtml.h \
    ui/refined_widget/include/labelwithclickevent.h \
    ui/refined_widget/include/collapsiblewidgetwithtitle.h \
    ui/resource_bar/include/abouttab.h \
    ui/resource_bar/include/resourcebar.h \
    ui/resource_bar/include/verifiertab.h \
    ui/resource_bar/component_editor/include/stateeditortab.h \
    ui/resource_bar/component_editor/include/transitioneditortab.h \
    ui/resource_bar/component_editor/include/actionlisteditor.h \
    ui/resource_bar/component_editor/include/componenteditortab.h \
    ui/resource_bar/component_editor/include/actioneditor.h \
    ui/resource_bar/component_editor/include/conditioneditor.h \
    ui/resource_bar/machine_builder/include/fsmtoolspanel.h \
    ui/resource_bar/machine_builder/include/machinetoolspanel.h \
    ui/resource_bar/machine_builder/include/machinebuildertab.h \
    ui/resource_bar/signal_editor/include/signaleditortab.h \
    ui/resource_bar/signal_editor/include/signallisteditor.h \
    ui/resource_bar/simulator_tab/include/simulatortab.h \
    ui/resource_bar/simulator_tab/include/inputsignalselector.h \
    ui/resource_bar/simulator_tab/include/inputbitselector.h \
    ui/resource_bar/simulator_tab/include/inputsselector.h \
    ui/scene/include/scenewidget.h \
    ui/scene/include/genericscene.h \
    ui/scene/include/blankscene.h \
    ui/scene/include/machineimageexporter.h \
    ui/scene/fsm/include/fsmgraphicalstate.h \
    ui/scene/fsm/include/fsmgraphicaltransition.h \
    ui/scene/fsm/include/fsmgraphicaltransitionneighborhood.h \
    ui/scene/fsm/include/fsmscene.h \
    ui/simulation/include/graphictimeline.h \
    ui/simulation/include/graphicclocktimeline.h \
    ui/simulation/include/clocktimeline.h \
    ui/simulation/include/signaltimeline.h \
    ui/simulation/include/simulationwidget.h \
    ui/static/include/svgimagegenerator.h

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
    ui/dialogs/include \
    ui/equation_editor/include \
    ui/refined_widget/include \
    ui/resource_bar/include \
    ui/resource_bar/component_editor/include \
    ui/resource_bar/machine_builder/include \
    ui/resource_bar/signal_editor/include \
    ui/resource_bar/simulator_tab/include \
    ui/scene/include \
    ui/scene/fsm/include \
    ui/simulation/include \
    ui/static/include

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

