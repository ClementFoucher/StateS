/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "simulatedactuatorcomponent.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "simulatedactiononvariable.h"


SimulatedActuatorComponent::SimulatedActuatorComponent(componentId_t componentId) :
	SimulatedComponent(componentId)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto logicComponent = machine->getActuatorComponent(componentId);
	if (logicComponent == nullptr) return;


	for (auto& action : logicComponent->getActions())
	{
		auto simulatedAction = shared_ptr<SimulatedActionOnVariable>(new SimulatedActionOnVariable(action));
		this->actionList.append(simulatedAction);
	}
}

const QList<shared_ptr<SimulatedActionOnVariable>> SimulatedActuatorComponent::getActions() const
{
	return this->actionList;
}

void SimulatedActuatorComponent::resetActions()
{
	for (auto& simulatedAction : this->actionList)
	{
		if (simulatedAction == nullptr) continue;


		simulatedAction->resetAction();
	}
}
