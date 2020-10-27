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

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H

// Parent
#include <QMainWindow>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QTabWidget;

// StateS classes
#include "machine.h"
class SceneWidget;
class SimulationWidget;
class MainToolBar;
class DrawingToolBar;
class GenericScene;
class ViewConfiguration;
class MachineComponent;


/**
 * @brief The DisplayArea class handles the central display.
 * Usually, it just displays the editor area (which includes
 * the machine graphic representation and its drawing tool bar),
 * but in simulation mode, the timeline is also displayed here,
 * and a tabbed widget is used to display both.
 *
 * This class also owns the main tool bar for display purpose,
 * but this one is handled at the StatesUI object level.
 */
class DisplayArea : public QMainWindow
{
	Q_OBJECT

public:
	explicit DisplayArea(QWidget* parent = nullptr);

	void setMachine(shared_ptr<Machine> newMachine, bool maintainView);

	MainToolBar* getMainToolBar() const;

	GenericScene* getScene() const;

	void clearSelection();
	void setViewConfiguration(shared_ptr<ViewConfiguration> configuration);
	shared_ptr<ViewConfiguration> getViewConfiguration() const;

signals:
	void itemSelectedEvent(shared_ptr<MachineComponent> component);
	void editSelectedItemEvent();
	void renameSelectedItemEvent();

private slots:
	void simulationModeToggledEventHandler(Machine::simulation_mode newMode);
	void setTimelineDetachedState(bool detach);

private:
	void displayTabs();
	void resetDisplay();
	void setCurrentDisplay(QWidget* newDisplay);

private:
	// Current display may be editor area or tabbed display depending on current mode
	MainToolBar*      mainToolBar        = nullptr; // Persistant through object life; Managed in StatesUI class.
	QMainWindow*      editorArea         = nullptr; // Persistant through object life; Contains display area and drawing tool bar
	SceneWidget*      machineDisplayArea = nullptr; // Persistant through object life
	DrawingToolBar*   drawingToolBar     = nullptr; // Created when display area is valid
	SimulationWidget* timeline           = nullptr; // Displayed in simulation mode, either as a tab or as an independant window
	QTabWidget*       tabbedDisplayArea  = nullptr; // Used if containing both widgets at the same time

	weak_ptr<Machine> machine;
};

#endif // DISPLAYAREA_H
