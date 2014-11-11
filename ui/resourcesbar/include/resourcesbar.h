/*
 * Copyright © 2014 Clément Foucher
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

#ifndef RESOURCESBAR_H
#define RESOURCESBAR_H

#include <QTabWidget>

class ToolResourcesTab;
class MachineTools;
class InterfaceResourcesTab;
class Machine;
class EditorTab;
class FsmState;
class FsmTransition;
class SimulatorTab;
class AboutTab;

class ResourcesBar : public QTabWidget
{
    Q_OBJECT

public:
    enum class mode{voidMode, editMode, simulateMode};

public:
    explicit ResourcesBar(QWidget* parent, Machine* machine = 0);
    ~ResourcesBar();

    MachineTools* getBuildTools() const;

    void setMachine(Machine* value);

    mode getCurrentMode() const;

public slots:
    void selectedState(FsmState* state, bool showTab = false, bool editName = false);
    void selectedTransition(FsmTransition* transition, bool showTab = false);
    void clearSelection();
    void beginSimulation();
    void terminateSimulation();

signals:
    void simulationToggled();

private slots:
    void tabChanged(int index);

private:
    Machine* machine = nullptr;

    ToolResourcesTab      * toolResources      = nullptr;
    InterfaceResourcesTab * interfaceResources = nullptr;
    EditorTab             * editorTab          = nullptr;
    SimulatorTab          * simulatorTab       = nullptr;
    AboutTab              * aboutTab           = nullptr;

    mode currentMode = mode::voidMode;
};

#endif // RESOURCESBAR_H
