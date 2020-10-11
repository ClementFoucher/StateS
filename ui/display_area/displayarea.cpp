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


DisplayArea::DisplayArea(QWidget* parent) :
    QMainWindow(parent)
{
	this->setWindowFlags(Qt::Widget);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	this->toolBar = new MainToolBar(this);
	this->toolBar->setMovable(true);
	this->addToolBar(Qt::LeftToolBarArea, this->toolBar);

	this->machineDisplayArea = new SceneWidget(this);

	this->setCurrentDisplay(this->machineDisplayArea);
}

void DisplayArea::setMachine(shared_ptr<Machine> newMachine)
{
	// Clear
	shared_ptr<Machine> oldMachine = this->machine.lock();
	if (oldMachine != nullptr)
	{
		disconnect(oldMachine.get(), &Machine::simulationModeChangedEvent, this, &DisplayArea::simulationModeToggledEventHandler);
	}

	// Set
	this->machine = newMachine;

	if (newMachine != nullptr)
	{
		connect(newMachine.get(), &Machine::simulationModeChangedEvent, this, &DisplayArea::simulationModeToggledEventHandler);
	}

	this->resetDisplay();
}

MainToolBar* DisplayArea::getMainToolbar() const
{
	return this->toolBar;
}

SceneWidget* DisplayArea::getSceneWidget() const
{
	return this->machineDisplayArea;
}

void DisplayArea::simulationModeToggledEventHandler(Machine::simulation_mode newMode)
{
	shared_ptr<Machine> l_machine = this->machine.lock();

	if ( (l_machine != nullptr) && (newMode == Machine::simulation_mode::simulateMode) )
	{
		this->timeline = new SimulationWidget(l_machine);
		connect(this->timeline, &SimulationWidget::detachTimelineEvent, this, &DisplayArea::setTimelineDetachedState);

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
}

void DisplayArea::setCurrentDisplay(QWidget* newDisplay)
{
	this->setCentralWidget(newDisplay);
	newDisplay->show();
}
