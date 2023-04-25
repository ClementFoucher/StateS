/*
 * Copyright © 2017-2023 Clément Foucher
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
#include "statesexception.h"
#include "machine.h"
#include "StateS_signal.h"
#include "viewconfiguration.h"
#include "graphicattributes.h"
#include "actiononsignal.h"
#include "machineactuatorcomponent.h"
#include "equation.h"
#include "exceptiontypes.h"


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

void MachineXmlParser::parseSignal()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	// Get name
	QString signalName = attributes.value("Name").toString();
	if (signalName.isNull())
	{
		this->warnings.append(tr("Error!") + " " + tr("Unnamed signal encountered while parsing signal list: unable to extract name."));
		this->warnings.append("    " + tr("Signal ignored."));

		return;
	}

	// Get type
	shared_ptr<Signal> signal;
	if (nodeName == "Input")
	{
		signal = machine->addSignal(SignalType_t::Input, signalName);
	}
	else if (nodeName == "Output")
	{
		signal = machine->addSignal(SignalType_t::Output, signalName);
	}
	else if (nodeName == "Variable")
	{
		signal = machine->addSignal(SignalType_t::LocalVariable, signalName);
	}
	else if (nodeName == "Constant")
	{
		signal = machine->addSignal(SignalType_t::Constant, signalName);
	}
	else
	{
		this->warnings.append(tr("Error!") + " " + tr("Unexpected signal type encountered while parsing signal list:"));
		this->warnings.append("    " + tr("Expected") + " \"Input\", \"Output\", \"Variable\" " + tr("or") + " \"Constant\", " + tr("got") + " \"" + nodeName + "\".");
		this->warnings.append("    " + tr("Signal name was:") + " " + signalName + ".");
		this->warnings.append("    " + tr("Signal ignored."));

		return;
	}

	// Get size
	bool ok;
	uint size = attributes.value("Size").toUInt(&ok);

	if (ok == true)
	{
		if (size != 1)
		{
			try
			{
				machine->resizeSignal(signalName, size); // Throws StatesException (Signal, Equation and Machine)
				// Equation: ignored, this is not an equation,
				// Machine: ignored, we just created the signal,
				// Only Signal has to be handled
			}
			catch (const StatesException& e)
			{
				if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == SignalError_t::signal_resized_to_0) )
				{
					this->warnings.append(tr("Unable to resize signal") + " \"" + signalName + "\".");
					this->warnings.append("    " + tr("Signal size ignored and defaulted to") + " \"1\".");

					return;
				}
				else
					throw;
			}
		}
	}
	else
	{
		this->warnings.append(tr("Error!") + " " + tr("Unable to extract signal size for signal") + " \"" + signalName + "\".");
		this->warnings.append("    " + tr("Signal size ignored and defaulted to") + " \"1\".");

		return;
	}

	// Get initial value
	if (nodeName != "Output")
	{
		try
		{
			LogicValue initialValue = LogicValue::fromString(attributes.value("Initial_value").toString()); // Throws StatesException

			this->machine->changeSignalInitialValue(signalName, initialValue); // Throws StatesException
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueError_t::unsupported_char) )
			{
				this->warnings.append(tr("Error!") + " " + "Unable to extract initial value of signal " + signalName + ".");
				this->warnings.append("    " + tr("Given initial value was") + " \"" + attributes.value("Initial_value").toString() + "\".");
				this->warnings.append("    " + tr("Initial value ignored and defaulted to") + " \"" + QString::number(signal->getSize()) + "\".");
			}
			else if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == SignalError_t::size_mismatch) )
			{
				this->warnings.append("Error in initial value of signal " + signalName + ".");
				this->warnings.append("    " + tr("The initial value size does not match signal size."));
				this->warnings.append("    " + tr("Initial value ignored and defaulted to") + " \"" + QString::number(signal->getSize()) + "\".");
			}
			else
				throw;
		}
	}
}

void MachineXmlParser::parseAction()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	if (nodeName == "Action")
	{
		QString signalName = attributes.value("Name").toString();

		shared_ptr<Signal> signal;
		for (shared_ptr<Signal> var : this->machine->getWrittableSignals())
		{
			if (var->getName() == signalName)
				signal = var;
		}

		if (signal == nullptr)
		{
			this->warnings.append(tr("Reference to undeclared signal encountered while parsing action list:"));
			this->warnings.append("    " + tr("Signal name was") + " \"" + signalName + "\".");
			this->warnings.append("    " + tr("Action ignored."));

			return;
		}

		ActionOnSignalType_t actionType;
		QString actionTypeText = attributes.value("Action_Type").toString();

		if (actionTypeText == "Pulse")
		{
			actionType = ActionOnSignalType_t::pulse;
		}
		else if (actionTypeText == "ActiveOnState")
		{
			actionType = ActionOnSignalType_t::activeOnState;
		}
		else if (actionTypeText == "Set")
		{
			actionType = ActionOnSignalType_t::set;
		}
		else if (actionTypeText == "Reset")
		{
			actionType = ActionOnSignalType_t::reset;
		}
		else if (actionTypeText == "Assign")
		{
			actionType = ActionOnSignalType_t::assign;
		}
		else
		{
			this->warnings.append(tr("Unexpected action type encountered while parsing action list:"));
			this->warnings.append("    " + tr("Action type was") + " \"" + actionTypeText + "\".");
			this->warnings.append("    " + tr("Action ignored."));

			return;
		}

		shared_ptr<ActionOnSignal> action = this->currentActuator->addAction(signal);

		try
		{
			action->setActionType(actionType); // Throws StatesException
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignalError_t::illegal_type) )
			{
				this->warnings.append(tr("Error in action type for signal") + " \"" + signalName + "\".");
				this->warnings.append("    " + tr("Default action type used instead."));
			}
			else
				throw;
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
			rangeL = -1;

		if (! sranger.isEmpty())
		{
			rangeR = sranger.toInt();
		}
		else
			rangeR = -1;

		try
		{
			action->setActionRange(rangeL, rangeR);
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignalError_t::illegal_range) )
			{
				this->warnings.append(tr("Error in action range for signal") + " \"" + signalName + "\".");
				this->warnings.append("    " + tr("Range ignored. Default value will be ignored too if present."));

				return;
			}
			else
				throw;
		}

		if(! sactval.isEmpty())
		{
			LogicValue actionValue;
			try
			{
				actionValue = LogicValue::fromString(sactval); // Throws StatesException
			}
			catch (const StatesException& e)
			{
				if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueError_t::unsupported_char) )
				{
					uint avsize;
					if ( (rangeL != -1) && (rangeR == -1) )
						avsize = 1;
					else if ( (rangeL != -1) && (rangeR != -1) )
						avsize = rangeL - rangeR + 1;
					else
						avsize = 1; // TODO: determine actual size

					actionValue = LogicValue::getValue0(avsize);

					this->warnings.append(tr("Error in action value for signal") + " \"" + signalName + "\".");
					this->warnings.append("    " + tr("Value ignored and set to") + " \"" + actionValue.toString() + "\".");

					return;
				}
				else
					throw;
			}

			if (action->isActionValueEditable() == true)
			{
				try
				{
					action->setActionValue(actionValue);
				}
				catch (const StatesException& e)
				{
					if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignalError_t::illegal_value) )
					{
						this->warnings.append(tr("Error in action value for signal") + " \"" + signalName + "\".");
						this->warnings.append("    " + tr("Value ignored and set to") + " \"" + action->getActionValue().toString() + "\".");
					}
					else
						throw;
				}
			}
		}
	}
	else
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

	shared_ptr<Signal> equation;

	if (nodeName == "LogicVariable")
	{
		for (shared_ptr<Signal> var : this->machine->getReadableSignals())
		{
			if (var->getName() == attributes.value("Name"))
			{
				equation = var;
				break;
			}
		}
	}
	else if (nodeName == "LogicEquation")
	{
		EquationNature_t equationType;
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
			equationType = EquationNature_t::notOp;
		else if (valueNature == "and")
			equationType = EquationNature_t::andOp;
		else if (valueNature == "or")
			equationType = EquationNature_t::orOp;
		else if (valueNature == "xor")
			equationType = EquationNature_t::xorOp;
		else if (valueNature == "nand")
			equationType = EquationNature_t::nandOp;
		else if (valueNature == "nor")
			equationType = EquationNature_t::norOp;
		else if (valueNature == "xnor")
			equationType = EquationNature_t::xnorOp;
		else if (valueNature == "equals")
			equationType = EquationNature_t::equalOp;
		else if (valueNature == "differs")
			equationType = EquationNature_t::diffOp;
		else if (valueNature == "concatenate")
			equationType = EquationNature_t::concatOp;
		else if (valueNature == "extract")
		{
			equationType = EquationNature_t::extractOp;

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
			equationType = EquationNature_t::constant;

			try
			{
				constantValue = LogicValue::fromString(attributes.value("Value").toString()); // Throws StatesException
			}
			catch (const StatesException& e)
			{
				if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueError_t::unsupported_char) )
				{
					uint constantsize;
					if ( (rangeL != -1) && (rangeR == -1) )
						constantsize = 1;
					else if ( (rangeL != -1) && (rangeR != -1) )
						constantsize = rangeL - rangeR + 1;
					else
						constantsize = 1; // TODO: determine actual size

					constantValue = LogicValue::getValue0(constantsize);

					this->warnings.append(tr("Error in constant value while parsing equation:"));
					this->warnings.append("    " + tr("Value ignored and set to") + " \"" + constantValue.toString() + "\".");
				}
				else
					throw;
			}
		}
		else
		{
			this->warnings.append(tr("Unexpected equation nature encountered while parsing logic equation:"));
			this->warnings.append("    " + tr("Equation nature was:") + " \"" + valueNature + "\".");
			this->warnings.append("    " + tr("Token ignored. Will retry with other tokens if existing."));

			return;
		}

		shared_ptr<Equation> newEquation;
		if (operandCount != 0)
		{
			newEquation = shared_ptr<Equation>(new Equation(equationType, operandCount));
		}
		else
		{
			newEquation = shared_ptr<Equation>(new Equation(equationType));
		}

		if (equationType == EquationNature_t::constant)
		{
			newEquation->setConstantValue(constantValue); // Throws StatesException - constantValue is built for signal size - ignored
		}
		else if (equationType == EquationNature_t::extractOp)
		{
			newEquation->setRange(rangeL, rangeR);
		}

		equation = newEquation;
	}

	if (this->rootLogicEquation == nullptr)
	{
		this->rootLogicEquation = equation;
	}

	this->currentLogicEquation = equation;
}

void MachineXmlParser::treatBeginOperand(uint operandRank)
{
	this->equationStack.push(dynamic_pointer_cast<Equation>(this->currentLogicEquation));
	this->operandRankStack.push(operandRank);
}

void MachineXmlParser::treatEndOperand()
{
	shared_ptr<Equation> parentEquation = this->equationStack.pop();
	parentEquation->setOperand(this->operandRankStack.pop(), this->currentLogicEquation);
	this->currentLogicEquation = parentEquation;
}
