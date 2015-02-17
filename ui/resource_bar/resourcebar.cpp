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
#include "machinebuildertab.h"
#include "signaleditortab.h"
#include "stateeditortab.h"
#include "machinetools.h"
#include "transitioneditortab.h"
#include "simulatortab.h"
#include "abouttab.h"


ResourceBar::ResourceBar(QWidget* parent, Machine* machine) :
    QTabWidget(parent)
{
    setMachine(machine);

    connect(this, &QTabWidget::currentChanged, this, &ResourceBar::tabChanged);
}

ResourceBar::~ResourceBar()
{
    // Do I have to do this?
    // Widgets are not deleted on tab removal...
    // Is this the same on widget deletion?
    // Does QTabWidget take parenthood of tab widgets?
    delete toolResources;
    delete interfaceResources;
    delete editorTab;
    delete simulatorTab;
}

void ResourceBar::setMachine(Machine* machine)
{
    if (this->machine != nullptr)
        disconnect(this, &ResourceBar::simulationToggled, machine, &Machine::simulationModeChanged);

    this->machine = machine;

    if (this->machine != nullptr)
        connect(this, &ResourceBar::simulationToggled, machine, &Machine::simulationModeChanged);

    // Clear
    this->clear();

    delete toolResources;
    toolResources = nullptr;

    delete interfaceResources;
    interfaceResources = nullptr;

    delete editorTab;
    editorTab = nullptr;

    if (simulatorTab != nullptr)
    {
        disconnect(this->simulatorTab, &SimulatorTab::beginSimulation, this, &ResourceBar::beginSimulation);
        disconnect(this->simulatorTab, &SimulatorTab::endSimulation,   this, &ResourceBar::terminateSimulation);

        delete simulatorTab;
        simulatorTab = nullptr;
    }

    // Build
    if (machine != nullptr)
    {
        this->toolResources      = new MachineBuilderTab(machine->getType());
        this->interfaceResources = new SignalEditorTab(machine);
        this->simulatorTab       = new SimulatorTab((Fsm*)machine);

        connect(this->simulatorTab, &SimulatorTab::beginSimulation, this, &ResourceBar::beginSimulation);
        connect(this->simulatorTab, &SimulatorTab::endSimulation,   this, &ResourceBar::terminateSimulation);

        this->insertTab(0, toolResources,      tr("Builder"));
        this->insertTab(1, interfaceResources, tr("Signals"));
        this->insertTab(2, new QWidget(),      tr("Editor"));
        this->insertTab(3, simulatorTab,       tr("Simulator"));
        this->insertTab(4, new QWidget(),      tr("Options"));
        this->insertTab(5, new AboutTab(),     tr("About"));

        this->setTabEnabled(2, false);

        this->setCurrentIndex(0);

        this->currentMode = mode::editMode;
    }
    else
    {
        this->insertTab(0, new QWidget(),  tr("Builder"));
        this->insertTab(1, new QWidget(),  tr("Signals"));
        this->insertTab(2, new QWidget(),  tr("Editor"));
        this->insertTab(3, new QWidget(),  tr("Simulator"));
        this->insertTab(4, new QWidget(),  tr("Options"));
        this->insertTab(5, new AboutTab(), tr("About"));

        this->setTabEnabled(0, false);
        this->setTabEnabled(1, false);
        this->setTabEnabled(2, false);
        this->setTabEnabled(3, false);

        this->setCurrentIndex(5);

        this->currentMode = mode::voidMode;
    }
}

void ResourceBar::selectedState(FsmState* state, bool showTab, bool editName)
{
    if (state == nullptr)
    {
        qDebug() << "(Resources bar:) error, null pointer state given for edition!";
        clearSelection();
    }
    else
    {
        if (currentMode == mode::editMode)
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

void ResourceBar::selectedTransition(FsmTransition* transition, bool showTab)
{
    if (transition == nullptr)
    {
        qDebug() << "(Resources bar:) error, null pointer transition given for edition!";
        clearSelection();
    }
    else
    {
        if (currentMode == mode::editMode)
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

    currentMode = mode::simulateMode;

    emit simulationToggled();
}

void ResourceBar::terminateSimulation()
{
    this->setTabEnabled(0, true);
    this->setTabEnabled(1, true);

    currentMode = mode::editMode;

    emit simulationToggled();
}

MachineTools* ResourceBar::getBuildTools() const
{
    if (toolResources != nullptr)
        return toolResources->getBuildTools();
    else
        return nullptr;
}

void ResourceBar::tabChanged(int index)
{
    // Clear selected tool on tab change
    if ((toolResources != nullptr) && (index != 0))
        toolResources->getBuildTools()->setTool(MachineTools::tool::none);
}

ResourceBar::mode ResourceBar::getCurrentMode() const
{
    return currentMode;
}
