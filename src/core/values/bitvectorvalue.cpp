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

QString BitVectorValue::toRawString() const
{
	return this->toDisplayString();
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

// Return value for increment indicates if there is a carry
bool BitVectorValue::increment()
{
	if (this->value == this->maxValue())
	{
		this->value = 0;
		return true;
	}
	else
	{
		this->value++;
		return false;
	}
}

// Return value for decrement indicates if value goes below 0
bool BitVectorValue::decrement()
{
	if (this->value == 0)
	{
		this->value = this->maxValue();
		return true;
	}
	else
	{
		this->value--;
		return false;
	}
}

BitVectorValue BitVectorValue::getSubrange(uint msb, uint lsb) const
{
	if ( (msb < 0) && (lsb >= 0) ) return BitVectorValue{};
	if (msb >= this->vectorSize) return BitVectorValue{};
	if (lsb >= this->vectorSize) return BitVectorValue{};
	if ( (lsb >= 0) && (lsb > msb) ) return BitVectorValue{};


	if (msb < 0)
	{
		// Get full value
		return *this;
	}
	else if (lsb < 0)
	{
		// Get single bit
		BitVectorValue result{this->getBit(msb), 1};

		return result;
	}
	else
	{
		// Get sub-range
		BitVectorValue result{0, msb-lsb+1};
		for (int i = 0 ; i <= msb-lsb ; i++)
		{
			result.setBit(i, this->getBit(lsb+i));
		}

		return result;
	}
}

bool BitVectorValue::getBit(int rank) const
{
	return (*this)[rank];
}

bool BitVectorValue::setSubrange(BitVectorValue value, int msb, int lsb)
{
	if ( (msb < 0) && (lsb >= 0) ) return false;
	if (msb >= this->vectorSize) return false;
	if (lsb >= this->vectorSize) return false;
	if ( (lsb >= 0) && (lsb > msb) ) return false;


	if (msb < 0)
	{
		// Full range affectation
		if (this->vectorSize == value.getSize())
		{
			for (int i = 0 ; i < this->vectorSize ; i++)
			{
				this->setBit(i, value[i]);
			}
			return true;
		}
	}
	else if (lsb < 0)
	{
		// Single bit affectation
		if (value.getSize() == 1)
		{
			this->setBit(msb, value[0]);
			return true;
		}
	}
	else
	{
		// Sub-range affectation
		if (value.getSize() == static_cast<uint>(msb-lsb+1))
		{
			for (int i = 0 ; i <= msb-lsb ; i++)
			{
				this->setBit(i+lsb, value[i]);
			}
			return true;
		}
	}

	return false;
}

void BitVectorValue::setBit(int rank, bool value)
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
	if (this->vectorSize != otherValue.vectorSize) return false;
	else if (this->value != otherValue.value) return false;
	else return true;
}

bool BitVectorValue::operator!=(const BitVectorValue& otherValue) const
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

bool BitVectorValue::operator[](uint memberNumber) const
{
	if (memberNumber < this->vectorSize)
	{
		uint64_t result = this->value & (static_cast<uint64_t>(1)<<memberNumber);
		return (result == 0) ? false : true;
	}

	return false;
}

uint64_t BitVectorValue::maxValue() const
{
	return pow(2, this->vectorSize) - 1;
}
