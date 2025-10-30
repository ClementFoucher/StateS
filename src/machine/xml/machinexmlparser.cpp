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
#include <QFile>

// StateS classes
#include "machine.h"
#include "variable.h"
#include "viewconfiguration.h"
#include "graphicattributes.h"
#include "actiononvariable.h"
#include "machineactuatorcomponent.h"
#include "equation.h"


MachineXmlParser::MachineXmlParser()
{
	this->graphicAttributes = make_shared<GraphicAttributes>();
	this->viewConfiguration = make_shared<ViewConfiguration>();
}

MachineXmlParser::MachineXmlParser(const QString& xmlString) :
	MachineXmlParser()
{
	this->xmlReader = make_shared<QXmlStreamReader>(xmlString);
}

MachineXmlParser::MachineXmlParser(shared_ptr<QFile> file) :
	MachineXmlParser()
{
	this->file = file;
	if (file->isOpen() == false)
	{
		file->open(QIODevice::ReadOnly);
	}
	else
	{
		file->reset();
	}

	this->xmlReader = make_shared<QXmlStreamReader>(file.get());
}

void MachineXmlParser::doParse()
{
	while (this->xmlReader->atEnd() == false)
	{
		this->xmlReader->readNext();

		if (this->xmlReader->isStartElement())
		{
			this->parseStartElement();
		}
		else if (this->xmlReader->isEndElement())
		{
			this->parseEndElement();
		}
	}
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

QList<QString> MachineXmlParser::getIssues()
{
	return this->issues;
}

void MachineXmlParser::parseActionNode()
{
	auto currentActuator = this->machine->getActuatorComponent(this->currentComponentId);
	if (currentActuator == nullptr)
	{
		this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"Action\" " + tr("node due to previous errors."));
		return;
	}

	// Get variable name
	QString variableName = this->getCurrentNodeStringAttribute("Name");

	shared_ptr<Variable> variable = this->getVariableByName(variableName);
	if (variable == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("Reference to undeclared variable encountered while parsing action list."));
		this->addIssue("    " + tr("Variable name was") + " \"" + variableName + "\".");
		this->addIssue("    " + tr("Action ignored."));

		return;
	}

	// Get action type
	ActionOnVariableType_t actionType;
	QString actionTypeText = this->getCurrentNodeStringAttribute("Action_Type");

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
		this->addIssue(tr("Error!") + " " + tr("Unexpected action type encountered while parsing action list."));
		this->addIssue("    " + tr("Action type was") + " \"" + actionTypeText + "\".");
		this->addIssue("    " + tr("Action ignored."));

		return;
	}

	// Get action range
	QString srangel = this->getCurrentNodeStringAttribute("RangeL");
	QString sranger = this->getCurrentNodeStringAttribute("RangeR");
	// For compatibility with previous saves
	if (srangel.isNull() == true)
	{
		srangel = this->getCurrentNodeStringAttribute("Param1");
	}
	if (sranger.isNull() == true)
	{
		sranger = this->getCurrentNodeStringAttribute("Param2");
	}

	int rangeL;
	int rangeR;

	if (srangel.isEmpty() == false)
	{
		rangeL = srangel.toInt();
	}
	else
	{
		rangeL = -1;
	}

	if (sranger.isEmpty() == false)
	{
		rangeR = sranger.toInt();
	}
	else
	{
		rangeR = -1;
	}

	// Get action value
	QString sactval = this->getCurrentNodeStringAttribute("Action_Value");
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

			this->addIssue(tr("Warning:") + " " + tr("Error in action value for variable") + " \"" + variableName + "\".");
			this->addIssue("    " + tr("Value ignored and set to") + " \"" + actionValue.toString() + "\".");
		}
	}

	// Build action
	auto action = make_shared<ActionOnVariable>(variable, actionType, actionValue, rangeL, rangeR);
	currentActuator->addAction(action, variable);
}

