/*
 * Copyright © 2014-2026 Clément Foucher
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

// StateS
#include "machinemanager.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "truthtable.h"
#include "equation.h"
#include "fsmvhdlexport.h"
#include "variable.h"


const QList<shared_ptr<FsmVerifier::Issue> >& FsmVerifier::getIssues()
{
	return this->issues;
}

void FsmVerifier::setCheckVhdl()
{
	this->checkVhdl = true;
}

void FsmVerifier::abort()
{
	this->doAbort = true;
}

void FsmVerifier::run()
{
	this->setTerminationEnabled(true);
	this->issues.clear();

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());

	if (fsm == nullptr)
	{
		auto issue = make_shared<Issue>();
		issue->text = tr("No FSM.");
		issue->type = VerifierSeverityLevel_t::blocking;
		this->issues.append(issue);
	}
	else if (fsm->getAllStatesIds().isEmpty())
	{
		auto issue = make_shared<Issue>();
		issue->text = tr("Empty FSM.");
		issue->type = VerifierSeverityLevel_t::blocking;
		this->issues.append(issue);
	}
	else
	{
		// Check initial state
		if (fsm->getInitialStateId() == nullId)
		{
			auto issue = make_shared<Issue>();
			issue->text = tr("No initial state.");
			issue->type = VerifierSeverityLevel_t::blocking;
			this->issues.append(issue);
		}

		// Check transitions
		for (auto stateId : fsm->getAllStatesIds())
		{
			auto state = fsm->getState(stateId);

			QList<shared_ptr<const Equation>> equations;

			bool errorOnTransition = false;

			for (auto transitionId : state->getOutgoingTransitionsIds())
			{
				auto transition = fsm->getTransition(transitionId);
				if (transition == nullptr) continue;

				auto condition = transition->getCondition();
				if (condition == nullptr) continue;


				if (condition->getSize() != 0)
				{
					equations.append(condition);
				}
				else
				{
					errorOnTransition = true;
					equations.clear();

					auto issue = make_shared<Issue>();
					issue->text = tr("Error on transition condition from state") + " \"" + state->getName() + "\". " + tr("Please correct this equation:") + " " + condition->getText();
					issue->type = VerifierSeverityLevel_t::structure;
					this->issues.append(issue);

					break;
				}

			}

			if (!errorOnTransition)
			{
				// Check for conflicts between transitions
				int constantToOneConditions = state->getOutgoingTransitionsIds().count() - equations.count();

				if (constantToOneConditions > 1)
				{
					auto issue = make_shared<Issue>();
					issue->text = tr("Multiple transitions from state") + " \"" + state->getName() + "\" " + tr("have a condition value always true.");
					issue->type = VerifierSeverityLevel_t::structure;
					this->issues.append(issue);
				}
				else if ( (constantToOneConditions == 1) && (state->getOutgoingTransitionsIds().count() > 1) )
				{
					auto issue = make_shared<Issue>();
					issue->text = tr("One transition from state") + " \"" + state->getName() + "\" " + tr("has a condition value always true.") + " " + tr("Using an always true condition on a transition is only allowed if there is no other transition that origins from the same state.");
					issue->type = VerifierSeverityLevel_t::structure;
					this->issues.append(issue);
				}
				else if (state->getOutgoingTransitionsIds().count() > 1)
				{
					auto currentTruthTable = make_shared<TruthTable>(equations);
					bool finished = false;
					while (finished == false)
					{
						finished = currentTruthTable->buildRow();
						if (this->doAbort == true)
						{
							this->issues.clear();
							return;
						}
					}

					if (currentTruthTable->getTableBuiltSuccessfully() == false)
					{
						auto issue = make_shared<Issue>();
						issue->text = tr("StateS was unable to build the truth table for transitions going out of state") + " \"" + state->getName() + "\". "
						            + tr("This is probably because there are too many combinations to compute.") + " "
						            + tr("This means that there may be transitions going out of this state that are not mutually exclusive.") + " ";
						issue->type = VerifierSeverityLevel_t::tool;
						this->issues.append(issue);
						continue;
					}

					bool detected = false;
					uint rowcount = 0;
					shared_ptr<Issue> currentIssue = nullptr;
					for (uint rowRank = 0 ; rowRank < currentTruthTable->getRowsCount() ; rowRank++)
					{
						uint trueCount = 0;
						LogicValue valueTrue = LogicValue::getValue1(1);
						for (uint columnRank = 0 ; columnRank < currentTruthTable->getOutputCount() ; columnRank++)
						{
							auto value = currentTruthTable->getOutputValue(rowRank, columnRank);
							if (value == valueTrue)
							{
								trueCount++;
							}
						}

						if (trueCount > 1)
						{
							if (!detected)
							{
								currentIssue = make_shared<Issue>();
								currentIssue->text = tr("Transitions from state") + " \"" + state->getName() + "\" " + tr("are not mutually exclusive.") + " " + tr("Two transitions or more can be active at the same time.");
								currentIssue->proof = currentTruthTable;
								currentIssue->type = VerifierSeverityLevel_t::structure;
								this->issues.append(currentIssue);

								detected = true;
							}

							currentIssue->proofsHighlight.append(rowcount);
						}

						rowcount++;
					}
				}
			}
		}

		// Check VHDL export support
		if (this->checkVhdl == true)
		{
			unique_ptr<FsmVhdlExport> vhdlExporter(new FsmVhdlExport());

			shared_ptr<FsmVhdlExport::ExportCompatibility> compat = vhdlExporter->checkCompatibility();
			vhdlExporter.reset();

			if (!compat->isCompatible())
			{
				for (auto& variableId : compat->bothMooreAndMealy)
				{
					auto variable = fsm->getVariable(variableId);
					if (variable == nullptr) continue;


					auto issue = make_shared<Issue>();
					issue->text = tr("Variable") + " \"" + variable->getName() + "\" "
					            + tr("has both Moore and Mealy behaviors.") + " "
					            + tr("StateS VHDL exporter is currently unable to handle these variables.") + " "
					            + tr("This variable will be ignored on VHDL export.");
					issue->type = VerifierSeverityLevel_t::tool;
					this->issues.append(issue);
				}
				for (auto& variableId : compat->rangeAdressed)
				{
					auto variable = fsm->getVariable(variableId);
					if (variable == nullptr) continue;


					auto issue = make_shared<Issue>();
					issue->text = tr("Variable") + " \"" + variable->getName() + "\" "
					            + tr("has range-adressed output generation.") + " "
					            + tr("StateS VHDL exporter is currently unable to handle these variables.") + " "
					            + tr("This variable will be ignored on VHDL export.");
					issue->type = VerifierSeverityLevel_t::tool;
					this->issues.append(issue);
				}
				for (auto& variableId : compat->mealyWithKeep)
				{
					auto variable = fsm->getVariable(variableId);
					if (variable == nullptr) continue;


					auto issue = make_shared<Issue>();
					issue->text = tr("Variable") + " \"" + variable->getName() + "\" "
					            + tr("has Mealy outputs affectation (remembered value).") + " "
					            + tr("StateS VHDL exporter is currently unable to handle these variables.") + " "
					            + tr("This variable will be ignored on VHDL export.");
					issue->type = VerifierSeverityLevel_t::tool;
					this->issues.append(issue);
				}
			}
		}
	}

	emit this->verificationOver();
}
