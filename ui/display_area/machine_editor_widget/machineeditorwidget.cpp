/*
 * Copyright © 2020-2021 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "machineeditorwidget.h"

// StateS classes
#include "machinemanager.h"
#include "scenewidget.h"
#include "drawingtoolbar.h"
#include "fsmdrawingtoolbar.h"
#include "machine.h"
#include "fsm.h"
#include "viewconfiguration.h"


MachineEditorWidget::MachineEditorWidget(shared_ptr<MachineManager> machineManager, QWidget* parent) :
    QMainWindow(parent)
{
	this->machineManager = machineManager;
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &MachineEditorWidget::machineUpdatedEventHandler);

	this->setWindowFlags(Qt::Widget);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	this->machineDisplayArea = new SceneWidget(machineManager, this);
	this->setCentralWidget(this->machineDisplayArea);

	// Transmit events to upper level
	connect(this->machineDisplayArea, &SceneWidget::itemSelectedEvent,       this, &MachineEditorWidget::itemSelectedEvent);
	connect(this->machineDisplayArea, &SceneWidget::editSelectedItemEvent,   this, &MachineEditorWidget::editSelectedItemEvent);
	connect(this->machineDisplayArea, &SceneWidget::renameSelectedItemEvent, this, &MachineEditorWidget::renameSelectedItemEvent);
}

GenericScene* MachineEditorWidget::getScene() const
{
	return this->machineDisplayArea->getScene();
}

void MachineEditorWidget::clearSelection()
{
	this->machineDisplayArea->clearSelection();
}

void MachineEditorWidget::setViewConfiguration(shared_ptr<ViewConfiguration> configuration)
{
	if (configuration != nullptr)
	{
		this->machineDisplayArea->setZoomLevel(configuration->zoomLevel);
		this->machineDisplayArea->centerOn(configuration->viewCenter);
	}
}

shared_ptr<ViewConfiguration> MachineEditorWidget::getViewConfiguration() const
{
	shared_ptr<ViewConfiguration> configuration(new ViewConfiguration());

	configuration->sceneTranslation = -(this->machineDisplayArea->getVisibleArea().topLeft());
	configuration->zoomLevel        = this->machineDisplayArea->getZoomLevel();
	configuration->viewCenter       = this->machineDisplayArea->getVisibleArea().center();

	return configuration;
}

void MachineEditorWidget::machineUpdatedEventHandler(bool isNewMachine)
{
	shared_ptr<Machine> newMachine = this->machineManager->getMachine();
	if (newMachine != nullptr)
	{
		this->machineManager->addConnection(connect(newMachine.get(), &Machine::simulationModeChangedEvent, this, &MachineEditorWidget::simulationModeToggledEventHandler));
	}

	if (isNewMachine == true)
	{
		this->resetToolbar();
	}
}

void MachineEditorWidget::simulationModeToggledEventHandler(Machine::simulation_mode newMode)
{
	if (newMode == Machine::simulation_mode::simulateMode)
	{
		this->drawingToolBar->setEnabled(false);
	}
	else
	{
		this->drawingToolBar->setEnabled(true);
	}
}

void MachineEditorWidget::resetToolbar()
{
	delete this->drawingToolBar;
	this->drawingToolBar = nullptr;

	shared_ptr<Machine> newMachine = this->machineManager->getMachine();
	if (newMachine != nullptr)
	{
		shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(newMachine);
		if (fsm != nullptr)
		{
			this->drawingToolBar = new FsmDrawingToolBar(newMachine->getMachineBuilder(), this);
			this->drawingToolBar->setMovable(true);
			this->addToolBar(Qt::TopToolBarArea, this->drawingToolBar);
		}
	}
}
