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

#ifndef RESOURCEBAR_H
#define RESOURCEBAR_H

// Parent
#include <QTabWidget>

// StateS classes
class MachineBuilderTab;
class MachineTools;
class SignalEditorTab;
class Machine;
class ComponentEditorTab;
class FsmState;
class FsmTransition;
class SimulatorTab;
class AboutTab;

class ResourceBar : public QTabWidget
{
    Q_OBJECT

public:
    enum class mode{voidMode, editMode, simulateMode};

public:
    explicit ResourceBar(QWidget* parent, Machine* machine = nullptr);
    ~ResourceBar();

    MachineTools* getBuildTools() const;

    void setMachine(Machine* value);

    mode getCurrentMode() const;

    void selectedState(FsmState* state, bool showTab = false, bool editName = false);
    void selectedTransition(FsmTransition* transition, bool showTab = false);

public slots:
    void clearSelection();
    void beginSimulation();
    void terminateSimulation();

signals:
    void simulationToggled();

private slots:
    void tabChanged(int index);

private:
    Machine* machine = nullptr;

    MachineBuilderTab  * toolResources      = nullptr;
    SignalEditorTab    * interfaceResources = nullptr;
    ComponentEditorTab * editorTab          = nullptr;
    SimulatorTab       * simulatorTab       = nullptr;
    AboutTab           * aboutTab           = nullptr;

    mode currentMode = mode::voidMode;
};

#endif // RESOURCEBAR_H
