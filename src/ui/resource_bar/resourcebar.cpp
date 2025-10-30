/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "machine.h"
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
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent,        this, &ResourceBar::clearSelection);
	connect(machineManager.get(), &MachineManager::machineReplacedEvent,       this, &ResourceBar::machineReplacedEventHandler);
	connect(machineManager.get(), &MachineManager::simulationModeChangedEvent, this, &ResourceBar::machineModeChangedEventHandler);

	this->build();
}

void ResourceBar::setSelectedItem(componentId_t componentId)
{
	if (componentId == nullId)
	{
		this->clearSelection();
		return;
	}

	if (machineManager->getCurrentSimulationMode() != SimulationMode_t::editMode)
	{
		this->clearSelection();
		return;
	}

	auto machine = machineManager->getMachine();
	if (machine == nullptr)
	{
		this->clearSelection();
		return;
	}

	auto item = machine->getComponent(componentId);
	if (dynamic_pointer_cast<FsmState>(item) != nullptr)
	{
		int current_tab = this->currentIndex();

		delete this->widget(TabIndex_t::componentEditorTabIndex);

		auto componentEditorTab = new StateEditorTab(componentId);
		this->insertTab(TabIndex_t::componentEditorTabIndex, componentEditorTab, tr("State"));

		this->setCurrentIndex(current_tab);
	}
	else if (dynamic_pointer_cast<FsmTransition>(item) != nullptr)
	{
		int current_tab = this->currentIndex();

		delete this->widget(TabIndex_t::componentEditorTabIndex);

		auto componentEditorTab = new TransitionEditorTab(componentId);
		this->insertTab(TabIndex_t::componentEditorTabIndex, componentEditorTab, tr("Transition"));

		this->setCurrentIndex(current_tab);
	}
	else
	{
		this->clearSelection();
	}
}

void ResourceBar::editSelectedItem()
{
	auto componentEditorTab = dynamic_cast<ComponentEditorTab*>(this->widget(TabIndex_t::componentEditorTabIndex));
	if (componentEditorTab == nullptr) return;

	this->setCurrentIndex(TabIndex_t::componentEditorTabIndex);
}

void ResourceBar::renameSelectedItem()
{
	StateEditorTab* stateEditorTab = dynamic_cast<StateEditorTab*>(this->widget(TabIndex_t::componentEditorTabIndex));
	if (stateEditorTab == nullptr) return;

	this->setCurrentIndex(TabIndex_t::componentEditorTabIndex);
	stateEditorTab->setEditName();
}

shared_ptr<QGraphicsScene> ResourceBar::getComponentVisualizationScene() const
{
	return this->machineComponentScene->getComponentVisualizationScene();
}

void ResourceBar::machineReplacedEventHandler()
{
	// Delete previous content
	this->machineComponentScene = nullptr;

	while(this->count() != 0)
	{
		delete this->widget(0);
	}

	// Then rebuild
	this->build();
}

void ResourceBar::clearSelection()
{
	int currentTab = this->currentIndex();

	delete this->widget(TabIndex_t::componentEditorTabIndex);

	this->insertTab(TabIndex_t::componentEditorTabIndex, new QWidget(), tr("Editor"));
	this->setTabEnabled(TabIndex_t::componentEditorTabIndex, false);

	if (currentTab != TabIndex_t::componentEditorTabIndex)
	{
		this->setCurrentIndex(currentTab);
	}
	else
	{
		this->setCurrentIndex(TabIndex_t::hintTabIndex);
	}
}

void ResourceBar::machineModeChangedEventHandler(SimulationMode_t newMode)
{
	if (newMode == SimulationMode_t::simulateMode)
	{
		this->clearSelection();

		this->setTabEnabled(TabIndex_t::aboutTabIndex,         false);
		this->setTabEnabled(TabIndex_t::machineEditorTabIndex, false);
		this->setTabEnabled(TabIndex_t::verifierTabIndex,      false);
	}
	else
	{
		this->setTabEnabled(TabIndex_t::aboutTabIndex,         true);
		this->setTabEnabled(TabIndex_t::machineEditorTabIndex, true);
		this->setTabEnabled(TabIndex_t::verifierTabIndex,      true);
	}
}

void ResourceBar::build()
{
	auto machine = machineManager->getMachine();
	if (machine != nullptr)
	{
		this->machineComponentScene = make_shared<MachineComponentVisualizer>();

		auto  hintTab          = new HintTab         (this->machineComponentScene);
		auto  machineEditorTab = new MachineEditorTab(this->machineComponentScene);
		auto  simulatorTab     = new SimulatorTab    ();
		auto  verifierTab      = new VerifierTab     ();

		this->insertTab(TabIndex_t::hintTabIndex,            hintTab,          tr("Hints"));
		this->insertTab(TabIndex_t::machineEditorTabIndex,   machineEditorTab, tr("Machine"));
		this->insertTab(TabIndex_t::componentEditorTabIndex, new QWidget(),          tr("Editor"));
		this->insertTab(TabIndex_t::simulatorTabIndex,       simulatorTab,     tr("Simulator"));
		this->insertTab(TabIndex_t::verifierTabIndex,        verifierTab,      tr("Verifier"));
		this->insertTab(TabIndex_t::aboutTabIndex,           new AboutTab(),         tr("About"));

		this->setTabEnabled(TabIndex_t::componentEditorTabIndex, false);

		this->setCurrentIndex(TabIndex_t::hintTabIndex);
	}
	else
	{
		this->insertTab(TabIndex_t::hintTabIndex,            new QWidget(),  tr("Hints"));
		this->insertTab(TabIndex_t::machineEditorTabIndex,   new QWidget(),  tr("Machine"));
		this->insertTab(TabIndex_t::componentEditorTabIndex, new QWidget(),  tr("Editor"));
		this->insertTab(TabIndex_t::simulatorTabIndex,       new QWidget(),  tr("Simulator"));
		this->insertTab(TabIndex_t::verifierTabIndex,        new QWidget(),  tr("Verifier"));
		this->insertTab(TabIndex_t::aboutTabIndex,           new AboutTab(), tr("About"));

		this->setTabEnabled(TabIndex_t::hintTabIndex,            false);
		this->setTabEnabled(TabIndex_t::machineEditorTabIndex,   false);
		this->setTabEnabled(TabIndex_t::componentEditorTabIndex, false);
		this->setTabEnabled(TabIndex_t::simulatorTabIndex,       false);
		this->setTabEnabled(TabIndex_t::verifierTabIndex,        false);

		this->setCurrentIndex(TabIndex_t::aboutTabIndex);
	}
}
