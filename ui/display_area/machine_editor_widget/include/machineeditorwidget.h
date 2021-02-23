/*
 * Copyright © 2020-2021 Clément Foucher
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
#include "machine.h"
class MachineManager;
class GenericScene;
class ViewConfiguration;
class SceneWidget;
class DrawingToolBar;
class MachineComponent;


class MachineEditorWidget : public QMainWindow
{
	Q_OBJECT

public:
	explicit MachineEditorWidget(shared_ptr<MachineManager> machineManager, QWidget* parent = nullptr);

	GenericScene* getScene() const;

	void clearSelection();

	void setViewConfiguration(shared_ptr<ViewConfiguration> configuration);
	shared_ptr<ViewConfiguration> getViewConfiguration() const;

signals:
	void itemSelectedEvent(shared_ptr<MachineComponent> component);
	void editSelectedItemEvent();
	void renameSelectedItemEvent();

private slots:
	void machineUpdatedEventHandler(bool isNewMachine);

	void simulationModeToggledEventHandler(Machine::simulation_mode newMode);

private:
	void resetToolbar();

private:
	shared_ptr<MachineManager> machineManager;

	SceneWidget*    machineDisplayArea = nullptr;
	DrawingToolBar* drawingToolBar     = nullptr;
};

#endif // MACHINEEDITORWIDGET_H
