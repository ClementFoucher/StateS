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

#ifndef STATESUI_H
#define STATESUI_H

// Parent
#include <QMainWindow>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QString;
class QAction;
class QStackedWidget;

// StateS classes
class Machine;
class ResourceBar;
class SceneWidget;
class SimulationWidget;


class StatesUi : public QMainWindow
{
    Q_OBJECT

public:
    explicit StatesUi(shared_ptr<Machine> machine);

    void setMachine(shared_ptr<Machine> machine);

signals:
    void newFsmRequestEvent();
    void clearMachineRequestEvent();
    void loadMachineRequestEvent(const QString& path);
    void machineSavedEvent(const QString& path);
    QString getCurrentFileEvent();

protected slots:
    void closeEvent(QCloseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private slots:
    void newMachineRequestEventHandler();
    void clearMachineRequestEventHandler();
    void exportImageRequestEventHandler();
    void exportVhdlRequestEventHandler();
    void saveMachineRequestEventHandler();
    void loadMachineRequestEventHandler();
    void simulationToggledEventHandler();
    void triggerViewEventHandler();
    void detachTimelineEventHandler(bool detach);
    void machineLoadedEventHandler();
    void saveMachineOnCurrentFileEventHandler();

private:
    // Action bar
    QAction* actionSave        = nullptr;
    QAction* actionSaveCurrent = nullptr;
    QAction* actionLoad        = nullptr;
    QAction* actionNewFsm      = nullptr;
    QAction* actionClear       = nullptr;
    QAction* actionExportImage = nullptr;
    QAction* actionExportVhdl  = nullptr;

    // Display area
    QStackedWidget*   mainDisplayArea    = nullptr;
    SceneWidget*      machineDisplayArea = nullptr;
    SimulationWidget* timeline           = nullptr;

    // Resource bar
    ResourceBar* resourcesBar = nullptr;

    // Owned machine
    weak_ptr<Machine> machine;
};

#endif // STATESUI_H
