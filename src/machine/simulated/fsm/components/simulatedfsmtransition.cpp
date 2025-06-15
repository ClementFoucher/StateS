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
#include "simulatedfsmtransition.h"

// States classes
#include "machinemanager.h"
#include "fsm.h"
#include "fsmtransition.h"
#include "simulatedequation.h"


//
// Class object definition
//

SimulatedFsmTransition::SimulatedFsmTransition(componentId_t logicComponentId) :
	SimulatedActuatorComponent(logicComponentId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicTransition = fsm->getTransition(this->componentId);
	if (logicTransition == nullptr) return;


	this->sourceStateId = logicTransition->getSourceStateId();
	this->targetStateId = logicTransition->getTargetStateId();

	auto logicCondition = logicTransition->getCondition();
	if (logicCondition != nullptr)
	{
		this->condition = shared_ptr<SimulatedEquation>(new SimulatedEquation(logicCondition));
	}

	connect(this->condition.get(), &SimulatedEquation::equationCurrentValueChangedEvent, this, &SimulatedFsmTransition::conditionChangedEventHandler);
}

shared_ptr<SimulatedEquation> SimulatedFsmTransition::getCondition()
{
	return this->condition;
}

componentId_t SimulatedFsmTransition::getSourceStateId() const
{
	return this->sourceStateId;
}

componentId_t SimulatedFsmTransition::getTargetStateId() const
{
	return this->targetStateId;
}

void SimulatedFsmTransition::conditionChangedEventHandler()
{
	emit this->simulatedComponentUpdatedEvent(this->componentId);
}
