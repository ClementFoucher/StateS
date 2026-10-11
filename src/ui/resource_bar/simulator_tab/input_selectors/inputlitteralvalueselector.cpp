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
#include "inputlitteralvalueselector.h"

// Qt
#include <QHBoxLayout>

// StateS
#include "machinemanager.h"
#include "simulatedmachine.h"
#include "simulatedvariable.h"
#include "valueeditor.h"


InputLitteralValueSelector::InputLitteralValueSelector(ComponentId variableId, QWidget* parent) :
	QFrame(parent)
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto variable = simulatedMachine->getSimulatedVariable(variableId);
	if (variable == nullptr) return;


	this->variableId = variableId;

	auto layout = new QHBoxLayout(this);

	this->valueEditor = new ValueEditor();
	this->valueEditor->setMachineValue(variable->getInitialValue());
	layout->addWidget(this->valueEditor);

	this->setStyleSheet("valueEditor {border: 1px solid lightgray; border-radius: 10px}");

	connect(variable.get(), &SimulatedVariable::variableCurrentValueChangedEvent, this, &InputLitteralValueSelector::variableValueChangedEventHandler);

	connect(this->valueEditor, &ValueEditor::valueChangedEvent, this, &InputLitteralValueSelector::editorValueChangedEventHandler);
}

std::shared_ptr<SimulatedVariable> InputLitteralValueSelector::getVariable() const
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return nullptr;


	return simulatedMachine->getSimulatedVariable(variableId);
}

void InputLitteralValueSelector::enterEvent(QEnterEvent* event)
{
	this->setStyleSheet("InputToggleSelector {border: 1px solid blue; border-radius: 10px}");

	QFrame::enterEvent(event);
}

void InputLitteralValueSelector::leaveEvent(QEvent* event)
{
	this->setStyleSheet("InputToggleSelector {border: 1px solid lightgray; border-radius: 10px}");

	QFrame::leaveEvent(event);
}

void InputLitteralValueSelector::variableValueChangedEventHandler()
{
	auto variable = this->getVariable();
	if (variable == nullptr) return;


	this->valueEditor->setMachineValue(variable->getCurrentValue());
}

void InputLitteralValueSelector::editorValueChangedEventHandler(ValueEditor*)
{
	auto variable = this->getVariable();
	if (variable == nullptr) return;


	variable->setCurrentValue(this->valueEditor->getMachineValue());
}
