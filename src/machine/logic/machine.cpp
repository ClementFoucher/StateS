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

QList<shared_ptr<Variable> > Machine::getInputsAsSignals() const
{
	return getRankedSignalList(&this->inputs, &this->inputsRanks);
}

QList<shared_ptr<Variable> > Machine::getOutputsAsSignals() const
{
	return getRankedSignalList(&this->outputs, &this->outputsRanks);
}

QList<shared_ptr<Variable> > Machine::getLocalVariables() const
{
	return getRankedSignalList(&this->localVariables, &this->localVariablesRanks);
}

QList<shared_ptr<Variable> > Machine::getConstants() const
{
	return getRankedSignalList(&this->constants, &this->constantsRanks);
}

QList<shared_ptr<Variable>> Machine::getRankedSignalList(const QHash<QString, shared_ptr<Variable>>* signalHash, const QHash<QString, uint>* rankHash) const
{
	QList<shared_ptr<Variable> > rankedList;

	if (signalHash->count() != rankHash->count())
	{
		// Return empty list to signify error
		// TODO: do something better
		return rankedList;
	}

	for (int i = 0 ; i < signalHash->count() ; i++)
	{
		QString signalName = rankHash->key(i);
		rankedList.append((*signalHash)[signalName]);
	}

	return rankedList;
}

// Casted lists obtention

QList<shared_ptr<Input> > Machine::getInputs() const
{
	QList<shared_ptr<Input> > inputSignals;

	if (inputs.count() != inputsRanks.count())
	{
		// Return empty list to signify error
		// TODO: do something better
		return inputSignals;
	}

	for (int i = 0 ; i < this->inputs.count() ; i++)
	{
		QString signalName = this->inputsRanks.key(i);
		inputSignals.append(dynamic_pointer_cast<Input>(inputs[signalName]));
	}

	return inputSignals;
}

QList<shared_ptr<Output> > Machine::getOutputs() const
{
	QList<shared_ptr<Output> > outputSignals;

	if (outputs.count() != outputsRanks.count())
	{
		// Return empty list to signify error
		// TODO: do something better
		return outputSignals;
	}

	for (int i = 0 ; i < this->outputs.count() ; i++)
	{
		QString signalName = this->outputsRanks.key(i);
		outputSignals.append(dynamic_pointer_cast<Output>(outputs[signalName]));
	}

	return outputSignals;
}


// Aggregate lists obtention

QList<shared_ptr<Variable> > Machine::getWrittableSignals() const
{
	QList<shared_ptr<Variable>> writtableVariables;

	writtableVariables += this->getOutputsAsSignals();
	writtableVariables += this->getLocalVariables();

	return writtableVariables;
}

QList<shared_ptr<Variable> > Machine::getReadableSignals() const
{
	QList<shared_ptr<Variable>> readableSignals;

	readableSignals += this->getInputsAsSignals();
	readableSignals += this->getLocalVariables();
	readableSignals += this->getConstants();

	return readableSignals;
}

QList<shared_ptr<Variable> > Machine::getReadableVariableSignals() const
{
	QList<shared_ptr<Variable>> readableVariables;

	readableVariables += this->getInputsAsSignals();
	readableVariables += this->getLocalVariables();

	return readableVariables;
}

QList<shared_ptr<Variable> > Machine::getVariablesSignals() const
{
	QList<shared_ptr<Variable>> allVariables;

	allVariables += this->getInputsAsSignals();
	allVariables += this->getOutputsAsSignals();
	allVariables += this->getLocalVariables();

	return allVariables;
}

QList<shared_ptr<Variable> > Machine::getIoSignals() const
{
	QList<shared_ptr<Variable>> IOs;

	IOs += this->getInputsAsSignals();
	IOs += this->getOutputsAsSignals();

	return IOs;
}

QList<shared_ptr<Variable> > Machine::getAllSignals() const
{
	QList<shared_ptr<Variable>> allSignals;

	allSignals += this->getInputsAsSignals();
	allSignals += this->getOutputsAsSignals();
	allSignals += this->getLocalVariables();
	allSignals += this->getConstants();

	return allSignals;
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

shared_ptr<Variable> Machine::addSignal(VariableNature_t type, const QString& name, const LogicValue& value)
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

	return this->addSignalAtRank(type, name, rank, value);
}

