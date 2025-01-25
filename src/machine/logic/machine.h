/*
 * Copyright © 2014-2025 Clément Foucher
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

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class Variable;
class Input;
class Output;
class MachineComponent;


class Machine : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Machine();
	~Machine();

	/////
	// Object functions
public:

	// Accessors

	QString getName() const;

	QList<shared_ptr<Input>>    getInputs()            const; // TODO: throw exception
	QList<shared_ptr<Output>>   getOutputs()           const; // TODO: throw exception
	QList<shared_ptr<Variable>> getInternalVariables() const;
	QList<shared_ptr<Variable>> getConstants()         const;

	QList<shared_ptr<Variable>> getInputsAsVariables()  const;
	QList<shared_ptr<Variable>> getOutputsAsVariables() const;

	QList<shared_ptr<Variable>> getWrittableVariables()        const;
	QList<shared_ptr<Variable>> getReadableVariables()         const;
	QList<shared_ptr<Variable>> getReadableVariableVariables() const;
	QList<shared_ptr<Variable>> getAllVariables()              const;

	shared_ptr<MachineComponent> getComponent(componentId_t componentId) const;

	// Mutators

	bool setName(const QString& newName);

	shared_ptr<Variable> addVariable(VariableNature_t type, const QString& name, const LogicValue& value = LogicValue::getNullValue());
	bool deleteVariable(const QString& name);
	bool renameVariable(const QString& oldName, const QString& newName);
	void resizeVariable(const QString& name, uint newSize); // Throws StatesException
	void changeVariableInitialValue(const QString& name, LogicValue newValue); // Throws StatesException
	bool changeVariableRank(const QString& name, uint newRank);

	// Other

	QString getUniqueVariableName(const QString& prefix) const;

signals:
	// Machine changes
	void machineNameChangedEvent();
	void machineInputListChangedEvent();
	void machineOutputListChangedEvent();
	void machineLocalVariableListChangedEvent();
	void machineConstantListChangedEvent();

	// Components changes
	void graphicComponentNeedsRefreshEvent(componentId_t stateId);
	void componentDeletedEvent(componentId_t componentId);

protected:
	void registerComponent(shared_ptr<MachineComponent> newComponent);
	void removeComponent(componentId_t componentId);

private:
	shared_ptr<Variable> addVariableAtRank(VariableNature_t type, const QString& name, uint rank, const LogicValue& value);
	QList<shared_ptr<Variable>> getRankedVariableList(const QHash<QString, shared_ptr<Variable>>* variableHash, const QHash<QString, uint>* rankHash) const; // TODO: throw exception
	void addVariableToList(shared_ptr<Variable> variable, uint rank, QHash<QString, shared_ptr<Variable>>* variableHash, QHash<QString, uint>* rankHash);
	bool deleteVariableFromList(const QString& name, QHash<QString, shared_ptr<Variable>>* variableHash, QHash<QString, uint>* rankHash);
	bool renameVariableInList(const QString& oldName, const QString& newName, QHash<QString, shared_ptr<Variable>>* variableHash, QHash<QString, uint>* rankHash);
	bool changeRankInList(const QString& name, uint newRank, QHash<QString, shared_ptr<Variable>>* variableHash, QHash<QString, uint>* rankHash);

	QHash<QString, shared_ptr<Variable>> getAllVariablesMap() const;

	bool cleanVariableName(QString& nameToClean) const;

	/////
	// Object variables
private:
	// Store all variables as shared_ptr<Variable> for helper functions,
	// but can actually be shared_ptr<Input/Output/Constant>

	// Mutex required for list edition?
	QHash<QString, shared_ptr<Variable>> inputs;
	QHash<QString, shared_ptr<Variable>> outputs;
	QHash<QString, shared_ptr<Variable>> localVariables;
	QHash<QString, shared_ptr<Variable>> constants;

	QHash<QString, uint> inputsRanks;
	QHash<QString, uint> outputsRanks;
	QHash<QString, uint> localVariablesRanks;
	QHash<QString, uint> constantsRanks;

	QHash<componentId_t, shared_ptr<MachineComponent> > components;

	QString name;

};

#endif // MACHINE_H
