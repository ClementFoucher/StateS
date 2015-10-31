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

// Qt classes
#include <QVBoxLayout>

// StateS classes
#include "scenewidget.h"
#include "machinesimulator.h"
#include "simulationwidget.h"


DisplayArea::DisplayArea(QWidget* parent) :
    QWidget(parent)
{
    this->setLayout(new QVBoxLayout());
    this->machineDisplayArea = new SceneWidget();
    this->currentDisplayArea = this->machineDisplayArea;

    connect(this->machineDisplayArea, &SceneWidget::itemSelectedEvent,       this, &DisplayArea::itemSelectedEvent);
    connect(this->machineDisplayArea, &SceneWidget::editSelectedItemEvent,   this, &DisplayArea::editSelectedItemEvent);
    connect(this->machineDisplayArea, &SceneWidget::renameSelectedItemEvent, this, &DisplayArea::renameSelectedItemEvent);

    this->layout()->addWidget(this->currentDisplayArea);
}

void DisplayArea::setMachine(shared_ptr<Machine> newMachine)
{
    // Clear

    shared_ptr<Machine> oldMachine = this->machine.lock();

    if (oldMachine != nullptr)
        disconnect(oldMachine.get(), &Machine::changedModeEvent, this, &DisplayArea::simulationModeToggledEventHandler);

    delete this->timeline;
    delete this->tabbedDisplayArea;

    this->timeline           = nullptr;
    this->currentDisplayArea = nullptr;
    this->tabbedDisplayArea  = nullptr;

    // Set

    this->machineDisplayArea->setMachine(newMachine);
    this->machine = newMachine;

    if (newMachine != nullptr)
        connect(newMachine.get(), &Machine::changedModeEvent, this, &DisplayArea::simulationModeToggledEventHandler);
}

QGraphicsScene* DisplayArea::getScene()
{
    if (this->machineDisplayArea != nullptr)
        return this->machineDisplayArea->scene();
    else
        return nullptr;
}

void DisplayArea::simulationModeToggledEventHandler(Machine::mode newMode)
{
    this->layout()->removeWidget(this->currentDisplayArea);

    shared_ptr<Machine> l_machine = this->machine.lock();

    if ( (l_machine != nullptr) && (newMode == Machine::mode::simulateMode) )
    {
        this->timeline = new SimulationWidget(l_machine);
        connect(this->timeline, &SimulationWidget::detachTimelineEvent, this, &DisplayArea::setTimelineDetachedState);

        this->buildTabs();
    }
    else
    {
        this->clearTabs();

        delete this->timeline;
        this->timeline = nullptr;
    }
}

void DisplayArea::setTimelineDetachedState(bool detach)
{
    if (this->timeline != nullptr)
    {
        if (detach)
        {
            this->clearTabs();
            this->timeline->show();
        }
        else
        {
            this->buildTabs();
        }
    }
}

void DisplayArea::buildTabs()
{
    if (this->timeline != nullptr)
    {
        this->tabbedDisplayArea = new QTabWidget();

        this->tabbedDisplayArea->addTab(this->machineDisplayArea, tr("Machine"));
        this->tabbedDisplayArea->addTab(this->timeline,           tr("Timeline"));

        this->currentDisplayArea = this->tabbedDisplayArea;
        this->layout()->addWidget(this->currentDisplayArea);
        this->currentDisplayArea->show();
    }
}

void DisplayArea::clearTabs()
{
    if (this->tabbedDisplayArea != nullptr)
    {
        this->machineDisplayArea->setParent(this);
        this->timeline->setParent(nullptr);

        delete this->tabbedDisplayArea;
        this->tabbedDisplayArea = nullptr;

        this->currentDisplayArea = this->machineDisplayArea;
        this->layout()->addWidget(this->currentDisplayArea);
        this->currentDisplayArea->show();
    }
}
