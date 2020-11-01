/*
 * Copyright © 2014-2020 Clément Foucher
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
#include "fsm.h"

// StateS classes
#include "fsmstate.h"
#include "fsmtransition.h"
#include "fsmsimulator.h"
#include "fsmundocommand.h"


Fsm::Fsm(shared_ptr<MachineStatus> machineStatus) :
    Machine(machineStatus)
{

}

Fsm::Fsm() :
    Machine()
{

}

Fsm::~Fsm()
{
	this->isBeingDestroyed = true;

	// Force cleaning order
	this->initialState.reset();
	this->transitions.clear();
	this->states.clear();
}

QList<shared_ptr<FsmState> > Fsm::getStates() const
{
	return this->states;
}

shared_ptr<FsmState> Fsm::addState(QPointF position, QString name)
{
	this->setInhibitEvents(true);

	shared_ptr<FsmState> state(new FsmState(this->shared_from_this(), this->getUniqueStateName(name), position)); // Clear to use shared_from_this: this function can't be called in constructor
	connect(state.get(), &MachineActuatorComponent::actionListChangedEvent, this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	connect(state.get(), &FsmState::stateRenamedEvent,                      this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	connect(state.get(), &FsmState::statePositionChangedEvent,              this, &Fsm::statePositionChangedEventHandler);
	this->states.append(state);

	this->setInhibitEvents(false);
	this->emitMachineEditedWithoutUndoCommand();

	return state;
}

void Fsm::removeState(shared_ptr<FsmState> state)
{
	this->setInhibitEvents(true);

	disconnect(state.get(), &MachineActuatorComponent::actionListChangedEvent, this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	disconnect(state.get(), &FsmState::stateRenamedEvent,                      this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	disconnect(state.get(), &FsmState::statePositionChangedEvent,              this, &Fsm::statePositionChangedEventHandler);

	for (shared_ptr<FsmTransition> transition : state->getOutgoingTransitions())
	{
		this->removeTransition(transition);
	}

	for (shared_ptr<FsmTransition> transition : state->getIncomingTransitions())
	{
		this->removeTransition(transition);
	}

	this->states.removeAll(state);

	this->setInhibitEvents(false);
	this->emitMachineEditedWithoutUndoCommand();
}

bool Fsm::renameState(shared_ptr<FsmState> state, QString newName)
{
	QString cleanedName = newName.trimmed();

	if (state->getName() == cleanedName)
	{
		// Nothing to do
		return true;
	}

	// By this point, we know the new name is at least different from current
	QString actualName = getUniqueStateName(cleanedName);

	if (actualName == cleanedName)
	{
		state->setName(actualName);
		return true;
	}
	else
	{
		return false;
	}
}

void Fsm::setSimulator(shared_ptr<MachineSimulator> simulator)
{
	// Clear simulator
	if (simulator == nullptr)
	{
		Machine::setSimulator(nullptr);
	}
	else
	{
		// Check if simulator is of right type
		shared_ptr<FsmSimulator> fsmSimulator = dynamic_pointer_cast<FsmSimulator>(simulator);

		if (fsmSimulator != nullptr)
		{
			Machine::setSimulator(fsmSimulator);
		}
	}
}

void Fsm::forceStateActivation(shared_ptr<FsmState> stateToActivate)
{
	shared_ptr<FsmSimulator> simulator = dynamic_pointer_cast<FsmSimulator>(this->getSimulator());
	if (simulator != nullptr)
	{
		simulator->forceStateActivation(stateToActivate);
	}
}

QString Fsm::getUniqueStateName(QString nameProposal)
{
	QString baseName;
	QString currentName;
	uint i;

	if (nameProposal.isEmpty())
	{
		baseName = tr("State");
		currentName = baseName + " #0";
		i = 0;
	}
	else
	{
		baseName = nameProposal;
		currentName = nameProposal;
		i = 1;
	}

	bool nameIsValid = false;
	while (!nameIsValid)
	{
		nameIsValid = true;
		foreach(shared_ptr<FsmState> colleage, this->states)
		{
			if (colleage->getName() == currentName)
			{
				nameIsValid = false;
				i++;
				currentName = baseName + " #" + QString::number(i);
				break;
			}
		}
	}

	return currentName;
}

QList<shared_ptr<FsmTransition>> Fsm::getTransitions() const
{
	return this->transitions;
}

void Fsm::setInitialState(shared_ptr<FsmState> newInitialState)
{
	shared_ptr<FsmState> previousInitialState = this->initialState.lock();

	if (newInitialState != previousInitialState)
	{
		this->setInhibitEvents(true);

		this->initialState = newInitialState;

		if (previousInitialState != nullptr)
		{
			previousInitialState->notifyInitialStatusChanged();
		}
		if (newInitialState != nullptr)
		{
			newInitialState->notifyInitialStatusChanged();
		}

		this->setInhibitEvents(false);
		this->emitMachineEditedWithoutUndoCommand();
	}
}

/**
 * @brief Fsm::getInitialState
 * @return Returns the initial state as a shared pointer.
 * We can use a shared pointer here as states are owned by
 * this object, so we are sure that the reference is not expired.
 */