void MachineXmlParser::parseLogicEquationNode()
{
	OperatorType_t operatorType;
	int rangeL = -1;
	int rangeR = -1;

	bool ok;
	int operandCount = this->getCurrentNodeIntAttribute("OperandCount", &ok);

	if (ok == false)
	{
		// TODO: warning in some cases

		operandCount = -1;
	}

	QString valueOperator = this->getCurrentNodeStringAttribute("Nature"); // TODO: use "Operator"
	if (valueOperator == "not")
	{
		operatorType = OperatorType_t::notOp;
	}
	else if (valueOperator == "and")
	{
		operatorType = OperatorType_t::andOp;
	}
	else if (valueOperator == "or")
	{
		operatorType = OperatorType_t::orOp;
	}
	else if (valueOperator == "xor")
	{
		operatorType = OperatorType_t::xorOp;
	}
	else if (valueOperator == "nand")
	{
		operatorType = OperatorType_t::nandOp;
	}
	else if (valueOperator == "nor")
	{
		operatorType = OperatorType_t::norOp;
	}
	else if (valueOperator == "xnor")
	{
		operatorType = OperatorType_t::xnorOp;
	}
	else if (valueOperator == "equals")
	{
		operatorType = OperatorType_t::equalOp;
	}
	else if (valueOperator == "differs")
	{
		operatorType = OperatorType_t::diffOp;
	}
	else if (valueOperator == "concatenate")
	{
		operatorType = OperatorType_t::concatOp;
	}
	else if (valueOperator == "extract")
	{
		operatorType = OperatorType_t::extractOp;

		auto srangel = this->getCurrentNodeStringAttribute("RangeL");
		auto sranger = this->getCurrentNodeStringAttribute("RangeR");
		// For compatibility with previous saves
		if (srangel.isNull())
		{
			srangel = this->getCurrentNodeStringAttribute("Param1");
		}
		if (sranger.isNull())
		{
			sranger = this->getCurrentNodeStringAttribute("Param2");
		}

		rangeL = srangel.toInt();
		rangeR = sranger.toInt();
	}
	else
	{
		this->addIssue(tr("Error!") + " " + tr("Unexpected equation nature encountered while parsing logic equation."));
		this->addIssue("    " + tr("Equation nature was:") + " \"" + valueOperator + "\".");
		this->addIssue("    " + tr("Node ignored."));

		return;
	}

	// Build equation
	auto equation = make_shared<Equation>(operatorType, operandCount);

	if (operatorType == OperatorType_t::extractOp)
	{
		equation->setRange(rangeL, rangeR);
	}

	this->equationStack.push(equation);
}

void MachineXmlParser::parseOperandNode()
{
	if (this->equationStack.isEmpty() == true)
	{
		this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"Operand\" " + tr("node due to previous errors."));

		return;
	}


	bool ok;
	uint operandRank = this->getCurrentNodeUintAttribute("Number", &ok);
	if (ok == false)
	{
		this->addIssue(tr("Warning:") + " " + tr("Unable to parse operand rank for an equation."));
		this->addIssue("    " + tr("Operand will be placed in position 0, potentially erasing existing operand."));
		operandRank = 0;
	}

	this->operandRankStack.push(operandRank);
}

void MachineXmlParser::parseOperandVariableNode()
{
	auto variableName = this->getCurrentNodeStringAttribute("Name");

	shared_ptr<Variable> variable = this->getVariableByName(variableName);
	if (variable == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("Reference to undeclared variable encountered while parsing an equation."));
		this->addIssue("    " + tr("Variable name was") + " \"" + variableName + "\".");
		this->addIssue("    " + tr("Operand ignored."));

		return;
	}


	if (this->equationStack.isEmpty() == false)
	{
		if (this->operandRankStack.isEmpty() == true)
		{
			this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"LogicVariable\" " + tr("node due to previous errors."));
			return;
		}


		auto parentEquation = this->equationStack.top();
		parentEquation->setOperand(this->operandRankStack.pop(), variable);
	}
	else
	{
		auto equation = make_shared<Equation>(OperatorType_t::identity);
		equation->setOperand(0, variable);

		this->equationStack.push(equation);
	}
}

