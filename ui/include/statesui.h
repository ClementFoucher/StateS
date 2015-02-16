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

#ifndef STATESUI_H
#define STATESUI_H

// Parent
#include <QMainWindow>

// Qt classes
#include <QAction>

// StateS classes
class Machine;
class ResourceBar;
class SceneWidget;

class StatesUi : public QMainWindow
{
    Q_OBJECT

public:
    explicit StatesUi(Machine* machine = nullptr);

    void setMachine(Machine* machine);

public slots:
    void clearMachine();
    void newMachine();

protected slots:
    void closeEvent(QCloseEvent*) override;

private slots:
    void newMachineRequestEvent();
    void clearMachineRequestEvent();
    void exportPdfRequestEvent();
    void saveMachineRequestEvent();
    void loadMachineRequestEvent();
    void handleSimulationToggled();

private:
    QAction* actionSave = nullptr;
    QAction* actionLoad = nullptr;
    QAction* actionNewFsm = nullptr;
    QAction* actionClear = nullptr;
    QAction* actionExportPdf = nullptr;

    SceneWidget* machineDisplayArea = nullptr;
    ResourceBar* resourcesBar       = nullptr;

    // For debug only
    Machine* machine = nullptr;

};

#endif // STATESUI_H
