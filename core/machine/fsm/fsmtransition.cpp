/*
 * Copyright © 2014-2017 Clément Foucher
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
#include "fsmtransition.h"

// StateS classes
#include "equation.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmgraphictransition.h"


FsmTransition::FsmTransition(shared_ptr<Fsm> parent, shared_ptr<FsmState> source, shared_ptr<FsmState> target, FsmGraphicTransition* representation) :
    FsmComponent(parent)
{
	this->source = source;
	this->target = target;

	if (representation != nullptr)
	{
		this->graphicRepresentation = representation;
	}
	else
	{
		this->graphicRepresentation = new FsmGraphicTransition();
	}

	connect(this->graphicRepresentation, &QObject::destroyed,                                         this, &FsmTransition::graphicRepresentationDeletedEventHandler);
	connect(this->graphicRepresentation, &FsmGraphicTransition::transitionSliderPositionChangedEvent, this, &FsmTransition::transitionSliderPositionChangedEvent);

	// Propagates local events to the more general "configuration changed" event
	connect(this, &FsmTransition::conditionChangedEvent, this, &MachineComponent::componentNeedsGraphicUpdateEvent);
}

FsmTransition::~FsmTransition()
{
	delete this->graphicRepresentation;
}

FsmGraphicTransition* FsmTransition::getGraphicRepresentation()
{
	if (this->graphicRepresentation != nullptr)
	{
		this->graphicRepresentation->setLogicTransition(this->shared_from_this());
	}
	return this->graphicRepresentation;
}

void FsmTransition::graphicRepresentationDeletedEventHandler()
{
	this->graphicRepresentation = nullptr;
}

shared_ptr<FsmState> FsmTransition::getSource() const
{
	return this->source.lock();
}

void FsmTransition::setSource(shared_ptr<FsmState> newSource)
{
	this->source = newSource;
}

shared_ptr<FsmState> FsmTransition::getTarget() const
{
	return this->target.lock();
}

void FsmTransition::setTarget(shared_ptr<FsmState> newTarget)
{
	this->target = newTarget;
}

shared_ptr<Signal> FsmTransition::getCondition() const
{
	if (this->condition != nullptr)
	{
		if (this->condition->getFunction() != Equation::nature::identity)
			return this->condition;
		else
			return this->condition->getOperand(0); // Throws StatesException - Identity op always has operand 0, even if nullptr - ignored
	}
	else
	{
		return nullptr;
	}
}

void FsmTransition::setCondition(shared_ptr<Signal> signalNewCondition)
{
	if (this->condition != nullptr)
	{
		disconnect(this->condition.get(), &Signal::signalDynamicStateChangedEvent,        this, &MachineComponent::componentSimulatedStateChangedEvent);
		disconnect(this->condition.get(), &Signal::signalStaticConfigurationChangedEvent, this, &FsmTransition::conditionChangedEvent);
	}

	shared_ptr<Equation> equationNewCondition = dynamic_pointer_cast<Equation>(signalNewCondition);
	if (equationNewCondition == nullptr)
	{
		QVector<shared_ptr<Signal>> operand;
		operand.append(signalNewCondition);
		equationNewCondition = shared_ptr<Equation>(new Equation(Equation::nature::identity, operand));
	}

	this->condition = equationNewCondition;

	if (this->condition != nullptr)
	{
		// Propagate events
		connect(this->condition.get(), &Signal::signalDynamicStateChangedEvent,        this, &MachineComponent::componentSimulatedStateChangedEvent);
		connect(this->condition.get(), &Signal::signalStaticConfigurationChangedEvent, this, &FsmTransition::conditionChangedEvent);
	}

	emit conditionChangedEvent();
}

void FsmTransition::clearCondition()
{
	this->setCondition(nullptr);
}

uint FsmTransition::getAllowedActionTypes() const
{
	return (pulse | set | reset | assign);
}
