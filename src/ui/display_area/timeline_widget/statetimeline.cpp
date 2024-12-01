/*
 * Copyright © 2024 Clément Foucher
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
#include "clock.h"
#include "graphicvectortimeline.h"
#include "fsm.h"
#include "machinemanager.h"
#include "machinesimulator.h"
#include "fsmsimulator.h"
#include "fsmstate.h"


StateTimeLine::StateTimeLine(shared_ptr<Clock> clock, QWidget* parent) :
    QWidget(parent)
{
	auto fsmSimulator = dynamic_pointer_cast<FsmSimulator>(machineManager->getMachineSimulator());
	if (fsmSimulator == nullptr) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto initialStateId = fsm->getInitialStateId();
	auto initialState = fsm->getState(initialStateId);
	if (initialState == nullptr) return;


	QHBoxLayout* globalLayout = new QHBoxLayout(this);

	QLabel* titleLabel = new QLabel(tr("State"));
	globalLayout->addWidget(titleLabel);

	QVBoxLayout* bitsLayout = new QVBoxLayout();
	QHBoxLayout* innerLayout = new QHBoxLayout();

	this->stateDisplay = new GraphicVectorTimeLine(0, initialState->getName());
	this->stateDisplay->setMinimumHeight(30);
	this->stateDisplay->setMaximumHeight(30);
	innerLayout->addWidget(this->stateDisplay);

	bitsLayout->addLayout(innerLayout);


	globalLayout->addLayout(bitsLayout);

	connect(fsmSimulator.get(), &FsmSimulator::stateChangedEvent, this, &StateTimeLine::updateCurrentValue);

	connect(clock.get(), &Clock::prepareForClockEvent, this, &StateTimeLine::clockEventHandler);
	connect(clock.get(), &Clock::resetGraphicEvent,    this, &StateTimeLine::resetEventHandler);
}

void StateTimeLine::clockEventHandler()
{
	auto fsmSimulator = dynamic_pointer_cast<FsmSimulator>(machineManager->getMachineSimulator());
	if (fsmSimulator == nullptr) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto currentStateId = fsmSimulator->getActiveStateId();
	auto currentState = fsm->getState(currentStateId);
	if (currentState == nullptr) return;


	this->stateDisplay->addPoint(currentState->getName());
}

void StateTimeLine::updateCurrentValue()
{
	auto fsmSimulator = dynamic_pointer_cast<FsmSimulator>(machineManager->getMachineSimulator());
	if (fsmSimulator == nullptr) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto currentStateId = fsmSimulator->getActiveStateId();
	auto currentState = fsm->getState(currentStateId);
	if (currentState == nullptr) return;


	this->stateDisplay->updateLastPoint(currentState->getName());
}

void StateTimeLine::resetEventHandler()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto initialStateId = fsm->getInitialStateId();
	auto initialState = fsm->getState(initialStateId);
	if (initialState == nullptr) return;


	this->stateDisplay->reset(initialState->getName());
}
