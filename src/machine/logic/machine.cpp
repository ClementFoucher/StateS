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

// Current class header
#include "machine.h"

// StateS
#include "variable.h"
#include "machinecomponent.h"
#include "machineactuatorcomponent.h"
#include "actiononvariable.h"


/////
// Constructors/destructors

Machine::Machine()
{
	this->name = tr("Machine");
}

void Machine::finalizeLoading()
{
	// When machine has just been loaded, actions are raw and can
	// even be erroneous if there was an issue in the save file
	for (auto& component : this->components)
	{
		auto machineActuator = dynamic_pointer_cast<MachineActuatorComponent>(component);
		if (machineActuator != nullptr)
		{
			for (auto& action : machineActuator->getActions())
			{
				action->checkAndFixAction();
			}
		}
	}
}

/////
// Mutators

bool Machine::setName(const QString& newName)
{
	// Clean name
	auto cleanedName = newName;
	this->cleanName(cleanedName);

	// Make sure there is actually a name
	if (cleanedName.isEmpty() == true) return false;

	// If new name is identical to current name, nothing to do
	if (this->name == cleanedName) return true;


	// Set new name
	this->name = cleanedName;

	emit this->machineNameChangedEvent();
	emit this->machineExternalViewChangedEvent();

	return true;
}

ComponentId Machine::addVariable(VariableNature_t nature, const QString& name, MachineValue::Type_t type, ComponentId id)
{
	if (type == MachineValue::Type_t::nullType) return nullId;

	// First clean name
	auto cleanedName = name;
	this->cleanName(cleanedName);

	// Make sure there is actually a name
	if (cleanedName.isEmpty() == true) return nullId;

	// Then check that name doesn't already exist
	for (auto existingVariableId : this->getAllVariablesIds())
	{
		auto existingVariable = this->getVariable(existingVariableId);
		if (existingVariable == nullptr) continue;


		if (existingVariable->getName() == cleanedName)
		{
			return nullId;
		}
	}


	// Create variable
	shared_ptr<Variable> variable;
	auto componentId = id;
	if (id != nullId)
	{
		variable = make_shared<Variable>(id, cleanedName, type);
	}
	else
	{
		variable = make_shared<Variable>(cleanedName, type);
		componentId = variable->getId();
	}

	this->registerComponent(variable);

	// Add variable to the relevant list
	switch(nature)
	{
	case VariableNature_t::input:
		connect(variable.get(), &Variable::variableRenamedEvent,             this, &Machine::machineExternalViewChangedEvent);
		connect(variable.get(), &Variable::variableTypeChangedEvent,         this, &Machine::machineExternalViewChangedEvent);
		connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Machine::machineExternalViewChangedEvent);

		this->inputVariables.append(componentId);

		emit this->machineExternalViewChangedEvent();

		break;
	case VariableNature_t::output:
		connect(variable.get(), &Variable::variableRenamedEvent,             this, &Machine::machineExternalViewChangedEvent);
		connect(variable.get(), &Variable::variableTypeChangedEvent,         this, &Machine::machineExternalViewChangedEvent);
		connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Machine::machineExternalViewChangedEvent);

		this->outputVariables.append(componentId);

		emit this->machineExternalViewChangedEvent();

		break;
	case VariableNature_t::internal:
		this->internalVariables.append(componentId);
		break;
	case VariableNature_t::constant:
		this->constants.append(componentId);
		break;
	}

	return componentId;
}

void Machine::removeVariable(ComponentId variableId)
{
	auto variable = this->getVariable(variableId);
	if (variable == nullptr) return;


	if (this->inputVariables.contains(variableId))
	{
		this->inputVariables.removeOne(variableId);
		this->removeComponent(variableId);

		emit this->machineExternalViewChangedEvent();
	}
	else if (this->outputVariables.contains(variableId))
	{
		this->outputVariables.removeOne(variableId);
		this->removeComponent(variableId);

		emit this->machineExternalViewChangedEvent();
	}
	else if (this->internalVariables.contains(variableId))
	{
		this->internalVariables.removeOne(variableId);
		this->removeComponent(variableId);
	}
	else if (this->constants.contains(variableId))
	{
		this->constants.removeOne(variableId);
		this->removeComponent(variableId);
	}
}

bool Machine::renameVariable(ComponentId variableId, const QString& newName)
{
	auto variable = this->getVariable(variableId);
	if (variable == nullptr) return false;

	// Clean name
	QString cleanedNewName = newName;
	this->cleanName(cleanedNewName);

	// Make sure name is not empty
	if (cleanedNewName.isEmpty()) return false;

	// Rename to same name is always success
	QString oldName = variable->getName();
	if (oldName == cleanedNewName) return true;

	// Check if no other variable already has new name
	auto variablesIds = this->getAllVariablesIds();
	for (auto variableId : variablesIds)
	{
		auto variable = this->getVariable(variableId);
		if (variable == nullptr) continue;


		if (variable->getName() == cleanedNewName)
		{
			return false;
		}
	}


	// Do rename
	variable->setName(cleanedNewName);

	return true;
}

