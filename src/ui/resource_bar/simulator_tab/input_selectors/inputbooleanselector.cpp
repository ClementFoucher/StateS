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
#include "inputbooleanselector.h"

// Qt
#include <QLabel>

// StateS
#include "simulatedvariable.h"


InputBooleanSelector::InputBooleanSelector(ComponentId variableId, QWidget* parent) :
	InputToggleSelector(variableId, parent)
{
	auto variable = this->getVariable();
	if (variable == nullptr) return;

	if (variable->getType() != MachineValue::Type_t::boolean) return;


	auto value = variable->getCurrentValue();
	this->toggleValue->setText(value.toDisplayString());
	this->toggleValue->setToolTip(tr("Variable") + " " + variable->getName());
}

void InputBooleanSelector::mousePressEvent(QMouseEvent* event)
{
	InputToggleSelector::mousePressEvent(event);

	auto variable = this->getVariable();
	if (variable == nullptr) return;


	auto variableValue = variable->getCurrentValue().getBooleanValue();
	variable->setCurrentValue(!variableValue);
}

void InputBooleanSelector::variableValueChangedEventHandler()
{
	auto variable = this->getVariable();
	if (variable == nullptr) return;


	auto currentVariableValue = variable->getCurrentValue().getBooleanValue();
	this->toggleValue->setText(currentVariableValue.toDisplayString());
}
