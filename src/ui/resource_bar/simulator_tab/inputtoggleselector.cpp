/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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
#include "inputtoggleselector.h"

// Qt
#include <QHBoxLayout>
#include <QLabel>

// StateS
#include "machinemanager.h"
#include "simulatedmachine.h"
#include "simulatedvariable.h"


InputToggleSelector::InputToggleSelector(componentId_t variableId, QWidget* parent) :
	QFrame(parent)
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto variable = simulatedMachine->getSimulatedVariable(variableId);
	if (variable == nullptr) return;


	this->variableId = variableId;

	auto layout = new QHBoxLayout(this);

	this->toggleValue = new QLabel();
	layout->addWidget(this->toggleValue);

	this->setStyleSheet("InputToggleSelector {border: 1px solid lightgray; border-radius: 10px}");

	connect(variable.get(), &SimulatedVariable::variableCurrentValueChangedEvent, this, &InputToggleSelector::variableValueChangedEventHandler);
}

shared_ptr<SimulatedVariable> InputToggleSelector::getVariable() const
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return nullptr;


	return simulatedMachine->getSimulatedVariable(variableId);
}

void InputToggleSelector::enterEvent(QEnterEvent* event)
{
	this->setStyleSheet("InputToggleSelector {border: 1px solid blue; border-radius: 10px}");

	QFrame::enterEvent(event);
}

void InputToggleSelector::leaveEvent(QEvent* event)
{
	this->setStyleSheet("InputToggleSelector {border: 1px solid lightgray; border-radius: 10px}");

	QFrame::leaveEvent(event);
}