void Machine::changeVariableRank(ComponentId variableId, uint newRank)
{
	if (inputVariables.contains(variableId))
	{
		auto oldRank = this->inputVariables.indexOf(variableId);
		this->inputVariables.move(oldRank, newRank);

		emit this->machineExternalViewChangedEvent();
	}
	else if (outputVariables.contains(variableId))
	{
		auto oldRank = this->outputVariables.indexOf(variableId);
		this->outputVariables.move(oldRank, newRank);

		emit this->machineExternalViewChangedEvent();
	}
	else if (internalVariables.contains(variableId))
	{
		auto oldRank = this->internalVariables.indexOf(variableId);
		this->internalVariables.move(oldRank, newRank);
	}
	else if (constants.contains(variableId))
	{
		auto oldRank = this->constants.indexOf(variableId);
		this->constants.move(oldRank, newRank);
	}
}

/////
// Accessors

QString Machine::getName() const
{
	return this->name;
}

shared_ptr<MachineComponent> Machine::getComponent(ComponentId componentId) const
{
	if (this->components.contains(componentId) == false) return nullptr;


	return this->components[componentId];
}

shared_ptr<MachineActuatorComponent> Machine::getActuatorComponent(ComponentId componentId) const
{
	return dynamic_pointer_cast<MachineActuatorComponent>(this->getComponent(componentId));
}

shared_ptr<Variable> Machine::getVariable(ComponentId variableId) const
{
	return dynamic_pointer_cast<Variable>(this->getComponent(variableId));
}

const QList<ComponentId> Machine::getInputVariablesIds() const
{
	return this->inputVariables;
}

const QList<ComponentId> Machine::getOutputVariablesIds() const
{
	return this->outputVariables;
}

const QList<ComponentId> Machine::getInternalVariablesIds() const
{
	return this->internalVariables;
}

const QList<ComponentId> Machine::getConstantsIds() const
{
	return this->constants;
}

const QList<ComponentId> Machine::getVariablesIds(VariableNature_t nature) const
{
	switch (nature)
	{
	case VariableNature_t::input:
		return this->inputVariables;
		break;
	case VariableNature_t::internal:
		return this->internalVariables;
		break;
	case VariableNature_t::output:
		return this->outputVariables;
		break;
	case VariableNature_t::constant:
		return this->constants;
		break;
	}
}

const QList<ComponentId> Machine::getWrittableVariablesIds() const
{
	QList<ComponentId> writtableVariablesIds;

	writtableVariablesIds += this->getInternalVariablesIds();
	writtableVariablesIds += this->getOutputVariablesIds();

	return writtableVariablesIds;
}

const QList<ComponentId> Machine::getReadableVariablesIds() const
{
	QList<ComponentId> readableVariablesIds;

	readableVariablesIds += this->getInputVariablesIds();
	readableVariablesIds += this->getInternalVariablesIds();
	readableVariablesIds += this->getConstantsIds();

	return readableVariablesIds;
}

const QList<ComponentId> Machine::getAllVariablesIds() const
{
	QList<ComponentId> allVariablesIds;

	allVariablesIds += this->getInputVariablesIds();
	allVariablesIds += this->getInternalVariablesIds();
	allVariablesIds += this->getOutputVariablesIds();
	allVariablesIds += this->getConstantsIds();

	return allVariablesIds;
}

ComponentId Machine::getVariableId(VariableNature_t nature, uint rank) const
{
	auto variablesIds = this->getVariablesIds(nature);

	if (rank >= variablesIds.count()) return nullId;


	return variablesIds.at(rank);
}

/////
// Protected functions

void Machine::registerComponent(shared_ptr<MachineComponent> newComponent)
{
	this->components[newComponent->getId()] = newComponent;

	connect(newComponent.get(), &MachineComponent::componentEditedEvent,  this, &Machine::componentEditedEvent);
	connect(newComponent.get(), &MachineComponent::componentDeletedEvent, this, &Machine::componentDeletedEvent);
}

void Machine::removeComponent(ComponentId componentId)
{
	this->components.remove(componentId);
}

void Machine::cleanName(QString& nameToClean) const
{
	QString nameBeingCleaned = nameToClean.trimmed();
	QString cleanName;

	for (QChar c : as_const(nameBeingCleaned))
	{
		if ( ( (c.isLetterOrNumber()) ) ||
		     ( (c == '_')             ) ||
		     ( (c == '#')             ) ||
		     ( (c == '@')             ) ||
		     ( (c == '-')             ) ||
		     ( (c.isSpace() )         )
		     )
		{
			cleanName += c;
		}
	}

	nameToClean = cleanName;
}
