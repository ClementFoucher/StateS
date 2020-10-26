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
#include "displayarea.h"

// StateS classes
#include "scenewidget.h"
#include "simulationwidget.h"
#include "maintoolbar.h"
#include "fsmdrawingtoolbar.h"
#include "fsm.h"
#include "machineconfiguration.h"


DisplayArea::DisplayArea(QWidget* parent) :
    QMainWindow(parent)
{
	this->setWindowFlags(Qt::Widget);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	this->mainToolBar = new MainToolBar(this);
	this->mainToolBar->setMovable(true);
	this->addToolBar(Qt::LeftToolBarArea, this->mainToolBar);

	this->machineDisplayArea = new SceneWidget(this);

	this->setCurrentDisplay(this->machineDisplayArea);

	// Transmit events to upper level
	connect(this->machineDisplayArea, &SceneWidget::itemSelectedEvent,       this, &DisplayArea::itemSelectedEvent);
	connect(this->machineDisplayArea, &SceneWidget::editSelectedItemEvent,   this, &DisplayArea::editSelectedItemEvent);
	connect(this->machineDisplayArea, &SceneWidget::renameSelectedItemEvent, this, &DisplayArea::renameSelectedItemEvent);
}

void DisplayArea::setMachine(shared_ptr<Machine> newMachine, bool maintainView)
{
	// Clear
	shared_ptr<Machine> oldMachine = this->machine.lock();
	if (oldMachine != nullptr)
	{
		disconnect(oldMachine.get(), &Machine::simulationModeChangedEvent, this, &DisplayArea::simulationModeToggledEventHandler);
		this->drawingToolBar->deleteLater();
		this->drawingToolBar = nullptr;
	}

	// Set
	this->machine = newMachine;

	if (newMachine != nullptr)
	{
		shared_ptr<MachineBuilder> machineBuilder = newMachine->getMachineBuilder();

		connect(newMachine.get(), &Machine::simulationModeChangedEvent, this, &DisplayArea::simulationModeToggledEventHandler);

		shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(newMachine);

		if (fsm != nullptr)
		{
			this->drawingToolBar = new FsmDrawingToolBar(machineBuilder);
			this->drawingToolBar->setMovable(true);
			this->addToolBar(Qt::TopToolBarArea, this->drawingToolBar);
		}
	}

	this->resetDisplay();

	this->machineDisplayArea->setMachine(newMachine, maintainView);
}

MainToolBar* DisplayArea::getMainToolBar() const
{
	return this->mainToolBar;
}

SceneWidget* DisplayArea::getSceneWidget() const
{
	return this->machineDisplayArea;
}

GenericScene* DisplayArea::getScene() const
{
	return this->machineDisplayArea->getScene();
}

void DisplayArea::clearSelection()
{
	this->machineDisplayArea->clearSelection();
}

void DisplayArea::setConfiguration(shared_ptr<MachineConfiguration> configuration)
{
	if (configuration != nullptr)
	{
		this->machineDisplayArea->setZoomLevel(configuration->zoomLevel);
		this->machineDisplayArea->centerOn(configuration->viewCenter);
	}
}

shared_ptr<MachineConfiguration> DisplayArea::getConfiguration() const
{
	shared_ptr<MachineConfiguration> configuration(new MachineConfiguration());

	configuration->sceneTranslation = -(this->machineDisplayArea->getVisibleArea().topLeft());
	configuration->zoomLevel        = this->machineDisplayArea->getZoomLevel();
	configuration->viewCenter       = this->machineDisplayArea->getVisibleArea().center();

	return configuration;
}

void DisplayArea::simulationModeToggledEventHandler(Machine::simulation_mode newMode)
{
	shared_ptr<Machine> l_machine = this->machine.lock();

	if ( (l_machine != nullptr) && (newMode == Machine::simulation_mode::simulateMode) )
	{
		this->timeline = new SimulationWidget(l_machine);
		connect(this->timeline, &SimulationWidget::detachTimelineEvent, this, &DisplayArea::setTimelineDetachedState);

		this->drawingToolBar->setEnabled(false);

		this->displayTabs();
	}
	else
	{
		this->resetDisplay();
	}
}

void DisplayArea::setTimelineDetachedState(bool detach)
{
	if (this->timeline != nullptr)
	{
		if (detach)
		{
			// Must be done first to not delete display area along with tabs
			this->setCurrentDisplay(this->machineDisplayArea);

			// Detach timeline as an independent window
			this->timeline->setParent(nullptr);
			this->timeline->show();

			delete this->tabbedDisplayArea;
			this->tabbedDisplayArea = nullptr;
		}
		else
		{
			this->displayTabs();
		}
	}
}

void DisplayArea::displayTabs()
{
	if (this->timeline != nullptr)
	{
		this->tabbedDisplayArea = new QTabWidget();

		this->tabbedDisplayArea->addTab(this->machineDisplayArea, tr("Machine"));
		this->tabbedDisplayArea->addTab(this->timeline,           tr("Timeline"));

		this->setCurrentDisplay(this->tabbedDisplayArea);
	}
}

void DisplayArea::resetDisplay()
{
	// Must be done first to not delete display area along with tabs
	this->setCurrentDisplay(this->machineDisplayArea);

	delete this->timeline;
	delete this->tabbedDisplayArea;

	this->timeline           = nullptr;
	this->tabbedDisplayArea  = nullptr;

	if (this->drawingToolBar != nullptr)
	{
		this->drawingToolBar->setEnabled(true);
	}
}

void DisplayArea::setCurrentDisplay(QWidget* newDisplay)
{
	this->setCentralWidget(newDisplay);
	newDisplay->show();
}
