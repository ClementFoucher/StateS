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
#include "machinexmlparser.h"

// Qt classes
#include <QXmlStreamReader>

// StateS classes
#include "statestypes.h"
#include "machine.h"
#include "variable.h"
#include "viewconfiguration.h"
#include "graphicattributes.h"
#include "actiononvariable.h"
#include "machineactuatorcomponent.h"
#include "equation.h"
#include "operand.h"


MachineXmlParser::MachineXmlParser()
{
	this->graphicAttributes = shared_ptr<GraphicAttributes>(new GraphicAttributes());
	this->viewConfiguration = shared_ptr<ViewConfiguration>(new ViewConfiguration());
}

void MachineXmlParser::doParse()
{
	this->buildMachineFromXml();
}

shared_ptr<Machine> MachineXmlParser::getMachine()
{
	return this->machine;
}

shared_ptr<GraphicAttributes> MachineXmlParser::getGraphicMachineConfiguration()
{
	return this->graphicAttributes;
}

shared_ptr<ViewConfiguration> MachineXmlParser::getViewConfiguration()
{
	return this->viewConfiguration;
}

QList<QString> MachineXmlParser::getWarnings()
{
	return this->warnings;
}

void MachineXmlParser::parseMachineName(const QString& fileName)
{
	QString nameAttribute = this->xmlReader->attributes().value("Name").toString();

	QString machineName;

	if (nameAttribute.isNull() == false)
	{
		machineName = nameAttribute;
	}
	else if (fileName.isNull() == false)
	{
		machineName = fileName;
		machineName = machineName.section("/", -1, -1);             // Extract file name from path
		machineName.remove("." + machineName.section(".", -1, -1)); // Remove extension

		this->warnings.append(tr("Information:") + " " + tr("No name was found for the machine."));
		this->warnings.append("    " + tr("Used file name to name machine:") + " " + machineName + ".");
	}
	else // In case we still have no name
	{
		machineName = tr("Machine");

		this->warnings.append(tr("Information:") + " " + tr("No name was found for the machine."));
		this->warnings.append("    " + tr("Name defaulted to:") + " " + machineName + ".");
	}

	this->machine->setName(machineName);
}

void MachineXmlParser::parseConfiguration()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	if (nodeName == "Scale")
	{
		bool ok;
		float level = attributes.value("Value").toFloat(&ok);
		if (ok == true)
		{
			this->viewConfiguration->zoomLevel = level;
		}
		else
		{
			this->warnings.append(tr("Warning.") + " " + tr("Unable to parse zoom level.") + tr("Found value was:") + " " + attributes.value("Value").toString());
		}
	}
	else if (nodeName == "ViewCentralPoint")
	{
		bool parseOk = true;
		bool ok;
		float x = attributes.value("X").toFloat(&ok);
		if (ok == false)
		{
			parseOk = false;
		}

		float y = attributes.value("Y").toFloat(&ok);
		if (ok == false)
		{
			parseOk = false;
		}

		if (parseOk == true)
		{
			this->viewConfiguration->viewCenter = QPointF(x,y);
		}
		else
		{
			this->warnings.append(tr("Warning.") + " " + tr("Unable to parse view position.") + tr("Found values were:") + " (x=" + attributes.value("X").toString() + ";y=" + attributes.value("Y").toString() +")" );
		}
	}
	else
	{
		this->warnings.append(tr("Warning.") + " " + tr("Unexpected node found while parsing configuration.") + " " + tr("Node name was:") + nodeName);
		this->warnings.append("    " + tr("Node ignored."));
	}
}

