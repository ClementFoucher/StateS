/*
 * Copyright © 2014-2026 Clément Foucher
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
#include "inputbitselector.h"

// Qt
#include <QLabel>

// StateS
#include "simulatedvariable.h"


InputBitSelector::InputBitSelector(componentId_t variableId, uint bitNumber, QWidget* parent) :
	InputToggleSelector(variableId, parent)
{
	auto variable = this->getVariable();
	if (variable == nullptr) return;

	if (variable->getType() != MachineValue::Type_t::bitVector) return;


	this->bitNumber = bitNumber;

	auto initialVariableValue = variable->getInitialValue().getBitVectorValue();
	this->toggleValue->setText(QString::number(initialVariableValue[this->bitNumber]));
	this->toggleValue->setToolTip(tr("Bit") + " " + QString::number(this->bitNumber) + " " + tr("of variable") + " " + variable->getName());
}

void InputBitSelector::mousePressEvent(QMouseEvent* event)
{
	InputToggleSelector::mousePressEvent(event);

	auto variable = this->getVariable();
	if (variable == nullptr) return;


	auto variableValue = variable->getCurrentValue().getBitVectorValue();
	variableValue.setBit(this->bitNumber, !variableValue[this->bitNumber]);
	variable->setCurrentValue(variableValue);
}

void InputBitSelector::variableValueChangedEventHandler()
{
	auto variable = this->getVariable();
	if (variable == nullptr) return;


	auto currentVariableValue = variable->getCurrentValue().getBitVectorValue();
	this->toggleValue->setText(QString::number(currentVariableValue[this->bitNumber]));
}
