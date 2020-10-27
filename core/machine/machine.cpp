/*
 * Copyright © 2014-2020 Clément Foucher
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

// Qt classes
#include <QGraphicsItem>

// StateS classes
#include "input.h"
#include "output.h"
#include "machinebuilder.h"
#include "constant.h"
#include "statesexception.h"
#include "machineundocommand.h"
#include "machinestatus.h"


Machine::Machine(shared_ptr<MachineStatus> machineStatus)
{
	this->machineStatus  = machineStatus;
	this->machineBuilder = shared_ptr<MachineBuilder>(new MachineBuilder());
	this->name = tr("Machine");

	this->rebuildComponentVisualization();
}

Machine::Machine() :
    Machine(shared_ptr<MachineStatus> (new MachineStatus()))
{

}

Machine::~Machine()
{
	this->isBeingDestroyed = true;

	// Force cleaning order
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

shared_ptr<MachineStatus> Machine::getMachineStatus() const
{
	return this->machineStatus;
}

// General lists obtention

QList<shared_ptr<Signal> > Machine::getInputsAsSignals() const
{
	return getRankedSignalList(&this->inputs, &this->inputsRanks);
}

QList<shared_ptr<Signal> > Machine::getOutputsAsSignals() const
{
	return getRankedSignalList(&this->outputs, &this->outputsRanks);
}

QList<shared_ptr<Signal> > Machine::getLocalVariables() const
{
	return getRankedSignalList(&this->localVariables, &this->localVariablesRanks);
}

QList<shared_ptr<Signal> > Machine::getConstants() const
{
	return getRankedSignalList(&this->constants, &this->constantsRanks);
}

QList<shared_ptr<Signal>> Machine::getRankedSignalList(const QHash<QString, shared_ptr<Signal>>* signalHash, const QHash<QString, uint>* rankHash) const
{
	QList<shared_ptr<Signal> > rankedList;

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

QList<shared_ptr<Signal> > Machine::getWrittableSignals() const
{
	QList<shared_ptr<Signal>> writtableVariables;

	writtableVariables += this->getOutputsAsSignals();
	writtableVariables += this->getLocalVariables();

	return writtableVariables;
}

QList<shared_ptr<Signal> > Machine::getReadableSignals() const
{
	QList<shared_ptr<Signal>> readableSignals;

	readableSignals += this->getInputsAsSignals();
	readableSignals += this->getLocalVariables();
	readableSignals += this->getConstants();

	return readableSignals;
}

QList<shared_ptr<Signal> > Machine::getReadableVariableSignals() const
{
	QList<shared_ptr<Signal>> readableVariables;

	readableVariables += this->getInputsAsSignals();
	readableVariables += this->getLocalVariables();

	return readableVariables;
}

QList<shared_ptr<Signal> > Machine::getVariablesSignals() const
{
	QList<shared_ptr<Signal>> allVariables;

	allVariables += this->getInputsAsSignals();
	allVariables += this->getOutputsAsSignals();
	allVariables += this->getLocalVariables();

	return allVariables;
}

QList<shared_ptr<Signal> > Machine::getIoSignals() const
{
	QList<shared_ptr<Signal>> IOs;

	IOs += this->getInputsAsSignals();
	IOs += this->getOutputsAsSignals();

	return IOs;
}

QList<shared_ptr<Signal> > Machine::getAllSignals() const
{
	QList<shared_ptr<Signal>> allSignals;

	allSignals += this->getInputsAsSignals();
	allSignals += this->getOutputsAsSignals();
	allSignals += this->getLocalVariables();
	allSignals += this->getConstants();

	return allSignals;
}

void Machine::setSimulationMode(Machine::simulation_mode newMode)
{
	this->currentMode = newMode;
	emit simulationModeChangedEvent(newMode);
}

void Machine::emitMachineEditedWithoutUndoCommand(MachineUndoCommand::undo_command_id commandId)
{
	if ( (this->isBeingDestroyed == false) && (this->eventInhibitionLevel == 0) && (this->atomicEditionOngoing == false) )
	{
		emit machineEditedWithoutUndoCommandGeneratedEvent(commandId);
	}
}

void Machine::emitMachineEditedWithUndoCommand(MachineUndoCommand* undoCommand)
{
	if ( (this->isBeingDestroyed == false) && (this->eventInhibitionLevel == 0) )
	{
		emit machineEditedWithUndoCommandGeneratedEvent(undoCommand);
	}
}

Machine::simulation_mode Machine::getCurrentSimulationMode() const
{
	return this->currentMode;
}

shared_ptr<MachineBuilder> Machine::getMachineBuilder() const
{
	return this->machineBuilder;
}

/**
 * @brief Machine::getComponentVisualization
 * Object calling this function takes ownership of
 * visu. We need to rebuild it, but silently as this
 * is not an update, just a kind of copy.
 * @return
 */
