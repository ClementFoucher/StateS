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

#ifndef RESOURCEBAR_H
#define RESOURCEBAR_H

// Parent
#include <QTabWidget>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QGraphicsScene;

// StateS classes
#include "statestypes.h"
class MachineComponentVisualizer;


/**
 * @brief The ResourceBar class displays tabs
 * to edit and act upon the current machine.
 * On machine change, tabs are deleted and
 * new ones are created.
 */
class ResourceBar : public QTabWidget
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum TabIndex_t : int
	{
		hintTabIndex            = 0,
		machineEditorTabIndex   = 1,
		componentEditorTabIndex = 2,
		simulatorTabIndex       = 3,
		verifierTabIndex        = 4,
		aboutTabIndex           = 5
	};

	/////
	// Constructors/destructors
public:
	explicit ResourceBar(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setSelectedItem(componentId_t componentId);
	void editSelectedItem();
	void renameSelectedItem();

	shared_ptr<QGraphicsScene> getComponentVisualizationScene() const;

private slots:
	void machineReplacedEventHandler();

	void clearSelection();
	void machineModeChangedEventHandler(SimulationMode_t newMode);

private:
	void build();

	/////
	// Object variables
private:
	shared_ptr<MachineComponentVisualizer> machineComponentScene;

};

#endif // RESOURCEBAR_H
