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

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QTabWidget;
class QGraphicsScene;

// StateS classes
#include "machine.h"
class SceneWidget;
class SimulationWidget;
class MachineComponent;


/**
 * @brief The DisplayArea class handles the central display.
 * Usually, this is just the machine graphical representation,
 * but in simulation mode, the timeline is also handled here.
 */
class DisplayArea : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayArea(QWidget* parent = nullptr);

    void setMachine(shared_ptr<Machine> newMachine);
    QGraphicsScene* getScene();

signals:
    void itemSelectedEvent(shared_ptr<MachineComponent> component);
    void editSelectedItemEvent();
    void renameSelectedItemEvent();

private slots:
    void simulationModeToggledEventHandler(Machine::mode newMode);
    void setTimelineDetachedState(bool detach);

private:
    void buildTabs();
    void clearTabs();

private:
    // Content
    SceneWidget*      machineDisplayArea = nullptr;
    SimulationWidget* timeline           = nullptr;

    // Used if containing both widgets at the same time
    QWidget*          currentDisplayArea = nullptr;

    // Used to remember which widget is currently displayed
    QTabWidget*       tabbedDisplayArea  = nullptr;

    weak_ptr<Machine> machine;
};

#endif // DISPLAYAREA_H