QGraphicsItem* Machine::getComponentVisualization()
{
	QGraphicsItem* currentVisu = this->componentVisu;
	this->componentVisu = nullptr;

	this->inhibitRepresentationEvent = true;
	rebuildComponentVisualization();
	this->inhibitRepresentationEvent = false;

	return currentVisu;
}

bool Machine::setName(const QString& newName)
{
	QString correctedName = newName.trimmed();

	if (correctedName.length() != 0)
	{
		QString oldName = this->name;
		this->name = correctedName;
		this->rebuildComponentVisualization();

		emit machineNameChangedEvent(this->name);

		MachineUndoCommand* undoCommand = new MachineUndoCommand(oldName);
		this->emitMachineEditedWithUndoCommand(undoCommand);

		return true;
	}
	else
	{
		return false;
	}
}

void Machine::rebuildComponentVisualization()
{
	// /!\ QGraphicsItemGroup bounding box seems not to be updated
	// if item is added using its constructor's parent parameter

	if (this->isBeingDestroyed == false)
	{
		delete this->componentVisu;
		this->componentVisu = nullptr;

		QGraphicsItemGroup* visu = new QGraphicsItemGroup();

		//
		// Main sizes

		qreal signalsLinesWidth = 20;
		qreal horizontalSignalsNamesSpacer = 50;
		qreal verticalElementsSpacer = 5;
		qreal busesLineHeight = 10;
		qreal busesLineWidth = 5;


		//
		// Draw inputs

		QGraphicsItemGroup* inputsGroup = new QGraphicsItemGroup();

		{
			// Items position wrt. subgroup:
			// All items @ Y = 0, and rising
			// Signals names @ X > 0
			// Lines @ X < 0

			QList<shared_ptr<Input>> inputs = this->getInputs();

			qreal currentInputY = 0;
			for(int i = 0 ; i < inputs.count() ; i++)
			{
				QGraphicsTextItem* text = new QGraphicsTextItem(inputs[i]->getText());//, inputsGroup);
				inputsGroup->addToGroup(text);
				text->setPos(0, currentInputY);

				qreal currentLineY = currentInputY + text->boundingRect().height()/2;
				inputsGroup->addToGroup(new QGraphicsLineItem(-signalsLinesWidth, currentLineY, 0, currentLineY));//, inputsGroup);

				if (inputs[i]->getSize() > 1)
				{
					inputsGroup->addToGroup(new QGraphicsLineItem(-signalsLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, -signalsLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
					QGraphicsTextItem* sizeText = new QGraphicsTextItem(QString::number(inputs[i]->getSize()));
					inputsGroup->addToGroup(sizeText);
					sizeText->setPos(-signalsLinesWidth/2 - sizeText->boundingRect().width(), currentLineY - sizeText->boundingRect().height());
				}

				currentInputY += text->boundingRect().height();
			}
		}


		//
		// Draw outputs

		QGraphicsItemGroup* outputsGroup = new QGraphicsItemGroup();

		{
			// Items position wrt. subgroup:
			// All items @ Y = 0, and rising
			// Signals names @ X < 0
			// Lines @ X > 0

			QList<shared_ptr<Output>> outputs = this->getOutputs();

			qreal currentOutputY = 0;
			for(int i = 0 ; i < outputs.count() ; i++)
			{
				QGraphicsTextItem* text = new QGraphicsTextItem(outputs[i]->getText()); //, outputsGroup);
				outputsGroup->addToGroup(text);
				text->setPos(-text->boundingRect().width(), currentOutputY);

				qreal currentLineY = currentOutputY + text->boundingRect().height()/2;
				outputsGroup->addToGroup(new QGraphicsLineItem(0, currentLineY, signalsLinesWidth, currentLineY));

				if (outputs[i]->getSize() > 1)
				{
					outputsGroup->addToGroup(new QGraphicsLineItem(signalsLinesWidth/2 - busesLineWidth/2 , currentLineY + busesLineHeight/2, signalsLinesWidth/2 + busesLineWidth/2, currentLineY - busesLineHeight/2));
					QGraphicsTextItem* sizeText = new QGraphicsTextItem(QString::number(outputs[i]->getSize()));
					outputsGroup->addToGroup(sizeText);
					sizeText->setPos(signalsLinesWidth/2, currentLineY - sizeText->boundingRect().height());
				}

				currentOutputY += text->boundingRect().height();
			}
		}

		//
		// Draw component name

		QGraphicsTextItem* title = new QGraphicsTextItem();
		title->setHtml("<b>" + this->name + "</b>");

		//
		// Compute component size

		qreal componentWidth;
		qreal componentHeight;

		{
			// Width

			qreal inputsNamesWidth = inputsGroup->boundingRect().width() - signalsLinesWidth;
			qreal outputsNamesWidth = outputsGroup->boundingRect().width() - signalsLinesWidth;

			componentWidth = inputsNamesWidth + horizontalSignalsNamesSpacer + outputsNamesWidth;

			if (componentWidth <= title->boundingRect().width() + horizontalSignalsNamesSpacer)
			{
				componentWidth = title->boundingRect().width() + horizontalSignalsNamesSpacer;
			}

			// Height

			qreal maxSignalsHeight = max(inputsGroup->boundingRect().height(), outputsGroup->boundingRect().height());

			componentHeight =
			        verticalElementsSpacer +
			        title->boundingRect().height() +
			        verticalElementsSpacer +
			        maxSignalsHeight +
			        verticalElementsSpacer;

		}

		//
		// Draw component border

		QGraphicsPolygonItem* border = nullptr;

		{
			QPolygonF borderPolygon;
			borderPolygon.append(QPoint(0,              0));
			borderPolygon.append(QPoint(componentWidth, 0));
			borderPolygon.append(QPoint(componentWidth, componentHeight));
			borderPolygon.append(QPoint(0,              componentHeight));

			border = new QGraphicsPolygonItem(borderPolygon);
		}

		//
		// Place components in main group

		{
			// Items position wrt. main group:
			// Component top left corner @ (0; 0)

			visu->addToGroup(border);
			visu->addToGroup(title);
			visu->addToGroup(inputsGroup);
			visu->addToGroup(outputsGroup);

			border->setPos(0, 0);

			title->setPos( (componentWidth-title->boundingRect().width())/2, verticalElementsSpacer);

			qreal verticalSignalsNameOffset = title->boundingRect().bottom() + verticalElementsSpacer;

			qreal inoutsDeltaHeight = inputsGroup->boundingRect().height() - outputsGroup->boundingRect().height();
			qreal additionalInputsOffet  = (inoutsDeltaHeight > 0 ? 0 : -inoutsDeltaHeight/2);
			qreal additionalOutputsOffet = (inoutsDeltaHeight < 0 ? 0 : inoutsDeltaHeight/2);

			inputsGroup-> setPos(0,              verticalSignalsNameOffset + additionalInputsOffet);
			outputsGroup->setPos(componentWidth, verticalSignalsNameOffset + additionalOutputsOffet);
		}

		//
		// Done

		this->componentVisu = visu;

		if (this->inhibitRepresentationEvent == false)
		{
			emit componentVisualizationUpdatedEvent();
		}
	}
}

shared_ptr<Signal> Machine::addSignal(signal_type type, const QString& name, const LogicValue& value)
{
	uint rank;

	switch(type)
	{
	case signal_type::Input:
		rank = this->inputs.count();
		break;
	case signal_type::Output:
		rank = this->outputs.count();
		break;
	case signal_type::LocalVariable:
		rank = this->localVariables.count();
		break;
	case signal_type::Constant:
		rank = this->constants.count();
		break;
	default:
		return nullptr;
	}

	return this->addSignalAtRank(type, name, rank, value);
}

shared_ptr<Signal> Machine::addSignalAtRank(signal_type type, const QString& name, uint rank, const LogicValue& value)
{
	this->setInhibitEvents(true);

	// First check if name doesn't already exist
	foreach (shared_ptr<Signal> signal, getAllSignals())
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
	shared_ptr<Signal> signal;
	switch(type)
	{
	case signal_type::Input:
		signal = dynamic_pointer_cast<Signal>(shared_ptr<Input>(new Input(name, size))); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->inputs, &this->inputsRanks);

		if (! value.isNull())
		{
			signal->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		this->rebuildComponentVisualization();

		emit inputListChangedEvent();

		break;
	case signal_type::Output:
		signal = dynamic_pointer_cast<Signal>(shared_ptr<Output>(new Output(name, size)));  // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->outputs, &this->outputsRanks);

		this->rebuildComponentVisualization();

		emit outputListChangedEvent();

		break;
	case signal_type::LocalVariable:
		signal = shared_ptr<Signal>(new Signal(name, size)); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->localVariables, &this->localVariablesRanks);

		if (! value.isNull())
		{
			signal->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit localVariableListChangedEvent();

		break;
	case signal_type::Constant:
		signal = dynamic_pointer_cast<Signal>(shared_ptr<Constant>(new Constant(name, size))); // Throws StatesException: size checked previously, should not be 0 or value is corrupted - ignored
		this->addSignalToList(signal, rank, &this->constants, &this->constantsRanks);

		if (! value.isNull())
		{
			signal->setInitialValue(value); // Throws StatesException: size determined from value, should not fail or value is corrupted - ignored
		}

		emit constantListChangedEvent();

		break;
	}

	if (signal != nullptr)
	{
		this->setInhibitEvents(false);
		this->emitMachineEditedWithoutUndoCommand();
	}

	return signal;
}

void Machine::addSignalToList(shared_ptr<Signal> signal, uint rank, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash)
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
	this->setInhibitEvents(true);

	bool result;

	if (inputs.contains(name))
	{
		this->deleteSignalFromList(name, &this->inputs, &this->inputsRanks);

		this->rebuildComponentVisualization();

		if (this->isBeingDestroyed == false)
			emit inputListChangedEvent();

		result = true;
	}
	else if (outputs.contains(name))
	{
		this->deleteSignalFromList(name, &this->outputs, &this->outputsRanks);

		this->rebuildComponentVisualization();

		if (this->isBeingDestroyed == false)
			emit outputListChangedEvent();

		result = true;
	}
	else if (localVariables.contains(name))
	{
		this->deleteSignalFromList(name, &this->localVariables, &this->localVariablesRanks);

		if (this->isBeingDestroyed == false)
			emit localVariableListChangedEvent();

		result = true;
	}
	else if (constants.contains(name))
	{
		this->deleteSignalFromList(name, &this->constants, &this->constantsRanks);

		if (this->isBeingDestroyed == false)
			emit constantListChangedEvent();

		result = true;
	}
	else
		result = false;

	if (result == true)
	{
		this->setInhibitEvents(false);
		this->emitMachineEditedWithoutUndoCommand();
	}

	return result;
}

bool Machine::deleteSignalFromList(const QString& name, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash)
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
	this->setInhibitEvents(true);

	QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

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
			emit inputListChangedEvent();
		}
		else if (outputs.contains(oldName))
		{
			emit outputListChangedEvent();
		}
		else if (localVariables.contains(oldName))
		{
			emit localVariableListChangedEvent();
		}
		else if (constants.contains(oldName))
		{
			emit constantListChangedEvent();
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
			this->rebuildComponentVisualization();

			emit inputListChangedEvent();
		}
		else if (outputs.contains(oldName))
		{
			this->renameSignalInList(oldName, correctedNewName, &this->outputs, &this->outputsRanks);
			this->rebuildComponentVisualization();

			emit outputListChangedEvent();
		}
		else if (localVariables.contains(oldName))
		{
			this->renameSignalInList(oldName, correctedNewName, &this->localVariables, &this->localVariablesRanks);

			emit localVariableListChangedEvent();
		}
		else if (constants.contains(oldName))
		{
			this->renameSignalInList(oldName, correctedNewName, &this->constants, &this->constantsRanks);

			emit constantListChangedEvent();
		}
		else // Should not happen as we checked all lists
			return false;

		this->setInhibitEvents(false);
		this->emitMachineEditedWithoutUndoCommand();
		return true;
	}
}

