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
#include "input.h"
#include "output.h"
#include "constant.h"
#include "machinecomponent.h"
#include "statesexception.h"
#include "exceptiontypes.h"


Machine::Machine()
{
	this->name = tr("Machine");
}

Machine::~Machine()
{
	// Force cleaning order

	this->components.clear();

	this->inputsRanks.clear();
	this->outputsRanks.clear();
	this->localVariablesRanks.clear();
	this->constantsRanks.clear();

	this->inputs.clear();
	this->outputs.clear();
	this->localVariables.clear();
	this->constants.clear();
}

QString Machine::getName() const
{
	return this->name;
}

// General lists obtention

QList<shared_ptr<Variable> > Machine::getInputsAsVariables() const
{
	return getRankedVariableList(&this->inputs, &this->inputsRanks);
}

QList<shared_ptr<Variable> > Machine::getOutputsAsVariables() const
{
	return getRankedVariableList(&this->outputs, &this->outputsRanks);
}

QList<shared_ptr<Variable> > Machine::getInternalVariables() const
{
	return getRankedVariableList(&this->localVariables, &this->localVariablesRanks);
}

QList<shared_ptr<Variable> > Machine::getConstants() const
{
	return getRankedVariableList(&this->constants, &this->constantsRanks);
}

QList<shared_ptr<Variable>> Machine::getRankedVariableList(const QHash<QString, shared_ptr<Variable>>* variableHash, const QHash<QString, uint>* rankHash) const
{
	QList<shared_ptr<Variable> > rankedList;

	if (variableHash->count() != rankHash->count())
	{
		// Return empty list to signify error
		// TODO: do something better
		return rankedList;
	}

	for (int i = 0 ; i < variableHash->count() ; i++)
	{
		QString variableName = rankHash->key(i);
		rankedList.append((*variableHash)[variableName]);
	}

	return rankedList;
}

// Casted lists obtention

QList<shared_ptr<Input> > Machine::getInputs() const
{
	QList<shared_ptr<Input> > inputVariables;

	if (inputs.count() != inputsRanks.count())
	{
		// Return empty list to signify error
		// TODO: do something better
		return inputVariables;
	}

	for (int i = 0 ; i < this->inputs.count() ; i++)
	{
		QString variableName = this->inputsRanks.key(i);
		inputVariables.append(dynamic_pointer_cast<Input>(inputs[variableName]));
	}

	return inputVariables;
}

QList<shared_ptr<Output> > Machine::getOutputs() const
{
	QList<shared_ptr<Output> > outputVariables;

	if (outputs.count() != outputsRanks.count())
	{
		// Return empty list to signify error
		// TODO: do something better
		return outputVariables;
	}

	for (int i = 0 ; i < this->outputs.count() ; i++)
	{
		QString variableName = this->outputsRanks.key(i);
		outputVariables.append(dynamic_pointer_cast<Output>(outputs[variableName]));
	}

	return outputVariables;
}


// Aggregate lists obtention

QList<shared_ptr<Variable> > Machine::getWrittableVariables() const
{
	QList<shared_ptr<Variable>> writtableVariables;

	writtableVariables += this->getOutputsAsVariables();
	writtableVariables += this->getInternalVariables();

	return writtableVariables;
}

QList<shared_ptr<Variable> > Machine::getReadableVariables() const
{
	QList<shared_ptr<Variable>> readableVariables;

	readableVariables += this->getInputsAsVariables();
	readableVariables += this->getInternalVariables();
	readableVariables += this->getConstants();

	return readableVariables;
}

QList<shared_ptr<Variable> > Machine::getReadableVariableVariables() const
{
	QList<shared_ptr<Variable>> readableVariables;

	readableVariables += this->getInputsAsVariables();
	readableVariables += this->getInternalVariables();

	return readableVariables;
}

QList<shared_ptr<Variable> > Machine::getAllVariables() const
{
	QList<shared_ptr<Variable>> allVariables;

	allVariables += this->getInputsAsVariables();
	allVariables += this->getOutputsAsVariables();
	allVariables += this->getInternalVariables();
	allVariables += this->getConstants();

	return allVariables;
}

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

shared_ptr<Variable> Machine::addVariable(VariableNature_t type, const QString& name, const LogicValue& value)
{
	uint rank;

	switch(type)
	{
	case VariableNature_t::input:
		rank = this->inputs.count();
		break;
	case VariableNature_t::output:
		rank = this->outputs.count();
		break;
	case VariableNature_t::internal:
		rank = this->localVariables.count();
		break;
	case VariableNature_t::constant:
		rank = this->constants.count();
		break;
	default:
		return nullptr;
	}

	return this->addVariableAtRank(type, name, rank, value);
}

