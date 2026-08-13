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
#include "machinevalue.h"


MachineValue MachineValue::fromType(Type_t type)
{
	switch (type)
	{
	case Type_t::boolean:
		return BooleanValue::falseValue();
		break;
	case Type_t::bitVector:
		return BitVectorValue::allZeros(2);
		break;
	case Type_t::nullType:
		return MachineValue{};
		break;
	}
}

QString MachineValue::toDisplayString() const
{
	if (this->isNull())
	{
		return "(null value)";
	}
	else
	{
		auto extractToString = [&](auto const& value){
			return value.toDisplayString();
		};

		return std::visit(extractToString, this->value);
	}
}

QString MachineValue::toRawString() const
{
	if (this->isNull())
	{
		return "NULL";
	}
	else
	{
		auto extractToString = [&](auto const& value){
			return value.toRawString();
		};

		return std::visit(extractToString, this->value);
	}
}

bool MachineValue::isNull() const
{
	if (this->valueType == Type_t::nullType)
	{
		return true;
	}
	else
	{
		return false;
	}
}

MachineValue::Type_t MachineValue::getType() const
{
	return this->valueType;
}

BooleanValue MachineValue::getBooleanValue() const
{
	if (this->valueType != Type_t::boolean) return BooleanValue{};


	return std::get<BooleanValue>(this->value);
}

BitVectorValue MachineValue::getBitVectorValue() const
{
	if (this->valueType != Type_t::bitVector) return BitVectorValue{};


	return std::get<BitVectorValue>(this->value);
}

MachineValue MachineValue::operator=(const MachineValue& otherValue)
{
	this->valueType = otherValue.valueType;
	this->value     = otherValue.value;

	return *this;
}

bool MachineValue::operator==(const MachineValue& otherValue) const
{
	if ( (this->valueType != otherValue.valueType) || (this->value != otherValue.value) )
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool MachineValue::operator!=(const MachineValue& otherValue) const
{
	if ( (this->valueType != otherValue.valueType) || (this->value != otherValue.value) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

MachineValue MachineValue::operator=(const MachineValue&& otherValue)
{
	this->valueType = otherValue.valueType;
	this->value     = otherValue.value;

	return *this;
}

MachineValue MachineValue::operator=(const BooleanValue& otherValue)
{
	this->valueType = Type_t::boolean;
	this->value     = otherValue;

	return *this;
}

MachineValue MachineValue::operator=(const BooleanValue&& otherValue)
{
	this->valueType = Type_t::boolean;
	this->value     = otherValue;

	return *this;
}

MachineValue MachineValue::operator=(const BitVectorValue& otherValue)
{
	this->valueType = Type_t::bitVector;
	this->value     = otherValue;

	return *this;
}

MachineValue MachineValue::operator=(const BitVectorValue&& otherValue)
{
	this->valueType = Type_t::bitVector;
	this->value     = otherValue;

	return *this;
}