bool Machine::renameSignalInList(const QString& oldName, const QString& newName, QHash<QString, shared_ptr<Signal> > *signalHash, QHash<QString, uint> *rankHash)
{
	if (!signalHash->contains(oldName))
		return false;

	shared_ptr<Signal> itemToRename = (*signalHash)[oldName];
	itemToRename->setName(newName);

	signalHash->remove(oldName);
	(*signalHash)[newName] = itemToRename;

	(*rankHash)[newName] = (*rankHash)[oldName];
	rankHash->remove(oldName);

	return true;
}

void Machine::resizeSignal(const QString &name, uint newSize) // Throws StatesException
{
	this->setInhibitEvents(true);

	QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

	if ( !allSignals.contains(name) ) // First check if signal exists
		throw StatesException("Machine", unknown_signal, "Trying to change initial value of unknown signal");
	else
	{
		allSignals[name]->resize(newSize); // Throws StatesException - propagated

		if (inputs.contains(name))
		{
			this->rebuildComponentVisualization();
			emit inputListChangedEvent();
		}
		else if (outputs.contains(name))
		{
			this->rebuildComponentVisualization();
			emit outputListChangedEvent();
		}
		else if (localVariables.contains(name))
		{
			emit localVariableListChangedEvent();
		}
		else if (constants.contains(name))
		{
			emit constantListChangedEvent();
		}
		else // Should not happen as we checked all lists
		{
			throw StatesException("Machine", impossible_error, "Unable to emit listChangedEvent");
		}

		this->setInhibitEvents(false);
		this->emitMachineEditedWithoutUndoCommand();
	}
}