shared_ptr<Variable> Machine::addVariableAtRank(VariableNature_t type, const QString& name, uint rank, const LogicValue& value)
{
	// First check if name doesn't already exist
	for (shared_ptr<Variable> variable : this->getAllVariables())
	{
		if (variable->getName() == name)
			return nullptr;
	}

	// Then check for illegal characters
	QString cleanName = name;
	if (!this->cleanVariableName(cleanName))
		return nullptr;

	// Determine size
	uint size;
	if (! value.isNull())
	{
		size = value.getSize();
	}
	else
	{
		size = 1;
	}

	// Determine list to reference variable in
	shared_ptr<Variable> variable;
	switch(type)
	{
	case VariableNature_t::input:
		variable = dynamic_pointer_cast<Variable>(shared_ptr<Input>(new Input(name, size))); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addVariableToList(variable, rank, &this->inputs, &this->inputsRanks);

		if (! value.isNull())
		{
			variable->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit this->machineInputListChangedEvent();

		break;
	case VariableNature_t::output:
		variable = dynamic_pointer_cast<Variable>(shared_ptr<Output>(new Output(name, size)));  // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addVariableToList(variable, rank, &this->outputs, &this->outputsRanks);

		emit this->machineOutputListChangedEvent();

		break;
	case VariableNature_t::internal:
		variable = shared_ptr<Variable>(new Variable(name, size)); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addVariableToList(variable, rank, &this->localVariables, &this->localVariablesRanks);

		if (! value.isNull())
		{
			variable->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit this->machineLocalVariableListChangedEvent();

		break;
	case VariableNature_t::constant:
		variable = dynamic_pointer_cast<Variable>(shared_ptr<Constant>(new Constant(name, size))); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addVariableToList(variable, rank, &this->constants, &this->constantsRanks);

		if (! value.isNull())
		{
			variable->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit this->machineConstantListChangedEvent();

		break;
	}

	return variable;
}

void Machine::addVariableToList(shared_ptr<Variable> variable, uint rank, QHash<QString, shared_ptr<Variable>>* variableHash, QHash<QString, uint>* rankHash)
{
	// Fix rank to avoid not-continuous ranks
	uint actualRank = ((int)rank < variableHash->count()) ? rank : variableHash->count();

	// Shift all upper variables ranks, beginning from top
	if ((int)rank < variableHash->count())
	{
		for (int i = variableHash->count()-1 ; i >= (int)actualRank; i--)
		{
			QString variableName = rankHash->key(i);
			(*rankHash)[variableName]++;
		}
	}

	// Add variable to lists
	(*variableHash)[variable->getName()] = variable;
	(*rankHash)[variable->getName()]   = actualRank;
}

bool Machine::deleteVariable(const QString& name)
{
	bool result;

	if (inputs.contains(name))
	{
		this->deleteVariableFromList(name, &this->inputs, &this->inputsRanks);

		emit this->machineInputListChangedEvent();

		result = true;
	}
	else if (outputs.contains(name))
	{
		this->deleteVariableFromList(name, &this->outputs, &this->outputsRanks);

		emit this->machineOutputListChangedEvent();

		result = true;
	}
	else if (localVariables.contains(name))
	{
		this->deleteVariableFromList(name, &this->localVariables, &this->localVariablesRanks);

		emit this->machineLocalVariableListChangedEvent();

		result = true;
	}
	else if (constants.contains(name))
	{
		this->deleteVariableFromList(name, &this->constants, &this->constantsRanks);

		emit this->machineConstantListChangedEvent();

		result = true;
	}
	else
		result = false;

	return result;
}

bool Machine::deleteVariableFromList(const QString& name, QHash<QString, shared_ptr<Variable>>* variableHash, QHash<QString, uint>* rankHash)
{
	if (!variableHash->contains(name))
		return false;

	// Store rank
	uint variableRank = (*rankHash)[name];

	// Remove
	variableHash->remove(name);
	rankHash->remove(name);

	// Shift upper variables
	if ((int)variableRank < variableHash->count())
	{
		for (int i = variableRank+1 ; i <= variableHash->count(); i++)
		{
			QString variableName = rankHash->key(i);
			(*rankHash)[variableName]--;
		}
	}

	// Done
	return true;
}


bool Machine::renameVariable(const QString& oldName, const QString& newName)
{
	QHash<QString, shared_ptr<Variable>> allVariables = getAllVariablesMap();

	QString correctedNewName = newName;
	this->cleanVariableName(correctedNewName);

	if ( !allVariables.contains(oldName) ) // First check if variable exists
		return false;
	else if (oldName == newName) // Rename to same name is always success
		return true;
	else if (oldName == correctedNewName)
	{
		// Just to update any text with false new name...
		// A little bit heavy
		if (inputs.contains(oldName))
		{
			emit this->machineInputListChangedEvent();
		}
		else if (outputs.contains(oldName))
		{
			emit this->machineOutputListChangedEvent();
		}
		else if (localVariables.contains(oldName))
		{
			emit this->machineLocalVariableListChangedEvent();
		}
		else if (constants.contains(oldName))
		{
			emit this->machineConstantListChangedEvent();
		}

		return true;
	}
	else if ( allVariables.contains(correctedNewName) ) // Do not allow rename to existing name
	{
		return false;
	}
	else
	{
		// Update map
		if (inputs.contains(oldName))
		{
			this->renameVariableInList(oldName, correctedNewName, &this->inputs, &this->inputsRanks);

			emit this->machineInputListChangedEvent();
		}
		else if (outputs.contains(oldName))
		{
			this->renameVariableInList(oldName, correctedNewName, &this->outputs, &this->outputsRanks);

			emit this->machineOutputListChangedEvent();
		}
		else if (localVariables.contains(oldName))
		{
			this->renameVariableInList(oldName, correctedNewName, &this->localVariables, &this->localVariablesRanks);

			emit this->machineLocalVariableListChangedEvent();
		}
		else if (constants.contains(oldName))
		{
			this->renameVariableInList(oldName, correctedNewName, &this->constants, &this->constantsRanks);

			emit this->machineConstantListChangedEvent();
		}
		else // Should not happen as we checked all lists
			return false;

		return true;
	}
}

bool Machine::renameVariableInList(const QString& oldName, const QString& newName, QHash<QString, shared_ptr<Variable> > *variableHash, QHash<QString, uint> *rankHash)
{
	if (!variableHash->contains(oldName))
		return false;

	shared_ptr<Variable> itemToRename = (*variableHash)[oldName];
	itemToRename->setName(newName);

	variableHash->remove(oldName);
	(*variableHash)[newName] = itemToRename;

	(*rankHash)[newName] = (*rankHash)[oldName];
	rankHash->remove(oldName);

	return true;
}

void Machine::resizeVariable(const QString &name, uint newSize) // Throws StatesException
{
	QHash<QString, shared_ptr<Variable>> allVariable = getAllVariablesMap();

	if ( !allVariable.contains(name) ) // First check if variable exists
		throw StatesException("Machine", MachineError_t::unknown_variable, "Trying to change initial value of unknown variable");
	else
	{
		allVariable[name]->resize(newSize); // Throws StatesException - propagated

		if (inputs.contains(name))
		{
			emit this->machineInputListChangedEvent();
		}
		else if (outputs.contains(name))
		{
			emit this->machineOutputListChangedEvent();
		}
		else if (localVariables.contains(name))
		{
			emit this->machineLocalVariableListChangedEvent();
		}
		else if (constants.contains(name))
		{
			emit this->machineConstantListChangedEvent();
		}
		else // Should not happen as we checked all lists
		{
			throw StatesException("Machine", MachineError_t::impossible_error, "Unable to emit listChangedEvent");
		}
	}
}

void Machine::changeVariableInitialValue(const QString &name, LogicValue newValue) // Throws StatesException
{
	QHash<QString, shared_ptr<Variable>> allVariable = getAllVariablesMap();

	if ( !allVariable.contains(name) ) // First check if variable exists
		throw StatesException("Machine", MachineError_t::unknown_variable, "Trying to change initial value of unknown variable");
	else
	{
		allVariable[name]->setInitialValue(newValue);// Throws StatesException - propagated

		if (inputs.contains(name))
		{
			emit this->machineInputListChangedEvent();
		}
		else if (outputs.contains(name))
		{
			emit this->machineOutputListChangedEvent();
		}
		else if (localVariables.contains(name))
		{
			emit this->machineLocalVariableListChangedEvent();
		}
		else if (constants.contains(name))
		{
			emit this->machineConstantListChangedEvent();
		}
		else // Should not happen as we checked all lists
			throw StatesException("Machine", MachineError_t::impossible_error, "Unable to emit listChangedEvent");
	}
}

bool Machine::changeVariableRank(const QString& name, uint newRank)
{
	QHash<QString, shared_ptr<Variable>> allVariables = getAllVariablesMap();

	if ( !allVariables.contains(name) ) // First check if variable exists
	{
		return false;
	}
	else
	{
		if (inputs.contains(name))
		{
			this->changeRankInList(name, newRank, &this->inputs, &this->inputsRanks);

			emit this->machineInputListChangedEvent();
		}
		else if (outputs.contains(name))
		{
			this->changeRankInList(name, newRank, &this->outputs, &this->outputsRanks);

			emit this->machineOutputListChangedEvent();
		}
		else if (localVariables.contains(name))
		{
			this->changeRankInList(name, newRank, &this->localVariables, &this->localVariablesRanks);

			emit this->machineLocalVariableListChangedEvent();
		}
		else if (constants.contains(name))
		{
			this->changeRankInList(name, newRank, &this->constants, &this->constantsRanks);

			emit this->machineConstantListChangedEvent();
		}
		else // Should not happen as we checked all lists
			return false;

		return true;
	}
}

bool Machine::changeRankInList(const QString& name, uint newRank, QHash<QString, shared_ptr<Variable>>* variableHash, QHash<QString, uint>* rankHash)
{
	if (!variableHash->contains(name))
		return false;
	else if ((int)newRank >= variableHash->count())
		return false;

	// Store rank
	uint oldRank = (*rankHash)[name];

	if (oldRank == newRank)
		return true;

	// Algorithm is a as follows:
	// The list is divided in 3 parts:
	// - The lower part, before low(oldRank, newRank) is untouched
	// - The middle part is all messed up
	// - The upper part, after high(oldRank, newRank) is untouched
	// We call lower bound and upper bound the boundaries of the middle part (with boudaries belonging to middle part)
	uint lowerBound = min(oldRank, newRank);
	uint upperBound = max(oldRank, newRank);

	// Extract reRanked variable from list (place it at a higher rank than the higest rank)
	(*rankHash)[name] = variableHash->count();

	// Shift other variables
	if (oldRank < newRank)
	{
		for (int i = (int)lowerBound+1 ; i <= (int)upperBound ; i++)
		{
			QString variableName = rankHash->key(i);
			(*rankHash)[variableName]--;
		}
	}
	else
	{
		for (int i = (int)upperBound-1 ; i >= (int)lowerBound ; i--)
		{
			QString variableName = rankHash->key(i);
			(*rankHash)[variableName]++;
		}
	}

	// Reintegrate extracted variable
	(*rankHash)[name] = newRank;

	// Done
	return true;
}

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

	for (QChar c : nameBeingCleaned)
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
		return true;
}

QString Machine::getUniqueVariableName(const QString& prefix) const
{
	QString baseName = prefix;
	this->cleanVariableName(baseName);

	QString currentName;

	uint i = 0;
	bool nameIsValid = false;

	while (!nameIsValid)
	{
		currentName = baseName + QString::number(i);

		nameIsValid = true;
		for (shared_ptr<Variable> colleage : this->getAllVariables())
		{
			if (colleage->getName() == currentName)
			{
				nameIsValid = false;
				i++;
				break;
			}
		}
	}

	return currentName;
}

void Machine::registerComponent(shared_ptr<MachineComponent> newComponent)
{
	this->components[newComponent->getId()] = newComponent;

	connect(newComponent.get(), &MachineComponent::componentNeedsGraphicUpdateEvent, this, &Machine::graphicComponentNeedsRefreshEvent);
}

void Machine::removeComponent(componentId_t componentId)
{
	this->components.remove(componentId);

	emit this->componentDeletedEvent(componentId);
}

shared_ptr<MachineComponent> Machine::getComponent(componentId_t componentId) const
{
	if (this->components.contains(componentId))
	{
		return this->components[componentId];
	}
	else
	{
		return nullptr;
	}
}

QHash<QString, shared_ptr<Variable> > Machine::getAllVariablesMap() const
{
	QHash<QString, shared_ptr<Variable>> allVariables;

	for (shared_ptr<Variable> variable : this->getAllVariables())
	{
		allVariables[variable->getName()] = variable;
	}

	return allVariables;
}