void MachineXmlParser::parseOperandConstantNode()
{
	auto constantValue = LogicValue::fromString(this->getCurrentNodeStringAttribute("Value"));
	if (constantValue.isNull() == true)
	{
		constantValue = LogicValue::getValue0(1);

		this->addIssue(tr("Warning:") + " " + tr("Error in constant value while parsing equation."));
		this->addIssue("    " + tr("Value ignored and set to") + " \"" + constantValue.toString() + "\".");
	}

	if (this->equationStack.isEmpty() == false)
	{
		if (this->operandRankStack.isEmpty() == true)
		{
			this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"LogicEquation\" " + tr("node due to previous errors."));

			return;
		}


		auto parentEquation = this->equationStack.top();
		parentEquation->setOperand(this->operandRankStack.pop(), constantValue);
		this->isParsingConstantOperand = true;
	}
	else
	{
		auto equation = make_shared<Equation>(OperatorType_t::identity);
		equation->setOperand(0, constantValue);

		this->equationStack.push(equation);
	}
}

/**
 * @brief MachineXmlParser::processEndLogicVariableNode is called when
 *        a LogicVariable end tag is encountered. It identifies
 *        if the current variable is root or is an equation operand.
 * @return IsRoot_t::no if equation was an operand,
 *         IsRoot_t::yes if it was root
 */
MachineXmlParser::IsRoot_t MachineXmlParser::processEndLogicVariableNode()
{
	if (this->equationStack.count() == 0)
	{
		// This is an error case, but we should declare
		// this is root to fall back in the correct tag.
		return IsRoot_t::yes;
	}

	if (this->equationStack.count() == 1)
	{
		auto equation = this->equationStack.top();

		if (equation->getOperatorType() == OperatorType_t::identity)
		{
			// Current equation is root
			return IsRoot_t::yes;
		}
	}

	// Current equation is an equation operand
	return IsRoot_t::no;
}

/**
 * @brief MachineXmlParser::processEndLogicEquationNode is called when
 *        a LogicEquation end tag is encountered. It identifies
 *        if the current equation is root or is another equation
 *        operand. In the second case, it sets the current equation
 *        as an operand of the parent equation.
 * @return IsRoot_t::no if equation was an operand,
 *         IsRoot_t::yes if it was root
 */
MachineXmlParser::IsRoot_t MachineXmlParser::processEndLogicEquationNode()
{
	if (this->isParsingConstantOperand == true)
	{
		// Constant is a special case as we removed the operator type,
		// but it is still used in save files.
		this->isParsingConstantOperand = false;
		return IsRoot_t::no;
	}

	if (this->equationStack.count() == 1)
	{
		// Current equation is root
		return IsRoot_t::yes;
	}
	else
	{
		// Current equation is another equation operand
		if (this->operandRankStack.isEmpty() == true)
		{
			this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"LogicEquation\" " + tr("node due to previous errors."));

			return IsRoot_t::no;
		}


		auto currentEquation = this->equationStack.pop();
		auto parentEquation = this->equationStack.top();
		parentEquation->setOperand(this->operandRankStack.pop(), currentEquation);

		return IsRoot_t::no;
	}
}

QString MachineXmlParser::getCurrentNodeName() const
{
	return this->xmlReader->name().toString();
}

QString MachineXmlParser::getCurrentNodeStringAttribute(const QString& name) const
{
	return this->xmlReader->attributes().value(name).toString();
}

uint MachineXmlParser::getCurrentNodeUintAttribute(const QString& name, bool* ok) const
{
	return this->xmlReader->attributes().value(name).toUInt(ok);
}

int MachineXmlParser::getCurrentNodeIntAttribute(const QString& name, bool* ok) const
{
	return this->xmlReader->attributes().value(name).toInt(ok);
}

float MachineXmlParser::getCurrentNodeFloatAttribute(const QString& name, bool* ok) const
{
	return this->xmlReader->attributes().value(name).toFloat(ok);
}

