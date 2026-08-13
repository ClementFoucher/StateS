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
#include "variable.h"


/**
 * @brief Variable::setName changes the name of the variable.
 * Only the Machine is allowed to call this function.
 * @param newName
 */
void Variable::setName(const QString& newName)
{
	if (newName.isNull() == true) return;

	if (newName == this->name) return;


	this->name = newName;

	emit this->variableRenamedEvent();
}

void Variable::setType(MachineValue::Type_t newType)
{
	if (newType == MachineValue::Type_t::nullType) return;


	this->initialValue = MachineValue::fromType(newType);

	emit this->variableTypeChangedEvent();
}

void Variable::setInitialValue(MachineValue newInitialValue)
{
	if (newInitialValue.getType() != this->getType()) return;

	if (newInitialValue == this->initialValue) return;


	// A change in size is considered a type change
	bool typeChanged = false;
	if (this->getType() == MachineValue::Type_t::bitVector)
	{
		if (this->initialValue.getBitVectorValue().getSize() != newInitialValue.getBitVectorValue().getSize())
		{
			typeChanged = true;
		}
	}

	this->initialValue = newInitialValue;

	if (typeChanged == false)
	{
		emit this->variableInitialValueChangedEvent();
	}
	else // (typeChanged == true)
	{
		emit this->variableTypeChangedEvent();
	}
}

void Variable::setMemorized(bool memorized)
{
	if (memorized == this->memorized) return;


	this->memorized = memorized;

	emit this->variableMemorizedStateChangedEvent();
}

QString Variable::getName() const
{
	return this->name;
}

MachineValue::Type_t Variable::getType() const
{
	return this->initialValue.getType();
}

MachineValue Variable::getInitialValue() const
{
	return this->initialValue;
}

bool Variable::getMemorized() const
{
	return this->memorized;
}
