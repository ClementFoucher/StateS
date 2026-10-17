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
#include "bitvectorvalue.h"

// Qt
#include <QString>

// StateS
#include "booleanvalue.h"


BitVectorValue BitVectorValue::fromRawString(const QString& textValue)
{
	uint64_t bitVectorValue = 0;
	uint bitVectorSize = 0;
	for (QChar c : textValue)
	{
		if (c == '0')
		{
			bitVectorValue <<= 1;
			bitVectorSize++;
		}
		else if (c == '1')
		{
			bitVectorValue <<= 1;
			bitVectorValue |= 1;
			bitVectorSize++;
		}
		else
		{
			// Incorrect text for bit vector
			return BitVectorValue{};
		}

		if (bitVectorSize == 64)
		{
			break;
		}
	}

	if (bitVectorSize == 0)
	{
		// Default to size 2 vector in case of error
		bitVectorSize = 2;
	}

	return BitVectorValue{bitVectorValue, bitVectorSize};
}

BitVectorValue BitVectorValue::allOnes(uint size)
{
	return BitVectorValue{static_cast<uint64_t>(pow(2, size)) - 1, size};
}

BitVectorValue BitVectorValue::allZeros(uint size)
{
	return BitVectorValue{0, size};
}

uint8_t BitVectorValue::maxSize()
{
	return 64;
}

BitVectorValue::BitVectorValue(uint64_t value, uint size)
{
	if (size > 64)
	{
		this->vectorSize = 64;
	}
	else if (size == 0)
	{
		this->vectorSize = 1;
	}
	else
	{
		this->vectorSize = static_cast<uint8_t>(size);
	}

	// Store value making sure value is within range
	this->value = value & this->maxValue();
}

QString BitVectorValue::toDisplayString() const
{
	return this->toRawString();
}

QString BitVectorValue::toRawString() const
{
	QString text;

	for (int i = this->vectorSize-1 ; i >= 0 ; i--)
	{
		if (this->getBit(i) == true)
		{
			text += '1';
		}
		else
		{
			text += '0';
		}
	}

	return text;
}

uint64_t BitVectorValue::toInt() const
{
	return this->value;
}

void BitVectorValue::resize(uint newSize)
{
	if (newSize == 0) return;

	if (newSize > 64) return;

	if (newSize == this->getSize()) return;


	this->vectorSize = newSize;

	// Make sure value is within new range
	this->value &= this->maxValue();
}

uint BitVectorValue::getSize() const
{
	return static_cast<uint>(this->vectorSize);
}

void BitVectorValue::setSubrange(uint msb, uint lsb, BitVectorValue value)
{
	if (msb >= this->vectorSize) return;
	if (lsb > msb) return;
	if (value.getSize() != msb-lsb+1) return;


	for (int i = 0 ; i <= msb-lsb ; i++)
	{
		this->setBit(i+lsb, value[i]);
	}
}

void BitVectorValue::setBit(uint rank, BooleanValue value)
{
	if (value == false)
	{
		this->value &= ~(static_cast<uint64_t>(1) << rank);
	}
	else
	{
		this->value |= static_cast<uint64_t>(1) << rank;
	}
}

bool BitVectorValue::operator==(const BitVectorValue& otherValue) const
{
	if (this->vectorSize != otherValue.vectorSize)
	{
		return false;
	}
	else
	{
		return this->value == otherValue.value;
	}
}

bool BitVectorValue::operator!=(const BitVectorValue& otherValue) const
{
	return !(*this == otherValue);
}

bool BitVectorValue::operator==(const BooleanValue& otherValue) const
{
	if (this->vectorSize != 1)
	{
		return false;
	}
	else
	{
		return this->getBit(0) == bool{otherValue};
	}
}

bool BitVectorValue::operator!=(const BooleanValue& otherValue) const
{
	return !(*this == otherValue);
}

BitVectorValue BitVectorValue::operator!() const
{
	return BitVectorValue{~this->value, this->vectorSize};
}

BitVectorValue BitVectorValue::operator~() const
{
	return BitVectorValue{~this->value, this->vectorSize};
}

BitVectorValue BitVectorValue::operator&(const BitVectorValue& otherValue) const
{
	if (otherValue.vectorSize != this->vectorSize) return BitVectorValue{};


	return BitVectorValue{otherValue.value & this->value, this->vectorSize};
}

BitVectorValue BitVectorValue::operator|(const BitVectorValue& otherValue) const
{
	if (otherValue.vectorSize != this->vectorSize) return BitVectorValue{};


	return BitVectorValue{otherValue.value | this->value, this->vectorSize};
}

BitVectorValue BitVectorValue::operator^(const BitVectorValue& otherValue) const
{
	if (otherValue.vectorSize != this->vectorSize) return BitVectorValue{};


	return BitVectorValue{otherValue.value ^ this->value, this->vectorSize};
}

BitVectorValue BitVectorValue::operator=(const BitVectorValue& otherValue)
{
	this->value      = otherValue.value;
	this->vectorSize = otherValue.vectorSize;

	return *this;
}

BitVectorValue BitVectorValue::operator=(const BitVectorValue&& otherValue)
{
	this->value      = otherValue.value;
	this->vectorSize = otherValue.vectorSize;

	return *this;
}

BitVectorValue BitVectorValue::operator&=(const BitVectorValue& otherValue)
{
	if (otherValue.vectorSize != this->vectorSize)
	{
		this->vectorSize = 0;
		this->value      = 0;

		return BitVectorValue{};
	}


	this->value = otherValue.value & this->value;

	return *this;
}

BitVectorValue BitVectorValue::operator|=(const BitVectorValue& otherValue)
{
	if (otherValue.vectorSize != this->vectorSize)
	{
		this->vectorSize = 0;
		this->value      = 0;

		return BitVectorValue{};
	}


	this->value = otherValue.value | this->value;

	return *this;
}

BitVectorValue BitVectorValue::operator^=(const BitVectorValue& otherValue)
{
	if (otherValue.vectorSize != this->vectorSize)
	{
		this->vectorSize = 0;
		this->value      = 0;

		return BitVectorValue{};
	}


	this->value = otherValue.value ^ this->value;

	return *this;
}

BooleanValue BitVectorValue::operator[](uint memberNumber) const
{
	if (memberNumber < this->vectorSize)
	{
		uint64_t result = this->value & (static_cast<uint64_t>(1)<<memberNumber);
		return (result == 0) ? BooleanValue::falseValue() : BooleanValue::trueValue();
	}

	return BooleanValue::falseValue();
}

BooleanValue BitVectorValue::getBit(uint rank) const
{
	return (*this)[rank];
}

uint64_t BitVectorValue::maxValue() const
{
	return pow(2, this->vectorSize) - 1;
}