componentId_t MachineXmlParser::getCurrentNodeIdAttribute() const
{
	bool ok;
	ulong idValue = this->xmlReader->attributes().value("Id").toULong(&ok);

	if (ok == true)
	{
		return static_cast<componentId_t>(idValue);
	}
	else
	{
		return nullId;
	}
}

shared_ptr<Equation> MachineXmlParser::getCurrentEquation()
{
	if (this->equationStack.isEmpty() == true) return nullptr;


	return this->equationStack.pop();
}

void MachineXmlParser::addGraphicAttribute(uint componentId, QString name, QString value)
{
	this->graphicAttributes->addAttribute(componentId, name, value);
}

void MachineXmlParser::addIssue(const QString& warning)
{
	this->issues.append(warning);
}

void MachineXmlParser::parseStartElement()
{
	auto nodeName = this->getCurrentNodeName();

	if (this->unexpectedTagLevel != 0)
	{
		this->unexpectedTagLevel++;

		this->addIssue("    "  + tr("Ignoring node") + " " + nodeName + " " + tr("due to previous errors."));

		return;
	}


	switch (this->currentTag)
	{
	case Tag_t::none:
		if (nodeName == "FSM")
		{
			this->currentTag = Tag_t::machine;
			this->parseMachineName();
		}
		else
		{
			this->unexpectedTagLevel++;

			this->addIssue(tr("Error!") + " " + tr("Unexpected root node.") + " " + tr("Root node should be") + " \"FSM\", " + tr("but found") + " \"" + nodeName + "\".");
		}
		break;
	case Tag_t::machine:
		if (nodeName == "Configuration")
		{
			this->currentTag = Tag_t::configuration;
		}
		else if (nodeName == "Signals")
		{
			this->currentTag = Tag_t::variables;
		}
		else
		{
			this->currentTag = Tag_t::submachineTag;
			this->parseSubmachineStartElement();
		}
		break;
	case Tag_t::configuration:
		if (nodeName == "Scale")
		{
			this->currentTag = Tag_t::configurationViewScale;
			this->parseConfigurationViewScale();
		}
		else if (nodeName == "ViewCentralPoint")
		{
			this->currentTag = Tag_t::configurationViewCentralPoint;
			this->parseConfigurationViewCentralPoint();
		}
		else
		{
			this->unexpectedTagLevel++;

			this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing configuration."));
			this->addIssue("    " + tr("Expected") + " \"Scale\" " + tr("or") + " \"ViewCentralPoint\", " + tr("got") + " \"" + nodeName + "\".");
			this->addIssue("    " + tr("Node ignored."));
		}
		break;
	case Tag_t::variables:
		if (nodeName == "Input")
		{
			this->currentTag = Tag_t::variablesInput;
			this->parseVariableNode();
		}
		else if (nodeName == "Output")
		{
			this->currentTag = Tag_t::variablesOutput;
			this->parseVariableNode();
		}
		else if (nodeName == "Variable")
		{
			this->currentTag = Tag_t::variablesInternal;
			this->parseVariableNode();
		}
		else if (nodeName == "Constant")
		{
			this->currentTag = Tag_t::variablesConstant;
			this->parseVariableNode();
		}
		else
		{
			this->unexpectedTagLevel++;

			this->addIssue(tr("Error!") + " " + tr("Unexpected variable nature encountered while parsing variable list."));
			this->addIssue("    " + tr("Expected") + " \"Input\", \"Output\", \"Variable\" " + tr("or") + " \"Constant\", " + tr("got") + " \"" + nodeName + "\".");
			this->addIssue("    " + tr("Variable ignored."));
		}
		break;
	case Tag_t::submachineTag:
		this->parseSubmachineStartElement();
		break;
	case Tag_t::configurationViewScale:
	case Tag_t::configurationViewCentralPoint:
	case Tag_t::variablesInput:
	case Tag_t::variablesOutput:
	case Tag_t::variablesInternal:
	case Tag_t::variablesConstant:
		this->unexpectedTagLevel++;

		this->addIssue(tr("Error!") + " " + tr("Unexpected node found in a node that doesn't accept subnodes."));
		this->addIssue("    " + tr("Found node was:") + " \"" + nodeName + "\".");
		this->addIssue("    " + tr("Node ignored."));
		break;
	}
}

