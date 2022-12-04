/*
 * Copyright © 2014-2022 Clément Foucher
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
#include "machine.h"
class MachineManager;
class ResourceBar;
class DisplayArea;
class MachineComponent;
class MainToolBar;
class MachineEditorWidget;
class TimelineWidget;


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
	explicit StatesUi(shared_ptr<MachineManager> machineManager);

	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

signals:
	void newFsmRequestEvent();
	void clearMachineRequestEvent();
	void loadMachineRequestEvent(const QString& path);
	void saveMachineRequestEvent(const QString& path);
	void saveMachineInCurrentFileRequestEvent();

protected:
	void closeEvent     (QCloseEvent* event) override;
	void keyPressEvent  (QKeyEvent*   event) override;
	void keyReleaseEvent(QKeyEvent*   event) override;

private slots:
	void machineUpdatedEventHandler(bool isNewMachine);

	void beginSaveAsProcedure();
	void beginSaveProcedure();
	void beginLoadProcedure();
	void beginNewMachineProcedure();
	void beginClearMachineProcedure();
	void beginExportImageProcedure();
	void beginExportVhdlProcedure();

	void undo();
	void redo();

	void itemSelectedInSceneEventHandler(shared_ptr<MachineComponent> item);
	void editSelectedItem();
	void renameSelectedItem();

	void machineFilePathUpdated();
	void machineUnsavedStateUpdated();

	void simulationModeToggledEventHandler(Machine::simulation_mode newMode);
	void setTimelineDetachedState(bool detach);

	void undoActionAvailabilityChangeEventHandler(bool undoAvailable);
	void redoActionAvailabilityChangeEventHandler(bool redoAvailable);

private:
	void resetUi();
	void updateTitle();
	bool displayUnsavedConfirmation(const QString& cause);

private:
	shared_ptr<MachineManager> machineManager;

	// Top level widgets
	DisplayArea* displayArea = nullptr;
	ResourceBar* resourceBar = nullptr;
	// Widgets displayed in the display area
	MainToolBar*         toolbar  = nullptr;
	MachineEditorWidget* editor   = nullptr;
	TimelineWidget*      timeline = nullptr;

};

#endif // STATESUI_H
