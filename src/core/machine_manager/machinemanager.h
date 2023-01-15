/*
 * Copyright © 2021-2023 Clément Foucher
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

#ifndef MACHINEMANAGER_H
#define MACHINEMANAGER_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
#include "undoredomanager.h"
class Machine;
class MachineStatus;
class MachineBuilder;
class GraphicMachine;
class MachineSimulator;
class GraphicAttributes;
class MachineUndoCommand;


/**
 * @brief The MachineManager class is in charge of
 * handling the current machine.
 *
 * Its purposes are:
 * - handling undo and redo.
 * - building the graphic machine associated to the logic machine.
 * - acting as a relay for the machine signals. This allows
 *   other classes to connect to manager instead of machine
 *   and avoid the necessity for reconnecting all signas
 *   when machine is changed.
 * - stores and makes available the current machine status.
 */
class MachineManager : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineManager();

	/////
	// Object functions
public:

	// Mutators
	void setMachine(shared_ptr<Machine> newMachine, shared_ptr<GraphicAttributes> newGraphicAttributes);

	// Acessors
	shared_ptr<Machine>          getMachine()          const;
	shared_ptr<MachineStatus>    getMachineStatus()    const;
	shared_ptr<MachineBuilder>   getMachineBuilder()   const;
	shared_ptr<GraphicMachine>   getGraphicMachine()   const;
	shared_ptr<MachineSimulator> getMachineSimulator() const;

	// Undo/redo
	void undo();
	void redo();
	void notifyMachineEdited(MachineUndoCommand* undoCommand = nullptr);
	void setUndoRedoMode(bool undoRedoMode);

	// Simulation
	void setSimulationMode(SimulationMode_t newMode);
	SimulationMode_t getCurrentSimulationMode() const;

signals:
	/////
	// Machine manager events

	// Indicates the machine under edit has been replaced by a new one
	void machineReplacedEvent();

	// Indicates the machine under edit has been updated due to diff undo or redo action,
	// so the new structure is potentially dramatically different from the previous one.
	// Notably, graphic machine is replaced so all components depending on it should be rebuilt.
	void machineUpdatedEvent();

	void simulationModeChangedEvent(SimulationMode_t newMode);

	/////
	// Undo/redo manager events propagated by the manager

	void undoActionAvailabilityChangedEvent(bool undoAvailable);
	void redoActionAvailabilityChangedEvent(bool redoAvailable);

	/////
	// Machine events propagated by the manager so that connections can be rerouted in one place when machine changes

	void machineNameChangedEvent();
	void machineInputListChangedEvent();
	void machineOutputListChangedEvent();
	void machineLocalVariableListChangedEvent();
	void machineConstantListChangedEvent();

private slots:
	// Undo/redo
	void freshMachineAvailableFromUndoRedo(shared_ptr<Machine> updatedMachine, shared_ptr<GraphicAttributes> updatedGraphicAttributes);
	void machineUnsavedFlagChangedEventHandler();

	void logicComponentDeletedEventHandler(componentId_t componentId);
	void graphicComponentNeedsRefreshEventHandler(componentId_t componentId);

private:
	void setMachineInternal(shared_ptr<Machine> newMachine, shared_ptr<GraphicAttributes> newGraphicAttributes);

	/////
	// Object variables
private:
	// Holders
	shared_ptr<Machine>          machine;
	shared_ptr<MachineStatus>    machineStatus;
	shared_ptr<MachineBuilder>   machineBuilder;
	shared_ptr<GraphicMachine>   graphicMachine;
	shared_ptr<MachineSimulator> machineSimulator;

	// Internal
	unique_ptr<UndoRedoManager> undoRedoManager;
	bool undoRedoMode = false;
	SimulationMode_t currentSimulationMode = SimulationMode_t::editMode;

};


/////
// Public global object: most classes need access
// to machine manager, so make it a global object
extern unique_ptr<MachineManager> machineManager;


#endif // MACHINEMANAGER_H
