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
#include "simulatedfsmtransition.h"

// StateS
#include "machinemanager.h"
#include "statesexception.h"
#include "fsm.h"
#include "fsmtransition.h"
#include "simulatedequation.h"


//
// Class object definition
//

SimulatedFsmTransition::SimulatedFsmTransition(ComponentId logicComponentId) :
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
		try
		{
			this->condition = std::make_shared<SimulatedEquation>(logicCondition); // Throws StatesException
		}
		catch (const StatesException& e)
		{
			if (e.getCause() != StatesException::Cause_t::ill_formed_equation) throw;

			// Ill-formed equations are always false
			auto falseCondition = std::make_shared<Equation>(Equation::Operator_t::identity);
			falseCondition->setOperand(0, BooleanValue::falseValue());

			this->condition = std::make_shared<SimulatedEquation>(falseCondition); // Throws StatesException: Ignored. Equation is well-formed.
		}
	}
	else // (logicCondition == nullptr)
	{
		// Implicit condition is always true
		auto trueCondition = std::make_shared<Equation>(Equation::Operator_t::identity);
		trueCondition->setOperand(0, BooleanValue::trueValue());

		this->condition = std::make_shared<SimulatedEquation>(trueCondition); // Throws StatesException: Ignored. Equation is well-formed.
	}

	connect(this->condition.get(), &SimulatedEquation::equationCurrentValueChangedEvent, this, &SimulatedFsmTransition::conditionChangedEventHandler);
}

std::shared_ptr<const SimulatedEquation> SimulatedFsmTransition::getCondition() const
{
	return this->condition;
}

ComponentId SimulatedFsmTransition::getSourceStateId() const
{
	return this->sourceStateId;
}

ComponentId SimulatedFsmTransition::getTargetStateId() const
{
	return this->targetStateId;
}

void SimulatedFsmTransition::conditionChangedEventHandler()
{
	emit this->simulatedComponentUpdatedEvent(this->componentId);
}
