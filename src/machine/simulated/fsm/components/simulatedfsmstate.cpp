/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "simulatedfsmstate.h"

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "fsmstate.h"


//
// Class object definition
//

SimulatedFsmState::SimulatedFsmState(componentId_t logicComponentId) :
	SimulatedActuatorComponent(logicComponentId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->componentId);
	if (logicState == nullptr) return;


	this->name = logicState->getName();
	this->outgoingTransitionsIds = logicState->getOutgoingTransitionsIds();
}

void SimulatedFsmState::setActive(bool active)
{
	this->isActive = active;

	emit this->simulatedComponentUpdatedEvent(this->componentId);
}

bool SimulatedFsmState::getIsActive() const
{
	return this->isActive;
}

QString SimulatedFsmState::getName() const
{
	return this->name;
}

const QList<componentId_t> SimulatedFsmState::getOutgoingTransitionsIds() const
{
	return this->outgoingTransitionsIds;
}
