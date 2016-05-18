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
#include "displayarea.h"

// StateS classes
#include "scenewidget.h"
#include "simulationwidget.h"
#include "toolbar.h"


DisplayArea::DisplayArea(QWidget* parent) :
    QMainWindow(parent)
{
    this->setWindowFlags(Qt::Widget);
    this->machineDisplayArea = new SceneWidget();

    connect(this->machineDisplayArea, &SceneWidget::itemSelectedEvent,       this, &DisplayArea::itemSelectedEvent);
    connect(this->machineDisplayArea, &SceneWidget::editSelectedItemEvent,   this, &DisplayArea::editSelectedItemEvent);
    connect(this->machineDisplayArea, &SceneWidget::renameSelectedItemEvent, this, &DisplayArea::renameSelectedItemEvent);

    this->toolBar = new ToolBar(this);
    this->toolBar->setMovable(true);
    this->addToolBar(Qt::LeftToolBarArea, this->toolBar);

    this->currentDisplayArea = this->machineDisplayArea;
    this->showCurrentDisplay();
}

void DisplayArea::setMachine(shared_ptr<Machine> newMachine)
{
    // Clear
    shared_ptr<Machine> oldMachine = this->machine.lock();

    if (oldMachine != nullptr)
        disconnect(oldMachine.get(), &Machine::changedModeEvent, this, &DisplayArea::simulationModeToggledEventHandler);

    // Set
    this->machineDisplayArea->setMachine(newMachine);
    this->machine = newMachine;

    if (newMachine != nullptr)
        connect(newMachine.get(), &Machine::changedModeEvent, this, &DisplayArea::simulationModeToggledEventHandler);

    this->resetDisplay();
}

GenericScene* DisplayArea::getScene() const
{
    return this->machineDisplayArea->getScene();
}

ToolBar* DisplayArea::getToolbar() const
{
    return this->toolBar;
}

void DisplayArea::simulationModeToggledEventHandler(Machine::mode newMode)
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if ( (l_machine != nullptr) && (newMode == Machine::mode::simulateMode) )
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
            this->displayScene();
            this->timeline->show();
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

        this->currentDisplayArea = this->tabbedDisplayArea;
        this->showCurrentDisplay();
    }
}

void DisplayArea::displayScene()
{
    this->machineDisplayArea->setParent(this);
    if (this->timeline != nullptr)
    {
        this->timeline->setParent(nullptr);
        this->timeline->show();
    }

    delete this->tabbedDisplayArea;
    this->tabbedDisplayArea = nullptr;

    this->currentDisplayArea = this->machineDisplayArea;
    this->showCurrentDisplay();
}

void DisplayArea::resetDisplay()
{
    this->machineDisplayArea->setParent(this);

    delete this->timeline;
    delete this->tabbedDisplayArea;

    this->timeline           = nullptr;
    this->tabbedDisplayArea  = nullptr;

    this->currentDisplayArea = this->machineDisplayArea;
    this->showCurrentDisplay();
}

void DisplayArea::showCurrentDisplay()
{
    this->setCentralWidget(this->currentDisplayArea);
    this->currentDisplayArea->show();
}
