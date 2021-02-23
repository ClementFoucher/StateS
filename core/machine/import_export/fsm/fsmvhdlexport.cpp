/*
 * Copyright © 2014-2016 Clément Foucher
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
#include "fsmvhdlexport.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QDate>
#include <QFile>
#include <QTextStream>

// StateS classes
#include "states.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "input.h"
#include "output.h"
#include "equation.h"
#include "actiononsignal.h"


FsmVhdlExport::FsmVhdlExport(shared_ptr<Fsm> machine)
{
	this->machine = machine;
}

void FsmVhdlExport::setOptions(bool resetLogicPositive, bool prefixSignals)
{
	this->resetLogicPositive = resetLogicPositive;
	this->prefixSignals = prefixSignals;
}

bool FsmVhdlExport::writeToFile(const QString& path)
{
	shared_ptr<Fsm> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		QFile* file = new QFile(path);
		file->open(QIODevice::WriteOnly);

		QTextStream stream(file);

		this->generateVhdlCharacteristics(l_machine);

		this->writeHeader(stream);
		this->writeEntity(stream, l_machine);
		this->writeArchitecture(stream, l_machine);

		file->close();
		delete file;

		return true;
	}
	else
	{
		return false;
	}
}

shared_ptr<FsmVhdlExport::ExportCompatibility> FsmVhdlExport::checkCompatibility()
{
	shared_ptr<Fsm> l_machine = this->machine.lock();

	if (l_machine != nullptr)
	{
		shared_ptr<ExportCompatibility> compatibility(new ExportCompatibility());
		WrittableSignalCharacteristics charac;

		foreach (shared_ptr<Output> output, l_machine->getOutputs())
		{
			charac = this->determineWrittableSignalCharacteristics(l_machine, output, false);

			if (charac.isKeepValue && charac.isTempValue)
				compatibility->bothTempAndKeepValue.append(output);

			if (charac.isMoore && charac.isMealy)
				compatibility->bothMooreAndMealy.append(output);

			if (charac.isRangeAdressed)
				compatibility->rangeAdressed.append(output);

			if (charac.isMealy && charac.isKeepValue)
				compatibility->mealyWithKeep.append(output);
		}
		foreach (shared_ptr<Signal> variable, l_machine->getLocalVariables())
		{
			charac = this->determineWrittableSignalCharacteristics(l_machine, variable, false);

			if (charac.isKeepValue && charac.isTempValue)
				compatibility->bothTempAndKeepValue.append(variable);

			if (charac.isMoore && charac.isMealy)
				compatibility->bothMooreAndMealy.append(variable);

			if (charac.isRangeAdressed)
				compatibility->rangeAdressed.append(variable);

			if (charac.isMealy && charac.isKeepValue)
				compatibility->mealyWithKeep.append(variable);
		}

		return compatibility;
	}
	else
		return nullptr;
}

void FsmVhdlExport::generateVhdlCharacteristics(shared_ptr<Fsm> l_machine)
{
	// Machine
	this->machineVhdlName = this->cleanNameForVhdl(l_machine->getName());

	// States
	foreach(shared_ptr<FsmState> state, l_machine->getStates())
	{
		QString stateRadical = "S_";

		stateRadical += cleanNameForVhdl(state->getName());

		// Check for ducplicates
		int occurence = 2;
		QString stateName = stateRadical;
		while (this->stateVhdlName.values().contains(stateName))
		{
			stateName = stateRadical + QString::number(occurence);
			occurence++;
		}

		this->stateVhdlName[state] = stateName;
	}

	// Signals
	QString signalName;

	foreach (shared_ptr<Input> input, l_machine->getInputs())
	{
		signalName = this->generateSignalVhdlName("I_", input->getName());
		this->signalVhdlName[input] = signalName;
	}
	foreach (shared_ptr<Output> output, l_machine->getOutputs())
	{
		signalName = this->generateSignalVhdlName("O_", output->getName());
		this->determineWrittableSignalCharacteristics(l_machine, output, true);
		this->signalVhdlName[output] = signalName;
	}
	foreach (shared_ptr<Signal> variable, l_machine->getLocalVariables())
	{
		signalName = this->generateSignalVhdlName("SIG_", variable->getName());
		this->determineWrittableSignalCharacteristics(l_machine, variable, true);
		this->signalVhdlName[variable] = signalName;
	}
	foreach (shared_ptr<Signal> constant, l_machine->getConstants())
	{
		signalName = this->generateSignalVhdlName("CONST_", constant->getName());
		this->signalVhdlName[constant] = signalName;
	}
}

FsmVhdlExport::WrittableSignalCharacteristics FsmVhdlExport::determineWrittableSignalCharacteristics(shared_ptr<Fsm> l_machine, shared_ptr<Signal> signal, bool storeResults)
{
	WrittableSignalCharacteristics characteristics;

	bool doNotGenerate = !storeResults;

	foreach(shared_ptr<FsmState> state, l_machine->getStates())
	{
		foreach (shared_ptr<ActionOnSignal> action, state->getActions())
		{
			if (action->getSignalActedOn() == signal)
			{
				characteristics.isMoore = true;

				ActionOnSignal::action_types actionType = action->getActionType();

				switch (actionType)
				{
				case ActionOnSignal::action_types::assign:
				case ActionOnSignal::action_types::set:
				case ActionOnSignal::action_types::reset:
					characteristics.isKeepValue = true;
					break;
				case ActionOnSignal::action_types::activeOnState:
				case ActionOnSignal::action_types::pulse:
					characteristics.isTempValue = true;
					break;
				}

				if (action->getActionRangeL() != -1)
				{
					characteristics.isRangeAdressed = true;
				}
			}
		}
	}

	foreach(shared_ptr<FsmTransition> transition, l_machine->getTransitions())
	{
		foreach (shared_ptr<ActionOnSignal> action, transition->getActions())
		{
			if (action->getSignalActedOn() == signal)
			{
				characteristics.isMealy = true;

				ActionOnSignal::action_types actionType = action->getActionType();

				switch (actionType)
				{
				case ActionOnSignal::action_types::assign:
				case ActionOnSignal::action_types::set:
				case ActionOnSignal::action_types::reset:
					characteristics.isKeepValue = true;
					break;
				case ActionOnSignal::action_types::activeOnState:
				case ActionOnSignal::action_types::pulse:
					characteristics.isTempValue = true;
					break;
				}

				if (action->getActionRangeL() != -1)
				{
					characteristics.isRangeAdressed = true;
				}
			}
		}
	}

	if (characteristics.isMealy && characteristics.isMoore)
		doNotGenerate = true;
	else if (characteristics.isTempValue && characteristics.isKeepValue)
		doNotGenerate = true;
	else if (characteristics.isRangeAdressed)
		doNotGenerate = true;

	if (!doNotGenerate)
	{
		if (characteristics.isMoore)
			this->mooreSignals.append(signal);
		else if (characteristics.isMealy)
			this->mealySignals.append(signal);

		if (characteristics.isTempValue)
			this->tempValueSignals.append(signal);
		else if (characteristics.isKeepValue)
			this->keepValueSignals.append(signal);

//        if (signalIsRangeAdressed)
//            this->rangeAdressedSignals.append(signal);
	}

	return characteristics;
}

QString FsmVhdlExport::generateSignalVhdlName(const QString& prefix, const QString& name) const
{
	QString signalRadical;

	if (this->prefixSignals)
		signalRadical += prefix;

	signalRadical += cleanNameForVhdl(name);

	// Check for ducplicates
	int occurence = 2;
	QString signalName = signalRadical;
	while (this->signalVhdlName.values().contains(signalName))
	{
		signalName = signalRadical + QString::number(occurence);
		occurence++;
	}

	return signalName;
}

QString FsmVhdlExport::cleanNameForVhdl(const QString& name) const
{
	// TODO
	QString newName = name;

	newName.replace(" ", "_");
	newName.replace("#", "_");

	while (newName.contains("__"))
	{
		newName.replace("__", "_");
	}

	return newName;
}

void FsmVhdlExport::writeHeader(QTextStream& stream) const
{
	stream << "-- FSM generated with StateS v" << StateS::getVersion() << " on " << QDate::currentDate().toString() << " at " << QTime::currentTime().toString() << "\n";
	stream << "-- https://sourceforge.net/projects/states/\n\n";

	stream << "library IEEE;\n";
	stream << "use IEEE.std_logic_1164.all;\n\n\n";
}

void FsmVhdlExport::writeEntity(QTextStream& stream, shared_ptr<Fsm> l_machine) const
{
	stream << "entity " << this->machineVhdlName << " is\n";

	stream << "  port(clock : in std_logic;\n       reset : in std_logic;\n       ";


	QList<shared_ptr<Input>> inputs = l_machine->getInputs();

	foreach (shared_ptr<Input> input, inputs)
	{
		stream << this->signalVhdlName[input] << " : in std_logic";

		if (input->getSize() > 1)
		{
			stream << "_vector(";

			stream << QString::number(input->getSize() - 1) << " downto 0)";
		}

		stream << ";\n       ";
	}

	QList<shared_ptr<Output>> outputs = l_machine->getOutputs();

	foreach (shared_ptr<Output> output, outputs)
	{
		stream << this->signalVhdlName[output] << " : out std_logic";

		if (output->getSize() > 1)
		{
			stream << "_vector(";

			stream << QString::number(output->getSize() - 1) << " downto 0)";
		}

		if (output == outputs.last())
			stream << ");\n";
		else
			stream << ";\n       ";
	}


	stream << "end entity;\n\n\n";
}

void FsmVhdlExport::writeArchitecture(QTextStream& stream, shared_ptr<Fsm> l_machine) const
{
	stream << "architecture FSM_body of " << this->machineVhdlName << " is\n\n";

	stream << "  type state_type is (";

	foreach(QString stateName, this->stateVhdlName)
	{
		stream << stateName;

		if (!(stateName == this->stateVhdlName.last()))
			stream << ", ";
	}

	stream << ");\n\n";

	stream << "  signal current_state : state_type;\n";
	stream << "  signal next_state    : state_type;\n\n";

	//QList<shared_ptr<Input>> inputs = l_machine->getInputs();
	QList<shared_ptr<Signal>> localVars = l_machine->getLocalVariables();
	QList<shared_ptr<Signal>> constants = l_machine->getConstants();
	QList<shared_ptr<FsmState>> states = l_machine->getStates();

	foreach(shared_ptr<Signal> localVar, localVars)
	{
		stream << "  signal " + this->signalVhdlName[localVar] << " : std_logic";

		if (localVar->getSize() > 1)
		{
			stream << "_vector(";

			stream << QString::number(localVar->getSize() - 1) << " downto 0)";

			stream << " := \"" << localVar->getInitialValue().toString() << "\";\n";
		}
		else
		{
			stream << " := '" << localVar->getInitialValue().toString() << "';\n";
		}
	}

	stream << "\n";

	foreach(shared_ptr<Signal> constant, constants)
	{
		stream << "  constant " + this->signalVhdlName[constant] << " : std_logic";

		if (constant->getSize() > 1)
		{
			stream << "_vector(";

			stream << QString::number(constant->getSize() - 1) << " downto 0)";

			stream << " := \"" << constant->getInitialValue().toString() << "\";\n";
		}
		else
		{
			stream << " := '" << constant->getInitialValue().toString() << "';\n";
		}
	}

	stream << "\nbegin\n\n";

	// Next step computation : asynchronous process
	stream << "  compute_next_step : process(current_state,\n";

	this->writeAsynchronousProcessSensitivityList(stream, l_machine);

	stream << "  begin\n";

	stream << "    case current_state is\n";

	foreach(shared_ptr<FsmState> state, states)
	{
		stream << "    when ";
		stream << this->stateVhdlName[state];
		stream << " =>\n";

		QList<shared_ptr<FsmTransition>> transitions = state->getOutgoingTransitions();
		foreach (shared_ptr<FsmTransition> transition, transitions)
		{
			stream << "      ";

			if (transition != transitions.first())
				stream << "els";

			stream << "if ";

			shared_ptr<Signal> condition = transition->getCondition();
			stream << generateEquationText(condition, l_machine);

			// Add compare for single signal equations
			if ((condition != nullptr) && (dynamic_pointer_cast<Equation>(condition) == nullptr))
				stream << " = '1'";

			stream << " then\n";
			stream << "        next_state <= " << this->stateVhdlName[transition->getTarget()] << ";\n";
		}

		stream << "      end if;\n";

	}

	stream << "    end case;\n";

	stream << "  end process;\n\n";

	// Current state update : synchronous process

	stream << "  update_state : process(clock, reset)\n";
	stream << "  begin\n";
	stream << "    if reset='" << (resetLogicPositive?"1":"0") << "' then\n";
	stream << "      current_state <= " << this->stateVhdlName[l_machine->getInitialState()] << ";\n";
	stream << "    elsif rising_edge(clock) then\n";
	stream << "      current_state <= next_state;\n";
	stream << "    end if;\n";
	stream << "  end process;\n\n";


	// Output computation : asynchronous processes

	if (!this->mooreSignals.isEmpty())
		this->writeMooreOutputs(stream, l_machine);

	if (!this->mealySignals.isEmpty())
		this->writeMealyOutputs(stream, l_machine);

	// The end
	stream << "\nend architecture;\n";
}

void FsmVhdlExport::writeMooreOutputs(QTextStream& stream, shared_ptr<Fsm> l_machine) const
{
	stream << "  compute_moore : process(current_state)\n";
	stream << "  begin\n";

	foreach(shared_ptr<Signal> signal, this->mooreSignals)
	{
		if (this->tempValueSignals.contains(signal))
		{
			// Write default value for temp signals
			stream << "    " << this->signalVhdlName[signal] << " <= \"" << LogicValue::getValue0(signal->getSize()).toString() << "\";\n";
		}
	}
	stream << "    -- Signals handled in this process but not listed above this line implicitly maintain their value.\n";

	stream << "    case current_state is\n";

	foreach(shared_ptr<FsmState> state, l_machine->getStates())
	{
		stream << "    when ";
		stream << this->stateVhdlName[state];
		stream << " =>\n";

		int writtenActions = 0;
		foreach(shared_ptr<ActionOnSignal> action, state->getActions())
		{
			if (this->mooreSignals.contains(action->getSignalActedOn()))
			{
				writtenActions++;

				this->writeSignalAffectationValue(stream, action);
			}
		}

		if (writtenActions == 0)
		{
			stream << "      null;\n";
		}
	}

	stream << "    end case;\n";

	stream << "  end process;\n\n";
}

void FsmVhdlExport::writeMealyOutputs(QTextStream& stream, shared_ptr<Fsm> l_machine) const
{
	stream << "  -- Compute Mealy outputs\n";

	foreach(shared_ptr<Signal> signal, this->mealySignals)
	{
		if (this->tempValueSignals.contains(signal))
		{
			QList<shared_ptr<FsmTransition>> transitions;

			foreach(shared_ptr<FsmTransition> transition, l_machine->getTransitions())
			{
				foreach (shared_ptr<ActionOnSignal> action, transition->getActions())
				{
					if (action->getSignalActedOn() == signal)
					{
						transitions.append(transition);
						break;
					}
				}
			}

			if (!transitions.isEmpty())
			{
				stream << "  affect_" << this->signalVhdlName[signal] << ":";
				stream << this->signalVhdlName[signal] << " <= ";

				foreach(shared_ptr<FsmTransition> transition, transitions)
				{
					foreach (shared_ptr<ActionOnSignal> action, transition->getActions())
					{
						if (action->getSignalActedOn() == signal)
						{
							if (signal->getSize() > 1)
								stream << "\"" << action->getActionValue().toString() << "\"";
							else
								stream << "'1'";

							stream << " when (current_state = " << this->stateVhdlName[transition->getSource()] << ")";
							stream << " and " << this->generateEquationText(transition->getCondition(), l_machine);
							stream << " else\n    ";
						}
					}
				}

				stream << LogicValue::getValue0(signal->getSize()).toString() << ";\n";
			}
		}
	}
}

void FsmVhdlExport::writeAsynchronousProcessSensitivityList(QTextStream& stream, shared_ptr<Fsm> l_machine) const
{
	QList<shared_ptr<Input>> inputs = l_machine->getInputs();
	QList<shared_ptr<Signal>> localVars = l_machine->getLocalVariables();

	foreach (shared_ptr<Input> input, inputs)
	{
		stream << "                              ";

		stream << this->signalVhdlName[input];

		if ( (input == inputs.last()) && (localVars.count() == 0) )
		{
			stream << ")\n";
		}
		else
		{
			stream << ",\n";
		}
	}

	foreach (shared_ptr<Signal> localVar, l_machine->getLocalVariables())
	{
		stream << "                              ";
		stream << this->signalVhdlName[localVar];

		if (localVar == localVars.last())
		{
			stream << ")\n";
		}
		else
		{
			stream << ",\n";
		}
	}
}

void FsmVhdlExport::writeSignalAffectationValue(QTextStream& stream, shared_ptr<ActionOnSignal> action) const
{
	shared_ptr<Signal> signal = action->getSignalActedOn();

	stream << "      ";
	stream << this->signalVhdlName[signal];
	stream << " <= ";

	ActionOnSignal::action_types type = action->getActionType();

	if (signal->getSize() == 1)
	{
		switch(type)
		{
		case ActionOnSignal::action_types::activeOnState:
		case ActionOnSignal::action_types::pulse:
		case ActionOnSignal::action_types::set:
			stream << "'1'";
			break;
		case ActionOnSignal::action_types::reset:
			stream << "'0'";
			break;
		case ActionOnSignal::action_types::assign:
			// Impossible case
			break;
		}
	}
	else
	{
		switch(type)
		{
		case ActionOnSignal::action_types::activeOnState:
		case ActionOnSignal::action_types::pulse:
			stream << "\"" <<  action->getActionValue().toString() << "\"";
			break;
		case ActionOnSignal::action_types::set:
			stream << "\"" << LogicValue::getValue1(signal->getSize()).toString() << "\"";
			break;
		case ActionOnSignal::action_types::reset:
			stream << "\"" << LogicValue::getValue0(signal->getSize()).toString() << "\"";
			break;
		case ActionOnSignal::action_types::assign:
			stream << "\"" << action->getActionValue().toString() << "\"";
			break;

		}
	}

	stream << ";\n";
}

QString FsmVhdlExport::generateEquationText(shared_ptr<Signal> equation, shared_ptr<Fsm> l_machine) const
{
	QString text;

	shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation>(equation);

	if (complexEquation != nullptr)
	{
		Equation::nature function = complexEquation->getFunction();

		// Prefix
		if (function == Equation::nature::constant)
		{
			text += "\"" + complexEquation->getCurrentValue().toString() + "\"";
		}
		else if (function == Equation::nature::extractOp)
		{
			int rangeL = complexEquation->getRangeL();
			int rangeR = complexEquation->getRangeR();

			text += generateEquationText(complexEquation->getOperand(0), l_machine); // Throws StatesException - Extract op always has operand 0, even if nullptr - ignored

			text += "(";
			text += QString::number(rangeL);

			if (rangeR != -1)
			{
				text += " downto ";
				text += QString::number(rangeR);
			}

			text += ")";
		}
		else if (function == Equation::nature::notOp)
		{
			text += "not ";
			text += generateEquationText(complexEquation->getOperand(0), l_machine); // Throws StatesException - Not op always has operand 0, even if nullptr - ignored
		}
		else
		{
			text += "(";

			QVector<shared_ptr<Signal>> operands = complexEquation->getOperands();

			for (int i = 0 ; i < operands.count() ; i++)
			{
				text += generateEquationText(operands.at(i), l_machine);

				if (i < operands.count() - 1)
				{
					switch(function)
					{
					case Equation::nature::andOp:
						text += " and ";
						break;
					case Equation::nature::orOp:
						text += " or ";
						break;
					case Equation::nature::xorOp:
						text += " xor ";
						break;
					case Equation::nature::nandOp:
						text += " nand ";
						break;
					case Equation::nature::norOp:
						text += " nor ";
						break;
					case Equation::nature::xnorOp:
						text += " xnor ";
						break;
					case Equation::nature::equalOp:
						text += " = ";
						break;
					case Equation::nature::diffOp:
						text += " /= ";
						break;
					case Equation::nature::concatOp:
						text += "&";
						break;
					case Equation::nature::extractOp:
					case Equation::nature::notOp:
					case Equation::nature::constant:
					case Equation::nature::identity:
						break;
					}
				}
			}

			text += ")";
		}

	}
	else if (equation != nullptr)
	{
		text += this->signalVhdlName[equation];
	}
	else
	{
		text += "'1'";
	}

	return text;
}
