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
#include "simulatedvariable.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"


SimulatedVariable::SimulatedVariable(componentId_t componentId) :
	SimulatedComponent(componentId)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto logicVariable = machine->getVariable(componentId);
	if (logicVariable == nullptr) return;


	this->name         = logicVariable->getName();
	this->initialValue = logicVariable->getInitialValue();
	this->currentValue = this->initialValue;
	this->memorized    = logicVariable->getMemorized();
}

void SimulatedVariable::setCurrentValue(const LogicValue &value)
{
	this->setCurrentValueSubRange(value, -1, -1);
}

void SimulatedVariable::setCurrentValueSubRange(const LogicValue& value, int rangeL, int rangeR)
{
	bool setOk = this->currentValue.setSubrange(value, rangeL, rangeR);

	if (setOk == true)
	{
		emit this->variableCurrentValueChangedEvent();
	}
}

void SimulatedVariable::reinitialize()
{
	this->currentValue = this->initialValue;

	emit this->variableCurrentValueChangedEvent();
}

QString SimulatedVariable::getName() const
{
	return this->name;
}

uint SimulatedVariable::getSize() const
{
	return this->initialValue.getSize();
}

LogicValue SimulatedVariable::getInitialValue() const
{
	return this->initialValue;
}

LogicValue SimulatedVariable::getCurrentValue() const
{
	return this->currentValue;
}

bool SimulatedVariable::getMemorized() const
{
	return this->memorized;
}
