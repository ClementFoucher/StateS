/*
 * Copyright © 2014-2017 Clément Foucher
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

// StateS classes
class Machine;
class ResourceBar;
class DisplayArea;
class MachineComponent;
class MachineConfiguration;
class ToolBar;
class SceneWidget;


/**
 * @brief The StatesUi class handles the main window.
 *
 * This window has tree areas:
 * - an action bar (on the left at startup, but can be moved at top or bottom),
 * - a central widget wichi is a slider made of:
 * -- The display area, holding notably the machine graphic representation,
 * -- The resources bar, holding tools to edit and use the machine.
 *
 * This object can receive orders to use switch to a different machine
 * edition, or to update the currently used save file (for display only).
 *
 * It can request some actions on the currently edited machine (save, load, new, ...)
 */
class StatesUi : public QMainWindow
{
    Q_OBJECT

public:
    explicit StatesUi();

    void setMachine(shared_ptr<Machine> newMachine);
    void setTitle(const QString& title);
    void setUnsavedFlag(bool unsaved);
    void setConfiguration(shared_ptr<MachineConfiguration> configuration);
    shared_ptr<MachineConfiguration> getConfiguration() const;

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    void displayErrorMessage(const QString& errorTitle, const QList<QString>& errorList);
    void displayErrorMessage(const QString& errorTitle, const QString& errorList);

    void setSaveAsActionEnabled       (bool enable);
    void setSaveActionEnabled         (bool enable);
    void setExportActionsEnabled      (bool enable);
    void setAddCheckpointButtonEnabled(bool enabled);
    void setUndoButtonEnabled         (bool enabled);

signals:
    void newFsmRequestEvent();
    void clearMachineRequestEvent();
    void loadMachineRequestEvent(const QString& path);
    void saveMachineRequestEvent(const QString& path);
    void saveMachineInCurrentFileRequestEvent();
    void addCheckpointRequestEvent();
    void undoRequestEvent();

protected:
    void closeEvent     (QCloseEvent* event) override;
    void keyPressEvent  (QKeyEvent*   event) override;
    void keyReleaseEvent(QKeyEvent*   event) override;

private slots:
    void beginSaveAsProcedure();
    void beginSaveProcedure();
    void beginLoadProcedure();
    void beginNewMachineProcedure();
    void beginClearMachineProcedure();
    void beginExportImageProcedure();
    void beginExportVhdlProcedure();

    void itemSelectedInSceneEventHandler(shared_ptr<MachineComponent> item);
    void editSelectedItem();
    void renameSelectedItem();

private:
    void updateTitle();
    bool displayUnsavedConfirmation(const QString& cause);

private:
    // Display area and resource bar
    DisplayArea* displayArea = nullptr;
    ToolBar*     toolbar     = nullptr;
    SceneWidget* sceneWidget = nullptr;
    ResourceBar* resourceBar = nullptr;

    // Current machine
    weak_ptr<Machine> machine;
    QString windowTitle = QString::null;
    bool unsavedFlag = false;
};

#endif // STATESUI_H