void MachineXmlParser::parseEndElement()
{
	if (this->unexpectedTagLevel != 0)
	{
		this->unexpectedTagLevel--;
		return;
	}


	switch (this->currentTag)
	{
	case Tag_t::machine:
		this->currentTag = Tag_t::none;
		break;
	case Tag_t::configuration:
		this->currentTag = Tag_t::machine;
		break;
	case Tag_t::configurationViewScale:
		this->currentTag = Tag_t::configuration;
		break;
	case Tag_t::configurationViewCentralPoint:
		this->currentTag = Tag_t::configuration;
		break;
	case Tag_t::variables:
		this->currentTag = Tag_t::machine;
		break;
	case Tag_t::variablesInput:
		this->currentTag = Tag_t::variables;
		break;
	case Tag_t::variablesOutput:
		this->currentTag = Tag_t::variables;
		break;
	case Tag_t::variablesInternal:
		this->currentTag = Tag_t::variables;
		break;
	case Tag_t::variablesConstant:
		this->currentTag = Tag_t::variables;
		break;
	case Tag_t::submachineTag:
	{
		auto isSubmachineEnd =this->parseSubmachineEndElement();

		if (isSubmachineEnd == IsSubmachineEnd_t::yes)
		{
			this->currentTag = Tag_t::machine;
		}
		break;
	}
	case Tag_t::none:
		break;
	}
}

void MachineXmlParser::parseMachineName()
{
	QString nameAttribute = this->getCurrentNodeStringAttribute("Name");
	if (nameAttribute.isNull() == false)
	{
		this->machine->setName(nameAttribute);
	}
	else if ( (this->file != nullptr) && (this->file->fileName().isNull() == false) )
	{
		auto machineName = this->file->fileName();
		machineName = machineName.section("/", -1, -1);             // Extract file name from path
		machineName.remove("." + machineName.section(".", -1, -1)); // Remove extension
		this->machine->setName(machineName);

		this->addIssue(tr("Info:") + " " + tr("No name was found for the machine."));
		this->addIssue("    " + tr("Used file name to name machine:") + " \"" + machineName + "\".");
	}
	else // In case we still have no name
	{
		this->machine->setName(tr("Machine"));

		this->addIssue(tr("Info:") + " " + tr("No name was found for the machine."));
		this->addIssue("    " + tr("Name defaulted to:") + " \"" + tr("Machine") + "\".");
	}
}

void MachineXmlParser::parseConfigurationViewScale()
{
	bool ok;
	float level = this->getCurrentNodeFloatAttribute("Value", &ok);
	if (ok == false)
	{
		this->addIssue(tr("Info:") + " " + tr("Unable to parse zoom level."));
		this->addIssue("    " + tr("Found value was:") + " \"" + this->getCurrentNodeStringAttribute("Value") + "\".");

		return;
	}

	this->viewConfiguration->zoomLevel = level;
}

void MachineXmlParser::parseConfigurationViewCentralPoint()
{
	bool parseOk = true;
	bool ok;
	float x = this->getCurrentNodeFloatAttribute("X", &ok);
	if (ok == false)
	{
		parseOk = false;
	}

	float y = this->getCurrentNodeFloatAttribute("Y", &ok);
	if (ok == false)
	{
		parseOk = false;
	}

	if (parseOk == false)
	{
		this->addIssue(tr("Info:") + " " + tr("Unable to parse view position."));
		this->addIssue("    " + tr("Found values were:") + " (x=" + this->getCurrentNodeStringAttribute("X") + ";y=" + this->getCurrentNodeStringAttribute("Y") +")" );

		return;
	}

	this->viewConfiguration->viewCenter = QPointF(x,y);
}

