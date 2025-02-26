/*
 * Copyright © 2014-2025 Clément Foucher
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

// Qt classes
#include <QDate>
#include <QFile>
#include <QTextStream>

// StateS classes
#include "machinemanager.h"
#include "states.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "variable.h"
#include "equation.h"
#include "actiononvariable.h"
#include "operand.h"


FsmVhdlExport::FsmVhdlExport()
{

}

void FsmVhdlExport::setOptions(bool resetLogicPositive, bool prefixSignals)
{
	this->resetLogicPositive = resetLogicPositive;
	this->prefixSignals = prefixSignals;
}

bool FsmVhdlExport::writeToFile(const QString& path)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return false;

	QFile* file = new QFile(path);
	file->open(QIODevice::WriteOnly);

	QTextStream stream(file);

	this->generateVhdlCharacteristics(fsm);

	this->writeHeader(stream);
	this->writeEntity(stream, fsm);
	this->writeArchitecture(stream, fsm);

	file->close();
	delete file;

	return true;
}

shared_ptr<FsmVhdlExport::ExportCompatibility> FsmVhdlExport::checkCompatibility()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return nullptr;

	shared_ptr<ExportCompatibility> compatibility(new ExportCompatibility());
	WrittableVariableCharacteristics_t charac;

	for (auto& output : fsm->getOutputs())
	{
		charac = this->determineWrittableVariableCharacteristics(fsm, output, false);

		if (charac.isKeepValue && charac.isTempValue)
			compatibility->bothTempAndKeepValue.append(output);

		if (charac.isMoore && charac.isMealy)
			compatibility->bothMooreAndMealy.append(output);

		if (charac.isRangeAdressed)
			compatibility->rangeAdressed.append(output);

		if (charac.isMealy && charac.isKeepValue)
			compatibility->mealyWithKeep.append(output);
	}
	for (auto& variable : fsm->getInternalVariables())
	{
		charac = this->determineWrittableVariableCharacteristics(fsm, variable, false);

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

void FsmVhdlExport::generateVhdlCharacteristics(shared_ptr<Fsm> l_machine)
{
	// Machine
	this->machineVhdlName = this->cleanNameForVhdl(l_machine->getName());

	// States
	for (auto& stateId : l_machine->getAllStatesIds())
	{
		auto state = l_machine->getState(stateId);

		QString stateRadical = "S_";

		stateRadical += cleanNameForVhdl(state->getName());

		// Check for duplicates
		int occurence = 2;
		QString stateName = stateRadical;
		auto statesNames = this->stateVhdlName.values();
		while (statesNames.contains(stateName))
		{
			stateName = stateRadical + QString::number(occurence);
			occurence++;
		}

		this->stateVhdlName[stateId] = stateName;
	}

	// Variables
	QString signalName;

	for (auto& input : l_machine->getInputs())
	{
		signalName = this->generateVhdlSignalName("I_", input->getName());
		this->variableVhdlName[input] = signalName;
	}
	for (auto& output : l_machine->getOutputs())
	{
		signalName = this->generateVhdlSignalName("O_", output->getName());
		this->determineWrittableVariableCharacteristics(l_machine, output, true);
		this->variableVhdlName[output] = signalName;
	}
	for (auto& variable : l_machine->getInternalVariables())
	{
		signalName = this->generateVhdlSignalName("SIG_", variable->getName());
		this->determineWrittableVariableCharacteristics(l_machine, variable, true);
		this->variableVhdlName[variable] = signalName;
	}
	for (auto& constant : l_machine->getConstants())
	{
		signalName = this->generateVhdlSignalName("CONST_", constant->getName());
		this->variableVhdlName[constant] = signalName;
	}
}

FsmVhdlExport::WrittableVariableCharacteristics_t FsmVhdlExport::determineWrittableVariableCharacteristics(shared_ptr<Fsm> l_machine, shared_ptr<Variable> variable, bool storeResults)
{
	WrittableVariableCharacteristics_t characteristics;

	bool doNotGenerate = !storeResults;

	for (auto& stateId : l_machine->getAllStatesIds())
	{
		auto state = l_machine->getState(stateId);

		for (auto& action : state->getActions())
		{
			if (action->getVariableActedOn() == variable)
			{
				characteristics.isMoore = true;

				ActionOnVariableType_t actionType = action->getActionType();

				switch (actionType)
				{
				case ActionOnVariableType_t::assign:
				case ActionOnVariableType_t::set:
				case ActionOnVariableType_t::reset:
				case ActionOnVariableType_t::increment:
				case ActionOnVariableType_t::decrement:
					characteristics.isKeepValue = true;
					break;
				case ActionOnVariableType_t::activeOnState:
				case ActionOnVariableType_t::pulse:
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

	for (auto& transitionId : l_machine->getAllTransitionsIds())
	{
		auto transition = l_machine->getTransition(transitionId);

		for (auto& action : transition->getActions())
		{
			if (action->getVariableActedOn() == variable)
			{
				characteristics.isMealy = true;

				ActionOnVariableType_t actionType = action->getActionType();

				switch (actionType)
				{
				case ActionOnVariableType_t::assign:
				case ActionOnVariableType_t::set:
				case ActionOnVariableType_t::reset:
				case ActionOnVariableType_t::increment:
				case ActionOnVariableType_t::decrement:
					characteristics.isKeepValue = true;
					break;
				case ActionOnVariableType_t::activeOnState:
				case ActionOnVariableType_t::pulse:
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
			this->mooreVariables.append(variable);
		else if (characteristics.isMealy)
			this->mealyVariables.append(variable);

		if (characteristics.isTempValue)
			this->tempValueVariables.append(variable);
		else if (characteristics.isKeepValue)
			this->keepValueVariables.append(variable);

//        if (variableIsRangeAdressed)
//            this->rangeAdressedVariables.append(variable);
	}

	return characteristics;
}

QString FsmVhdlExport::generateVhdlSignalName(const QString& prefix, const QString& name) const
{
	QString signalRadical;

	if (this->prefixSignals)
		signalRadical += prefix;

	signalRadical += cleanNameForVhdl(name);

	// Check for duplicates
	int occurence = 2;
	QString signalName = signalRadical;
	auto vhdlNames = this->variableVhdlName.values();
	while (vhdlNames.contains(signalName))
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

	for (auto& input : l_machine->getInputs())
	{
		stream << this->variableVhdlName[input] << " : in std_logic";

		if (input->getSize() > 1)
		{
			stream << "_vector(";

			stream << QString::number(input->getSize() - 1) << " downto 0)";
		}

		stream << ";\n       ";
	}

	auto outputs = l_machine->getOutputs();
	for (auto& output : outputs)
	{
		stream << this->variableVhdlName[output] << " : out std_logic";

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

	for (const auto& stateName : this->stateVhdlName)
	{
		stream << stateName;

		if (!(stateName == this->stateVhdlName.last()))
			stream << ", ";
	}

	stream << ");\n\n";

	stream << "  signal current_state : state_type;\n";
	stream << "  signal next_state    : state_type;\n\n";

	for (auto& localVar : l_machine->getInternalVariables())
	{
		stream << "  signal " + this->variableVhdlName[localVar] << " : std_logic";

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

	for (auto& constant : l_machine->getConstants())
	{
		stream << "  constant " + this->variableVhdlName[constant] << " : std_logic";

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

	for (auto& stateId : l_machine->getAllStatesIds())
	{
		auto state = l_machine->getState(stateId);

		stream << "    when ";
		stream << this->stateVhdlName[stateId];
		stream << " =>\n";

		auto transitionsIds = state->getOutgoingTransitionsIds();
		for (auto& transitionId : transitionsIds)
		{
			stream << "      ";

			if (transitionId != transitionsIds.first())
				stream << "els";

			stream << "if ";

			auto transition = l_machine->getTransition(transitionId);

			auto condition = transition->getCondition();
			if (condition == nullptr)
			{
				// Empty condition is considered always true
				stream << "true";
			}
			else
			{
				stream << generateEquationText(condition, l_machine);
				stream << " = '1'";
			}

			stream << " then\n";
			stream << "        next_state <= " << this->stateVhdlName[transition->getTargetStateId()] << ";\n";
		}

		stream << "      end if;\n";

	}

	stream << "    end case;\n";

	stream << "  end process;\n\n";

	// Current state update : synchronous process

	stream << "  update_state : process(clock, reset)\n";
	stream << "  begin\n";
	stream << "    if reset='" << (resetLogicPositive?"1":"0") << "' then\n";
	stream << "      current_state <= " << this->stateVhdlName[l_machine->getInitialStateId()] << ";\n";
	stream << "    elsif rising_edge(clock) then\n";
	stream << "      current_state <= next_state;\n";
	stream << "    end if;\n";
	stream << "  end process;\n\n";


	// Output computation : asynchronous processes

	if (!this->mooreVariables.isEmpty())
		this->writeMooreOutputs(stream, l_machine);

	if (!this->mealyVariables.isEmpty())
		this->writeMealyOutputs(stream, l_machine);

	// The end
	stream << "\nend architecture;\n";
}

void FsmVhdlExport::writeMooreOutputs(QTextStream& stream, shared_ptr<Fsm> l_machine) const
{
	stream << "  compute_moore : process(current_state)\n";
	stream << "  begin\n";

	for (auto& variable : this->mooreVariables)
	{
		if (this->tempValueVariables.contains(variable))
		{
			// Write default value for temp variables
			stream << "    " << this->variableVhdlName[variable] << " <= \"" << LogicValue::getValue0(variable->getSize()).toString() << "\";\n";
		}
	}
	stream << "    -- Signals handled in this process but not listed above this line implicitly maintain their value.\n";

	stream << "    case current_state is\n";

	for (auto& stateId : l_machine->getAllStatesIds())
	{
		auto state = l_machine->getState(stateId);

		stream << "    when ";
		stream << this->stateVhdlName[stateId];
		stream << " =>\n";

		int writtenActions = 0;
		for (auto& action : state->getActions())
		{
			if (this->mooreVariables.contains(action->getVariableActedOn()))
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

	for (const auto& variable : this->mealyVariables)
	{
		if (this->tempValueVariables.contains(variable))
		{
			QList<shared_ptr<FsmTransition>> transitions;

			for (auto& transitionId : l_machine->getAllTransitionsIds())
			{
				auto transition = l_machine->getTransition(transitionId);

				for (auto& action : transition->getActions())
				{
					if (action->getVariableActedOn() == variable)
					{
						transitions.append(transition);
						break;
					}
				}
			}

			if (!transitions.isEmpty())
			{
				stream << "  affect_" << this->variableVhdlName[variable] << ":";
				stream << this->variableVhdlName[variable] << " <= ";

				for (const auto& transition : transitions)
				{
					for (auto& action : transition->getActions())
					{
						if (action->getVariableActedOn() == variable)
						{
							if (variable->getSize() > 1)
								stream << "\"" << action->getActionValue().toString() << "\"";
							else
								stream << "'1'";

							stream << " when (current_state = " << this->stateVhdlName[transition->getSourceStateId()] << ")";
							stream << " and " << this->generateEquationText(transition->getCondition(), l_machine);
							stream << " else\n    ";
						}
					}
				}

				stream << LogicValue::getValue0(variable->getSize()).toString() << ";\n";
			}
		}
	}
}

void FsmVhdlExport::writeAsynchronousProcessSensitivityList(QTextStream& stream, shared_ptr<Fsm> l_machine) const
{
	auto inputs    = l_machine->getInputs();
	auto localVars = l_machine->getInternalVariables();

	for (auto& input : inputs)
	{
		stream << "                              ";

		stream << this->variableVhdlName[input];

		if ( (input == inputs.last()) && (localVars.count() == 0) )
		{
			stream << ")\n";
		}
		else
		{
			stream << ",\n";
		}
	}

	for (auto& localVar : l_machine->getInternalVariables())
	{
		stream << "                              ";
		stream << this->variableVhdlName[localVar];

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

void FsmVhdlExport::writeSignalAffectationValue(QTextStream& stream, shared_ptr<ActionOnVariable> action) const
{
	shared_ptr<Variable> variable = action->getVariableActedOn();

	stream << "      ";
	stream << this->variableVhdlName[variable];
	stream << " <= ";

	ActionOnVariableType_t type = action->getActionType();

	if (variable->getSize() == 1)
	{
		switch(type)
		{
		case ActionOnVariableType_t::activeOnState:
		case ActionOnVariableType_t::pulse:
		case ActionOnVariableType_t::set:
			stream << "'1'";
			break;
		case ActionOnVariableType_t::reset:
			stream << "'0'";
			break;
		case ActionOnVariableType_t::increment:
		case ActionOnVariableType_t::decrement:
		case ActionOnVariableType_t::assign:
			// Impossible cases
			break;
		}
	}
	else
	{
		switch(type)
		{
		case ActionOnVariableType_t::activeOnState:
		case ActionOnVariableType_t::pulse:
			stream << "\"" <<  action->getActionValue().toString() << "\"";
			break;
		case ActionOnVariableType_t::set:
			stream << "\"" << LogicValue::getValue1(variable->getSize()).toString() << "\"";
			break;
		case ActionOnVariableType_t::reset:
			stream << "\"" << LogicValue::getValue0(variable->getSize()).toString() << "\"";
			break;
		case ActionOnVariableType_t::assign:
			stream << "\"" << action->getActionValue().toString() << "\"";
			break;
		case ActionOnVariableType_t::increment:
			stream << "std_logic_vector(unsigned(" << this->variableVhdlName[variable] << " + 1)";
			break;
		case ActionOnVariableType_t::decrement:
			stream << "std_logic_vector(unsigned(" << this->variableVhdlName[variable] << " - 1)";
			break;
		}
	}

	stream << ";\n";
}

QString FsmVhdlExport::generateEquationText(shared_ptr<Equation> equation, shared_ptr<Fsm> l_machine) const
{
	if (equation == nullptr)  return "[" + tr("Error: empty equation") + "]";


	QString text;

	OperatorType_t function = equation->getOperatorType();
	if (equation->getOperatorType() == OperatorType_t::identity) // Equation is actually a single variable or constant
	{
		// This should only happen at condition root for a well-formed condition
		auto operand = equation->getOperand(0);
		text = generateOperandText(operand, l_machine);
	}
	else if (function == OperatorType_t::extractOp)
	{
		auto operand = equation->getOperand(0);
		text = generateOperandText(operand, l_machine);

		int rangeL = equation->getRangeL();
		int rangeR = equation->getRangeR();

		text += "(";
		text += QString::number(rangeL);

		if (rangeR != -1)
		{
			text += " downto ";
			text += QString::number(rangeR);
		}

		text += ")";
	}
	else if (function == OperatorType_t::notOp)
	{
		text = "not ";

		auto operand = equation->getOperand(0);
		text += generateOperandText(operand, l_machine);
	}
	else
	{
		text = "(";

		int operandCount = equation->getOperandCount();
		for (int i = 0 ; i < operandCount ; i++)
		{
			auto operand = equation->getOperand(i);
			text += generateOperandText(operand, l_machine);

			if (i < operandCount - 1)
			{
				switch(function)
				{
				case OperatorType_t::andOp:
					text += " and ";
					break;
				case OperatorType_t::orOp:
					text += " or ";
					break;
				case OperatorType_t::xorOp:
					text += " xor ";
					break;
				case OperatorType_t::nandOp:
					text += " nand ";
					break;
				case OperatorType_t::norOp:
					text += " nor ";
					break;
				case OperatorType_t::xnorOp:
					text += " xnor ";
					break;
				case OperatorType_t::equalOp:
					text += " = ";
					break;
				case OperatorType_t::diffOp:
					text += " /= ";
					break;
				case OperatorType_t::concatOp:
					text += "&";
					break;
				case OperatorType_t::extractOp:
				case OperatorType_t::notOp:
				case OperatorType_t::identity:
					// Cases treated in another branch of the if
					break;
				}
			}
		}

		text += ")";
	}

	return text;
}

QString FsmVhdlExport::generateOperandText(shared_ptr<Operand> operand, shared_ptr<Fsm> l_machine) const
{
	if (operand == nullptr) return "[" + tr("Error: empty operand") + "]";


	switch (operand->getSource())
	{
	case OperandSource_t::equation:
		return generateEquationText(operand->getEquation(), l_machine);
		break;
	case OperandSource_t::variable:
	{
		auto variable = operand->getVariable();
		if (variable == nullptr) return "[" + tr("Error: empty variable") + "]";


		return this->variableVhdlName[variable];
		break;
	}
	case OperandSource_t::constant:
	{
		auto constantText = operand->getText();

		if (constantText.size() == 1)
		{
			return "'" + constantText + "'";
		}
		else
		{
			return "\"" + constantText + "\"";
		}
		break;
	}
	}
}
