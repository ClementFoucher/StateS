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


/**
 * @brief The DisplayArea class handles the central display.
 * Usually, this is just the machine graphic representation,
 * but in simulation mode, the timeline is also handled here.
 * DisplayArea ilso owns the tool bar to allow placing it near
 * the resoure bar.
 */
class DisplayArea : public QMainWindow
{
	Q_OBJECT

public:
	explicit DisplayArea(QWidget* parent = nullptr);

	void setMachine(shared_ptr<Machine> newMachine);

	MainToolBar*    getMainToolBar()    const;
	DrawingToolBar* getDrawingToolBar() const;
	SceneWidget*    getSceneWidget()    const;

private slots:
	void simulationModeToggledEventHandler(Machine::simulation_mode newMode);
	void setTimelineDetachedState(bool detach);

private:
	void displayTabs();
	void resetDisplay();
	void setCurrentDisplay(QWidget* newDisplay);

private:
	SceneWidget*      machineDisplayArea = nullptr; // Persistant through object life
	MainToolBar*      mainToolBar        = nullptr; // Persistant through object life
	DrawingToolBar*   drawingToolBar     = nullptr;
	SimulationWidget* timeline           = nullptr; // Displayed in simulation mode, either as a tab or as an independant window
	QTabWidget*       tabbedDisplayArea  = nullptr; // Used if containing both widgets at the same time

	weak_ptr<Machine> machine;
};

#endif // DISPLAYAREA_H