void MachineXmlParser::parseVariable()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	// Get name
	QString variableName = attributes.value("Name").toString();
	if (variableName.isNull())
	{
		this->warnings.append(tr("Error!") + " " + tr("Unnamed variable encountered while parsing variable list: unable to extract name."));
		this->warnings.append("    " + tr("Variable ignored."));

		return;
	}

	// Get ID if it is defined
	bool ok;
	componentId_t extractedId = attributes.value("Id").toULong(&ok);
	if (ok == false)
	{
		extractedId = nullId;
	}

	// Get type
	componentId_t variableId;
	if (nodeName == "Input")
	{
		variableId = machine->addVariable(VariableNature_t::input, variableName, extractedId);
	}
	else if (nodeName == "Output")
	{
		variableId = machine->addVariable(VariableNature_t::output, variableName, extractedId);
	}
	else if (nodeName == "Variable")
	{
		variableId = machine->addVariable(VariableNature_t::internal, variableName, extractedId);
	}
	else if (nodeName == "Constant")
	{
		variableId = machine->addVariable(VariableNature_t::constant, variableName, extractedId);
	}
	else
	{
		this->warnings.append(tr("Error!") + " " + tr("Unexpected variable type encountered while parsing variable list:"));
		this->warnings.append("    " + tr("Expected") + " \"Input\", \"Output\", \"Variable\" " + tr("or") + " \"Constant\", " + tr("got") + " \"" + nodeName + "\".");
		this->warnings.append("    " + tr("Variable name was:") + " " + variableName + ".");
		this->warnings.append("    " + tr("Variable ignored."));

		return;
	}

	auto variable = machine->getVariable(variableId);
	if (variable != nullptr)
	{
		// Get size
		bool ok;
		uint size = attributes.value("Size").toUInt(&ok);

		if (ok == true)
		{
			if (size != 1)
			{
				variable->resize(size);

				uint variableNewSize = variable->getSize();
				if (size != variableNewSize)
				{
					this->warnings.append(tr("Unable to resize variable") + " \"" + variableName + "\".");
					this->warnings.append("    " + tr("Variable size ignored and defaulted to") + " \"1\".");
				}
			}
		}
		else
		{
			this->warnings.append(tr("Error!") + " " + tr("Unable to extract variable size for variable") + " \"" + variableName + "\".");
			this->warnings.append("    " + tr("Variable size ignored and defaulted to") + " \"1\".");

			return;
		}

		// Get initial value
		if (nodeName != "Output")
		{
			LogicValue initialValue = LogicValue::fromString(attributes.value("Initial_value").toString());
			if (initialValue.isNull() == false)
			{
				variable->setInitialValue(initialValue);
			}
			else // (initialValue.isNull() == true)
			{
				this->warnings.append(tr("Error!") + " " + "Unable to extract initial value of variable " + variableName + ".");
				this->warnings.append("    " + tr("Given initial value was") + " \"" + attributes.value("Initial_value").toString() + "\".");
				this->warnings.append("    " + tr("Initial value ignored and defaulted to") + " \"" + QString::number(variable->getSize()) + "\".");
			}
		}
	}
}

