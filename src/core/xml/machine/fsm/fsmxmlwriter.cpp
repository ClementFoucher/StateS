/*
 * Copyright © 2014-2021 Clément Foucher
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

// Qt classes
#include <QXmlStreamWriter>

// StateS classes
#include "states.h"
#include "machinemanager.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmgraphicstate.h"
#include "fsmtransition.h"
#include "fsmgraphictransition.h"
#include "viewconfiguration.h"


FsmXmlWriter::FsmXmlWriter(shared_ptr<MachineManager> machineManager) :
    MachineXmlWriter(machineManager)
{

}

void FsmXmlWriter::writeFsmStates()
{
	shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(this->machineManager->getMachine());

	this->stream->writeStartElement("States");

	foreach (shared_ptr<FsmState> state, fsm->getStates())
	{
		this->stream->writeStartElement("State");

		// Name
		this->stream->writeAttribute("Name", state->getName());

		// Initial
		if (state->isInitial())
		{
			this->stream->writeAttribute("IsInitial", "true");
		}

		if (this->writingToFile == true)
		{
			shared_ptr<ViewConfiguration> viewConfiguration = this->machineManager->getViewConfiguration();
			// Position => offseted so that scene top-left corner is in (0,0)
			this->stream->writeAttribute("X", QString::number(state->getGraphicRepresentation()->scenePos().x() + viewConfiguration->sceneTranslation.x()));
			this->stream->writeAttribute("Y", QString::number(state->getGraphicRepresentation()->scenePos().y() + viewConfiguration->sceneTranslation.y()));
		}
		else
		{
			// Position
			this->stream->writeAttribute("X", QString::number(state->getGraphicRepresentation()->scenePos().x()));
			this->stream->writeAttribute("Y", QString::number(state->getGraphicRepresentation()->scenePos().y()));
		}

		// Actions
		this->writeActuatorActions(state);

		this->stream->writeEndElement();
	}

	this->stream->writeEndElement();
}

void FsmXmlWriter::writeFsmTransitions()
{
	shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(this->machineManager->getMachine());

	this->stream->writeStartElement("Transitions");

	foreach (shared_ptr<FsmTransition> transition, fsm->getTransitions())
	{
		this->stream->writeStartElement("Transition");

		this->stream->writeAttribute("Source", transition->getSource()->getName());
		this->stream->writeAttribute("Target", transition->getTarget()->getName());

		int sliderPosition = transition->getGraphicRepresentation()->getConditionLineSliderPosition()*100;
		this->stream->writeAttribute("SliderPos", QString::number(sliderPosition));

		// Deal with equations
		if (transition->getCondition() != nullptr)
		{
			this->stream->writeStartElement("Condition");
			this->writeLogicEquation(transition->getCondition());
			this->stream->writeEndElement(); // Condition
		}

		// Actions
		this->writeActuatorActions(transition);

		this->stream->writeEndElement();
	}

	this->stream->writeEndElement();
}

void FsmXmlWriter::writeMachineToStream()
{
	this->stream->writeStartElement("FSM");
	this->stream->writeAttribute("Name", this->machineManager->getMachine()->getName());
	this->stream->writeAttribute("StateS_version", StateS::getVersion());

	this->writeMachineCommonElements();
	this->writeFsmStates();
	this->writeFsmTransitions();

	this->stream->writeEndElement(); // End FSM element
}
