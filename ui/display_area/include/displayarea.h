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
class MachineConfiguration;
class MachineComponent;


/**
 * @brief The DisplayArea class handles the central display.
 * Usually, this is just the machine graphic representation,
 * but in simulation mode, the timeline is also handled here.
 * DisplayArea also owns a drawing tool bar.
 * Finally, this class owns the main tool bar for display purpose,
 * but this one is handled directly by the StatesUI object.
 */
class DisplayArea : public QMainWindow
{
	Q_OBJECT

public:
	explicit DisplayArea(QWidget* parent = nullptr);

	void setMachine(shared_ptr<Machine> newMachine, bool maintainView);

	MainToolBar* getMainToolBar() const;
	SceneWidget* getSceneWidget() const;

	GenericScene* getScene() const;

	void clearSelection();
	void setConfiguration(shared_ptr<MachineConfiguration> configuration);
	shared_ptr<MachineConfiguration> getConfiguration() const;

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
	SceneWidget*      machineDisplayArea = nullptr; // Persistant through object life
	MainToolBar*      mainToolBar        = nullptr; // Persistant through object life; Managed in StatesUI class.
	DrawingToolBar*   drawingToolBar     = nullptr;
	SimulationWidget* timeline           = nullptr; // Displayed in simulation mode, either as a tab or as an independant window
	QTabWidget*       tabbedDisplayArea  = nullptr; // Used if containing both widgets at the same time

	weak_ptr<Machine> machine;
};

#endif // DISPLAYAREA_H
