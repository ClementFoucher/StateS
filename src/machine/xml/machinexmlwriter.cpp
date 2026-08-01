/*
 * Copyright © 2017-2026 Clément Foucher
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

// Qt
#include <QXmlStreamWriter>
#include <QFileInfo>
#include <QDir>

// StateS
#include "states.h"
#include "machinemanager.h"
#include "machine.h"
#include "viewconfiguration.h"
#include "machinestatus.h"
#include "machineactuatorcomponent.h"
#include "variable.h"
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


	if (this->mode == MachineXmlWriterMode_t::writeToFile)
	{
		this->stream->writeStartElement("StateS");
		this->stream->writeAttribute("Version", StateS::getVersion());

		this->writeUiConfiguration();
	}

	this->stream->writeStartElement("Machine");
	this->stream->writeAttribute("Name", machine->getName());
	this->writeMachineType();

	this->writeMachineVariables();
	this->writeSubmachineToStream();

	this->stream->writeEndElement(); // End Machine tag


	if (this->mode == MachineXmlWriterMode_t::writeToFile)
	{
		this->stream->writeEndElement(); // End StateS tag
	}
}

void MachineXmlWriter::writeActuatorActions(shared_ptr<MachineActuatorComponent> component)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	QList<shared_ptr<ActionOnVariable>> actions = component->getActions();

	if (actions.count() != 0)
	{
		this->stream->writeStartElement("Actions");
		for (shared_ptr<ActionOnVariable>& action : actions)
		{
			auto variableId = action->getVariableActedOnId();

			auto variable = machine->getVariable(variableId);
			if (variable == nullptr) continue;


			this->stream->writeStartElement("Action");

			this->stream->writeAttribute("Name", variable->getName());

			switch(action->getActionType())
			{
			case ActionOnVariableType_t::continuous:
				this->stream->writeAttribute("ActionType", "ActiveOnState");
				break;
			case ActionOnVariableType_t::pulse:
				this->stream->writeAttribute("ActionType", "Pulse");
				break;
			case ActionOnVariableType_t::set:
				this->stream->writeAttribute("ActionType", "Set");
				break;
			case ActionOnVariableType_t::reset:
				this->stream->writeAttribute("ActionType", "Reset");
				break;
			case ActionOnVariableType_t::assign:
				this->stream->writeAttribute("ActionType", "Assign");
				break;
			case ActionOnVariableType_t::increment:
				this->stream->writeAttribute("ActionType", "Increment");
				break;
			case ActionOnVariableType_t::decrement:
				this->stream->writeAttribute("ActionType", "Decrement");
				break;
			case ActionOnVariableType_t::none:
				break;
			}

			if (action->isActionValueEditable() == true)
			{
				auto actionValue = action->getActionValue();
				if (actionValue.isNull() == false)
				{
					this->stream->writeAttribute("ActionValue", actionValue.toString());
				}
			}
			if (action->getActionRangeL() != -1)
			{
				this->stream->writeAttribute("RangeL", QString::number(action->getActionRangeL()));
			}
			if (action->getActionRangeR() != -1)
			{
				this->stream->writeAttribute("RangeR", QString::number(action->getActionRangeR()));
			}

			this->stream->writeEndElement(); // Action
		}
		this->stream->writeEndElement(); // Actions
	}
}

void MachineXmlWriter::writeLogicEquation(shared_ptr<Equation> equation)
{
	if (equation == nullptr) return;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	this->stream->writeStartElement("LogicEquation");
	switch (equation->getOperatorType())
	{
	case OperatorType_t::andOp:
		this->stream->writeAttribute("Operator", "and");
		this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
		break;
	case OperatorType_t::nandOp:
		this->stream->writeAttribute("Operator", "nand");
		this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
		break;
	case OperatorType_t::norOp:
		this->stream->writeAttribute("Operator", "nor");
		this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
		break;
	case OperatorType_t::notOp:
		this->stream->writeAttribute("Operator", "not");
		break;
	case OperatorType_t::orOp:
		this->stream->writeAttribute("Operator", "or");
		this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
		break;
	case OperatorType_t::xnorOp:
		this->stream->writeAttribute("Operator", "xnor");
		this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
		break;
	case OperatorType_t::xorOp:
		this->stream->writeAttribute("Operator", "xor");
		this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
		break;
	case OperatorType_t::equalOp:
		this->stream->writeAttribute("Operator", "equals");
		break;
	case OperatorType_t::diffOp:
		this->stream->writeAttribute("Operator", "differs");
		break;
	case OperatorType_t::extractOp:
		this->stream->writeAttribute("Operator", "extract");
		this->stream->writeAttribute("RangeL", QString::number(equation->getRangeL()));
		this->stream->writeAttribute("RangeR", QString::number(equation->getRangeR()));
		break;
	case OperatorType_t::concatOp:
		this->stream->writeAttribute("Operator", "concatenate");
		this->stream->writeAttribute("OperandCount", QString::number(equation->getOperandCount()));
		break;
	case OperatorType_t::identity:
		this->stream->writeAttribute("Operator", "identity");
		break;
	}

	for (uint i = 0 ; i < equation->getOperandCount() ; i++)
	{
		auto operand = equation->getOperand(i);
		if (operand == nullptr) continue;


		this->stream->writeStartElement("Operand");
		this->stream->writeAttribute("Rank", QString::number(i));

		switch (operand->getSource())
		{
		case OperandSource_t::equation:
			this->stream->writeAttribute("Source", "Equation");
			this->writeLogicEquation(operand->getEquation());
			break;
		case OperandSource_t::variable:
		{
			auto variableId = operand->getVariableId();
			auto variable = machine->getVariable(variableId);
			if (variable != nullptr)
			{
				this->stream->writeAttribute("Source", "Variable");
				this->stream->writeAttribute("Name", variable->getName());
			}
			break;
		}
		case OperandSource_t::constant:
			auto constantValue = operand->getConstant();
			if (constantValue.isNull() == false)
			{
				this->stream->writeAttribute("Source", "Constant");
				this->stream->writeAttribute("Value", constantValue.toString());
			}
			break;
		}

		this->stream->writeEndElement(); // Operand
	}

	this->stream->writeEndElement(); // LogicEquation
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

	this->file = make_unique<QFile>(machineStatus->getSaveFileFullPath());
	bool fileOpened = file->open(QIODevice::WriteOnly);
	if (fileOpened == false)
	{
		throw StatesException("MachineXmlWriter", MachineaveFileManagerError_t::unable_to_open, tr("Unable to open file in write mode."));
	}

	this->stream = make_shared<QXmlStreamWriter>(this->file.get());

	this->stream->setAutoFormatting(true);
	this->stream->writeStartDocument();
}

void MachineXmlWriter::createSaveString()
{
	this->xmlString = QString();
	this->stream = make_shared<QXmlStreamWriter>(&this->xmlString);
	this->stream->setAutoFormatting(true);
	this->stream->setAutoFormattingIndent(0);
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

	this->stream->writeStartElement("ViewScale");
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


	this->stream->writeStartElement("Variables");

	for (auto& variableId : machine->getInputVariablesIds())
	{
		this->writeMachineVariable(VariableNature_t::input, variableId);
	}

	for (auto& variableId : machine->getInternalVariablesIds())
	{
		this->writeMachineVariable(VariableNature_t::internal, variableId);
	}

	for (auto& variableId : machine->getOutputVariablesIds())
	{
		this->writeMachineVariable(VariableNature_t::output, variableId);
	}

	for (auto& variableId : machine->getConstantsIds())
	{
		this->writeMachineVariable(VariableNature_t::constant, variableId);
	}

	this->stream->writeEndElement();
}

void MachineXmlWriter::writeMachineVariable(VariableNature_t nature, componentId_t variableId)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	switch (nature)
	{
	case VariableNature_t::input:
		this->stream->writeStartElement("Input");
		break;
	case VariableNature_t::internal:
		this->stream->writeStartElement("Internal");
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

	// Value
	this->stream->writeAttribute("Value", variable->getInitialValue().toString());

	// Memorized
	if (variable->getMemorized() == true)
	{
		this->stream->writeAttribute("Memorized", "true");
	}

	// Id
	if (this->mode == MachineXmlWriterMode_t::writeToUndo)
	{
		this->stream->writeAttribute("Id", QString::number(variableId));
	}

	this->stream->writeEndElement();
}
