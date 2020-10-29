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
class ViewConfiguration;
class MainToolBar;


/**
 * @brief The StatesUi class handles the main window.
 *
 * This window has three areas:
 * - an action bar; technically belongs to display area for display purpose but managed here,
 * - The display area,
 * - The resources bar.
 *
 * This object can request some actions on the currently edited machine (save, load, new, ...)
 */
class StatesUi : public QMainWindow
{
	Q_OBJECT

public:
	explicit StatesUi();

	void setMachine(shared_ptr<Machine> newMachine, bool maintainView);
	void setViewConfiguration(shared_ptr<ViewConfiguration> configuration);
	shared_ptr<ViewConfiguration> getViewConfiguration() const;

	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

	void displayErrorMessage(const QString& errorTitle, const QList<QString>& errorList);
	void displayErrorMessage(const QString& errorTitle, const QString& errorList);

	void setUndoButtonEnabled(bool enable);
	void setRedoButtonEnabled(bool enable);

signals:
	void newFsmRequestEvent();
	void clearMachineRequestEvent();
	void loadMachineRequestEvent(const QString& path);
	void saveMachineRequestEvent(const QString& path);
	void saveMachineInCurrentFileRequestEvent();
	void undoRequestEvent();
	void redoRequestEvent();

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

	void machineFilePathUpdated();
	void machineUnsavedStateUpdated();

private:
	void updateTitle();
	bool displayUnsavedConfirmation(const QString& cause);

private:
	// Display area and resource bar
	DisplayArea* displayArea = nullptr;
	ResourceBar* resourceBar = nullptr;
	MainToolBar* toolbar     = nullptr;

	// Current machine
	weak_ptr<Machine> machine;
};

#endif // STATESUI_H
