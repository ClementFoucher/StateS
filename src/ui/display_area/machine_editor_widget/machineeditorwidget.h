/*
 * Copyright © 2020-2023 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MACHINEEDITORWIDGET_H
#define MACHINEEDITORWIDGET_H

// Parent
#include <QMainWindow>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
class GenericScene;
class SceneWidget;
class DrawingToolBar;
class ViewConfiguration;


class MachineEditorWidget : public QMainWindow
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineEditorWidget(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	GenericScene* getScene() const;
	void setView(shared_ptr<ViewConfiguration> viewConfiguration);
	shared_ptr<ViewConfiguration> getView() const;

	void clearSelection();

signals:
	void itemSelectedEvent(componentId_t componentId);
	void editSelectedItemEvent();
	void renameSelectedItemEvent();

private slots:
	void machineReplacedEventHandler();

	void simulationModeToggledEventHandler(SimulationMode_t newMode);

private:
	void buildToolbar();

	/////
	// Object variables
private:
	SceneWidget*    machineDisplayArea = nullptr;
	DrawingToolBar* drawingToolBar     = nullptr;

};

#endif // MACHINEEDITORWIDGET_H
