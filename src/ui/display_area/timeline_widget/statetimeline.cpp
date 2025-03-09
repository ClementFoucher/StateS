/*
 * Copyright © 2024-2025 Clément Foucher
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
#include "statetimeline.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>

// StateS classes
#include "machinemanager.h"
#include "machinesimulator.h"
#include "simulatedfsm.h"
#include "simulatedfsmstate.h"
#include "graphicvectortimeline.h"


StateTimeLine::StateTimeLine(QWidget* parent) :
    QWidget(parent)
{
	auto machineSimulator = machineManager->getMachineSimulator();
	if (machineSimulator == nullptr) return;

	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto initialStateId = simulatedFsm->getInitialStateId();
	auto initialSimulatedState = simulatedFsm->getSimulatedState(initialStateId);
	if (initialSimulatedState == nullptr) return;


	QHBoxLayout* globalLayout = new QHBoxLayout(this);

	QLabel* titleLabel = new QLabel(tr("State"));
	globalLayout->addWidget(titleLabel);

	QVBoxLayout* bitsLayout = new QVBoxLayout();
	QHBoxLayout* innerLayout = new QHBoxLayout();

	this->stateDisplay = new GraphicVectorTimeLine(0, initialSimulatedState->getName());
	this->stateDisplay->setMinimumHeight(30);
	this->stateDisplay->setMaximumHeight(30);
	innerLayout->addWidget(this->stateDisplay);

	bitsLayout->addLayout(innerLayout);


	globalLayout->addLayout(bitsLayout);

	connect(simulatedFsm.get(), &SimulatedFsm::stateChangedEvent, this, &StateTimeLine::updateCurrentValue);

	connect(machineSimulator.get(), &MachineSimulator::timelineDoStepEvent, this, &StateTimeLine::doStepEventHandler);
	connect(machineSimulator.get(), &MachineSimulator::timelineResetEvent,  this, &StateTimeLine::resetEventHandler);
}

void StateTimeLine::doStepEventHandler()
{
	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto currentStateId = simulatedFsm->getActiveStateId();
	auto currentSimulatedState = simulatedFsm->getSimulatedState(currentStateId);
	if (currentSimulatedState == nullptr) return;


	this->stateDisplay->addPoint(currentSimulatedState->getName());
}

void StateTimeLine::updateCurrentValue()
{
	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto currentStateId = simulatedFsm->getActiveStateId();
	auto currentSimulatedState = simulatedFsm->getSimulatedState(currentStateId);
	if (currentSimulatedState == nullptr) return;


	this->stateDisplay->updateLastPoint(currentSimulatedState->getName());
}

void StateTimeLine::resetEventHandler()
{
	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto initialStateId = simulatedFsm->getInitialStateId();
	auto initialSimulatedState = simulatedFsm->getSimulatedState(initialStateId);
	if (initialSimulatedState == nullptr) return;


	this->stateDisplay->reset(initialSimulatedState->getName());
}