shared_ptr<FsmState> Fsm::getInitialState() const
{
	return this->initialState.lock();
}

shared_ptr<FsmTransition> Fsm::addTransition(shared_ptr<FsmState> source, shared_ptr<FsmState> target, FsmGraphicTransition* representation)
{
	this->setInhibitEvents(true);

	shared_ptr<FsmTransition> transition(new FsmTransition(this->shared_from_this(), source, target, representation));
	connect(transition.get(), &MachineActuatorComponent::actionListChangedEvent,    this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	connect(transition.get(), &FsmTransition::conditionChangedEvent,                this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	connect(transition.get(), &FsmTransition::transitionSliderPositionChangedEvent, this, &Fsm::transitionSliderPositionChangedEventHandler);

	source->addOutgoingTransition(transition);
	target->addIncomingTransition(transition);

	this->transitions.append(transition);

	this->setInhibitEvents(false);
	this->emitMachineEditedWithoutUndoCommand();

	return transition;
}

void Fsm::redirectTransition(shared_ptr<FsmTransition> transition, shared_ptr<FsmState> newSource, shared_ptr<FsmState> newTarget)
{
	this->setInhibitEvents(true);

	shared_ptr<FsmState> oldSource = transition->getSource();
	shared_ptr<FsmState> oldTarget = transition->getTarget();

	if (newSource != oldSource)
	{
		transition->setSource(newSource);
		oldSource->removeOutgoingTransition(transition);
		newSource->addOutgoingTransition(transition);
	}

	if (newTarget != oldTarget)
	{
		transition->setTarget(newTarget);
		oldTarget ->removeIncomingTransition(transition);
		newTarget->addIncomingTransition(transition);
	}

	this->setInhibitEvents(false);
	this->emitMachineEditedWithoutUndoCommand();
}

void Fsm::removeTransition(shared_ptr<FsmTransition> transition)
{
	this->setInhibitEvents(true);

	connect(transition.get(), &MachineActuatorComponent::actionListChangedEvent,    this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	connect(transition.get(), &FsmTransition::conditionChangedEvent,                this, &Fsm::unmonitoredFsmComponentEditionEventHandler);
	connect(transition.get(), &FsmTransition::transitionSliderPositionChangedEvent, this, &Fsm::transitionSliderPositionChangedEventHandler);

	transition->getSource()->removeOutgoingTransition(transition);
	transition->getTarget()->removeIncomingTransition(transition);

	this->transitions.removeAll(transition);

	this->setInhibitEvents(false);
	this->emitMachineEditedWithoutUndoCommand();
}

void Fsm::unmonitoredFsmComponentEditionEventHandler()
{
	this->emitMachineEditedWithoutUndoCommand();
}

void Fsm::statePositionChangedEventHandler(shared_ptr<FsmState> originator)
{
	FsmUndoCommand* undoCommand = new FsmUndoCommand(originator);
	this->emitMachineEditedWithUndoCommand(undoCommand);
}

void Fsm::transitionSliderPositionChangedEventHandler()
{
	this->emitMachineEditedWithoutUndoCommand(MachineUndoCommand::undo_command_id::fsmUndoMoveConditionSliderId);
}

shared_ptr<FsmState> Fsm::getStateByName(const QString &name) const
{
	shared_ptr<FsmState> ret = nullptr;

	foreach(shared_ptr<FsmState> ptr, this->states)
	{
		if (ptr->getName() == name)
		{
			ret = ptr;
			break;
		}
	}

	return ret;
}