shared_ptr<Variable> Machine::addSignalAtRank(VariableNature_t type, const QString& name, uint rank, const LogicValue& value)
{
	// First check if name doesn't already exist
	for (shared_ptr<Variable> signal : this->getAllSignals())
	{
		if (signal->getName() == name)
			return nullptr;
	}

	// Then check for illegal characters
	QString cleanName = name;
	if (!this->cleanSignalName(cleanName))
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

	// Determine list to reference signal in
	shared_ptr<Variable> signal;
	switch(type)
	{
	case VariableNature_t::input:
		signal = dynamic_pointer_cast<Variable>(shared_ptr<Input>(new Input(name, size))); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->inputs, &this->inputsRanks);

		if (! value.isNull())
		{
			signal->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit this->machineInputListChangedEvent();

		break;
	case VariableNature_t::output:
		signal = dynamic_pointer_cast<Variable>(shared_ptr<Output>(new Output(name, size)));  // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->outputs, &this->outputsRanks);

		emit this->machineOutputListChangedEvent();

		break;
	case VariableNature_t::internal:
		signal = shared_ptr<Variable>(new Variable(name, size)); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->localVariables, &this->localVariablesRanks);

		if (! value.isNull())
		{
			signal->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit this->machineLocalVariableListChangedEvent();

		break;
	case VariableNature_t::constant:
		signal = dynamic_pointer_cast<Variable>(shared_ptr<Constant>(new Constant(name, size))); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->constants, &this->constantsRanks);

		if (! value.isNull())
		{
			signal->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit this->machineConstantListChangedEvent();

		break;
	}

	return signal;
}

void Machine::addSignalToList(shared_ptr<Variable> signal, uint rank, QHash<QString, shared_ptr<Variable>>* signalHash, QHash<QString, uint>* rankHash)
{
	// Fix rank to avoid not-continuous ranks
	uint actualRank = ((int)rank < signalHash->count()) ? rank : signalHash->count();

	// Shift all upper signals ranks, beginning from top
	if ((int)rank < signalHash->count())
	{
		for (int i = signalHash->count()-1 ; i >= (int)actualRank; i--)
		{
			QString signalName = rankHash->key(i);
			(*rankHash)[signalName]++;
		}
	}

	// Add signal to lists
	(*signalHash)[signal->getName()] = signal;
	(*rankHash)[signal->getName()]   = actualRank;
}

bool Machine::deleteSignal(const QString& name)
{
	bool result;

	if (inputs.contains(name))
	{
		this->deleteSignalFromList(name, &this->inputs, &this->inputsRanks);

		emit this->machineInputListChangedEvent();

		result = true;
	}
	else if (outputs.contains(name))
	{
		this->deleteSignalFromList(name, &this->outputs, &this->outputsRanks);

		emit this->machineOutputListChangedEvent();

		result = true;
	}
	else if (localVariables.contains(name))
	{
		this->deleteSignalFromList(name, &this->localVariables, &this->localVariablesRanks);

		emit this->machineLocalVariableListChangedEvent();

		result = true;
	}
	else if (constants.contains(name))
	{
		this->deleteSignalFromList(name, &this->constants, &this->constantsRanks);

		emit this->machineConstantListChangedEvent();

		result = true;
	}
	else
		result = false;

	return result;
}

bool Machine::deleteSignalFromList(const QString& name, QHash<QString, shared_ptr<Variable>>* signalHash, QHash<QString, uint>* rankHash)
{
	if (!signalHash->contains(name))
		return false;

	// Store rank
	uint signalRank = (*rankHash)[name];

	// Remove
	signalHash->remove(name);
	rankHash->remove(name);

	// Shift upper signals
	if ((int)signalRank < signalHash->count())
	{
		for (int i = signalRank+1 ; i <= signalHash->count(); i++)
		{
			QString signalName = rankHash->key(i);
			(*rankHash)[signalName]--;
		}
	}

	// Done
	return true;
}


