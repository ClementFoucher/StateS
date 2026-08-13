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
#include "machinexmlparser.h"

// Qt
#include <QXmlStreamReader>
#include <QFile>

// StateS
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
	auto variableName = this->getCurrentNodeStringAttribute("Name");

	auto variable = this->getVariableByName(variableName);
	if (variable == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("Reference to undeclared variable encountered while parsing action list."));
		this->addIssue("    " + tr("Variable name was") + " \"" + variableName + "\".");
		this->addIssue("    " + tr("Action ignored."));

		return;
	}

	// Get action type
	ActionOnVariable::Type_t actionType;
	auto actionTypeText = this->getCurrentNodeStringAttribute("ActionType");

	if (actionTypeText == "Pulse")
	{
		actionType = ActionOnVariable::Type_t::pulse;
	}
	else if (actionTypeText == "ActiveOnState")
	{
		actionType = ActionOnVariable::Type_t::continuous;
	}
	else if (actionTypeText == "Set")
	{
		actionType = ActionOnVariable::Type_t::set;
	}
	else if (actionTypeText == "Reset")
	{
		actionType = ActionOnVariable::Type_t::reset;
	}
	else if (actionTypeText == "Assign")
	{
		actionType = ActionOnVariable::Type_t::assign;
	}
	else if (actionTypeText == "Increment")
	{
		actionType = ActionOnVariable::Type_t::increment;
	}
	else if (actionTypeText == "Decrement")
	{
		actionType = ActionOnVariable::Type_t::decrement;
	}
	else
	{
		this->addIssue(tr("Error!") + " " + tr("Unexpected action type encountered while parsing action list."));
		this->addIssue("    " + tr("Action type was") + " \"" + actionTypeText + "\".");
		this->addIssue("    " + tr("Action ignored."));

		return;
	}

	// Get action range
	auto srangel = this->getCurrentNodeStringAttribute("RangeL");
	auto sranger = this->getCurrentNodeStringAttribute("RangeR");

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

	// Get action value type
	auto actionValueType = MachineValue::Type_t::nullType;

	auto sactvaltype = this->getCurrentNodeStringAttribute("ActionValueType");
	if (sactvaltype.isEmpty() == false)
	{
		if (sactvaltype == "Boolean")
		{
			actionValueType = MachineValue::Type_t::boolean;
		}
		else if (sactvaltype == "BitVector")
		{
			actionValueType = MachineValue::Type_t::bitVector;
		}
	}

	// Get action value
	MachineValue actionValue{};

	if (actionValueType != MachineValue::Type_t::nullType)
	{
		auto sactval = this->getCurrentNodeStringAttribute("ActionValue");
		if (sactval.isEmpty() == false)
		{
			switch (actionValueType)
			{
			case MachineValue::Type_t::boolean:
				actionValue = BooleanValue::fromRawString(sactval);
				break;
			case MachineValue::Type_t::bitVector:
				actionValue = BitVectorValue::fromRawString(sactval);
				break;
			case MachineValue::Type_t::nullType:
				// Value is not null (checked before): should not happen
				break;
			}
		}
	}

	// Build action
	auto action = make_shared<ActionOnVariable>(variable, currentActuator->getAllowedActionTypes(), actionType, actionValue, rangeL, rangeR);
	currentActuator->addAction(action, variable);
}

