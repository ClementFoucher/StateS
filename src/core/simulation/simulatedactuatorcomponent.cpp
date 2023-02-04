/*
 * Copyright © 2023 Clément Foucher
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
#include "actiononsignal.h"


SimulatedActuatorComponent::SimulatedActuatorComponent(componentId_t componentId)
{
	this->componentId = componentId;
}

SimulatedActuatorComponent::~SimulatedActuatorComponent()
{

}

void SimulatedActuatorComponent::activateActions()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuatorComponent = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->componentId));
	if (actuatorComponent == nullptr) return;

	auto actionList = actuatorComponent->getActions();
	foreach (shared_ptr<ActionOnSignal> action, actionList)
	{
		action->beginAction();
	}
}

void SimulatedActuatorComponent::deactivateActions()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuatorComponent = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->componentId));
	if (actuatorComponent == nullptr) return;

	auto actionList = actuatorComponent->getActions();
	foreach (shared_ptr<ActionOnSignal> action, actionList)
	{
		action->endAction();
	}
}
