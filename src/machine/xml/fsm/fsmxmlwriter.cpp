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
#include "fsmxmlwriter.h"

// Qt
#include <QXmlStreamWriter>

// StateS
#include "machinemanager.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "viewconfiguration.h"
#include "graphicmachine.h"
#include "graphicattributes.h"


void FsmXmlWriter::writeSubmachineToStream()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	auto fsmGraphicAttributes = graphicMachine->getGraphicAttributes();
	if (fsmGraphicAttributes == nullptr) return;


	this->writeFsmStates(fsm, fsmGraphicAttributes);
	this->writeFsmTransitions(fsm, fsmGraphicAttributes);
}

void FsmXmlWriter::writeMachineType()
{
	this->stream->writeAttribute("Type", "FSM");
}

void FsmXmlWriter::writeFsmStates(std::shared_ptr<Fsm> fsm, std::shared_ptr<GraphicAttributes> fsmGraphicAttributes)
{
	this->stream->writeStartElement("States");

	for (auto stateId : fsm->getAllStatesIds())
	{
		auto state = fsm->getState(stateId);
		this->stream->writeStartElement("State");

		// Name
		this->stream->writeAttribute("Name", state->getName());

		// Initial
		if (stateId == fsm->getInitialStateId())
		{
			this->stream->writeAttribute("IsInitial", "true");
		}

		QString x = fsmGraphicAttributes->getAttribute(stateId, "X");
		QString y = fsmGraphicAttributes->getAttribute(stateId, "Y");

		if ( (this->mode == WriteMode_t::writeToFile) && (this->viewConfiguration != nullptr) ) // Full save to file
		{
			// Position => offseted so that scene top-left corner is in (0,0)
			this->stream->writeAttribute("X", QString::number(x.toDouble() + this->viewConfiguration->sceneTranslation.x()));
			this->stream->writeAttribute("Y", QString::number(y.toDouble() + this->viewConfiguration->sceneTranslation.y()));
		}
		else // Light save for undo
		{
			// Position
			this->stream->writeAttribute("X", x);
			this->stream->writeAttribute("Y", y);
		}

		if (this->mode == WriteMode_t::writeToUndo)
		{
			this->stream->writeAttribute("Id", QString::number(static_cast<uint32_t>(stateId)));
		}

		// Actions
		this->writeActuatorActions(state);

		this->stream->writeEndElement();
	}

	this->stream->writeEndElement();
}

void FsmXmlWriter::writeFsmTransitions(std::shared_ptr<Fsm> fsm, std::shared_ptr<GraphicAttributes> fsmGraphicAttributes)
{
	this->stream->writeStartElement("Transitions");

	for (auto transitionId : fsm->getAllTransitionsIds())
	{
		auto transition = fsm->getTransition(transitionId);

		this->stream->writeStartElement("Transition");

		auto sourceState = fsm->getState(transition->getSourceStateId());
		auto targetState = fsm->getState(transition->getTargetStateId());

		this->stream->writeAttribute("Source", sourceState->getName());
		this->stream->writeAttribute("Target", targetState->getName());

		QString sliderPosition = fsmGraphicAttributes->getAttribute(transitionId, "SliderPos");
		if (sliderPosition.isNull() == false)
		{
			this->stream->writeAttribute("SliderPos", sliderPosition);
		}

		if (this->mode == WriteMode_t::writeToUndo)
		{
			this->stream->writeAttribute("Id", QString::number(static_cast<uint32_t>(transitionId)));
		}

		// Deal with equations
		auto condition = transition->getCondition();
		if (condition != nullptr)
		{
			this->stream->writeStartElement("Condition");
			this->writeLogicEquation(condition);
			this->stream->writeEndElement(); // Condition
		}

		// Actions
		this->writeActuatorActions(transition);

		this->stream->writeEndElement();
	}

	this->stream->writeEndElement();
}