void Machine::changeSignalInitialValue(const QString &name, LogicValue newValue) // Throws StatesException
{
	this->setInhibitEvents(true);

	QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

	if ( !allSignals.contains(name) ) // First check if signal exists
		throw StatesException("Machine", unknown_signal, "Trying to change initial value of unknown signal");
	else
	{
		allSignals[name]->setInitialValue(newValue);// Throws StatesException - propagated

		if (inputs.contains(name))
		{
			emit inputListChangedEvent();
		}
		else if (outputs.contains(name))
		{
			emit outputListChangedEvent();
		}
		else if (localVariables.contains(name))
		{
			emit localVariableListChangedEvent();
		}
		else if (constants.contains(name))
		{
			emit constantListChangedEvent();
		}
		else // Should not happen as we checked all lists
			throw StatesException("Machine", impossible_error, "Unable to emit listChangedEvent");

		this->setInhibitEvents(false);
		this->emitMachineEditedWithoutUndoCommand();
	}
}

bool Machine::changeSignalRank(const QString& name, uint newRank)
{
	this->setInhibitEvents(true);

	QHash<QString, shared_ptr<Signal>> allSignals = getAllSignalsMap();

	if ( !allSignals.contains(name) ) // First check if signal exists
	{
		return false;
	}
	else
	{
		if (inputs.contains(name))
		{
			this->changeRankInList(name, newRank, &this->inputs, &this->inputsRanks);
			this->rebuildComponentVisualization();

			emit inputListChangedEvent();
		}
		else if (outputs.contains(name))
		{
			this->changeRankInList(name, newRank, &this->outputs, &this->outputsRanks);
			this->rebuildComponentVisualization();

			emit outputListChangedEvent();
		}
		else if (localVariables.contains(name))
		{
			this->changeRankInList(name, newRank, &this->localVariables, &this->localVariablesRanks);

			emit localVariableListChangedEvent();
		}
		else if (constants.contains(name))
		{
			this->changeRankInList(name, newRank, &this->constants, &this->constantsRanks);

			emit constantListChangedEvent();
		}
		else // Should not happen as we checked all lists
			return false;

		this->setInhibitEvents(false);
		return true;
	}
}