void MachineXmlParser::parseLogicEquationNode()
{
	Equation::Operator_t operatorType;
	int rangeL = -1;
	int rangeR = -1;

	bool ok;
	int operandCount = this->getCurrentNodeIntAttribute("OperandCount", &ok);

	if (ok == false)
	{
		// TODO: warning in some cases

		operandCount = -1;
	}

	auto valueOperator = this->getCurrentNodeStringAttribute("Operator");
	if (valueOperator == "not")
	{
		operatorType = Equation::Operator_t::notOp;
	}
	else if (valueOperator == "and")
	{
		operatorType = Equation::Operator_t::andOp;
	}
	else if (valueOperator == "or")
	{
		operatorType = Equation::Operator_t::orOp;
	}
	else if (valueOperator == "xor")
	{
		operatorType = Equation::Operator_t::xorOp;
	}
	else if (valueOperator == "nand")
	{
		operatorType = Equation::Operator_t::nandOp;
	}
	else if (valueOperator == "nor")
	{
		operatorType = Equation::Operator_t::norOp;
	}
	else if (valueOperator == "xnor")
	{
		operatorType = Equation::Operator_t::xnorOp;
	}
	else if (valueOperator == "equals")
	{
		operatorType = Equation::Operator_t::equalOp;
	}
	else if (valueOperator == "differs")
	{
		operatorType = Equation::Operator_t::diffOp;
	}
	else if (valueOperator == "concatenate")
	{
		operatorType = Equation::Operator_t::concatOp;
	}
	else if (valueOperator == "identity")
	{
		operatorType = Equation::Operator_t::identity;
	}
	else if (valueOperator == "extract")
	{
		operatorType = Equation::Operator_t::extractOp;

		auto srangel = this->getCurrentNodeStringAttribute("RangeL");
		auto sranger = this->getCurrentNodeStringAttribute("RangeR");

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

	if (operatorType == Equation::Operator_t::extractOp)
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

	auto operandSource = this->getCurrentNodeStringAttribute("Source");
	if ( (operandSource != "Equation") &&
	     (operandSource != "Variable") &&
	     (operandSource != "Constant") )
	{
		this->addIssue(tr("Error!") + " " + tr("Unable to parse operand source for an equation."));
		this->addIssue("    " + tr("Expected") + " \"Equation\", \"Variable\" " + tr("or") + "\"Constant\"" + tr("got") + " \"" + operandSource + "\".");
		this->addIssue("    " + tr("Operand will be ignored."));

		return;
	}


	bool ok;
	uint operandRank = this->getCurrentNodeUintAttribute("Rank", &ok);
	if (ok == false)
	{
		this->addIssue(tr("Warning:") + " " + tr("Unable to parse operand rank for an equation."));
		this->addIssue("    " + tr("Operand will be placed in position 0, potentially erasing existing operand."));
		operandRank = 0;
	}

	this->operandRankStack.push(operandRank);

	// Nothing to do now if operand is an equation: it will
	// be parsed when the LogicEquation tag is encountered
	if (operandSource == "Variable")
	{
		this->parseOperandVariableNode();
	}
	else if (operandSource == "Constant")
	{
		this->parseOperandConstantNode();
	}
}

void MachineXmlParser::parseOperandVariableNode()
{
	if (this->equationStack.isEmpty() == true)
	{
		this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"Operand\" " + tr("node due to previous errors") + ".");
		return;
	}

	if (this->operandRankStack.isEmpty() == true)
	{
		this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"Operand\" " + tr("node due to previous errors") + ".");
		return;
	}


	auto variableName = this->getCurrentNodeStringAttribute("Name");

	auto variable = this->getVariableByName(variableName);
	if (variable == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("Reference to undeclared variable encountered while parsing an equation."));
		this->addIssue("    " + tr("Variable name was") + " \"" + variableName + "\".");
		this->addIssue("    " + tr("Operand ignored."));

		return;
	}

	auto parentEquation = this->equationStack.top();
	parentEquation->setOperand(this->operandRankStack.pop(), variable);
}

void MachineXmlParser::parseOperandConstantNode()
{
	if (this->equationStack.isEmpty() == true)
	{
		this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"Operand\" " + tr("node due to previous errors") + ".");
		return;
	}

	if (this->operandRankStack.isEmpty() == true)
	{
		this->addIssue(tr("Info:") + " " + tr("Ignoring") + " \"Operand\" " + tr("node due to previous errors") + ".");
		return;
	}


	auto typeStr = this->getCurrentNodeStringAttribute("Type");
	auto valueStr = this->getCurrentNodeStringAttribute("Value");
	MachineValue constantValue{};
	if (typeStr == "Boolean")
	{
		constantValue = BooleanValue::fromRawString(valueStr);
	}
	else if (typeStr == "BitVector")
	{
		constantValue = BitVectorValue::fromRawString(valueStr);
	}

	if (constantValue.isNull() == false)
	{
		auto parentEquation = this->equationStack.top();
		parentEquation->setOperand(this->operandRankStack.pop(), constantValue);
	}
	else // (constantValue.isNull() == true)
	{
		this->operandRankStack.pop();

		this->addIssue(tr("Warning:") + " " + tr("Error in constant value while parsing equation."));
		this->addIssue("    " + tr("Value") + " \"" + valueStr + "\" " + tr("of type") + " \"" + typeStr + "\" " + tr("couldn't be parsed."));
		this->addIssue("    " + tr("Value ignored, the operand will be left empty."));
	}
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

