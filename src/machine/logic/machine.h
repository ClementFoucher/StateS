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
class Variable;
class MachineComponent;
class MachineActuatorComponent;


class Machine : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Machine();

	// Pseudo-constructor to process post-loading actions
	virtual void finalizeLoading();

	/////
	// Object functions
public:

	///
	// Mutators

	bool setName(const QString& newName);

	componentId_t addVariable(VariableNature_t nature, const QString& name, componentId_t id = nullId);
	void removeVariable(componentId_t variableId);
	bool renameVariable(componentId_t variableId, const QString& newName);
	void changeVariableRank(componentId_t variableId, uint newRank);

	///
	// Accessors

	QString getName() const;

	shared_ptr<MachineComponent> getComponent(componentId_t componentId) const;
	shared_ptr<MachineActuatorComponent> getActuatorComponent(componentId_t componentId) const;

	shared_ptr<Variable> getVariable(componentId_t variableId) const;

	// Ordered lists for each nature of variable
	const QList<componentId_t> getInputVariablesIds()    const;
	const QList<componentId_t> getOutputVariablesIds()   const;
	const QList<componentId_t> getInternalVariablesIds() const;
	const QList<componentId_t> getConstantsIds()         const;
	const QList<componentId_t> getVariablesIds(VariableNature_t nature) const;

	// Unordered lists for categories of variables
	const QList<componentId_t> getWrittableVariablesIds() const;
	const QList<componentId_t> getReadableVariablesIds()  const;
	const QList<componentId_t> getAllVariablesIds()       const;

	// Single ID using rank
	componentId_t getVariableId(VariableNature_t nature, uint rank) const;

protected:
	void registerComponent(shared_ptr<MachineComponent> newComponent);
	void removeComponent(componentId_t componentId);

	void cleanName(QString& nameToClean) const;

	/////
	// Signals
signals:
	// Machine changes
	void machineNameChangedEvent();
	void machineExternalViewChangedEvent();

	// Components changes
	void componentEditedEvent(componentId_t componentId);
	void componentDeletedEvent(componentId_t componentId);

	/////
	// Object variables
private:
	QString name;

	QHash<componentId_t, shared_ptr<MachineComponent>> components;

	QList<componentId_t> inputVariables;
	QList<componentId_t> outputVariables;
	QList<componentId_t> internalVariables;
	QList<componentId_t> constants;

};

#endif // MACHINE_H
