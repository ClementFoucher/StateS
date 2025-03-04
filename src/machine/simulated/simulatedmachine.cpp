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
#include "clock.h"
#include "simulatedcomponent.h"


SimulatedMachine::SimulatedMachine()
{
	this->clock = shared_ptr<Clock>(new Clock());
}

SimulatedMachine::~SimulatedMachine()
{
	auto components = this->simulatedComponents.values();
	for (auto component : components)
	{
		delete component;
	}
}

shared_ptr<Clock> SimulatedMachine::getClock() const
{
	return this->clock;
}

void SimulatedMachine::reset()
{
	this->clock->reset();
}

void SimulatedMachine::doStep()
{
	this->clock->nextStep();
}

void SimulatedMachine::start(uint period)
{
	this->clock->start(period);
	emit this->autoSimulationToggledEvent(true);
}

void SimulatedMachine::suspend()
{
	this->clock->stop();
	emit this->autoSimulationToggledEvent(false);
}

const QList<SimulatedComponent*> SimulatedMachine::getSimulatedComponents() const
{
	return this->simulatedComponents.values();
}

SimulatedComponent* SimulatedMachine::getComponent(componentId_t componentId) const
{
	if (this->simulatedComponents.contains(componentId))
	{
		return this->simulatedComponents[componentId];
	}
	else
	{
		return nullptr;
	}
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