bool MachineXmlParser::getCurrentNodeBoolAttribute(const QString& name) const
{
	auto stringValue = this->getCurrentNodeStringAttribute(name);
	if (stringValue.isEmpty()) return false;

	if (stringValue == "true")
	{
		return true;
	}
	else
	{
		return false;
	}
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
		if (nodeName == "StateS")
		{
			this->currentTag = Tag_t::states;
			// Nothing to do here: version is checked ahead
		}
		else if (nodeName == "Machine")
		{
			// In string representation (for undo commands), StateS node is omitted
			this->currentTag = Tag_t::machine;
			this->parseMachineName();
		}
		else
		{
			this->unexpectedTagLevel++;

			this->addIssue(tr("Error!") + " " + tr("Unexpected root node.") + " " + tr("Root node should be") + " \"StateS\", " + tr("but found") + " " + nodeName);
		}
		break;
	case Tag_t::states:
		if (nodeName == "Configuration")
		{
			this->currentTag = Tag_t::configuration;
		}
		else if (nodeName == "Machine")
		{
			this->currentTag = Tag_t::machine;
			this->parseMachineName();
		}
		else
		{
			this->unexpectedTagLevel++;

			this->addIssue(tr("Warning.") + " " + tr("Unexpected node found within \"StateS\" node."));
			this->addIssue("    " + tr("Expected") + " \"Configuration\" " + tr("or") + " \"Machine\", " + tr("got") + " \"" + nodeName + "\".");
			this->addIssue("    " + tr("Node ignored."));
		}
		break;
	case Tag_t::configuration:
		if (nodeName == "ViewScale")
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
			this->addIssue("    " + tr("Expected") + " \"ViewScale\" " + tr("or") + " \"ViewCentralPoint\", " + tr("got") + " \"" + nodeName + "\".");
			this->addIssue("    " + tr("Node ignored."));
		}
		break;
	case Tag_t::machine:
		if (nodeName == "Variables")
		{
			this->currentTag = Tag_t::variables;
		}
		else
		{
			this->currentTag = Tag_t::submachineTag;
			this->parseSubmachineStartElement();
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
		else if (nodeName == "Internal")
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
			this->addIssue("    " + tr("Expected") + " \"Input\", \"Output\", \"Internal\" " + tr("or") + " \"Constant\", " + tr("got") + " \"" + nodeName + "\".");
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
	case Tag_t::none:
		// The end
		break;
	case Tag_t::states:
		this->currentTag = Tag_t::none;
		break;
	case Tag_t::configuration:
		this->currentTag = Tag_t::states;
		break;
	case Tag_t::machine:
		this->currentTag = Tag_t::states;
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
		auto isSubmachineEnd = this->parseSubmachineEndElement();

		if (isSubmachineEnd == IsSubmachineEnd_t::yes)
		{
			this->currentTag = Tag_t::machine;
		}
		break;
	}
	}
}

