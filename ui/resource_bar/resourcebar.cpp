/*
 * Copyright © 2014-2020 Clément Foucher
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

// StateS classes
#include "hinttab.h"
#include "machineeditortab.h"
#include "stateeditortab.h"
#include "transitioneditortab.h"
#include "simulatortab.h"
#include "abouttab.h"
#include "verifiertab.h"
#include "machinecomponentvisualizer.h"
#include "fsmstate.h"
#include "fsmtransition.h"


ResourceBar::ResourceBar(QWidget* parent) :
    QTabWidget(parent)
{
	connect(this, &QTabWidget::currentChanged, this, &ResourceBar::tabChanged);

	this->setMachine(nullptr);
}

void ResourceBar::setMachine(shared_ptr<Machine> newMachine, bool maintainView)
{
	int index = (maintainView == true) ? this->currentIndex() : 0;
	bool builderHintCollapsed = false;
	bool builderVisuCollapsed = false;
	bool signalsHintCollapsed = false;
	bool signalsVisuCollapsed = false;

	if (!this->machine.expired())
	{
		disconnect(this->machine.lock().get(), &Machine::simulationModeChangedEvent, this, &ResourceBar::machineModeChangedEventHandler);
		builderHintCollapsed = this->hintsTab->getHintCollapsed();
		builderVisuCollapsed = this->hintsTab->getVisuCollapsed();
		signalsHintCollapsed = this->machineTab->getHintCollapsed();
		signalsVisuCollapsed = this->machineTab->getVisuCollapsed();
	}

	this->machineComponentScene = nullptr;
	this->machine = newMachine;

	while(this->count() != 0)
	{
		delete this->widget(0);
	}

	this->hintsTab     = nullptr;
	this->machineTab   = nullptr;
	this->editorTab    = nullptr;
	this->verifierTab  = nullptr;
	this->simulatorTab = nullptr;

	// Build
	if (newMachine != nullptr)
	{
		this->machineComponentScene = shared_ptr<MachineComponentVisualizer>(new MachineComponentVisualizer(newMachine));

		this->hintsTab     = new HintTab         (newMachine, this->machineComponentScene);
		this->machineTab   = new MachineEditorTab(newMachine, this->machineComponentScene);
		this->simulatorTab = new SimulatorTab    (newMachine);
		this->verifierTab  = new VerifierTab     (newMachine);

		connect(newMachine.get(), &Machine::simulationModeChangedEvent, this, &ResourceBar::machineModeChangedEventHandler);

		this->insertTab(0, this->hintsTab,     tr("Hints"));
		this->insertTab(1, this->machineTab,   tr("Signals"));
		this->insertTab(2, new QWidget(),      tr("Editor"));
		this->insertTab(3, this->simulatorTab, tr("Simulator"));
		this->insertTab(4, this->verifierTab,  tr("Verifier"));
//		this->insertTab(4, new QWidget(),      tr("Options"));
		this->insertTab(5, new AboutTab(),     tr("About"));

		this->setTabEnabled(2, false);

		this->setCurrentIndex(index);
		if (maintainView == true)
		{
			this->hintsTab->setHintCollapsed(builderHintCollapsed);
			this->hintsTab->setVisuCollapsed(builderVisuCollapsed);
			this->machineTab->setHintCollapsed(signalsHintCollapsed);
			this->machineTab->setVisuCollapsed(signalsVisuCollapsed);
		}

		if (newMachine->getCurrentSimulationMode() == Machine::simulation_mode::simulateMode)
		{
			this->beginSimulation();
		}
	}
	else
	{
		this->insertTab(0, new QWidget(),  tr("Hints"));
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

shared_ptr<QGraphicsScene> ResourceBar::getComponentVisualizationScene() const
{
	return this->machineComponentScene->getComponentVisualizationScene();
}

void ResourceBar::setSelectedItem(shared_ptr<MachineComponent> item)
{
	shared_ptr<Machine> l_machine = this->machine.lock();

	if ( (l_machine != nullptr) && (l_machine->getCurrentSimulationMode() == Machine::simulation_mode::editMode) )
	{
		shared_ptr<FsmState>      state      = dynamic_pointer_cast<FsmState>     (item);
		shared_ptr<FsmTransition> transition = dynamic_pointer_cast<FsmTransition>(item);

		if (state != nullptr)
		{
			uint current_tab = this->currentIndex();

			delete this->widget(2);

			this->editorTab = new StateEditorTab(state);
			this->insertTab(2, this->editorTab, tr("State"));

			this->setCurrentIndex(current_tab);
		}
		else if (transition != nullptr)
		{
			uint current_tab = this->currentIndex();

			delete this->widget(2);

			this->editorTab = new TransitionEditorTab(transition);
			this->insertTab(2, this->editorTab, tr("Transition"));

			this->setCurrentIndex(current_tab);
		}
		else
		{
			this->clearSelection();
		}
	}
	else
	{
		this->clearSelection();
	}
}

void ResourceBar::editSelectedItem()
{
	if (this->editorTab != nullptr)
		this->setCurrentIndex(2);
}

void ResourceBar::renameSelectedItem()
{
	StateEditorTab* stateEditorTab = dynamic_cast<StateEditorTab*>(this->editorTab);

	if (stateEditorTab != nullptr)
	{
		this->setCurrentIndex(2);
		stateEditorTab->setEditName();
	}
}

void ResourceBar::clearSelection()
{
	uint currentTab = this->currentIndex();

	delete this->widget(2);
	this->editorTab = nullptr;

	this->insertTab(2, new QWidget(), tr("Editor"));
	this->setTabEnabled(2, false);

	if (currentTab != 2)
	{
		this->setCurrentIndex(currentTab);
	}
	else
	{
		this->setCurrentIndex(0);
	}
}

void ResourceBar::machineModeChangedEventHandler(Machine::simulation_mode newMode)
{
	if (newMode == Machine::simulation_mode::simulateMode)
	{
		this->beginSimulation();
	}
	else
	{
		this->terminateSimulation();
	}
}

void ResourceBar::beginSimulation()
{
	shared_ptr<Machine> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		this->clearSelection();

		this->setTabEnabled(0, false);
		this->setTabEnabled(1, false);
		this->setTabEnabled(4, false);
	}
}

void ResourceBar::terminateSimulation()
{
	shared_ptr<Machine> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		this->setTabEnabled(0, true);
		this->setTabEnabled(1, true);
		this->setTabEnabled(4, true);
	}
}

void ResourceBar::tabChanged(int)
{
	// Clear selected tool on tab change
	shared_ptr<Machine> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		shared_ptr<MachineBuilder> builder = l_machine->getMachineBuilder();
		if (builder->getTool() != MachineBuilder::tool::none)
		{
			builder->setTool(MachineBuilder::tool::none);
		}
	}
}
