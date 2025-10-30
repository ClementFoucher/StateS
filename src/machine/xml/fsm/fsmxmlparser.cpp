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
#include "fsmxmlparser.h"

// StateS classes
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"


FsmXmlParser::FsmXmlParser(const QString& xmlString) :
	MachineXmlParser(xmlString)
{
	this->machine = make_shared<Fsm>();
}

FsmXmlParser::FsmXmlParser(shared_ptr<QFile> file) :
	MachineXmlParser(file)
{
	this->machine = make_shared<Fsm>();
}

void FsmXmlParser::parseSubmachineStartElement()
{
	auto nodeName = this->getCurrentNodeName();

	if (this->unexpectedTagLevel != 0)
	{
		this->addIssue("    "  + tr("Ignoring node") + " " + nodeName + " " + tr("due to previous errors."));
		this->unexpectedTagLevel++;
		return;
	}


	if (this->currentMainTag == MainTag_t::none) // Enter main group
	{
		if (nodeName == "States")
		{
			this->currentMainTag = MainTag_t::states_group;
		}
		else if (nodeName == "Transitions")
		{
			this->currentMainTag = MainTag_t::transitions_group;
		}
		else
		{
			this->unexpectedTagLevel++;

			this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing machine."));
			this->addIssue("    " + tr("Expected") + " \"States\" " + tr("or") + " \"Transitions\", " + tr("got") + " \"" + nodeName + "\".");
			this->addIssue("    " + tr("Node ignored."));
		}
	}
	else if (this->currentSubTag == SubTag_t::none) // Inside group
	{
		switch (this->currentMainTag)
		{
		case MainTag_t::states_group:
			if (nodeName == "State")
			{
				this->currentSubTag = SubTag_t::state;
				this->parseStateNode();
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing machine states."));
				this->addIssue("    " + tr("Expected") + " \"State\", " + tr("got") + " \"" + nodeName + "\".");
				this->addIssue("    " + tr("Node ignored."));
			}
			break;
		case MainTag_t::transitions_group:
			if (nodeName == "Transition")
			{
				this->currentSubTag = SubTag_t::transition;
				this->parseTransitionNode();
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing machine transitions."));
				this->addIssue("    " + tr("Expected") + " \"Transition\", " + tr("got") + " \"" + nodeName + "\".");
				this->addIssue("    " + tr("Node ignored."));
			}
			break;
		case MainTag_t::none:
			this->addIssue("    " + tr("Ignoring node") + " \"" + nodeName + "\".");
			break;
		}
	}
	else // Sub-group
	{
		switch (this->currentSubTag)
		{
		case SubTag_t::state:
			if (nodeName == "Actions")
			{
				this->currentSubTag = SubTag_t::actions_group;
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"State\"" + tr("node."));
				this->addIssue("    " + tr("Expected") + " \"Actions\", " + tr("got") + " \"" + nodeName + "\".");
				this->addIssue("    " + tr("Node ignored."));
			}
			break;
		case SubTag_t::transition:
			if (nodeName == "Actions")
			{
				this->currentSubTag = SubTag_t::actions_group;
			}
			else if (nodeName == "Condition")
			{
				this->currentSubTag = SubTag_t::condition;
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Transition\"" + tr("node."));
				this->addIssue("    " + tr("Expected") + " \"Actions\" or \"Condition\", " + tr("got") + " \"" + nodeName + "\".");
				this->addIssue("    " + tr("Node ignored."));
			}
			break;
		case SubTag_t::actions_group:
			if (nodeName == "Action")
			{
				this->currentSubTag = SubTag_t::action;
				this->parseActionNode();
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Actions\"" + tr("node."));
				this->addIssue("    " + tr("Expected") + " \"Action\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case SubTag_t::condition:
			if (nodeName == "LogicVariable")
			{
				this->currentSubTag = SubTag_t::logicVariable;
				this->parseOperandVariableNode();
			}
			else if (nodeName == "LogicEquation")
			{
				this->currentSubTag = SubTag_t::logicEquation;

				QString valueNature = this->getCurrentNodeStringAttribute("Nature");
				if (valueNature != "constant")
				{
					this->parseLogicEquationNode();
				}
				else
				{
					// Constant is a special case as we removed the operator type,
					// but it is still used in save files.
					this->parseOperandConstantNode();
				}
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing") + " \"Condition\" " + tr("node") + ".");
				this->addIssue("    " + tr("Expected") + " \"LogicVariable\" " + tr("or") + " \"LogicEquation\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case SubTag_t::logicEquation:
			if (nodeName == "Operand")
			{
				this->currentSubTag = SubTag_t::operand;
				this->parseOperandNode();
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"LogicEquation\"" + tr("node."));
				this->addIssue("    " + tr("Expected") + " \"Operand\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case SubTag_t::operand:
			if (nodeName == "LogicVariable")
			{
				this->currentSubTag = SubTag_t::logicVariable;
				this->parseOperandVariableNode();
			}
			else if (nodeName == "LogicEquation")
			{
				this->currentSubTag = SubTag_t::logicEquation;

				QString valueNature = this->getCurrentNodeStringAttribute("Nature");
				if (valueNature != "constant")
				{
					this->parseLogicEquationNode();
				}
				else
				{
					// Constant is a special case as we removed the operator type,
					// but it is still used in save files.
					this->parseOperandConstantNode();
				}
			}
			else
			{
				this->unexpectedTagLevel++;

				this->addIssue(tr("Error!") + " " + tr("Unexpected node found while parsing") + " \"Operand\" " + tr("node."));
				this->addIssue("    " + tr("Expected") + " \"LogicVariable\" " + tr("or") + " \"LogicEquation\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case SubTag_t::action:
		case SubTag_t::logicVariable:
			this->unexpectedTagLevel++;

			this->addIssue(tr("Error!") + " " + tr("Unexpected node found in a node that doesn't accept subnodes."));
			this->addIssue("    " + tr("Found node was:") + " \"" + nodeName + "\".");
			this->addIssue("    " + tr("Node ignored."));
			break;
		case SubTag_t::none:
			this->addIssue("    "  + tr("Ignoring node") + " \"" + nodeName + "\".");
			break;
		}
	}
}

MachineXmlParser::IsSubmachineEnd_t FsmXmlParser::parseSubmachineEndElement()
{
	if (this->unexpectedTagLevel != 0)
	{
		this->unexpectedTagLevel--;
		return IsSubmachineEnd_t::no;
	}


	if (this->currentSubTag != SubTag_t::none)
	{
		switch (this->currentSubTag)
		{
		case SubTag_t::action:
			this->currentSubTag = SubTag_t::actions_group;
			break;
		case SubTag_t::actions_group:
			if (this->currentMainTag == MainTag_t::states_group)
			{
				this->currentSubTag = SubTag_t::state;
			}
			else if (this->currentMainTag == MainTag_t::transitions_group)
			{
				this->currentSubTag = SubTag_t::transition;
			}
			break;
		case SubTag_t::condition:
			this->processEndCondition();
			this->currentSubTag = SubTag_t::transition;
			break;
		case SubTag_t::state:
		case SubTag_t::transition:
			this->currentComponentId = nullId;
			this->currentSubTag = SubTag_t::none;
			break;
		case SubTag_t::logicEquation:
		{
			auto isRoot = this->processEndLogicEquationNode();

			switch (isRoot)
			{
			case IsRoot_t::yes:
				this->currentSubTag = SubTag_t::condition;
				break;
			case IsRoot_t::no:
				this->currentSubTag = SubTag_t::operand;
				break;
			}
			break;
		}
		case SubTag_t::logicVariable:
		{
			auto isRoot = this->processEndLogicVariableNode();

			switch (isRoot)
			{
			case IsRoot_t::yes:
				this->currentSubTag = SubTag_t::condition;
				break;
			case IsRoot_t::no:
				this->currentSubTag = SubTag_t::operand;
				break;
			}
			break;
		}
		case SubTag_t::operand:
			this->currentSubTag = SubTag_t::logicEquation;
			break;
		case SubTag_t::none:
			// Will not happen
			break;
		}
	}
	else if (this->currentMainTag != MainTag_t::none)
	{
		this->currentMainTag = MainTag_t::none;

		return IsSubmachineEnd_t::yes;
	}

	return IsSubmachineEnd_t::no;
}

void FsmXmlParser::parseStateNode()
{
	auto fsm = dynamic_pointer_cast<Fsm>(this->machine);
	if (fsm == nullptr) return;


	// Get state name
	QString stateName = this->getCurrentNodeStringAttribute("Name");

	if (stateName.isNull() == true)
	{
		this->addIssue(tr("Error!") + " " + tr("Unable to extract state name."));
		this->addIssue("    " + tr("Node ignored."));

		return;
	}


	// Get ID if it is defined
	auto extractedId = this->getCurrentNodeIdAttribute();

	// Build state
	auto stateId = fsm->addState(stateName, extractedId);

	// Check if state was successfully added
	auto state = fsm->getState(stateId);
	if (state == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("The state named") + " \"" + stateName + "\" " + tr("in save file couldn't be added."));
		this->addIssue("    " + tr("This may be due to a duplicated name."));
		this->addIssue("    " + tr("State ignored."));

		return;
	}

	// Remember current component
	this->currentComponentId = stateId;

	// Check state name
	QString actualStateName = state->getName();
	if (actualStateName != stateName)
	{
		this->addIssue(tr("Warning:") + " " + tr("The state named") + " \"" + stateName + "\" " + tr("in save file was added under name") + " \"" + actualStateName + "\".");
		this->addIssue("    " + tr("This is probably due to an ill-formed name."));
		this->addIssue("    " + tr("This may trigger further errors if other components (e.g. transitions) were linked to that state."));
	}

	// Set initial status
	bool isInitial = this->getCurrentNodeStringAttribute("IsInitial").isNull() ? false : true;
	if (isInitial == true)
	{
		fsm->setInitialState(stateId);
	}

	// Get position
	bool positionOk = true;

	bool ok;
	QString xStr = this->getCurrentNodeStringAttribute("X");
	xStr.toDouble(&ok);
	if (ok == true)
	{
		this->addGraphicAttribute(stateId, "X", xStr);
	}
	else
	{
		positionOk = false;
	}

	QString yStr = this->getCurrentNodeStringAttribute("Y");
	yStr.toDouble(&ok);
	if (ok == true)
	{
		this->addGraphicAttribute(stateId, "Y", yStr);
	}
	else
	{
		positionOk = false;
	}

	if (positionOk == false)
	{
		this->addIssue(tr("Warning:") + " " + tr("Unable to extract state position for state ") + "\"" + stateName + "\".");
	}
}

void FsmXmlParser::parseTransitionNode()
{
	auto fsm = dynamic_pointer_cast<Fsm>(this->machine);
	if (fsm == nullptr) return;


	QString sourceName = this->getCurrentNodeStringAttribute("Source");
	QString targetName = this->getCurrentNodeStringAttribute("Target");

	// Check if states exist
	shared_ptr<FsmState> source = this->getStateByName(sourceName);
	shared_ptr<FsmState> target = this->getStateByName(targetName);
	if ( (source == nullptr) || (target == nullptr) )
	{
		this->addIssue(tr("Error!") + " " + tr("Unable to parse a transition: either source or target state do not exist."));
		this->addIssue("    " + tr("Source state was:") + " \"" + sourceName + "\", " + tr("target state was") + "\"" + targetName + "\".");
		this->addIssue("    " + tr("Node ignored."));

		return;
	}

	// Get ID if it is defined
	auto extractedId = this->getCurrentNodeIdAttribute();

	// Build transition
	auto transitionId = fsm->addTransition(source->getId(), target->getId(), extractedId);

	// Check if transition was successfully added
	if (fsm->getTransition(transitionId) == nullptr)
	{
		this->addIssue(tr("Error!") + " " + tr("A transition in save file couldn't be added."));
		this->addIssue("    " + tr("Transition ignored."));

		return;
	}

	// Remember current component
	this->currentComponentId = transitionId;

	// Get slider position
	QString sliderPosStr = this->getCurrentNodeStringAttribute("SliderPos");
	if (sliderPosStr.isNull() == false)
	{
		bool ok;
		sliderPosStr.toDouble(&ok);
		if (ok == true)
		{
			this->addGraphicAttribute(transitionId, "SliderPos", sliderPosStr);
		}
		else
		{
			this->addIssue(tr("Warning:") + " " + tr("Unable to extract slider position for a transition"));
		}
	}
}

void FsmXmlParser::processEndCondition()
{
	auto fsm = dynamic_pointer_cast<Fsm>(this->machine);
	if (fsm == nullptr) return;

	auto transition = fsm->getTransition(this->currentComponentId);
	if (transition == nullptr) return;


	transition->setCondition(this->getCurrentEquation());
}

shared_ptr<FsmState> FsmXmlParser::getStateByName(const QString& name) const
{
	auto fsm = dynamic_pointer_cast<Fsm>(this->machine);
	if (fsm == nullptr) return nullptr;


	shared_ptr<FsmState> ret = nullptr;
	for (auto& stateId : fsm->getAllStatesIds())
	{
		auto state = fsm->getState(stateId);
		if (state->getName() == name)
		{
			ret = state;
			break;
		}
	}

	return ret;
}