void MachineXmlParser::parseVariableNode()
{
	// Get name
	QString variableName = this->getCurrentNodeStringAttribute("Name");
	if (variableName.isNull())
	{
		this->addIssue(tr("Error!") + " " + tr("Name missing for a variable."));
		this->addIssue("    " + tr("Variable ignored."));

		return;
	}

	// Get nature
	VariableNature_t nature;
	switch (this->currentTag)
	{
	case Tag_t::variablesInput:
		nature = VariableNature_t::input;
		break;
	case Tag_t::variablesOutput:
		nature = VariableNature_t::output;
		break;
	case Tag_t::variablesInternal:
		nature = VariableNature_t::internal;
		break;
	case Tag_t::variablesConstant:
		nature = VariableNature_t::constant;
		break;
	default:
		this->addIssue(tr("Error!") + " XKCD 2200.");

		return;
	}

	// Get ID if it is defined
	auto extractedId = this->getCurrentNodeIdAttribute();

	// Create variable
	auto variableId = machine->addVariable(nature, variableName, extractedId);

	// Check if variable was successfully added
	auto variable = machine->getVariable(variableId);
	if (variable == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("The variable named") + " \"" + variableName + "\" " + tr("couldn't be added."));
		this->addIssue("    " + tr("This may be due to a duplicated name."));
		this->addIssue("    " + tr("Variable ignored."));

		return;
	}

	QString actualVariableName = variable->getName();
	if (actualVariableName != variableName)
	{
		this->addIssue(tr("Warning:") + " " + tr("The variable named") + " \"" + variableName + "\" " + tr("in save file was added under name") + " \"" + actualVariableName + "\".");
		this->addIssue("    " + tr("This can be due to an ill-formed name."));
		this->addIssue("    " + tr("This may trigger further errors if other components were referencing this variable."));
	}

	// Get size
	bool ok;
	uint size = this->getCurrentNodeUintAttribute("Size", &ok);
	if (ok == true)
	{
		if (size != 1)
		{
			variable->resize(size);

			uint variableNewSize = variable->getSize();
			if (size != variableNewSize)
			{
				this->addIssue(tr("Warning:") + " " + tr("Unable to resize variable") + " \"" + variableName + "\".");
				this->addIssue("    " + tr("Requested size was:") + " " + QString::number(size) + ".");
				this->addIssue("    " + tr("Variable size ignored and defaulted to") + " " + QString::number(variable->getSize()) + ".");
			}
		}
	}
	else
	{
		this->addIssue(tr("Warning:") + " " + tr("Unable to extract variable size for variable") + " \"" + variableName + "\".");
		this->addIssue("    " + tr("Variable size defaulted to") + " " + QString::number(variable->getSize()) + ".");
	}

	// Get value
	QString variableValueStr = this->getCurrentNodeStringAttribute("Initial_value");
	if (variableValueStr.isEmpty() == false)
	{
		auto initialValue = LogicValue::fromString(variableValueStr);

		if (initialValue.isNull() == false)
		{
			variable->setInitialValue(initialValue);
		}
		else // (initialValue.isNull() == true)
		{
			this->addIssue(tr("Warning:") + " " + tr("The extracted initial value for variable") + " \"" + actualVariableName + "\" " + tr("was incorrect."));
			this->addIssue("    " + tr("Requested initial value was") + " \"" + variableValueStr + "\".");
			this->addIssue("    " + tr("Initial value ignored and defaulted to") + " \"" + variable->getInitialValue().toString() + "\".");
		}
	}
	else // (variableValueStr.isEmpty() == true)
	{
		this->addIssue(tr("Warning:") + " " + tr("No value was defined for variable:") + " \"" + actualVariableName + "\".");
		this->addIssue("    " + tr("Value defaulted to") + " \"" + variable->getInitialValue().toString() + "\".");
	}
}

shared_ptr<Variable> MachineXmlParser::getVariableByName(const QString& variableName) const
{
	shared_ptr<Variable> variable;
	for (auto& variableId : this->machine->getAllVariablesIds())
	{
		auto currentVariable = this->machine->getVariable(variableId);
		if (currentVariable == nullptr) continue;


		if (currentVariable->getName() == variableName)
		{
			variable = currentVariable;
		}
	}

	return variable;
}
