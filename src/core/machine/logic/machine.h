/*
 * Copyright © 2014-2021 Clément Foucher
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

#ifndef MACHINE_H
#define MACHINE_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QHash>
class QGraphicsItem;

// StateS classes
#include "logicvalue.h"
#include "machineundocommand.h"
class Signal;
class Input;
class Output;
class MachineSimulator;


class Machine : public QObject
{
	Q_OBJECT

public: // Static
	enum MachineErrorEnum
	{
		unknown_signal   = 0,
		impossible_error = 1
	};

public:
	enum class signal_type     {Input, Output, LocalVariable, Constant};
	enum class simulation_mode {editMode, simulateMode};

public:
	explicit Machine();
	~Machine();

	// Accessors

	QString getName() const;

	QList<shared_ptr<Input>>  getInputs()         const; // TODO: throw exception
	QList<shared_ptr<Output>> getOutputs()        const; // TODO: throw exception
	QList<shared_ptr<Signal>> getLocalVariables() const;
	QList<shared_ptr<Signal>> getConstants()      const;

	QList<shared_ptr<Signal>> getInputsAsSignals()  const;
	QList<shared_ptr<Signal>> getOutputsAsSignals() const;

	QList<shared_ptr<Signal>> getWrittableSignals()        const;
	QList<shared_ptr<Signal>> getReadableSignals()         const;
	QList<shared_ptr<Signal>> getReadableVariableSignals() const;
	QList<shared_ptr<Signal>> getVariablesSignals()        const;
	QList<shared_ptr<Signal>> getIoSignals()               const;
	QList<shared_ptr<Signal>> getAllSignals()              const;

	// Mutators

	bool setName(const QString& newName);

	shared_ptr<Signal> addSignal(signal_type type, const QString& name, const LogicValue& value = LogicValue::getNullValue());
	bool deleteSignal(const QString& name);
	bool renameSignal(const QString& oldName, const QString& newName);
	void resizeSignal(const QString& name, uint newSize); // Throws StatesException
	void changeSignalInitialValue(const QString& name, LogicValue newValue); // Throws StatesException
	bool changeSignalRank(const QString& name, uint newRank);

	// Overriden in sub classes

	virtual void setSimulator(shared_ptr<MachineSimulator> simulator);

	// Undo/redo related

	void setInhibitEvents(bool inhibit);
	void beginAtomicEdit();
	void endAtomicEdit();

	// Other

	simulation_mode getCurrentSimulationMode() const;
	shared_ptr<MachineSimulator> getSimulator() const;
	QGraphicsItem* getComponentVisualization() const;

	bool cleanSignalName(QString& nameToClean) const;
	QString getUniqueSignalName(const QString& prefix) const;

signals:
	void machineEditedWithoutUndoCommandGeneratedEvent(MachineUndoCommand::undo_command_id commandId);
	void machineEditedWithUndoCommandGeneratedEvent(MachineUndoCommand* undoCommand);

	void machineNameChangedEvent(const QString& newName);
	void inputListChangedEvent();
	void outputListChangedEvent();
	void localVariableListChangedEvent();
	void constantListChangedEvent();

	void componentVisualizationUpdatedEvent();
	void simulationModeChangedEvent(simulation_mode newMode);

protected:
	void setSimulationMode(simulation_mode newMode);
	void emitMachineEditedWithoutUndoCommand(MachineUndoCommand::undo_command_id commandId = MachineUndoCommand::undo_command_id::machineGenericUndoId);
	void emitMachineEditedWithUndoCommand(MachineUndoCommand* undoCommand);

private:
	shared_ptr<Signal> addSignalAtRank(signal_type type, const QString& name, uint rank, const LogicValue& value);
	QList<shared_ptr<Signal>> getRankedSignalList(const QHash<QString, shared_ptr<Signal>>* signalHash, const QHash<QString, uint>* rankHash) const; // TODO: throw exception
	void addSignalToList(shared_ptr<Signal> signal, uint rank, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);
	bool deleteSignalFromList(const QString& name, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);
	bool renameSignalInList(const QString& oldName, const QString& newName, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);
	bool changeRankInList(const QString& name, uint newRank, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash);

	QHash<QString, shared_ptr<Signal>> getAllSignalsMap() const;

protected:
	bool isBeingDestroyed = false;

private:
	// Store all signals as shared_ptr<Signal> for helper functions,
	// but can actually be shared_ptr<Input/Output/Constant>

	// Mutex required for list edition?
	QHash<QString, shared_ptr<Signal>> inputs;
	QHash<QString, shared_ptr<Signal>> outputs;
	QHash<QString, shared_ptr<Signal>> localVariables;
	QHash<QString, shared_ptr<Signal>> constants;

	QHash<QString, uint> inputsRanks;
	QHash<QString, uint> outputsRanks;
	QHash<QString, uint> localVariablesRanks;
	QHash<QString, uint> constantsRanks;

	QString name;

	simulation_mode currentMode = simulation_mode::editMode;

	weak_ptr<MachineSimulator> simulator;

	bool atomicEditionOngoing = false;
	uint eventInhibitionLevel = 0;
};

#endif // MACHINE_H
