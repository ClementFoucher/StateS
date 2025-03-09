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

// Current class header
#include "machine.h"

// StateS classes
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
				action->checkActionValue();
			}
		}
	}
}

/////
// Mutators

bool Machine::setName(const QString& newName)
{
	QString correctedName = newName.trimmed();

	if (correctedName.length() != 0)
	{
		this->name = correctedName;

		emit this->machineNameChangedEvent();

		return true;
	}
	else
	{
		return false;
	}
}

componentId_t Machine::addVariable(VariableNature_t nature, const QString& name, componentId_t id)
{
	// Make sure the requested name is correct and unique
	auto cleanedName = name;
	this->cleanVariableName(cleanedName);

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
	componentId_t componentId = id;
	if (id != nullId)
	{
		variable = shared_ptr<Variable>(new Variable(id, cleanedName));
	}
	else
	{
		variable = shared_ptr<Variable>(new Variable(cleanedName));
		componentId = variable->getId();
	}

	this->registerComponent(variable);

	// Add variable to the relevant list
	switch(nature)
	{
	case VariableNature_t::input:
		connect(variable.get(), &Variable::variableRenamedEvent,             this, &Machine::machineInputVariableListChangedEvent);
		connect(variable.get(), &Variable::variableResizedEvent,             this, &Machine::machineInputVariableListChangedEvent);
		connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Machine::machineInputVariableListChangedEvent);

		this->inputVariables.append(componentId);

		emit this->machineInputVariableListChangedEvent();

		break;
	case VariableNature_t::output:
		connect(variable.get(), &Variable::variableRenamedEvent,             this, &Machine::machineOutputVariableListChangedEvent);
		connect(variable.get(), &Variable::variableResizedEvent,             this, &Machine::machineOutputVariableListChangedEvent);
		connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Machine::machineOutputVariableListChangedEvent);

		this->outputVariables.append(componentId);

		emit this->machineOutputVariableListChangedEvent();

		break;
	case VariableNature_t::internal:
		connect(variable.get(), &Variable::variableRenamedEvent,             this, &Machine::machineInternalVariableListChangedEvent);
		connect(variable.get(), &Variable::variableResizedEvent,             this, &Machine::machineInternalVariableListChangedEvent);
		connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Machine::machineInternalVariableListChangedEvent);

		this->internalVariables.append(componentId);

		emit this->machineInternalVariableListChangedEvent();

		break;
	case VariableNature_t::constant:
		connect(variable.get(), &Variable::variableRenamedEvent,             this, &Machine::machineConstantListChangedEvent);
		connect(variable.get(), &Variable::variableResizedEvent,             this, &Machine::machineConstantListChangedEvent);
		connect(variable.get(), &Variable::variableInitialValueChangedEvent, this, &Machine::machineConstantListChangedEvent);

		this->constants.append(componentId);

		emit this->machineConstantListChangedEvent();

		break;
	}

	return componentId;
}

void Machine::removeVariable(componentId_t variableId)
{
	auto variable = this->getVariable(variableId);
	if (variable == nullptr) return;


	if (this->inputVariables.contains(variableId))
	{
		this->inputVariables.removeOne(variableId);
		this->removeComponent(variableId);

		emit this->machineInputVariableListChangedEvent();
	}
	else if (this->outputVariables.contains(variableId))
	{
		this->outputVariables.removeOne(variableId);
		this->removeComponent(variableId);

		emit this->machineOutputVariableListChangedEvent();
	}
	else if (this->internalVariables.contains(variableId))
	{
		this->internalVariables.removeOne(variableId);
		this->removeComponent(variableId);

		emit this->machineInternalVariableListChangedEvent();
	}
	else if (this->constants.contains(variableId))
	{
		this->constants.removeOne(variableId);
		this->removeComponent(variableId);

		emit this->machineConstantListChangedEvent();
	}
}