void MachineXmlParser::parseMachineName()
{
	auto nameAttribute = this->getCurrentNodeStringAttribute("Name");
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
	auto variableName = this->getCurrentNodeStringAttribute("Name");
	if (variableName.isNull())
	{
		this->addIssue(tr("Error!") + " " + tr("Name missing for a variable."));
		this->addIssue("    " + tr("Variable ignored."));

		return;
	}

	// Get nature
	Machine::VariableNature_t nature;
	switch (this->currentTag)
	{
	case Tag_t::variablesInput:
		nature = Machine::VariableNature_t::input;
		break;
	case Tag_t::variablesOutput:
		nature = Machine::VariableNature_t::output;
		break;
	case Tag_t::variablesInternal:
		nature = Machine::VariableNature_t::internal;
		break;
	case Tag_t::variablesConstant:
		nature = Machine::VariableNature_t::constant;
		break;
	default:
		this->addIssue(tr("Error!") + " XKCD 2200.");

		return;
	}

	// Get ID if it is defined
	auto extractedId = this->getCurrentNodeIdAttribute();

	// Get type
	auto typeStr = this->getCurrentNodeStringAttribute("Type");
	auto variableType = MachineValue::Type_t::boolean; // Default to boolean
	if (typeStr.isNull() == false)
	{
		if (typeStr == "Boolean")
		{
			variableType = MachineValue::Type_t::boolean;
		}
		else if (typeStr == "BitVector")
		{
			variableType = MachineValue::Type_t::bitVector;
		}
		else
		{
			this->addIssue(tr("Error!") + " " + tr("Unknown type") + " \"" + typeStr + "\" " + tr("found for variable while parsing variable list."));
			this->addIssue("    " + tr("Defaulting to Boolean type."));
			this->addIssue("    " + tr("Variable name was:") + " " + variableName + ".");
		}
	}
	else // (typeStr.isNull() == true)
	{
		this->addIssue(tr("Error!") + " " + tr("No type found for variable while parsing variable list."));
		this->addIssue("    " + tr("Defaulting to Boolean type."));
		this->addIssue("    " + tr("Variable name was:") + " " + variableName + ".");
	}

	// Create variable
	auto variableId = machine->addVariable(nature, variableName, variableType, extractedId);

	// Check if variable was successfully added
	auto variable = machine->getVariable(variableId);
	if (variable == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("The variable named") + " \"" + variableName + "\" " + tr("couldn't be added."));
		this->addIssue("    " + tr("This may be due to a duplicated name."));
		this->addIssue("    " + tr("Variable ignored."));

		return;
	}

	auto actualVariableName = variable->getName();
	if (actualVariableName != variableName)
	{
		this->addIssue(tr("Warning:") + " " + tr("The variable named") + " \"" + variableName + "\" " + tr("in save file was added under name") + " \"" + actualVariableName + "\".");
		this->addIssue("    " + tr("This can be due to an ill-formed name."));
		this->addIssue("    " + tr("This may trigger further errors if other components were referencing this variable."));
	}

	// Get memorized attribute
	auto memorized = this->getCurrentNodeBoolAttribute("Memorized");
	if (memorized == true)
	{
		variable->setMemorized(true);
	}

	// Get value
	auto variableValueStr = this->getCurrentNodeStringAttribute("Value");
	if (variableValueStr.isEmpty() == false)
	{
		switch (variableType)
		{
		case MachineValue::Type_t::boolean:
			variable->setInitialValue(BooleanValue::fromRawString(variableValueStr));
			break;
		case MachineValue::Type_t::bitVector:
			variable->setInitialValue(BitVectorValue::fromRawString(variableValueStr));
			break;
		case MachineValue::Type_t::nullType:
			// Should not happen: defaulted to boolean
			break;
		}
	}
	else // (variableValueStr.isEmpty() == true)
	{
		this->addIssue(tr("Warning:") + " " + tr("No initial value was defined for variable:") + " \"" + actualVariableName + "\".");
		this->addIssue("    " + tr("Initial value defaulted to") + " \"" + variable->getInitialValue().toDisplayString() + "\".");
	}
}

shared_ptr<Variable> MachineXmlParser::getVariableByName(const QString& variableName) const
{
	for (auto& variableId : this->machine->getAllVariablesIds())
	{
		auto currentVariable = this->machine->getVariable(variableId);
		if (currentVariable == nullptr) continue;


		if (currentVariable->getName() == variableName)
		{
			return currentVariable;
		}
	}

	return nullptr;
}
