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
#include "fsmxmlparser.h"

// Qt classes
#include <QPointF>
#include <QXmlStreamReader>
#include <QFile>

// StateS classes
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "graphicattributes.h"


FsmXmlParser::FsmXmlParser()
{
	this->currentGroup = Group_t::none;
	this->currentSubGroup = Subgroup_t::none;
	this->currentLevel = 0;
}

FsmXmlParser::FsmXmlParser(const QString& xmlString) :
    FsmXmlParser()
{
	this->xmlReader = shared_ptr<QXmlStreamReader>(new QXmlStreamReader(xmlString));
}

FsmXmlParser::FsmXmlParser(shared_ptr<QFile> file) :
    FsmXmlParser()
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

	this->fileName = file->fileName();
	this->xmlReader = shared_ptr<QXmlStreamReader>(new QXmlStreamReader(file.get()));
}

void FsmXmlParser::treatStartElement()
{
	this->currentLevel++;

	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	if (this->currentLevel == 1) // Root: extract name
	{
		this->parseMachineName(this->fileName);
	}
	else if (this->currentLevel == 2) // Enter main group
	{
		if (nodeName == "Configuration")
		{
			this->currentGroup = Group_t::configuration_group;
		}
		else if (nodeName == "Signals")
		{
			this->currentGroup = Group_t::signals_group;
		}
		else if (nodeName == "States")
		{
			this->currentGroup = Group_t::states_group;
		}
		else if (nodeName == "Transitions")
		{
			this->currentGroup = Group_t::transitions_group;
		}
		else
		{
			this->warnings.append(tr("Error!") + " " + tr("Unexpected node found:") + " " + nodeName);
		}
	}
	else if (this->currentLevel == 3) // Inside group
	{
		switch (this->currentGroup)
		{
		case Group_t::configuration_group:
			this->parseConfiguration();
			break;
		case Group_t::signals_group:
			this->parseSignal();
			break;
		case Group_t::states_group:
			this->currentSubGroup = Subgroup_t::state;
			this->parseState();
			break;
		case Group_t::transitions_group:
			this->currentSubGroup = Subgroup_t::transition;
			this->parseTransition();
			break;
		case Group_t::none:
			this->warnings.append("    "  + tr("Ignored node") + " " + nodeName);
			break;
		}
	}
	else // Sub-group
	{
		switch (this->currentSubGroup)
		{
		case Subgroup_t::state:
			if (nodeName == "Actions")
			{
				this->currentSubGroup = Subgroup_t::actions_group;
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"State\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"Actions\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case Subgroup_t::transition:
			if (nodeName == "Actions")
			{
				this->currentSubGroup = Subgroup_t::actions_group;
			}
			else if (nodeName == "Condition")
			{
				this->currentSubGroup = Subgroup_t::condition;
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Transition\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"Actions\" or \"Condition\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case Subgroup_t::actions_group:
			if (nodeName == "Action")
			{
				this->currentSubGroup = Subgroup_t::action;
				this->parseAction();
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Actions\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"Action\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case Subgroup_t::condition:
		case Subgroup_t::operand:
			if (nodeName == "LogicVariable")
			{
				this->currentSubGroup = Subgroup_t::logicVariable;
				this->parseLogicEquation();
			}
			else if (nodeName == "LogicEquation")
			{
				this->currentSubGroup = Subgroup_t::logicEquation;
				this->parseLogicEquation();
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Operand\" or \"Condition\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"LogicVariable\" or \"LogicEquation\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case Subgroup_t::logicEquation:
			if (nodeName == "Operand")
			{
				bool ok;
				uint operandRank = attributes.value("Number").toUInt(&ok);

				if (ok == true)
				{
					this->treatBeginOperand(operandRank);
				}
				else
				{
					this->warnings.append(tr("Error!") + " " + tr("Unable to parse operand rank for an equation."));
					this->warnings.append("    " + tr("Operand will be set in position 0, erasing popential existing operand."));
					this->treatBeginOperand(0);
				}
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"LogicEquation\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"Operand\", " + tr("got") + " \"" + nodeName + "\".");
			}
			this->currentSubGroup = Subgroup_t::operand;
			break;
		case Subgroup_t::action:
		case Subgroup_t::logicVariable:
			// Nothing to do: these nodes does not have children
			break;
		case Subgroup_t::none:
			this->warnings.append("    "  + tr("Ignored node") + " " + nodeName);
			break;
		}
	}
}

void FsmXmlParser::treatEndElement()
{
	this->currentLevel--;

	shared_ptr<FsmTransition> transition;
	switch (this->currentSubGroup)
	{
	case Subgroup_t::action:
		this->currentSubGroup = Subgroup_t::actions_group;
		break;
	case Subgroup_t::actions_group:
		if (this->currentGroup == Group_t::states_group)
		{
			this->currentSubGroup = Subgroup_t::state;
		}
		else if (this->currentGroup == Group_t::transitions_group)
		{
			this->currentSubGroup = Subgroup_t::transition;
		}
		break;
	case Subgroup_t::condition:
		this->currentSubGroup = Subgroup_t::transition;
		transition = dynamic_pointer_cast<FsmTransition>(this->currentActuator);
		transition->setCondition(this->rootLogicEquation);
		this->rootLogicEquation = nullptr;
		break;
	case Subgroup_t::state:
	case Subgroup_t::transition:
		this->currentActuator = nullptr;
		this->currentSubGroup = Subgroup_t::none;
		break;
	case Subgroup_t::logicEquation:
	case Subgroup_t::logicVariable:
		if (this->currentLevel == 4)
		{
			this->currentSubGroup = Subgroup_t::condition;
		}
		else
		{
			this->currentSubGroup = Subgroup_t::operand;
		}
		break;
	case Subgroup_t::operand:
		this->currentSubGroup = Subgroup_t::logicEquation;
		this->treatEndOperand();
		break;
	case Subgroup_t::none:
		if (this->currentLevel == 1)
		{
			this->currentGroup = Group_t::none;
		}
		break;
	}
}

void FsmXmlParser::buildMachineFromXml()
{
	this->machine = shared_ptr<Fsm>(new Fsm());

	while (this->xmlReader->atEnd() == false)
	{
		this->xmlReader->readNext();

		if (this->xmlReader->isStartElement())
		{
			this->treatStartElement();
		}
		else if (this->xmlReader->isEndElement())
		{
			this->treatEndElement();
		}
	}
}

void FsmXmlParser::parseState()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	if (nodeName == "State")
	{
		// Get state name
		QString stateName = attributes.value("Name").toString();

		if (stateName.isNull())
		{
			this->warnings.append(tr("Error!") + " " + tr("Unable to extract state name."));
			this->warnings.append("    " + tr("Node ignored."));
			return;
		}

		// Get initial status (true if IsInitial property exists)
		bool isInitial = attributes.value("IsInitial").isNull() ? false : true;

		// Get ID if it is defined
		bool ok;
		componentId_t extractedId = attributes.value("Id").toULong(&ok);
		if (ok == false)
		{
			extractedId = 0;
		}

		// Build state
		auto fsm = dynamic_pointer_cast<Fsm>(this->machine);
		componentId_t stateId;
		if (extractedId != 0)
		{
			stateId = fsm->addState(isInitial, stateName, extractedId);
		}
		else
		{
			stateId = fsm->addState(isInitial, stateName);
		}
		auto state = fsm->getState(stateId);

		// Get position
		bool positionOk = true;

		QString xStr = attributes.value("X").toString();
		xStr.toDouble(&ok);
		if (ok == true)
		{
			this->graphicAttributes->addAttribute(stateId, "X", xStr);
		}
		else
		{
			positionOk = false;
		}

		QString yStr = attributes.value("Y").toString();
		yStr.toDouble(&ok);
		if (ok == true)
		{
			this->graphicAttributes->addAttribute(stateId, "Y", yStr);
		}
		else
		{
			positionOk = false;
		}

		if (positionOk == false)
		{
			this->warnings.append(tr("Warning!") + " " + tr("Unable to extract state position for state ") + stateName);
		}

		this->currentActuator = state;
	}
	else
	{
		this->warnings.append(tr("Unexpected node encountered while parsing") + " " + tr("state list:"));
		this->warnings.append("    " + tr("Expected") + " \"State\", " + tr("got") + " \"" + nodeName + "\".");
		this->warnings.append("    " + tr("Node ignored."));
	}
}

void FsmXmlParser::parseTransition()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	if (nodeName == "Transition")
	{
		QString sourceName = attributes.value("Source").toString();
		QString targetName = attributes.value("Target").toString();

		// TODO: check if states exist
		shared_ptr<FsmState> source = this->getStateByName(sourceName);
		shared_ptr<FsmState> target = this->getStateByName(targetName);

		// Get ID if it is defined
		bool ok;
		componentId_t extractedId = attributes.value("Id").toULong(&ok);
		if (ok == false)
		{
			extractedId = 0;
		}

		auto fsm = dynamic_pointer_cast<Fsm>(this->machine);
		componentId_t transitionId;
		if (extractedId != 0)
		{
			transitionId = fsm->addTransition(source->getId(), target->getId(), extractedId);
		}
		else
		{
			transitionId = fsm->addTransition(source->getId(), target->getId());
		}
		auto transition = fsm->getTransition(transitionId);

		// Get slider position
		QString sliderPosStr = attributes.value("SliderPos").toString();
		if (sliderPosStr.isNull() == false)
		{
			sliderPosStr.toDouble(&ok);
			if (ok == true)
			{
				this->graphicAttributes->addAttribute(transitionId, "SliderPos", sliderPosStr);
			}
			else
			{
				this->warnings.append(tr("Warning!") + " " + tr("Unable to extract slider position for a transition"));
			}
		}

		this->currentActuator = transition;
	}
	else
	{
		this->warnings.append(tr("Unexpected node encountered while parsing") + " " + tr("transition list:"));
		this->warnings.append("    " + tr("Expected") + " \"Transition\", " + tr("got") + " \"" + nodeName + "\".");
		this->warnings.append("    " + tr("Node ignored."));
	}
}

shared_ptr<FsmState> FsmXmlParser::getStateByName(const QString& name) const
{
	auto fsm = dynamic_pointer_cast<Fsm>(this->machine);

	shared_ptr<FsmState> ret = nullptr;
	foreach(auto stateId, fsm->getAllStatesIds())
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
