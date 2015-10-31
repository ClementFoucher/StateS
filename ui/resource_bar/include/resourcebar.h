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

#ifndef RESOURCEBAR_H
#define RESOURCEBAR_H

// Parent
#include <QTabWidget>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QGraphicsScene;

// For enums
#include "machine.h"

// StateS classes
class MachineBuilderTab;
class SignalEditorTab;
class ComponentEditorTab;
class FsmState;
class FsmTransition;
class SimulatorTab;
class AboutTab;
class VerifierTab;
class SimulationWidget;
class MachineComponentVisualizer;
class MachineComponent;


class ResourceBar : public QTabWidget
{
    Q_OBJECT

public:
    explicit ResourceBar(QWidget* parent = nullptr);
    ~ResourceBar();

    void setMachine(shared_ptr<Machine> newMachine);

    void setSelectedItem(shared_ptr<MachineComponent> item);
    void editSelectedItem();
    void renameSelectedItem();

    shared_ptr<QGraphicsScene> getComponentVisualizationScene();

private slots:
    void clearSelection();
    void beginSimulation();
    void terminateSimulation();
    void tabChanged(int);

private:
    weak_ptr<Machine> machine;
    shared_ptr<MachineComponentVisualizer> machineComponentScene;

    // QWdigets with parent
    MachineBuilderTab  * machineBuildTab = nullptr;
    SignalEditorTab    * signalsTab      = nullptr;
    ComponentEditorTab * editorTab       = nullptr;
    SimulatorTab       * simulatorTab    = nullptr;
    VerifierTab        * verifierTab     = nullptr;
    AboutTab           * aboutTab        = nullptr;
};

#endif // RESOURCEBAR_H