void MachineXmlParser::parseAction()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	if (nodeName == "Action")
	{
		QString variableName = attributes.value("Name").toString();

		shared_ptr<Variable> variable;
		for (auto variableId : this->machine->getWrittableVariablesIds())
		{
			auto currentVariable = this->machine->getVariable(variableId);
			if (currentVariable == nullptr) continue;


			if (currentVariable->getName() == variableName)
			{
				variable = currentVariable;
			}
		}

		if (variable == nullptr)
		{
			this->warnings.append(tr("Reference to undeclared variable encountered while parsing action list:"));
			this->warnings.append("    " + tr("Variable name was") + " \"" + variableName + "\".");
			this->warnings.append("    " + tr("Action ignored."));

			return;
		}

		ActionOnVariableType_t actionType;
		QString actionTypeText = attributes.value("Action_Type").toString();

		if (actionTypeText == "Pulse")
		{
			actionType = ActionOnVariableType_t::pulse;
		}
		else if (actionTypeText == "ActiveOnState")
		{
			actionType = ActionOnVariableType_t::activeOnState;
		}
		else if (actionTypeText == "Set")
		{
			actionType = ActionOnVariableType_t::set;
		}
		else if (actionTypeText == "Reset")
		{
			actionType = ActionOnVariableType_t::reset;
		}
		else if (actionTypeText == "Assign")
		{
			actionType = ActionOnVariableType_t::assign;
		}
		else if (actionTypeText == "Increment")
		{
			actionType = ActionOnVariableType_t::increment;
		}
		else if (actionTypeText == "Decrement")
		{
			actionType = ActionOnVariableType_t::decrement;
		}
		else
		{
			this->warnings.append(tr("Unexpected action type encountered while parsing action list:"));
			this->warnings.append("    " + tr("Action type was") + " \"" + actionTypeText + "\".");
			this->warnings.append("    " + tr("Action ignored."));

			return;
		}

		QString srangel = attributes.value("RangeL").toString();
		QString sranger = attributes.value("RangeR").toString();
		QString sactval = attributes.value("Action_Value").toString();
		// For compatibility with previous saves
		if (srangel.isNull())
			srangel = attributes.value("Param1").toString();
		if (sranger.isNull())
			sranger = attributes.value("Param2").toString();

		int rangeL;
		int rangeR;

		if (! srangel.isEmpty())
		{
			rangeL = srangel.toInt();
		}
		else
		{
			rangeL = -1;
		}

		if (! sranger.isEmpty())
		{
			rangeR = sranger.toInt();
		}
		else
		{
			rangeR = -1;
		}

		LogicValue actionValue;
		if (sactval.isEmpty() == false)
		{
			actionValue = LogicValue::fromString(sactval);
			if (actionValue.isNull() == true)
			{
				uint avsize;
				if ( (rangeL != -1) && (rangeR == -1) )
				{
					avsize = 1;
				}
				else if ( (rangeL != -1) && (rangeR != -1) )
				{
					avsize = rangeL - rangeR + 1;
				}
				else
				{
					avsize = variable->getSize();
				}

				actionValue = LogicValue::getValue0(avsize);

				this->warnings.append(tr("Error in action value for variable") + " \"" + variableName + "\".");
				this->warnings.append("    " + tr("Value ignored and set to") + " \"" + actionValue.toString() + "\".");
			}
		}

		auto action = shared_ptr<ActionOnVariable>(new ActionOnVariable(variable, actionType, actionValue, rangeL, rangeR));
		this->currentActuator->addAction(action, variable);
	}
	else // (nodeName != "Action")
	{
		this->warnings.append(tr("Unexpected node encountered while parsing action list:"));
		this->warnings.append("    " + tr("Expected") + " \"Action\", " + tr("got") + " \"" + nodeName + "\".");
		this->warnings.append("    " + tr("Action ignored."));
	}
}

