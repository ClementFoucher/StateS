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
#include "simulatedmachine.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "simulatedactuatorcomponent.h"
#include "simulatedvariable.h"


SimulatedMachine::SimulatedMachine()
{

}

void SimulatedMachine::build()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	auto variablesIds = machine->getAllVariablesIds();
	for (auto variableId : variablesIds)
	{
		auto simulatedVariable = shared_ptr<SimulatedVariable>(new SimulatedVariable(variableId));
		this->registerSimulatedComponent(variableId, simulatedVariable);
	}
}

shared_ptr<SimulatedActuatorComponent> SimulatedMachine::getSimulatedActuatorComponent(componentId_t actuatorId) const
{
	return dynamic_pointer_cast<SimulatedActuatorComponent>(this->getSimulatedComponent(actuatorId));
}

shared_ptr<SimulatedVariable> SimulatedMachine::getSimulatedVariable(componentId_t variableId) const
{
	return dynamic_pointer_cast<SimulatedVariable>(this->getSimulatedComponent(variableId));
}

void SimulatedMachine::reset()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	// Reset all machine variables
	for (auto& inputId : machine->getInputVariablesIds())
	{
		auto simulatedInput = this->getSimulatedVariable(inputId);
		if (simulatedInput == nullptr) continue;


		simulatedInput->reinitialize();
	}
	for (auto& variableId : machine->getInternalVariablesIds())
	{
		auto simulatedVariable = this->getSimulatedVariable(variableId);
		if (simulatedVariable == nullptr) continue;


		simulatedVariable->reinitialize();
	}
	for (auto& outputId : machine->getOutputVariablesIds())
	{
		auto simulatedOutput = this->getSimulatedVariable(outputId);
		if (simulatedOutput == nullptr) continue;


		simulatedOutput->reinitialize();
	}

	// Reset actions
	auto componentsIds = this->simulatedComponents.keys();
	for (auto& componentId : componentsIds)
	{
		auto simulatedActuator = this->getSimulatedActuatorComponent(componentId);
		if (simulatedActuator == nullptr) continue;


		simulatedActuator->resetActions();
	}

	this->subcomponentReset();
}

void SimulatedMachine::prepareStep()
{
	this->subcomponentPrepareStep();
}

void SimulatedMachine::prepareActions()
{
	this->subcomponentPrepareActions();
}

void SimulatedMachine::doStep()
{
	this->subcomponentDoStep();
}

void SimulatedMachine::setMemorizedStateActionBehavior(SimulationBehavior_t behv)
{
	this->memorizedStateActionBehavior = behv;
}

void SimulatedMachine::setContinuousStateActionBehavior(SimulationBehavior_t behv)
{
	this->continuousStateActionBehavior = behv;
}

void SimulatedMachine::setMemorizedTransitionActionBehavior(SimulationBehavior_t behv)
{
	this->memorizedTransitionActionBehavior = behv;
}

void SimulatedMachine::setPulseTransitionActionBehavior(SimulationBehavior_t behv)
{
	this->pulseTransitionActionBehavior = behv;
}

void SimulatedMachine::registerSimulatedComponent(componentId_t componentId, shared_ptr<SimulatedComponent> component)
{
	this->simulatedComponents[componentId] = component;

	connect(component.get(), &SimulatedComponent::simulatedComponentUpdatedEvent, this, &SimulatedMachine::simulatedComponentUpdatedEvent);
}

shared_ptr<SimulatedComponent> SimulatedMachine::getSimulatedComponent(componentId_t componentId) const
{
	if (this->simulatedComponents.contains(componentId) == false) return nullptr;


	return this->simulatedComponents[componentId];
}
