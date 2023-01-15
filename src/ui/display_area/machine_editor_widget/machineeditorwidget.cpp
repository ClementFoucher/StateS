/*
 * Copyright © 2020-2023 Clément Foucher
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
#include "drawingtoolbarbuilder.h"


MachineEditorWidget::MachineEditorWidget(QWidget* parent) :
    QMainWindow(parent)
{
	connect(machineManager.get(), &MachineManager::machineReplacedEvent, this, &MachineEditorWidget::machineReplacedEventHandler);

	this->setWindowFlags(Qt::Widget);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	this->machineDisplayArea = new SceneWidget(this);
	this->setCentralWidget(this->machineDisplayArea);

	// Transmit events to upper level
	connect(this->machineDisplayArea, &SceneWidget::itemSelectedEvent,       this, &MachineEditorWidget::itemSelectedEvent);
	connect(this->machineDisplayArea, &SceneWidget::editSelectedItemEvent,   this, &MachineEditorWidget::editSelectedItemEvent);
	connect(this->machineDisplayArea, &SceneWidget::renameSelectedItemEvent, this, &MachineEditorWidget::renameSelectedItemEvent);

	// Connect machine manager signal
	connect(machineManager.get(), &MachineManager::simulationModeChangedEvent, this, &MachineEditorWidget::simulationModeToggledEventHandler);

	// Initialize tool bar
	this->buildToolbar();
}

GenericScene* MachineEditorWidget::getScene() const
{
	return this->machineDisplayArea->getScene();
}

void MachineEditorWidget::setView(shared_ptr<ViewConfiguration> viewConfiguration)
{
	this->machineDisplayArea->setView(viewConfiguration);
}

shared_ptr<ViewConfiguration> MachineEditorWidget::getView() const
{
	return this->machineDisplayArea->getView();
}

void MachineEditorWidget::clearSelection()
{
	this->machineDisplayArea->clearSelection();
}

void MachineEditorWidget::machineReplacedEventHandler()
{
	delete this->drawingToolBar;
	this->buildToolbar();
}

void MachineEditorWidget::simulationModeToggledEventHandler(SimulationMode_t newMode)
{
	if (newMode == SimulationMode_t::simulateMode)
	{
		this->drawingToolBar->setEnabled(false);
	}
	else
	{
		this->drawingToolBar->setEnabled(true);
	}
}

void MachineEditorWidget::buildToolbar()
{
	this->drawingToolBar = DrawingToolBarBuilder::buildDrawingToolBar();
	if (this->drawingToolBar != nullptr)
	{
		this->drawingToolBar->setMovable(true);
		this->addToolBar(Qt::TopToolBarArea, this->drawingToolBar);
	}
}
