/*
 * Copyright © 2014-2026 Clément Foucher
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

// Stdlib
#include <memory>

// Qt
#include <QHash>

// StateS
#include "componentid.h"
#include "machinevalue.h"
class Variable;
class MachineComponent;
class MachineActuatorComponent;


class Machine : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class VariableNature_t { input, output, internal, constant };

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

	ComponentId addVariable(VariableNature_t nature, const QString& name, MachineValue::Type_t type, ComponentId id = nullId);
	void removeVariable(ComponentId variableId);
	bool renameVariable(ComponentId variableId, const QString& newName);
	void changeVariableRank(ComponentId variableId, uint newRank);

	///
	// Accessors

	QString getName() const;

	std::shared_ptr<MachineComponent> getComponent(ComponentId componentId) const;
	std::shared_ptr<MachineActuatorComponent> getActuatorComponent(ComponentId componentId) const;

	std::shared_ptr<Variable> getVariable(ComponentId variableId) const;

	// Ordered lists for each nature of variable
	const QList<ComponentId> getInputVariablesIds()    const;
	const QList<ComponentId> getOutputVariablesIds()   const;
	const QList<ComponentId> getInternalVariablesIds() const;
	const QList<ComponentId> getConstantsIds()         const;
	const QList<ComponentId> getVariablesIds(VariableNature_t nature) const;

	// Unordered lists for categories of variables
	const QList<ComponentId> getWrittableVariablesIds() const;
	const QList<ComponentId> getReadableVariablesIds()  const;
	const QList<ComponentId> getAllVariablesIds()       const;

	// Single ID using rank
	ComponentId getVariableId(VariableNature_t nature, uint rank) const;

protected:
	void registerComponent(std::shared_ptr<MachineComponent> newComponent);
	void removeComponent(ComponentId componentId);

	void cleanName(QString& nameToClean) const;

	/////
	// Signals
signals:
	// Machine changes
	void machineNameChangedEvent();
	void machineExternalViewChangedEvent();

	// Components changes
	void componentEditedEvent(ComponentId componentId);
	void componentDeletedEvent(ComponentId componentId);

	/////
	// Object variables
private:
	QString name;

	QHash<ComponentId, std::shared_ptr<MachineComponent>> components;

	QList<ComponentId> inputVariables;
	QList<ComponentId> outputVariables;
	QList<ComponentId> internalVariables;
	QList<ComponentId> constants;

};

#endif // MACHINE_H