bool Machine::renameVariable(componentId_t variableId, const QString& newName)
{
	auto variable = this->getVariable(variableId);
	if (variable == nullptr) return false;

	// Clean name
	QString cleanedNewName = newName;
	this->cleanVariableName(cleanedNewName);

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

void Machine::changeVariableRank(componentId_t variableId, uint newRank)
{
	if (inputVariables.contains(variableId))
	{
		auto oldRank = this->inputVariables.indexOf(variableId);
		this->inputVariables.move(oldRank, newRank);

		emit this->machineInputVariableListChangedEvent();
	}
	else if (outputVariables.contains(variableId))
	{
		auto oldRank = this->outputVariables.indexOf(variableId);
		this->outputVariables.move(oldRank, newRank);

		emit this->machineOutputVariableListChangedEvent();
	}
	else if (internalVariables.contains(variableId))
	{
		auto oldRank = this->internalVariables.indexOf(variableId);
		this->internalVariables.move(oldRank, newRank);

		emit this->machineInternalVariableListChangedEvent();
	}
	else if (constants.contains(variableId))
	{
		auto oldRank = this->constants.indexOf(variableId);
		this->constants.move(oldRank, newRank);

		emit this->machineConstantListChangedEvent();
	}
}

/////
// Accessors

QString Machine::getName() const
{
	return this->name;
}

shared_ptr<MachineComponent> Machine::getComponent(componentId_t componentId) const
{
	if (this->components.contains(componentId) == false) return nullptr;


	return this->components[componentId];
}

shared_ptr<MachineActuatorComponent> Machine::getActuatorComponent(componentId_t componentId) const
{
	return dynamic_pointer_cast<MachineActuatorComponent>(this->getComponent(componentId));
}

shared_ptr<Variable> Machine::getVariable(componentId_t variableId) const
{
	return dynamic_pointer_cast<Variable>(this->getComponent(variableId));
}

const QList<componentId_t> Machine::getInputVariablesIds() const
{
	return this->inputVariables;
}

const QList<componentId_t> Machine::getOutputVariablesIds() const
{
	return this->outputVariables;
}

const QList<componentId_t> Machine::getInternalVariablesIds() const
{
	return this->internalVariables;
}

const QList<componentId_t> Machine::getConstantsIds() const
{
	return this->constants;
}

const QList<componentId_t> Machine::getVariablesIds(VariableNature_t nature) const
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

const QList<componentId_t> Machine::getWrittableVariablesIds() const
{
	QList<componentId_t> writtableVariablesIds;

	writtableVariablesIds += this->getInternalVariablesIds();
	writtableVariablesIds += this->getOutputVariablesIds();

	return writtableVariablesIds;
}

const QList<componentId_t> Machine::getReadableVariablesIds() const
{
	QList<componentId_t> readableVariablesIds;

	readableVariablesIds += this->getInputVariablesIds();
	readableVariablesIds += this->getInternalVariablesIds();
	readableVariablesIds += this->getConstantsIds();

	return readableVariablesIds;
}

const QList<componentId_t> Machine::getAllVariablesIds() const
{
	QList<componentId_t> allVariablesIds;

	allVariablesIds += this->getInputVariablesIds();
	allVariablesIds += this->getInternalVariablesIds();
	allVariablesIds += this->getOutputVariablesIds();
	allVariablesIds += this->getConstantsIds();

	return allVariablesIds;
}

QString Machine::getUniqueVariableName(const QString& prefix) const
{
	QString baseName = prefix;
	this->cleanVariableName(baseName);

	QString currentName;

	uint i = 0;
	bool nameIsValid = false;

	while (nameIsValid == false)
	{
		currentName = baseName + QString::number(i);

		nameIsValid = true;
		for (auto& variableId : this->getAllVariablesIds())
		{
			auto variable = this->getVariable(variableId);
			if (variable == nullptr) continue;


			if (variable->getName() == currentName)
			{
				nameIsValid = false;
				i++;
				break;
			}
		}
	}

	return currentName;
}

/////
// Protected functions

void Machine::registerComponent(shared_ptr<MachineComponent> newComponent)
{
	this->components[newComponent->getId()] = newComponent;

	connect(newComponent.get(), &MachineComponent::componentEditedEvent,  this, &Machine::componentEditedEvent);
	connect(newComponent.get(), &MachineComponent::componentDeletedEvent, this, &Machine::componentDeletedEvent);
}

void Machine::removeComponent(componentId_t componentId)
{
	this->components.remove(componentId);
}

/////
// Private functions

/**
 * @brief Machine::cleanVariableName
 * @param nameToClean
 * @return
 *  True if name was clean,
 *  False if string has been cleaned.
 */
bool Machine::cleanVariableName(QString& nameToClean) const
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

	if (cleanName != nameToClean)
	{
		nameToClean = cleanName;
		return false;
	}
	else
	{
		return true;
	}
}
