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
#include "fsmverifier.h"

// StateS classes
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "truthtable.h"
#include "equation.h"


FsmVerifier::FsmVerifier(shared_ptr<Fsm> machine) :
    QObject()
{
    this->machine = machine;
}

FsmVerifier::~FsmVerifier()
{
    this->clearProofs();
}

QList<QString> FsmVerifier::verifyFsm()
{
    QList<QString> errors;

    this->clearProofs();

    shared_ptr<Fsm> machine = this->machine.lock();

    if (machine == nullptr)
    {
        errors.append(tr("No FSM."));
        proofs.append(nullptr);

    }
    else if (machine->getStates().isEmpty())
    {
        errors.append(tr("Empty FSM."));
        proofs.append(nullptr);
    }
    else
    {
        // Check initial state
        if (machine->getInitialState() == nullptr)
        {
            errors.append(tr("No initial state."));
            proofs.append(nullptr);
        }

        // Check transitions
        foreach(shared_ptr<FsmState> state, machine->getStates())
        {
            QList<shared_ptr<Equation>> equations;

            bool errorOnTransition = false;

            foreach(shared_ptr<FsmTransition> transition, state->getOutgoingTransitions())
            {
                shared_ptr<Signal> condition = transition->getCondition();

                if (condition != nullptr)
                {
                    if (condition->getSize() != 0)
                    {
                        shared_ptr<Equation> equation = dynamic_pointer_cast<Equation>(condition);

                        if (equation == nullptr)
                        {
                            QVector<shared_ptr<Signal>> operand;
                            operand.append(condition);
                            equation = shared_ptr<Equation>(new Equation(Equation::nature::identity, operand));

                        }

                        equations.append(equation);
                    }
                    else
                    {
                        errorOnTransition = true;
                        equations.clear();

                        errors.append(tr("Error on transition condition from state") + " " + state->getName() + ". " + tr("Please correct this equation:") + " " + condition->getText());
                        proofs.append(nullptr);

                        break;
                    }
                }
            }

            if (!errorOnTransition)
            {
                // Check for conflicts between transitions
                int constantToOneConditions = state->getOutgoingTransitions().count() - equations.count();

                if (constantToOneConditions > 1)
                {
                    errors.append(tr("Multiple transitions from state") + " " + state->getName() + " " + tr("have a condition value always true."));
                    proofs.append(nullptr);
                }
                else if ( (constantToOneConditions == 1) && (state->getOutgoingTransitions().count() > 1) )
                {
                    errors.append(tr("One transition from state") + " " + state->getName() + " " + tr("has a condition value always true.") + " " + tr("Using an always true condition on a transition is only allowed if there is no other transition that origins from the same state."));
                    proofs.append(nullptr);
                }
                else if (state->getOutgoingTransitions().count() > 1)
                {
                    internalProofs.append(TruthTable(equations));

                    QVector<QVector<LogicValue>> result = internalProofs.last().getOutputTable();

                    bool detected = false;
                    uint rowcount = 0;
                    foreach(QVector<LogicValue> row, result)
                    {
                        uint trueCount = 0;
                        LogicValue valueTrue = LogicValue::getValue1(1);
                        foreach(LogicValue val, row)
                        {
                            if (val == valueTrue)
                                trueCount++;
                        }

                        if (trueCount > 1)
                        {
                            TruthTable* currentProof = &internalProofs.last();
                            if (!detected)
                            {
                                errors.append(tr("Transitions from state") + " " + state->getName() + " " + tr("are not mutually exclusive.") + " " + tr("Two transitions or more can be active at the same time."));
                                proofs.append(currentProof);
                                detected = true;
                                this->proofsHighlight.insert(currentProof, QList<int>());
                            }

                            this->proofsHighlight[currentProof].append(rowcount);
                        }

                        rowcount++;
                    }

                }
            }
        }
    }

    return errors;
}

QVector<TruthTable*> FsmVerifier::getProofs()
{
    return this->proofs;
}

QHash<TruthTable*, QList<int>> FsmVerifier::getProofsHighlight()
{
    return this->proofsHighlight;
}

void FsmVerifier::clearProofs()
{
    this->internalProofs.clear();

    this->proofs.clear();
    this->proofsHighlight.clear();
}