bool Machine::renameSignal(const QString& oldName, const QString& newName)
{
	QHash<QString, shared_ptr<Variable>> allSignals = getAllSignalsMap();

	QString correctedNewName = newName;
	this->cleanSignalName(correctedNewName);

	if ( !allSignals.contains(oldName) ) // First check if signal exists
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
	else if ( allSignals.contains(correctedNewName) ) // Do not allow rename to existing name
	{
		return false;
	}
	else
	{
		// Update map
		if (inputs.contains(oldName))
		{
			this->renameSignalInList(oldName, correctedNewName, &this->inputs, &this->inputsRanks);

			emit this->machineInputListChangedEvent();
		}
		else if (outputs.contains(oldName))
		{
			this->renameSignalInList(oldName, correctedNewName, &this->outputs, &this->outputsRanks);

			emit this->machineOutputListChangedEvent();
		}
		else if (localVariables.contains(oldName))
		{
			this->renameSignalInList(oldName, correctedNewName, &this->localVariables, &this->localVariablesRanks);

			emit this->machineLocalVariableListChangedEvent();
		}
		else if (constants.contains(oldName))
		{
			this->renameSignalInList(oldName, correctedNewName, &this->constants, &this->constantsRanks);

			emit this->machineConstantListChangedEvent();
		}
		else // Should not happen as we checked all lists
			return false;

		return true;
	}
}

bool Machine::renameSignalInList(const QString& oldName, const QString& newName, QHash<QString, shared_ptr<Variable> > *signalHash, QHash<QString, uint> *rankHash)
{
	if (!signalHash->contains(oldName))
		return false;

	shared_ptr<Variable> itemToRename = (*signalHash)[oldName];
	itemToRename->setName(newName);

	signalHash->remove(oldName);
	(*signalHash)[newName] = itemToRename;

	(*rankHash)[newName] = (*rankHash)[oldName];
	rankHash->remove(oldName);

	return true;
}

void Machine::resizeSignal(const QString &name, uint newSize) // Throws StatesException
{
	QHash<QString, shared_ptr<Variable>> allSignals = getAllSignalsMap();

	if ( !allSignals.contains(name) ) // First check if signal exists
		throw StatesException("Machine", MachineError_t::unknown_variable, "Trying to change initial value of unknown signal");
	else
	{
		allSignals[name]->resize(newSize); // Throws StatesException - propagated

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

void Machine::changeSignalInitialValue(const QString &name, LogicValue newValue) // Throws StatesException
{
	QHash<QString, shared_ptr<Variable>> allSignals = getAllSignalsMap();

	if ( !allSignals.contains(name) ) // First check if signal exists
		throw StatesException("Machine", MachineError_t::unknown_variable, "Trying to change initial value of unknown signal");
	else
	{
		allSignals[name]->setInitialValue(newValue);// Throws StatesException - propagated

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

bool Machine::changeSignalRank(const QString& name, uint newRank)
{
	QHash<QString, shared_ptr<Variable>> allSignals = getAllSignalsMap();

	if ( !allSignals.contains(name) ) // First check if signal exists
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

bool Machine::changeRankInList(const QString& name, uint newRank, QHash<QString, shared_ptr<Variable>>* signalHash, QHash<QString, uint>* rankHash)
{
	if (!signalHash->contains(name))
		return false;
	else if ((int)newRank >= signalHash->count())
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

	// Extract reRanked signal from list (place it at a higher rank than the higest rank)
	(*rankHash)[name] = signalHash->count();

	// Shift other signals
	if (oldRank < newRank)
	{
		for (int i = (int)lowerBound+1 ; i <= (int)upperBound ; i++)
		{
			QString signalName = rankHash->key(i);
			(*rankHash)[signalName]--;
		}
	}
	else
	{
		for (int i = (int)upperBound-1 ; i >= (int)lowerBound ; i--)
		{
			QString signalName = rankHash->key(i);
			(*rankHash)[signalName]++;
		}
	}

	// Reintegrate extracted signal
	(*rankHash)[name] = newRank;

	// Done
	return true;
}

/**
 * @brief Machine::cleanSignalName
 * @param nameToClean
 * @return
 *  True if name was clean,
 *  False if string has been cleaned.
 */
bool Machine::cleanSignalName(QString& nameToClean) const
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

QString Machine::getUniqueSignalName(const QString& prefix) const
{
	QString baseName = prefix;
	this->cleanSignalName(baseName);

	QString currentName;

	uint i = 0;
	bool nameIsValid = false;

	while (!nameIsValid)
	{
		currentName = baseName + QString::number(i);

		nameIsValid = true;
		for (shared_ptr<Variable> colleage : this->getAllSignals())
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

QHash<QString, shared_ptr<Variable> > Machine::getAllSignalsMap() const
{
	QHash<QString, shared_ptr<Variable>> allSignals;

	for (shared_ptr<Variable> signal : this->getAllSignals())
	{
		allSignals[signal->getName()] = signal;
	}

	return allSignals;
}
