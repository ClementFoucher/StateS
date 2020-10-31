/*
 * Copyright © 2017-2020 Clément Foucher
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
#include "fsmgraphictransition.h"
#include "machinestatus.h"


FsmXmlParser::FsmXmlParser()
{
	this->currentGroup = group_e::none;
	this->currentSubGroup = subgroup_e::none;
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

void FsmXmlParser::buildMachineFromXml()
{
	if (this->status != nullptr)
	{
		if (this->fileName.isNull() == false)
		{
			this->status->setSaveFilePath(this->fileName);
			this->status->setHasSaveFile(true);
		}
		this->machine = shared_ptr<Fsm>(new Fsm(this->status));
	}
	else
	{
		this->machine = shared_ptr<Fsm>(new Fsm());
	}

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
			this->currentGroup = group_e::configuration_group;
		}
		else if (nodeName == "Signals")
		{
			this->currentGroup = group_e::signals_group;
		}
		else if (nodeName == "States")
		{
			this->currentGroup = group_e::states_group;
		}
		else if (nodeName == "Transitions")
		{
			this->currentGroup = group_e::transitions_group;
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
		case group_e::configuration_group:
			this->parseConfiguration();
			break;
		case group_e::signals_group:
			this->parseSignal();
			break;
		case group_e::states_group:
			this->currentSubGroup = subgroup_e::state;
			this->parseState();
			break;
		case group_e::transitions_group:
			this->currentSubGroup = subgroup_e::transition;
			this->parseTransition();
			break;
		case group_e::none:
			this->warnings.append("    "  + tr("Ignored node") + " " + nodeName);
			break;
		}
	}
	else // Sub-group
	{
		switch (this->currentSubGroup)
		{
		case subgroup_e::state:
			if (nodeName == "Actions")
			{
				this->currentSubGroup = subgroup_e::actions_group;
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"State\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"Actions\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case subgroup_e::transition:
			if (nodeName == "Actions")
			{
				this->currentSubGroup = subgroup_e::actions_group;
			}
			else if (nodeName == "Condition")
			{
				this->currentSubGroup = subgroup_e::condition;
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Transition\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"Actions\" or \"Condition\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case subgroup_e::actions_group:
			if (nodeName == "Action")
			{
				this->currentSubGroup = subgroup_e::action;
				this->parseAction();
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Actions\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"Action\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case subgroup_e::condition:
		case subgroup_e::operand:
			if (nodeName == "LogicVariable")
			{
				this->currentSubGroup = subgroup_e::logicVariable;
				this->parseLogicEquation();
			}
			else if (nodeName == "LogicEquation")
			{
				this->currentSubGroup = subgroup_e::logicEquation;
				this->parseLogicEquation();
			}
			else
			{
				this->warnings.append(tr("Error!") + " " + tr("Unexpected node found while parsing") + "\"Operand\" or \"Condition\"" + tr("node."));
				this->warnings.append("    " + tr("Expected") + " \"LogicVariable\" or \"LogicEquation\", " + tr("got") + " \"" + nodeName + "\".");
			}
			break;
		case subgroup_e::logicEquation:
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
			this->currentSubGroup = subgroup_e::operand;
			break;
		case subgroup_e::action:
		case subgroup_e::logicVariable:
			// Nothing to do: these nodes does not have children
			break;
		case subgroup_e::none:
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
	case subgroup_e::action:
		this->currentSubGroup = subgroup_e::actions_group;
		break;
	case subgroup_e::actions_group:
		if (this->currentGroup == group_e::states_group)
		{
			this->currentSubGroup = subgroup_e::state;
		}
		else if (this->currentGroup == group_e::transitions_group)
		{
			this->currentSubGroup = subgroup_e::transition;
		}
		break;
	case subgroup_e::condition:
		this->currentSubGroup = subgroup_e::transition;
		transition = dynamic_pointer_cast<FsmTransition>(this->currentActuator);
		transition->setCondition(this->rootLogicEquation);
		this->rootLogicEquation = nullptr;
		break;
	case subgroup_e::state:
	case subgroup_e::transition:
		this->currentActuator = nullptr;
		this->currentSubGroup = subgroup_e::none;
		break;
	case subgroup_e::logicEquation:
	case subgroup_e::logicVariable:
		if (this->currentLevel == 4)
		{
			this->currentSubGroup = subgroup_e::condition;
		}
		else
		{
			this->currentSubGroup = subgroup_e::operand;
		}
		break;
	case subgroup_e::operand:
		this->currentSubGroup = subgroup_e::logicEquation;
		this->treatEndOperand();
		break;
	case subgroup_e::none:
		if (this->currentLevel == 1)
		{
			this->currentGroup = group_e::none;
		}
		break;
	}
}


void FsmXmlParser::parseState()
{
	QXmlStreamAttributes attributes = this->xmlReader->attributes();
	QString nodeName = this->xmlReader->name().toString();

	if (nodeName == "State")
	{
		QString stateName = attributes.value("Name").toString();

		if (stateName.isNull())
		{
			this->warnings.append(tr("Error!") + " " + tr("Unable to extract state name."));
			this->warnings.append("    " + tr("Node ignored."));
			return;
		}

		bool ok;
		bool positionOk = true;
		qreal x = attributes.value("X").toDouble(&ok);
		if (ok == false)
		{
			positionOk = false;
		}
		qreal y = attributes.value("Y").toDouble(&ok);
		if (ok == false)
		{
			positionOk = false;
		}

		QPointF position;
		if (positionOk == true)
		{
			position = QPointF(x, y);
		}
		else
		{
			this->warnings.append(tr("Warning!") + " " + tr("Unable to extract state position for state ") + stateName);
		}

		shared_ptr<Fsm> fsm = this->getFsm();
		shared_ptr<FsmState> state = fsm->addState(position, stateName);

		if (attributes.value("IsInitial").isNull() == false)
		{
			fsm->setInitialState(state);
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

		shared_ptr<Fsm> fsm = this->getFsm();

		shared_ptr<FsmState> source = fsm->getStateByName(sourceName);
		shared_ptr<FsmState> target = fsm->getStateByName(targetName);

		// TODO: check if states exist
		shared_ptr<FsmTransition> transition = fsm->addTransition(source, target);

		QStringRef sliderPosString = attributes.value("SliderPos");
		qreal sliderPos;
		if (!sliderPosString.isEmpty())
		{
			bool ok;
			sliderPos = sliderPosString.toFloat(&ok)/100;

			if (ok == false)
			{
				sliderPos = 0.5;
				// TODO: warning
			}
		}
		else
		{
			sliderPos = 0.5;
		}
		transition->getGraphicRepresentation()->setConditionLineSliderPosition(sliderPos);

		this->currentActuator = transition;
	}
	else
	{
		this->warnings.append(tr("Unexpected node encountered while parsing") + " " + tr("transition list:"));
		this->warnings.append("    " + tr("Expected") + " \"Transition\", " + tr("got") + " \"" + nodeName + "\".");
		this->warnings.append("    " + tr("Node ignored."));
	}
}

shared_ptr<Fsm> FsmXmlParser::getFsm() const
{
	return dynamic_pointer_cast<Fsm>(this->machine);
}
