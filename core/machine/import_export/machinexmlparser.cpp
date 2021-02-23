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
#include "machinexmlparser.h"

// Qt classes
#include <QXmlStreamReader>

// StateS classes
#include "statesexception.h"
#include "statesxmlanalyzer.h"
#include "fsmxmlparser.h"
#include "machine.h"
#include "StateS_signal.h"
#include "viewconfiguration.h"
#include "actiononsignal.h"
#include "machineactuatorcomponent.h"
#include "equation.h"


MachineXmlParser::MachineXmlParser()
{
	this->viewConfiguration = shared_ptr<ViewConfiguration>(new ViewConfiguration());
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
			this->warnings.append(tr("Warning.") + " " + tr("Unable to parse zoom level.") + tr("Found value was:") + " " + attributes.value("Value"));
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
			this->warnings.append(tr("Warning.") + " " + tr("Unable to parse view position.") + tr("Found values were:") + " (x=" + attributes.value("X") + ";y=" + attributes.value("Y") +")" );
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
		signal = machine->addSignal(Machine::signal_type::Input, signalName);
	}
	else if (nodeName == "Output")
	{
		signal = machine->addSignal(Machine::signal_type::Output, signalName);
	}
	else if (nodeName == "Variable")
	{
		signal = machine->addSignal(Machine::signal_type::LocalVariable, signalName);
	}
	else if (nodeName == "Constant")
	{
		signal = machine->addSignal(Machine::signal_type::Constant, signalName);
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
				if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::resized_to_0) )
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
			if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
			{
				this->warnings.append(tr("Error!") + " " + "Unable to extract initial value of signal " + signalName + ".");
				this->warnings.append("    " + tr("Given initial value was") + " \"" + attributes.value("Initial_value") + "\".");
				this->warnings.append("    " + tr("Initial value ignored and defaulted to") + " \"" + QString::number(signal->getSize()) + "\".");
			}
			else if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::size_mismatch) )
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
		QStringRef signalName = attributes.value("Name");

		shared_ptr<Signal> signal;
		foreach (shared_ptr<Signal> var, this->machine->getWrittableSignals())
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

		ActionOnSignal::action_types actionType;
		QStringRef actionTypeText = attributes.value("Action_Type");

		if (actionTypeText == "Pulse")
		{
			actionType = ActionOnSignal::action_types::pulse;
		}
		else if (actionTypeText == "ActiveOnState")
		{
			actionType = ActionOnSignal::action_types::activeOnState;
		}
		else if (actionTypeText == "Set")
		{
			actionType = ActionOnSignal::action_types::set;
		}
		else if (actionTypeText == "Reset")
		{
			actionType = ActionOnSignal::action_types::reset;
		}
		else if (actionTypeText == "Assign")
		{
			actionType = ActionOnSignal::action_types::assign;
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
			if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_type) )
			{
				this->warnings.append(tr("Error in action type for signal") + " \"" + signalName + "\".");
				this->warnings.append("    " + tr("Default action type used instead."));
			}
			else
				throw;
		}

		QStringRef srangel = attributes.value("RangeL");
		QStringRef sranger = attributes.value("RangeR");
		QStringRef sactval = attributes.value("Action_Value");
		// For compatibility with previous saves
		if (srangel.isNull())
			srangel = attributes.value("Param1");
		if (sranger.isNull())
			sranger = attributes.value("Param2");

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
			if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_range) )
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
				actionValue = LogicValue::fromString(sactval.toString()); // Throws StatesException
			}
			catch (const StatesException& e)
			{
				if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
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
					if ( (e.getSourceClass() == "ActionOnSignal") && (e.getEnumValue() == ActionOnSignal::ActionOnSignalErrorEnum::illegal_value) )
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
		foreach (shared_ptr<Signal> var, this->machine->getReadableSignals())
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
		Equation::nature equationType;
		int rangeL = -1;
		int rangeR = -1;
		LogicValue constantValue;
		QStringRef srangel;
		QStringRef sranger;

		bool ok;
		int operandCount = attributes.value("OperandCount").toInt(&ok);

		if (ok == false)
		{
			// TODO
		}

		if (attributes.value("Nature") == "not")
			equationType = Equation::nature::notOp;
		else if (attributes.value("Nature") == "and")
			equationType = Equation::nature::andOp;
		else if (attributes.value("Nature") == "or")
			equationType = Equation::nature::orOp;
		else if (attributes.value("Nature") == "xor")
			equationType = Equation::nature::xorOp;
		else if (attributes.value("Nature") == "nand")
			equationType = Equation::nature::nandOp;
		else if (attributes.value("Nature") == "nor")
			equationType = Equation::nature::norOp;
		else if (attributes.value("Nature") == "xnor")
			equationType = Equation::nature::xnorOp;
		else if (attributes.value("Nature") == "equals")
			equationType = Equation::nature::equalOp;
		else if (attributes.value("Nature") == "differs")
			equationType = Equation::nature::diffOp;
		else if (attributes.value("Nature") == "concatenate")
			equationType = Equation::nature::concatOp;
		else if (attributes.value("Nature") == "extract")
		{
			equationType = Equation::nature::extractOp;

			srangel = attributes.value("RangeL");
			sranger = attributes.value("RangeR");
			// For compatibility with previous saves
			if (srangel.isNull())
				srangel = attributes.value("Param1");
			if (sranger.isNull())
				sranger = attributes.value("Param2");

			rangeL = srangel.toInt();
			rangeR = sranger.toInt();
		}
		else if (attributes.value("Nature") == "constant")
		{
			equationType = Equation::nature::constant;

			try
			{
				constantValue = LogicValue::fromString(attributes.value("Value").toString()); // Throws StatesException
			}
			catch (const StatesException& e)
			{
				if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
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
			this->warnings.append("    " + tr("Equation nature was:") + " \"" + attributes.value("Nature") + "\".");
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

		if (equationType == Equation::nature::constant)
		{
			newEquation->setConstantValue(constantValue); // Throws StatesException - constantValue is built for signal size - ignored
		}
		else if (equationType == Equation::nature::extractOp)
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

shared_ptr<MachineXmlParser> MachineXmlParser::buildStringParser(const QString& xmlString)
{
	shared_ptr<MachineXmlParser> machineParser;

	shared_ptr<StateSXmlAnalyzer> analyzer(new StateSXmlAnalyzer(xmlString));

	if (analyzer->getMachineType() == StateSXmlAnalyzer::machineType::Fsm)
	{
		machineParser = shared_ptr<FsmXmlParser>(new FsmXmlParser(xmlString));
	}

	return machineParser;
}

shared_ptr<MachineXmlParser> MachineXmlParser::buildFileParser(shared_ptr<QFile> file)
{
	shared_ptr<MachineXmlParser> machineParser;

	shared_ptr<StateSXmlAnalyzer> analyzer(new StateSXmlAnalyzer(file));

	if (analyzer->getMachineType() == StateSXmlAnalyzer::machineType::Fsm)
	{
		machineParser = shared_ptr<FsmXmlParser>(new FsmXmlParser(file));
	}

	return machineParser;
}

void MachineXmlParser::doParse()
{
	this->buildMachineFromXml();
}

shared_ptr<Machine> MachineXmlParser::getMachine()
{
	return this->machine;
}

shared_ptr<ViewConfiguration> MachineXmlParser::getViewConfiguration()
{
	return this->viewConfiguration;
}

QList<QString> MachineXmlParser::getWarnings()
{
	return this->warnings;
}
