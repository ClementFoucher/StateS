/*
 * Copyright © 2014-2015 Clément Foucher
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


void FsmVhdlExport::exportFSM(shared_ptr<const Fsm> machine, const QString& path, bool resetLogicPositive, bool prefixIOs)
{
    QFile* file = new QFile(path);
    file->open(QIODevice::WriteOnly);

    QTextStream stream(file);


    // Header
    stream << "-- FSM generated with StateS v" << StateS::getVersion() << " on " << QDate::currentDate().toString() << " at " << QTime::currentTime().toString() << "\n";
    stream << "-- https://sourceforge.net/projects/states/\n\n";

    stream << "library IEEE;\n";
    stream << "use IEEE.std_logic_1164.all;\n\n\n";


    // Entity
    stream << "entity StateS_FSM is\n";

    stream << "  port(clock : in std_logic;\n       reset : in std_logic;\n       ";



    QList<shared_ptr<Input>> inputs = machine->getInputs();

    foreach (shared_ptr<Input> input, inputs)
    {
        if (prefixIOs == true)
            stream << "I_";

        stream << correctName(input->getName()) << " : in std_logic";

        if (input->getSize() > 1)
        {
            stream << "_vector(";

            stream << QString::number(input->getSize() - 1) << " downto 0)";
        }

        stream << ";\n       ";
    }

    QList<shared_ptr<Output>> outputs = machine->getOutputs();

    foreach (shared_ptr<Output> output, outputs)
    {
        if (prefixIOs == true)
            stream << "O_";

        stream << correctName(output->getName()) << " : out std_logic";

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


    // Architecture
    stream << "architecture FSM_body of StateS_Fsm is\n\n";

    stream << "  type state_type is (";

    QList<shared_ptr<FsmState>> states = machine->getStates();
    foreach(shared_ptr<FsmState> state, states)
    {
        stream << "S_" << correctName(state->getName());

        if (!(state == states.last()))
            stream << ", ";
    }

    stream << ");\n\n";

    stream << "  signal current_state : state_type;\n";
    stream << "  signal next_state    : state_type;\n\n";

    QList<shared_ptr<Signal>> localVars = machine->getLocalVariables();
    foreach(shared_ptr<Signal> localVar, localVars)
    {
        stream << "  signal SIG_" << correctName(localVar->getName()) << " : std_logic";

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

    foreach(shared_ptr<Signal> constant, machine->getConstants())
    {
        stream << "  constant CST_" << correctName(constant->getName()) << " : std_logic";

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

    foreach (shared_ptr<Input> input, inputs)
    {
        stream << "                              ";
        if (prefixIOs == true)
            stream << "I_";
        stream << correctName(input->getName());

        if ( (input == inputs.last()) && (localVars.count() == 0) )
        {
            stream << ")\n";
        }
        else
        {
            stream << ",\n";
        }
    }

    foreach (shared_ptr<Signal> localVar, localVars)
    {
        stream << "                              SIG_" << correctName(localVar->getName());

        if (localVar == localVars.last())
        {
            stream << ")\n";
        }
        else
        {
            stream << ",\n";
        }
    }

    stream << "  begin\n";

    stream << "    case current_state is\n";

    foreach(shared_ptr<FsmState> state, states)
    {
        stream << "    when S_" << correctName(state->getName()) << " =>\n";

        QList<shared_ptr<FsmTransition>> transitions = state->getOutgoingTransitions();
        foreach (shared_ptr<FsmTransition> transition, transitions)
        {
            stream << "      ";
            if (transition != transitions.first())
                stream << "els";

            stream << "if (" << equationText(transition->getCondition(), machine, prefixIOs) << " = '1') then\n";
            stream << "        next_state <= S_" << correctName(transition->getTarget()->getName()) << ";\n";
        }

        stream << "      end if;\n";

    }

    stream << "    end case;\n";

    stream << "  end process;\n\n";

    // Current state update : synchronous process

    stream << "  update_state : process(clock, reset)\n";
    stream << "  begin\n";
    stream << "    if reset='" << (resetLogicPositive?"1":"0") << "' then\n";
    stream << "      current_state <= S_" + correctName(machine->getInitialState()->getName()) << ";\n";
    stream << "    elsif rising_edge(clock) then\n";
    stream << "      current_state <= next_state;\n";
    stream << "    end if;\n";
    stream << "  end process;\n\n";


    // Output computation : asynchronous process

    stream << "  compute_outputs : process(current_state)\n";
    stream << "  begin\n";

    stream << "    case current_state is\n";

    foreach(shared_ptr<FsmState> state, states)
    {
        stream << "    when S_" << correctName(state->getName()) << " =>\n";

        foreach(shared_ptr<Signal> sig, state->getActions())
        {
            MachineActuatorComponent::action_types type = state->getActionType(sig);

            stream << "      ";
            if ( (prefixIOs) && (dynamic_pointer_cast<Output>(sig) != nullptr))
                stream << "O_";
            else if (machine->getLocalVariables().contains(sig))
                stream << "SIG_";

            stream << correctName(sig->getName()) << " <= ";

            switch(type)
            {
            case MachineActuatorComponent::action_types::activeOnState:
                stream << "'1'";
                break;
            case MachineActuatorComponent::action_types::pulse:
                stream << "'1'";
                break;
            case MachineActuatorComponent::action_types::set:
                stream << "'1'";
                break;
            case MachineActuatorComponent::action_types::reset:
                if (sig->getSize() == 1)
                    stream << "'0'";
                else
                    stream << "\"" << LogicValue::getValue0(sig->getSize()).toString() << "\"";
                break;
            case MachineActuatorComponent::action_types::assign:
                // TODO: address range
                if (sig->getSize() == 1)
                    stream << "'" << state->getActionValue(sig).toString() <<"'";
                else
                    stream << "\"" << state->getActionValue(sig).toString() << "\"";
                break;

            }

            stream << ";\n";
        }
    }

    stream << "    end case;\n";

    stream << "  end process;\n\n";


    // The end
    stream << "end architecture;\n";

    file->close();
    delete file;
}

QString FsmVhdlExport::correctName(QString name)
{
    QString newName = name;

    newName.replace(" ", "_");
    newName.replace("#", "_");

    return newName;
}

QString FsmVhdlExport::equationText(shared_ptr<Signal> equation, shared_ptr<const Fsm> machine, bool prefixIOs)
{
    QString text;

    shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation>(equation);

    if (complexEquation != nullptr)
    {
        if (complexEquation->getFunction() == Equation::nature::notOp)
            text += "(not ";
        else
            text += "(";

        QVector<shared_ptr<Signal>> operands = complexEquation->getOperands();

        for (int i = 0 ; i < operands.count() ; i++)
        {
            text += equationText(operands.at(i), machine, prefixIOs);

            if (i < operands.count() - 1)
            {
                switch(complexEquation->getFunction())
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
                    // TODO
                    break;
                case Equation::nature::extractOp:
                    // TODO
                    break;
                case Equation::nature::notOp:
                case Equation::nature::constant:
                case Equation::nature::identity:
                    break;
                }
            }
        }

        text += ")";
    }
    else if (equation != nullptr)
    {
        if ( (prefixIOs) && (dynamic_pointer_cast<Input>(equation) != nullptr))
            text += "I_";
        else if (machine->getLocalVariables().contains(equation))
            text += "SIG_";
        else if (machine->getConstants().contains(equation))
            text += "CST_";

        text += correctName(equation->getName());
    }
    else
    {
        text += "'1'";
    }

    return text;
}
