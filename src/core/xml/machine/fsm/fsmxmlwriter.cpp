/*
 * Copyright © 2014-2023 Clément Foucher
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
#include "fsmtransition.h"
#include "viewconfiguration.h"
#include "graphicmachine.h"
#include "graphicattributes.h"


FsmXmlWriter::FsmXmlWriter(MachineXmlWriterMode_t mode, shared_ptr<ViewConfiguration> viewConfiguration) :
    MachineXmlWriter(mode, viewConfiguration)
{

}

void FsmXmlWriter::writeMachineToStream()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	auto fsmGraphicAttributes = graphicMachine->getGraphicAttributes();
	if (fsmGraphicAttributes == nullptr) return;

	this->stream->writeStartElement("FSM");
	this->stream->writeAttribute("Name", fsm->getName());
	this->stream->writeAttribute("StateS_version", StateS::getVersion());

	this->writeMachineCommonElements();
	this->writeFsmStates(fsm, fsmGraphicAttributes);
	this->writeFsmTransitions(fsm, fsmGraphicAttributes);

	this->stream->writeEndElement(); // End FSM element
}

void FsmXmlWriter::writeFsmStates(shared_ptr<Fsm> fsm, shared_ptr<GraphicAttributes> fsmGraphicAttributes)
{
	this->stream->writeStartElement("States");

	foreach (auto stateId, fsm->getAllStatesIds())
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

		if ( (this->mode == MachineXmlWriterMode_t::writeToFile) && (this->viewConfiguration != nullptr) ) // Full save to file
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

		if (this->mode == MachineXmlWriterMode_t::writeToUndo)
		{
			this->stream->writeAttribute("Id", QString::number(stateId));
		}

		// Actions
		this->writeActuatorActions(state);

		this->stream->writeEndElement();
	}

	this->stream->writeEndElement();
}

void FsmXmlWriter::writeFsmTransitions(shared_ptr<Fsm> fsm, shared_ptr<GraphicAttributes> fsmGraphicAttributes)
{
	this->stream->writeStartElement("Transitions");

	foreach (auto transitionId, fsm->getAllTransitionsIds())
	{
		auto transition = fsm->getTransition(transitionId);

		this->stream->writeStartElement("Transition");

		auto sourceState = fsm->getState(transition->getSourceStateId());
		auto targetState = fsm->getState(transition->getTargetStateId());

		this->stream->writeAttribute("Source", sourceState->getName());
		this->stream->writeAttribute("Target", targetState->getName());

		QString sliderPosition = fsmGraphicAttributes->getAttribute(transitionId, "SliderPos");

		this->stream->writeAttribute("SliderPos", sliderPosition);

		if (this->mode == MachineXmlWriterMode_t::writeToUndo)
		{
			this->stream->writeAttribute("Id", QString::number(transitionId));
		}

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