void Machine::setSimulator(shared_ptr<MachineSimulator> simulator)
{
	this->simulator = simulator;

	if (simulator != nullptr)
	{
		this->setSimulationMode(Machine::simulation_mode::simulateMode);
	}
	else
	{
		this->setSimulationMode(Machine::simulation_mode::editMode);
	}
}

shared_ptr<MachineSimulator> Machine::getSimulator() const
{
	return this->simulator.lock();
}

bool Machine::changeRankInList(const QString& name, uint newRank, QHash<QString, shared_ptr<Signal>>* signalHash, QHash<QString, uint>* rankHash)
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

	foreach (QChar c, nameBeingCleaned)
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
		foreach(shared_ptr<Signal> colleage, this->getAllSignals())
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

void Machine::setInhibitEvents(bool inhibit)
{
	if (inhibit == true)
	{
		this->eventInhibitionLevel++;
	}
	else if (this->eventInhibitionLevel != 0)
	{
		this->eventInhibitionLevel--;
	}
}

void Machine::beginAtomicEdit()
{
	this->atomicEditionOngoing = true;
}

void Machine::endAtomicEdit()
{
	if (this->atomicEditionOngoing == true)
	{
		this->atomicEditionOngoing = false;
		this->emitMachineEditedWithoutUndoCommand();
	}
}

QHash<QString, shared_ptr<Signal> > Machine::getAllSignalsMap() const
{
	QHash<QString, shared_ptr<Signal>> allSignals;

	foreach (shared_ptr<Signal> signal, getAllSignals())
	{
		allSignals[signal->getName()] = signal;
	}

	return allSignals;
}
