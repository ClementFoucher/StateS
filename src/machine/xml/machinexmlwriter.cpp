/*
 * Copyright © 2017-2025 Clément Foucher
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
#include "states.h"
#include "machinemanager.h"
#include "machine.h"
#include "viewconfiguration.h"
#include "machinestatus.h"
#include "machineactuatorcomponent.h"
#include "input.h"
#include "output.h"
#include "constant.h"
#include "equation.h"
#include "actiononvariable.h"
#include "statesexception.h"
#include "exceptiontypes.h"
#include "operand.h"


MachineXmlWriter::MachineXmlWriter(MachineXmlWriterMode_t mode, shared_ptr<ViewConfiguration> viewConfiguration)
{
	this->mode = mode;
	this->viewConfiguration = viewConfiguration;
}

void MachineXmlWriter::writeMachineToFile() // Throws StatesException
{
	this->createSaveFile(); // Throws StatesException
	this->writeMachineToStream();
	this->finalizeSaveFile();
}

QString MachineXmlWriter::getMachineXml()
{
	this->createSaveString();
	this->writeMachineToStream();
	return this->xmlString;
}

void MachineXmlWriter::writeMachineToStream()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	this->writeMachineType();
	this->stream->writeAttribute("Name", machine->getName());
	this->stream->writeAttribute("StateS_version", StateS::getVersion());

	if (this->mode == MachineXmlWriterMode_t::writeToFile)
	{
		this->writeUiConfiguration();
	}
	this->writeMachineVariables();
	this->writeSubmachineToStream();

	this->stream->writeEndElement(); // End "Machine" tag (i.e. only FSM currently)
}

void MachineXmlWriter::writeActuatorActions(shared_ptr<MachineActuatorComponent> component)
{
	QList<shared_ptr<ActionOnVariable>> actions = component->getActions();

	if (actions.count() != 0)
	{
		this->stream->writeStartElement("Actions");
		for (shared_ptr<ActionOnVariable>& action : actions)
		{
			auto variable = action->getVariableActedOn();
			if (variable == nullptr) continue;


			this->stream->writeStartElement("Action");

			this->stream->writeAttribute("Name", variable->getName());

			switch(action->getActionType())
			{
			case ActionOnVariableType_t::activeOnState:
				this->stream->writeAttribute("Action_Type", "ActiveOnState");
				break;
			case ActionOnVariableType_t::pulse:
				this->stream->writeAttribute("Action_Type", "Pulse");
				break;
			case ActionOnVariableType_t::set:
				this->stream->writeAttribute("Action_Type", "Set");
				break;
			case ActionOnVariableType_t::reset:
				this->stream->writeAttribute("Action_Type", "Reset");
				break;
			case ActionOnVariableType_t::assign:
				this->stream->writeAttribute("Action_Type", "Assign");
				break;
			case ActionOnVariableType_t::increment:
				this->stream->writeAttribute("Action_Type", "Increment");
				break;
			case ActionOnVariableType_t::decrement:
				this->stream->writeAttribute("Action_Type", "Decrement");
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

void MachineXmlWriter::writeLogicEquation(shared_ptr<Equation> equation)
{
	if (equation == nullptr) return;


	if (equation->getOperatorType() != OperatorType_t::identity)
	{
		this->stream->writeStartElement("LogicEquation");
		switch (equation->getOperatorType())
		{
		case OperatorType_t::andOp:
			this->stream->writeAttribute("Nature", "and");
			this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
			break;
		case OperatorType_t::nandOp:
			this->stream->writeAttribute("Nature", "nand");
			this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
			break;
		case OperatorType_t::norOp:
			this->stream->writeAttribute("Nature", "nor");
			this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
			break;
		case OperatorType_t::notOp:
			this->stream->writeAttribute("Nature", "not");
			break;
		case OperatorType_t::orOp:
			this->stream->writeAttribute("Nature", "or");
			this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
			break;
		case OperatorType_t::xnorOp:
			this->stream->writeAttribute("Nature", "xnor");
			this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
			break;
		case OperatorType_t::xorOp:
			this->stream->writeAttribute("Nature", "xor");
			this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
			break;
		case OperatorType_t::equalOp:
			this->stream->writeAttribute("Nature", "equals");
			break;
		case OperatorType_t::diffOp:
			this->stream->writeAttribute("Nature", "differs");
			break;
		case OperatorType_t::extractOp:
			this->stream->writeAttribute("Nature", "extract");
			this->stream->writeAttribute("RangeL", QString::number(equation->getRangeL()));
			this->stream->writeAttribute("RangeR", QString::number(equation->getRangeR()));
			break;
		case OperatorType_t::concatOp:
			this->stream->writeAttribute("Nature", "concatenate");
			this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
			break;
		case OperatorType_t::identity:
			// Handled in another branch of the if
			break;
		}

		for (uint i = 0 ; i < equation->getOperandCount() ; i++)
		{
			auto operand = equation->getOperand(i);
			if (operand != nullptr)
			{
				this->stream->writeStartElement("Operand");
				this->stream->writeAttribute("Number", QString::number(i));

				switch (operand->getSource())
				{
				case OperandSource_t::equation:
					this->writeLogicEquation(operand->getEquation());
					break;
				case OperandSource_t::variable:
					this->stream->writeStartElement("LogicVariable");
					this->stream->writeAttribute("Name", operand->getVariable()->getName());
					this->stream->writeEndElement(); // LogicVariable
					break;
				case OperandSource_t::constant:
					this->stream->writeStartElement("LogicEquation");
					this->stream->writeAttribute("Nature", "constant");
					this->stream->writeAttribute("Value", operand->getConstant().toString());
					this->stream->writeEndElement(); // LogicEquation
					break;
				}

				this->stream->writeEndElement(); // Operand
			}
		}

		this->stream->writeEndElement(); // LogicEquation
	}
	else // (equation->getOperatorType() == OperatorType_t::identity)
	{
		// Identity should only happen aa root equation to carry variables or constants
		auto operand = equation->getOperand(0);
		if (operand == nullptr) return;


		auto operandSource = operand->getSource();
		if (operandSource == OperandSource_t::variable)
		{
			auto variable = operand->getVariable();
			if (variable == nullptr) return;


			this->stream->writeStartElement("LogicVariable");
			this->stream->writeAttribute("Name", variable->getName());
			this->stream->writeEndElement(); // LogicVariable
		}
		else if (operandSource == OperandSource_t::constant)
		{
			this->stream->writeStartElement("LogicEquation");
			this->stream->writeAttribute("Nature", "constant");
			this->stream->writeAttribute("Value", operand->getConstant().toString());
			this->stream->writeEndElement(); // LogicEquation
		}
	}
}

void MachineXmlWriter::createSaveFile() // Throws StatesException
{
	shared_ptr<MachineStatus> machineStatus = machineManager->getMachineStatus();
	QFileInfo fileInfo(machineStatus->getSaveFileFullPath());
	if ( (fileInfo.exists()) && (!fileInfo.isWritable()) ) // Replace existing file
	{
		throw StatesException("MachineXmlWriter", MachineaveFileManagerError_t::unable_to_replace, tr("Unable to replace existing file: permission denied. Check if the file is writable and you have appropriate rights."));
	}
	else if ( !fileInfo.absoluteDir().exists() )
	{
		throw StatesException("MachineXmlWriter", MachineaveFileManagerError_t::unkown_directory, tr("Specified directory doesn't exist."));
	}

	this->file = unique_ptr<QFile>(new QFile(machineStatus->getSaveFileFullPath()));
	bool fileOpened = file->open(QIODevice::WriteOnly);
	if (fileOpened == false)
	{
		throw StatesException("MachineXmlWriter", MachineaveFileManagerError_t::unable_to_open, tr("Unable to open file in write mode."));
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

void MachineXmlWriter::writeUiConfiguration()
{
	if (this->viewConfiguration == nullptr) return;


	this->stream->writeStartElement("Configuration");

	this->stream->writeStartElement("Scale");
	this->stream->writeAttribute("Value", QString::number(this->viewConfiguration->zoomLevel));
	this->stream->writeEndElement();

	this->stream->writeStartElement("ViewCentralPoint");
	this->stream->writeAttribute("X", QString::number(this->viewConfiguration->viewCenter.x() + this->viewConfiguration->sceneTranslation.x()));
	this->stream->writeAttribute("Y", QString::number(this->viewConfiguration->viewCenter.y() + this->viewConfiguration->sceneTranslation.y()));
	this->stream->writeEndElement();

	this->stream->writeEndElement();
}

void MachineXmlWriter::writeMachineVariables()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	this->stream->writeStartElement("Signals");

	for (auto& variable : machine->getInputs())
	{
		this->writeMachineVariable(VariableNature_t::input, variable);
	}

	for (auto& variable : machine->getInternalVariables())
	{
		this->writeMachineVariable(VariableNature_t::internal, variable);
	}

	for (auto& variable : machine->getOutputs())
	{
		this->writeMachineVariable(VariableNature_t::output, variable);
	}

	for (auto& variable : machine->getConstants())
	{
		this->writeMachineVariable(VariableNature_t::constant, variable);
	}

	this->stream->writeEndElement();
}

void MachineXmlWriter::writeMachineVariable(VariableNature_t nature, shared_ptr<Variable> variable)
{
	switch (nature)
	{
	case VariableNature_t::input:
		this->stream->writeStartElement("Input");
		break;
	case VariableNature_t::internal:
		this->stream->writeStartElement("Variable");
		break;
	case VariableNature_t::output:
		this->stream->writeStartElement("Output");
		break;
	case VariableNature_t::constant:
		this->stream->writeStartElement("Constant");
		break;
	}

	// Name
	this->stream->writeAttribute("Name", variable->getName());

	// Size
	this->stream->writeAttribute("Size", QString::number(variable->getSize()));

	// Initial value (except for outputs)
	if (nature != VariableNature_t::output)
		this->stream->writeAttribute("Initial_value", variable->getInitialValue().toString());

	this->stream->writeEndElement();
}
