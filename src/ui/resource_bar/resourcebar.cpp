/*
 * Copyright © 2014-2021 Clément Foucher
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
#include "machinemanager.h"
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


ResourceBar::ResourceBar(shared_ptr<MachineManager> machineManager, QWidget* parent) :
    QTabWidget(parent)
{
	this->machineManager = machineManager;
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &ResourceBar::machineUpdatedEventHandler);

	this->build();
}

void ResourceBar::machineUpdatedEventHandler(bool isNewMachine)
{
	// Save current state
	int index = (isNewMachine == false) ? this->currentIndex() : 0;
	bool builderHintCollapsed = false;
	bool builderVisuCollapsed = false;
	bool signalsHintCollapsed = false;
	bool signalsVisuCollapsed = false;

	if (isNewMachine == false)
	{
		builderHintCollapsed = this->hintsTab->getHintCollapsed();
		builderVisuCollapsed = this->hintsTab->getVisuCollapsed();
		signalsHintCollapsed = this->machineTab->getHintCollapsed();
		signalsVisuCollapsed = this->machineTab->getVisuCollapsed();
	}

	// Renew content
	this->machineComponentScene = nullptr;

	while(this->count() != 0)
	{
		delete this->widget(0);
	}

	this->hintsTab     = nullptr;
	this->machineTab   = nullptr;
	this->editorTab    = nullptr;
	this->verifierTab  = nullptr;
	this->simulatorTab = nullptr;

	this->build();

	// Restore previous state
	if (isNewMachine == false)
	{
		this->setCurrentIndex(index);

		this->hintsTab->setHintCollapsed(builderHintCollapsed);
		this->hintsTab->setVisuCollapsed(builderVisuCollapsed);
		this->machineTab->setHintCollapsed(signalsHintCollapsed);
		this->machineTab->setVisuCollapsed(signalsVisuCollapsed);
	}
	else
	{
		if (this->hintsTab != nullptr)
		{
			this->hintsTab->setHintCollapsed(false);
			this->hintsTab->setVisuCollapsed(false);
		}
		if (this->machineTab != nullptr)
		{
			this->machineTab->setHintCollapsed(false);
			this->machineTab->setVisuCollapsed(false);
		}

		if (this->machineManager->getMachine() != nullptr)
		{
			this->setCurrentIndex(0);
		}
		else
		{
			this->setCurrentIndex(5);
		}
	}
}

shared_ptr<QGraphicsScene> ResourceBar::getComponentVisualizationScene() const
{
	return this->machineComponentScene->getComponentVisualizationScene();
}

void ResourceBar::setSelectedItem(shared_ptr<MachineComponent> item)
{
	shared_ptr<Machine> l_machine = this->machineManager->getMachine();
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
		this->clearSelection();

		this->setTabEnabled(0, false);
		this->setTabEnabled(1, false);
		this->setTabEnabled(4, false);
	}
	else
	{
		this->setTabEnabled(0, true);
		this->setTabEnabled(1, true);
		this->setTabEnabled(4, true);
	}
}

void ResourceBar::build()
{
	shared_ptr<Machine> newMachine = this->machineManager->getMachine();
	if (newMachine != nullptr)
	{
		this->machineComponentScene = shared_ptr<MachineComponentVisualizer>(new MachineComponentVisualizer(newMachine));

		this->hintsTab     = new HintTab         (this->machineManager, this->machineComponentScene);
		this->machineTab   = new MachineEditorTab(this->machineManager, this->machineComponentScene);
		this->simulatorTab = new SimulatorTab    (this->machineManager);
		this->verifierTab  = new VerifierTab     (this->machineManager);

		this->machineManager->addConnection(connect(newMachine.get(), &Machine::simulationModeChangedEvent, this, &ResourceBar::machineModeChangedEventHandler));

		this->insertTab(0, this->hintsTab,     tr("Hints"));
		this->insertTab(1, this->machineTab,   tr("Machine"));
		this->insertTab(2, new QWidget(),      tr("Editor"));
		this->insertTab(3, this->simulatorTab, tr("Simulator"));
		this->insertTab(4, this->verifierTab,  tr("Verifier"));
//		this->insertTab(4, new QWidget(),      tr("Options"));
		this->insertTab(5, new AboutTab(),     tr("About"));

		this->setTabEnabled(2, false);
	}
	else
	{
		this->insertTab(0, new QWidget(),  tr("Hints"));
		this->insertTab(1, new QWidget(),  tr("Machine"));
		this->insertTab(2, new QWidget(),  tr("Editor"));
		this->insertTab(3, new QWidget(),  tr("Simulator"));
		this->insertTab(4, new QWidget(),  tr("Verifier"));
//		this->insertTab(4, new QWidget(),  tr("Options"));
		this->insertTab(5, new AboutTab(), tr("About"));

		this->setTabEnabled(0, false);
		this->setTabEnabled(1, false);
		this->setTabEnabled(2, false);
		this->setTabEnabled(3, false);
		this->setTabEnabled(4, false);

		this->setCurrentIndex(5);
	}
}
