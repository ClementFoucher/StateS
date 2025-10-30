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
#include "machinesimulator.h"

// Qt classes
#include <QTimer>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "fsm.h"
#include "simulatedfsm.h"


MachineSimulator::MachineSimulator()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machine);
	if (fsm != nullptr) // Only fsm type existing for now
	{
		this->simulatedMachine = make_shared<SimulatedFsm>();
	}
	else
	{
		return;
	}


	connect(this->simulatedMachine.get(), &SimulatedMachine::emergencyShutDownEvent, this, &MachineSimulator::emergencyShutDownEventHandler);
}

void MachineSimulator::initialize()
{
	this->simulatedMachine->build();
	this->simulatedMachine->reset();
}

void MachineSimulator::reset()
{
	this->suspend();
	this->simulatedMachine->reset();

	emit this->timelineResetEvent();
}

void MachineSimulator::doStep()
{
	this->simulatedMachine->prepareStep();
	this->simulatedMachine->prepareActions();
	emit this->timelineDoStepEvent();
	this->simulatedMachine->doStep();
}

void MachineSimulator::start(uint period)
{
	if (this->timer == nullptr)
	{
		this->timer = make_shared<QTimer>();
		connect(this->timer.get(), &QTimer::timeout, this, &MachineSimulator::timerTimeoutEventHandler);
	}

	this->timer->setInterval(period);
	this->timer->start();

	emit this->autoSimulationToggledEvent(true);
}

void MachineSimulator::suspend()
{
	if (this->timer == nullptr) return;

	if (this->timer->isActive() == false) return;


	this->timer->stop();

	emit this->autoSimulationToggledEvent(false);
}

void MachineSimulator::setMemorizedStateActionBehavior(SimulationBehavior_t behv)
{
	if (this->simulatedMachine == nullptr) return;


	this->simulatedMachine->setMemorizedStateActionBehavior(behv);
}

void MachineSimulator::setContinuousStateActionBehavior(SimulationBehavior_t behv)
{
	if (this->simulatedMachine == nullptr) return;


	this->simulatedMachine->setContinuousStateActionBehavior(behv);
}

void MachineSimulator::setMemorizedTransitionActionBehavior(SimulationBehavior_t behv)
{
	if (this->simulatedMachine == nullptr) return;


	this->simulatedMachine->setMemorizedTransitionActionBehavior(behv);
}

void MachineSimulator::setPulseTransitionActionBehavior(SimulationBehavior_t behv)
{
	if (this->simulatedMachine == nullptr) return;


	this->simulatedMachine->setPulseTransitionActionBehavior(behv);
}

shared_ptr<SimulatedMachine> MachineSimulator::getSimulatedMachine() const
{
	return this->simulatedMachine;
}

void MachineSimulator::timerTimeoutEventHandler()
{
	this->doStep();
}

void MachineSimulator::emergencyShutDownEventHandler()
{
	this->suspend();
}
