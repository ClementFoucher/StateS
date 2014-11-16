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
    fsm/fsm.cpp \
    ui/drawingwindow.cpp \
    core/main.cpp \
    core/states.cpp \
    fsm/fsmstate.cpp \
    fsm/fsmtransition.cpp \
    ui/scene/fsm/fsmgraphicalstate.cpp \
    ui/scene/fsm/fsmgraphicaltransition.cpp \
    ui/scene/scenewidget.cpp \
    ui/scene/genericscene.cpp \
    ui/resourcesbar/resourcesbar.cpp \
    core/machine.cpp \
    ui/centralwidget.cpp \
    ui/resourcesbar/dynamiclineedit.cpp \
    ui/scene/fsm/fsmgraphicaltransitionneighborhood.cpp \
    ui/simulation/simulationtimeline.cpp \
    ui/simulation/variabletimeline.cpp \
    core/variables/inout.cpp \
    core/variables/input.cpp \
    core/variables/io.cpp \
    core/variables/logicequation.cpp \
    core/variables/logicvariable.cpp \
    core/simulation/clock.cpp \
    ui/simulation/graphictimeline.cpp \
    ui/simulation/graphicclocktimeline.cpp \
    ui/simulation/clocktimeline.cpp \
    ui/resourcesbar/editor/editortab.cpp \
    ui/resourcesbar/tools/fsmtools.cpp \
    ui/resourcesbar/editor/transitioneditortab.cpp \
    ui/resourcesbar/editor/stateeditortab.cpp \
    ui/resourcesbar/interface/interfaceresourcestab.cpp \
    ui/resourcesbar/interface/iolistdelegate.cpp \
    ui/equationEditor/equationeditor.cpp \
    ui/equationEditor/graphicequation.cpp \
    ui/resourcesbar/tools/machinetools.cpp \
    ui/resourcesbar/tools/toolresourcestab.cpp \
    ui/equationEditor/equationmimedata.cpp \
    ui/resourcesbar/interface/variableeditortool.cpp \
    ui/resourcesbar/abouttab.cpp \
    core/variables/output.cpp \
    ui/scene/fsm/fsmscene.cpp \
    ui/langselector.cpp \
    ui/reactivebutton.cpp \
#    core/variables/logicvariablevector.cpp \
    ui/resourcesbar/simulatortab.cpp \
    ui/contextmenu.cpp \
    ui/equationEditor/inverterbar.cpp


HEADERS  += \
    fsm/include/fsm.h \
    ui/include/drawingwindow.h \
    core/include/machine.h \
    core/include/states.h \
    fsm/include/fsmstate.h \
    fsm/include/fsmtransition.h \
    ui/scene/fsm/include/fsmgraphicalstate.h \
    ui/scene/fsm/include/fsmgraphicaltransition.h \
    ui/scene/include/scenewidget.h \
    ui/scene/include/genericscene.h \
    ui/resourcesbar/include/resourcesbar.h \
    ui/include/centralwidget.h \
    ui/scene/fsm/include/fsmgraphicaltransitionneighborhood.h \
    ui/simulation/include/simulationtimeline.h \
    ui/simulation/include/variabletimeline.h \
    core/variables/include/inout.h \
    core/variables/include/input.h \
    core/variables/include/io.h \
    core/variables/include/logicequation.h \
    core/variables/include/logicvariable.h \
    core/simulation/include/clock.h \
    ui/simulation/include/graphictimeline.h \
    ui/simulation/include/graphicclocktimeline.h \
    ui/resourcesbar/include/dynamiclineedit.h \
    ui/resourcesbar/editor/include/editortab.h \
    ui/resourcesbar/tools/include/fsmtools.h \
    ui/resourcesbar/editor/include/stateeditortab.h \
    ui/resourcesbar/editor/include/transitioneditortab.h \
    ui/resourcesbar/tools/include/toolresourcestab.h \
    ui/resourcesbar/interface/include/interfaceresourcestab.h \
    ui/resourcesbar/interface/include/iolistdelegate.h \
    ui/resourcesbar/tools/include/machinetools.h \
    ui/equationEditor/include/equationeditor.h \
    ui/equationEditor/include/graphicequation.h \
    ui/resourcesbar/interface/include/variableeditortools.h \
    ui/equationEditor/include/equationmimedata.h \
    core/variables/include/output.h \
    ui/scene/fsm/include/fsmscene.h \
    ui/resourcesbar/include/abouttab.h \
    ui/simulation/include/clocktimeline.h \
    ui/include/langselector.h \
    ui/include/reactivebutton.h \
#    core/variables/include/logicvariablevector.h \
    ui/resourcesbar/include/simulatortab.h \
    ui/include/contextmenu.h \
    ui/equationEditor/include/inverterbar.h

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += \
    core/include \
    core/simulation/include \
    core/variables/include \
    fsm/include \
    ui/include \
    ui/scene/include \
    ui/resourcesbar/include \
    ui/resourcesbar/editor/include \
    ui/resourcesbar/interface/include \
    ui/resourcesbar/tools/include \
    ui/scene/fsm/include \
    ui/simulation/include \
    ui/equationEditor/include

OTHER_FILES += \
    known_bugs.txt \
    README \
    README.txt \
    LICENSE \
    LICENSE.txt \
    release_notes.txt

RESOURCES += \
    art.qrc \
    translations.qrc

TRANSLATIONS += \
    translations/français.ts
