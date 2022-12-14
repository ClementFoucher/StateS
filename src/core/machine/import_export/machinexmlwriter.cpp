/*
 * Copyright © 2017-2021 Clément Foucher
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
#include "machinexmlwriter.h"

// Qt classes
#include <QXmlStreamWriter>
#include <QFileInfo>
#include <QDir>

// StateS classes
#include "machinemanager.h"
#include "statesexception.h"
#include "viewconfiguration.h"
#include "machinestatus.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "signal.h"
#include "input.h"
#include "output.h"
#include "constant.h"
#include "equation.h"
#include "actiononsignal.h"


void MachineXmlWriter::writeMachineToFile()
{
	this->writingToFile = true;
	this->createSaveFile();
	this->writeMachineToStream();
	this->finalizeSaveFile();
}

QString MachineXmlWriter::getMachineXml()
{
	this->createSaveString();
	this->writeMachineToStream();
	return this->xmlString;
}

MachineXmlWriter::MachineXmlWriter(shared_ptr<MachineManager> machineManager)
{
	this->machineManager = machineManager;
}

void MachineXmlWriter::writeMachineCommonElements()
{
	if (this->writingToFile == true)
	{
		this->writeMachineConfiguration();
	}
	this->writeMachineSignals();
}

void MachineXmlWriter::writeMachineConfiguration()
{
	shared_ptr<ViewConfiguration> viewConfiguration = this->machineManager->getViewConfiguration();

	this->stream->writeStartElement("Configuration");

	this->stream->writeStartElement("Scale");
	this->stream->writeAttribute("Value", QString::number(viewConfiguration->zoomLevel));
	this->stream->writeEndElement();

	this->stream->writeStartElement("ViewCentralPoint");
	this->stream->writeAttribute("X", QString::number(viewConfiguration->viewCenter.x() + viewConfiguration->sceneTranslation.x()));
	this->stream->writeAttribute("Y", QString::number(viewConfiguration->viewCenter.y() + viewConfiguration->sceneTranslation.y()));
	this->stream->writeEndElement();

	this->stream->writeEndElement();
}

void MachineXmlWriter::writeMachineSignals()
{
	this->stream->writeStartElement("Signals");

	foreach (shared_ptr<Signal> var, this->machineManager->getMachine()->getAllSignals())
	{
		// Type
		if (dynamic_pointer_cast<Input>(var) != nullptr)
			this->stream->writeStartElement("Input");
		else if (dynamic_pointer_cast<Output>(var) != nullptr)
			this->stream->writeStartElement("Output");
		else if (dynamic_pointer_cast<Constant>(var) != nullptr)
			this->stream->writeStartElement("Constant");
		else
			this->stream->writeStartElement("Variable");

		// Name
		this->stream->writeAttribute("Name", var->getName());

		// Size
		this->stream->writeAttribute("Size", QString::number(var->getSize()));

		// Initial value (except for outputs)
		if (dynamic_pointer_cast<Output>(var) == nullptr)
			this->stream->writeAttribute("Initial_value", var->getInitialValue().toString());

		this->stream->writeEndElement();
	}

	this->stream->writeEndElement();
}

void MachineXmlWriter::writeActuatorActions(shared_ptr<MachineActuatorComponent> component)
{
	QList<shared_ptr<ActionOnSignal>> actions = component->getActions();

	if (actions.count() != 0)
	{
		this->stream->writeStartElement("Actions");
		foreach (shared_ptr<ActionOnSignal> action, actions)
		{
			this->stream->writeStartElement("Action");

			if ((dynamic_pointer_cast<Output> (action)) != nullptr)
				this->stream->writeAttribute("Signal_Type", "Output");
			else
				this->stream->writeAttribute("Signal_Type", "Variable");

			this->stream->writeAttribute("Name", action->getSignalActedOn()->getName());

			switch(action->getActionType())
			{
			case ActionOnSignal::action_types::activeOnState:
				this->stream->writeAttribute("Action_Type", "ActiveOnState");
				break;
			case ActionOnSignal::action_types::pulse:
				this->stream->writeAttribute("Action_Type", "Pulse");
				break;
			case ActionOnSignal::action_types::set:
				this->stream->writeAttribute("Action_Type", "Set");
				break;
			case ActionOnSignal::action_types::reset:
				this->stream->writeAttribute("Action_Type", "Reset");
				break;
			case ActionOnSignal::action_types::assign:
				this->stream->writeAttribute("Action_Type", "Assign");
				break;
			}

			if (!action->getActionValue().isNull())
				this->stream->writeAttribute("Action_Value", action->getActionValue().toString());
			if (action->getActionRangeL() != -1)
				this->stream->writeAttribute("RangeL", QString::number(action->getActionRangeL()));
			if (action->getActionRangeR() != -1)
				this->stream->writeAttribute("RangeR", QString::number(action->getActionRangeR()));

			this->stream->writeEndElement(); // Action
		}
		this->stream->writeEndElement(); // Actions
	}
}

void MachineXmlWriter::writeLogicEquation(shared_ptr<Signal> equation)
{
	shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (equation);

	if (complexEquation != nullptr)
	{
		this->stream->writeStartElement("LogicEquation");
		switch (complexEquation->getFunction())
		{
		case Equation::nature::andOp:
			this->stream->writeAttribute("Nature", "and");
			this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
			break;
		case Equation::nature::nandOp:
			this->stream->writeAttribute("Nature", "nand");
			this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
			break;
		case Equation::nature::norOp:
			this->stream->writeAttribute("Nature", "nor");
			this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
			break;
		case Equation::nature::notOp:
			this->stream->writeAttribute("Nature", "not");
			break;
		case Equation::nature::orOp:
			this->stream->writeAttribute("Nature", "or");
			this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
			break;
		case Equation::nature::xnorOp:
			this->stream->writeAttribute("Nature", "xnor");
			this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
			break;
		case Equation::nature::xorOp:
			this->stream->writeAttribute("Nature", "xor");
			this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
			break;
		case Equation::nature::equalOp:
			this->stream->writeAttribute("Nature", "equals");
			break;
		case Equation::nature::diffOp:
			this->stream->writeAttribute("Nature", "differs");
			break;
		case Equation::nature::extractOp:
			this->stream->writeAttribute("Nature", "extract");
			this->stream->writeAttribute("RangeL", QString::number(complexEquation->getRangeL()));
			this->stream->writeAttribute("RangeR", QString::number(complexEquation->getRangeR()));
			break;
		case Equation::nature::concatOp:
			this->stream->writeAttribute("Nature", "concatenate");
			this->stream->writeAttribute("OperandCount", QString::number(complexEquation->getOperandCount()));
			break;
		case Equation::nature::constant:
			this->stream->writeAttribute("Nature", "constant");
			this->stream->writeAttribute("Value", complexEquation->getCurrentValue().toString());
			break;
		case Equation::nature::identity:
			// Should not happen
			break;
		}

		for (uint i = 0 ; i < complexEquation->getOperandCount() ; i++)
		{
			shared_ptr<Signal> operand = complexEquation->getOperand(i); // Throws StatesException - Constrained by operand count - ignored
			if (operand != nullptr)
			{
				this->stream->writeStartElement("Operand");
				this->stream->writeAttribute("Number", QString::number(i));
				this->writeLogicEquation(operand);
				this->stream->writeEndElement(); // Operand
			}
		}
	}
	else
	{
		this->stream->writeStartElement("LogicVariable");
		this->stream->writeAttribute("Name", equation->getName());
	}

	this->stream->writeEndElement(); // LogicEquation | LogicVariable
}

void MachineXmlWriter::createSaveFile() // Throws StatesException
{
	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	QFileInfo fileInfo(machineStatus->getSaveFileFullPath());
	if ( (fileInfo.exists()) && (!fileInfo.isWritable()) ) // Replace existing file
	{
		throw StatesException("MachineSaveFileManager", unable_to_replace, "Unable to replace existing file");
	}
	else if ( !fileInfo.absoluteDir().exists() )
	{
		throw StatesException("MachineSaveFileManager", unkown_directory, "Directory doesn't exist");
	}

	this->file = unique_ptr<QFile>(new QFile(machineStatus->getSaveFileFullPath()));
	bool fileOpened = file->open(QIODevice::WriteOnly);
	if (fileOpened == false)
	{
		throw StatesException("MachineSaveFileManager", unable_to_open, "Unable to open file");
	}

	this->stream = shared_ptr<QXmlStreamWriter>(new QXmlStreamWriter(this->file.get()));

	this->stream->setAutoFormatting(true);
	this->stream->writeStartDocument();
}

void MachineXmlWriter::createSaveString()
{
	this->xmlString = QString();
	this->stream = shared_ptr<QXmlStreamWriter>(new QXmlStreamWriter(&this->xmlString));
}

void MachineXmlWriter::finalizeSaveFile()
{
	this->file->close();
	this->file = nullptr;
}
