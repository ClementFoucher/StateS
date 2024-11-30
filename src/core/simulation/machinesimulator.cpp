/*
 * Copyright © 2014-2024 Clément Foucher
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
#include "machinesimulator.h"

// StateS classes
#include "clock.h"
#include "simulatedcomponent.h"


MachineSimulator::MachineSimulator()
{
	this->clock = shared_ptr<Clock>(new Clock());
}

MachineSimulator::~MachineSimulator()
{
	auto components = this->simulatedComponents.values();
	for (auto component : components)
	{
		delete component;
	}
}

shared_ptr<Clock> MachineSimulator::getClock() const
{
	return this->clock;
}

void MachineSimulator::reset()
{
	this->clock->reset();
}

void MachineSimulator::doStep()
{
	this->clock->nextStep();
}

void MachineSimulator::start(uint period)
{
	this->clock->start(period);
}

void MachineSimulator::suspend()
{
	this->clock->stop();
}

QList<SimulatedComponent*> MachineSimulator::getSimulatedComponents() const
{
	return this->simulatedComponents.values();
}

SimulatedComponent* MachineSimulator::getComponent(componentId_t componentId) const
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
