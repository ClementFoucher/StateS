/*
 * Copyright © 2014-2023 Clément Foucher
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
#include "statestypes.h"
class ResourceBar;
class DisplayArea;
class MachineComponent;
class MainToolBar;
class MachineEditorWidget;
class TimelineWidget;
class ViewConfiguration;


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

	/////
	// Constructors/destructors
public:
	explicit StatesUi();

	/////
	// Object functions
public:
	void setView(shared_ptr<ViewConfiguration> viewConfiguration);
	shared_ptr<ViewConfiguration> getView() const;

signals:
	void newFsmRequestEvent();
	void clearMachineRequestEvent();
	void loadMachineRequestEvent(const QString& path);
	void saveMachineRequestEvent(const QString& path);
	void saveMachineInCurrentFileRequestEvent();

protected:
	virtual void closeEvent     (QCloseEvent* event) override;
	virtual void keyPressEvent  (QKeyEvent*   event) override;
	virtual void keyReleaseEvent(QKeyEvent*   event) override;

	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent     (QDropEvent*      event) override;

private slots:
	void machineReplacedEventHandler();

	void beginSaveAsProcedure();
	void beginSaveProcedure();
	void beginLoadProcedure();
	void beginNewMachineProcedure();
	void beginClearMachineProcedure();
	void beginExportImageProcedure();
	void beginExportVhdlProcedure();

	void undo();
	void redo();

	void itemSelectedInSceneEventHandler(componentId_t componentId);
	void editSelectedItem();
	void renameSelectedItem();

	void machineFilePathUpdated();
	void machineUnsavedStateUpdated();

	void simulationModeToggledEventHandler(SimulationMode_t newMode);
	void setTimelineDetachedState(bool detach);

	void undoActionAvailabilityChangeEventHandler(bool undoAvailable);
	void redoActionAvailabilityChangeEventHandler(bool redoAvailable);

private:
	void resetUi();
	void updateTitle();
	bool displayUnsavedConfirmation(const QString& cause);

	/////
	// Object variables
private:
	// Top level widgets
	DisplayArea* displayArea = nullptr;
	ResourceBar* resourceBar = nullptr;
	// Widgets displayed in the display area
	MainToolBar*         toolbar  = nullptr;
	MachineEditorWidget* editor   = nullptr;
	TimelineWidget*      timeline = nullptr;

};

#endif // STATESUI_H