void MachineXmlParser::parseLogicEquation()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	shared_ptr<Equation> equation;

	if (nodeName == "LogicVariable")
	{
		for (auto& varId : this->machine->getReadableVariablesIds())
		{
			auto currentVariable = this->machine->getVariable(varId);
			if (currentVariable == nullptr) continue;


			if (currentVariable->getName() == attributes.value("Name"))
			{
				equation = shared_ptr<Equation>(new Equation(OperatorType_t::identity));
				equation->setOperand(0, currentVariable);
				break;
			}
		}
	}
	else if (nodeName == "LogicEquation")
	{
		OperatorType_t equationType;
		int rangeL = -1;
		int rangeR = -1;
		LogicValue constantValue;
		QString srangel;
		QString sranger;

		bool ok;
		int operandCount = attributes.value("OperandCount").toInt(&ok);

		if (ok == false)
		{
			// TODO
		}

		QString valueNature = attributes.value("Nature").toString();
		if (valueNature == "not")
			equationType = OperatorType_t::notOp;
		else if (valueNature == "and")
			equationType = OperatorType_t::andOp;
		else if (valueNature == "or")
			equationType = OperatorType_t::orOp;
		else if (valueNature == "xor")
			equationType = OperatorType_t::xorOp;
		else if (valueNature == "nand")
			equationType = OperatorType_t::nandOp;
		else if (valueNature == "nor")
			equationType = OperatorType_t::norOp;
		else if (valueNature == "xnor")
			equationType = OperatorType_t::xnorOp;
		else if (valueNature == "equals")
			equationType = OperatorType_t::equalOp;
		else if (valueNature == "differs")
			equationType = OperatorType_t::diffOp;
		else if (valueNature == "concatenate")
			equationType = OperatorType_t::concatOp;
		else if (valueNature == "extract")
		{
			equationType = OperatorType_t::extractOp;

			srangel = attributes.value("RangeL").toString();
			sranger = attributes.value("RangeR").toString();
			// For compatibility with previous saves
			if (srangel.isNull())
				srangel = attributes.value("Param1").toString();
			if (sranger.isNull())
				sranger = attributes.value("Param2").toString();

			rangeL = srangel.toInt();
			rangeR = sranger.toInt();
		}
		else if (valueNature == "constant")
		{
			equationType = OperatorType_t::identity;

			constantValue = LogicValue::fromString(attributes.value("Value").toString());
			if (constantValue.isNull() == true)
			{
				uint constantsize;
				if ( (rangeL != -1) && (rangeR == -1) )
				{
					constantsize = 1;
				}
				else if ( (rangeL != -1) && (rangeR != -1) )
				{
					constantsize = rangeL - rangeR + 1;
				}
				else
				{
					constantsize = 1; // TODO: determine actual size
				}

				constantValue = LogicValue::getValue0(constantsize);

				this->warnings.append(tr("Error in constant value while parsing equation:"));
				this->warnings.append("    " + tr("Value ignored and set to") + " \"" + constantValue.toString() + "\".");
			}
		}
		else
		{
			this->warnings.append(tr("Unexpected equation nature encountered while parsing logic equation:"));
			this->warnings.append("    " + tr("Equation nature was:") + " \"" + valueNature + "\".");
			this->warnings.append("    " + tr("Token ignored. Will retry with other tokens if existing."));

			return;
		}

		if (operandCount != 0)
		{
			equation = shared_ptr<Equation>(new Equation(equationType, operandCount));
		}
		else
		{
			equation = shared_ptr<Equation>(new Equation(equationType));
		}

		if (equationType == OperatorType_t::identity)
		{
			// In this context, identity is only used for constants
			equation->setOperand(0, constantValue);
		}
		else if (equationType == OperatorType_t::extractOp)
		{
			equation->setRange(rangeL, rangeR);
		}
	}

	if (this->rootLogicEquation == nullptr)
	{
		this->rootLogicEquation = equation;
	}

	this->currentLogicEquation = equation;
}

void MachineXmlParser::treatBeginOperand(uint operandRank)
{
	this->equationStack.push(this->currentLogicEquation);
	this->operandRankStack.push(operandRank);
}

void MachineXmlParser::treatEndOperand()
{
	shared_ptr<Equation> parentEquation = this->equationStack.pop();

	if (this->currentLogicEquation != nullptr)
	{
		if (this->currentLogicEquation->getOperatorType() != OperatorType_t::identity)
		{
			parentEquation->setOperand(this->operandRankStack.pop(), this->currentLogicEquation);
		}
		else // (this->currentLogicEquation->getOperatorType() == OperatorType_t::identity)
		{
			// Variable and constant operands are stored using identity equation on the stack: unwrap
			auto operand = this->currentLogicEquation->getOperand(0);
			if (operand != nullptr)
			{
				if (operand->getSource() == OperandSource_t::variable)
				{
					uint operandRank = this->operandRankStack.pop();
					auto variable = this->machine->getVariable(operand->getVariableId());
					if (variable != nullptr)
					{
						parentEquation->setOperand(operandRank, variable);
					}
				}
				else if (operand->getSource() == OperandSource_t::constant)
				{
					parentEquation->setOperand(this->operandRankStack.pop(), operand->getConstant());
				}
			}
		}
	}

	this->currentLogicEquation = parentEquation;
}
