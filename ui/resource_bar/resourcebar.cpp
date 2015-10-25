/*
 * Copyright © 2014-2015 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.txt.
 *
 *
 * This file is part of StateS.
 *
 * StateS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * StateS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StateS. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "resourcebar.h"

// Debug
#include <QDebug>

// StateS classes
#include "machinebuilder.h"
#include "machinebuildertab.h"
#include "signaleditortab.h"
#include "stateeditortab.h"
#include "transitioneditortab.h"
#include "simulatortab.h"
#include "abouttab.h"
#include "verifiertab.h"
#include "fsm.h"
#include "machinecomponentvisualizer.h"


ResourceBar::ResourceBar(shared_ptr<Machine> machine, QWidget* parent) :
    QTabWidget(parent)
{
    connect(this, &QTabWidget::currentChanged, this, &ResourceBar::tabChanged);

    setMachine(machine);
}

ResourceBar::~ResourceBar()
{
    // Do I have to do this?
    // Widgets are not deleted on tab removal...
    // Is this the same on widget deletion?
    // Does QTabWidget take parenthood of tab widgets?
    delete machineBuildTab;
    delete signalsTab;
    delete editorTab;
    delete simulatorTab;
    delete verifierTab;
}

void ResourceBar::setMachine(shared_ptr<Machine> newMachine)
{
    shared_ptr<Machine> oldMachine = this->machine.lock();
    if (oldMachine != nullptr)
    {
        oldMachine.reset();
    }

    this->machineComponentScene.reset();

    this->machine = newMachine;

    // Clear (does not delete widgets, but needed for tab change issues)
    this->clear();

    delete machineBuildTab;
    machineBuildTab = nullptr;

    delete signalsTab;
    signalsTab = nullptr;

    delete editorTab;
    editorTab = nullptr;

    delete verifierTab;
    verifierTab = nullptr;


    if (simulatorTab != nullptr)
    {
        disconnect(this->simulatorTab, &SimulatorTab::beginSimulationEvent, this, &ResourceBar::beginSimulation);
        disconnect(this->simulatorTab, &SimulatorTab::endSimulationEvent,   this, &ResourceBar::terminateSimulation);

        delete simulatorTab;
        simulatorTab = nullptr;
    }

    // Build
    if (newMachine != nullptr)
    {
        this->machineComponentScene = shared_ptr<MachineComponentVisualizer>(new MachineComponentVisualizer(newMachine));

        this->machineBuildTab = new MachineBuilderTab(newMachine, this->machineComponentScene);
        this->signalsTab      = new SignalEditorTab(newMachine, this->machineComponentScene);
        this->simulatorTab    = new SimulatorTab(dynamic_pointer_cast<Fsm>(newMachine));
        this->verifierTab     = new VerifierTab(dynamic_pointer_cast<Fsm>(newMachine));

        connect(this->simulatorTab, &SimulatorTab::beginSimulationEvent, this, &ResourceBar::beginSimulation);
        connect(this->simulatorTab, &SimulatorTab::endSimulationEvent,   this, &ResourceBar::terminateSimulation);

        this->insertTab(0, machineBuildTab, tr("Builder"));
        this->insertTab(1, signalsTab,      tr("Signals"));
        this->insertTab(2, new QWidget(),   tr("Editor"));
        this->insertTab(3, simulatorTab,    tr("Simulator"));
        this->insertTab(4, verifierTab,     tr("Verifier"));
//        this->insertTab(4, new QWidget(),   tr("Options"));
        this->insertTab(5, new AboutTab(),  tr("About"));

        this->setTabEnabled(2, false);

        this->setCurrentIndex(0);

        newMachine->setMode(Machine::mode::editMode);
    }
    else
    {
        this->insertTab(0, new QWidget(),  tr("Builder"));
        this->insertTab(1, new QWidget(),  tr("Signals"));
        this->insertTab(2, new QWidget(),  tr("Editor"));
        this->insertTab(3, new QWidget(),  tr("Simulator"));
        this->insertTab(4, new QWidget(),  tr("Verifier"));
//        this->insertTab(4, new QWidget(),  tr("Options"));
        this->insertTab(5, new AboutTab(), tr("About"));

        this->setTabEnabled(0, false);
        this->setTabEnabled(1, false);
        this->setTabEnabled(2, false);
        this->setTabEnabled(3, false);
        this->setTabEnabled(4, false);

        this->setCurrentIndex(5);
    }
}

void ResourceBar::selectedState(shared_ptr<FsmState> state, bool showTab, bool editName)
{
    if (state == nullptr)
    {
        qDebug() << "(Resources bar:) error, null pointer state given for edition!";
        clearSelection();
    }
    else
    {
        shared_ptr<Machine> machine = this->machine.lock();

        if (machine != nullptr)
        {
            if (machine->getCurrentMode() == Machine::mode::editMode)
            {
                StateEditorTab* editor = dynamic_cast<StateEditorTab*>(editorTab);

                uint current_tab = this->currentIndex();

                if (editor != nullptr)
                    editor->changeEditedState(state);
                else
                {
                    this->removeTab(2);
                    delete editorTab;
                    this->editorTab = new StateEditorTab(state);
                    this->insertTab(2, editorTab, tr("State"));
                }

                if (showTab)
                {
                    this->setCurrentIndex(2);
                    if (editName)
                        ((StateEditorTab*)this->editorTab)->setEditName();
                }
                else
                    this->setCurrentIndex(current_tab);
            }
        }
    }
}

void ResourceBar::selectedTransition(shared_ptr<FsmTransition> transition, bool showTab)
{
    if (transition == nullptr)
    {
        qDebug() << "(Resources bar:) error, null pointer transition given for edition!";
        clearSelection();
    }
    else
    {
        shared_ptr<Machine> machine = this->machine.lock();

        if (machine != nullptr)
        {
            if (machine->getCurrentMode() == Machine::mode::editMode)
            {
                TransitionEditorTab* editor = dynamic_cast<TransitionEditorTab*>(editorTab);

                uint current_tab = this->currentIndex();

                if (editor != nullptr)
                    editor->changeEditedTransition(transition);
                else
                {
                    this->removeTab(2);
                    delete editorTab;

                    this->editorTab = new TransitionEditorTab(transition);
                    this->insertTab(2, editorTab, tr("Transition"));
                }

                if (showTab)
                {
                    this->setCurrentIndex(2);
                }
                else
                    this->setCurrentIndex(current_tab);
            }
        }
    }
}

shared_ptr<QGraphicsScene> ResourceBar::getComponentVisualizationScene()
{
    return this->machineComponentScene->getComponentVisualizationScene();
}

void ResourceBar::clearSelection()
{
    uint currentTab = this->currentIndex();

    this->removeTab(2);

    delete this->editorTab;
    this->editorTab = nullptr;

    this->insertTab(2, new QWidget(), tr("Editor"));
    this->setTabEnabled(2, false);

    if (currentTab != 2)
        this->setCurrentIndex(currentTab);
    else
        this->setCurrentIndex(0);
}

void ResourceBar::beginSimulation()
{
    clearSelection();

    this->setTabEnabled(0, false);
    this->setTabEnabled(1, false);

    connect(this->simulatorTab, &SimulatorTab::triggerViewRequestEvent, this, &ResourceBar::triggerViewRequestEvent);

    shared_ptr<Machine> machine = this->machine.lock();
    if (machine != nullptr)
        machine->setMode(Machine::mode::simulateMode);

    emit simulationToggledEvent();
}

void ResourceBar::terminateSimulation()
{
    this->setTabEnabled(0, true);
    this->setTabEnabled(1, true);

    disconnect(this->simulatorTab, &SimulatorTab::triggerViewRequestEvent, this, &ResourceBar::triggerViewRequestEvent);

    shared_ptr<Machine> machine = this->machine.lock();
    if (machine != nullptr)
        machine->setMode(Machine::mode::editMode);

    emit simulationToggledEvent();
}

SimulationWidget* ResourceBar::getTimeline() const
{
    if (this->simulatorTab != nullptr)
        return this->simulatorTab->getTimeline();
    else
        return nullptr;
}

void ResourceBar::tabChanged(int)
{
    // Clear selected tool on tab change
    shared_ptr<Machine> machine = this->machine.lock();

    if (machine != nullptr)
        machine->getMachineBuilder()->setTool(MachineBuilder::tool::none);
}
