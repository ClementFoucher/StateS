/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "logicvalue.h"

// Qt classes
#include <QString>


LogicValue LogicValue::getValue0(uint size)
{
	return LogicValue(size, false);
}

LogicValue LogicValue::getValue1(uint size)
{
	return LogicValue(size, true);
}

LogicValue LogicValue::getNullValue()
{
	return LogicValue();
}

LogicValue LogicValue::fromString(const QString &textValue)
{
	LogicValue realValue;

	for (QChar c : textValue)
	{
		if (c == '0')
		{
			realValue.prepend(false);
		}
		else if (c == '1')
		{
			realValue.prepend(true);
		}
		else
		{
			return LogicValue::getNullValue();
		}
	}

	return realValue;
}

LogicValue::LogicValue(const LogicValue& stateToCopy) :
	LogicValue(stateToCopy.getSize())
{
	// Force deep copy
	for (uint i = 0 ; i < stateToCopy.getSize() ; i++)
	{
		(*this)[i] = stateToCopy[i];
	}
}

void LogicValue::resize(uint newSize)
{
	if (newSize == 0) return;

	if (newSize == this->getSize()) return;


	((QVector<bool>*)this)->resize(newSize);
}

uint LogicValue::getSize() const
{
	return this->size();
}

bool LogicValue::isNull() const
{
	if (this->size() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LogicValue::operator==(const LogicValue& otherValue) const
{
	if (this->size() == otherValue.size())
	{
		for (int i = 0 ; i < this->size() ; i++)
		{
			if ((*this)[i] != otherValue[i])
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool LogicValue::operator!=(const LogicValue& otherValue) const
{
	return !(*this == otherValue);
}

LogicValue LogicValue::operator!() const
{
	LogicValue result(this->size());

	for(int i = 0 ; i < this->size() ; i++)
	{
		result[i] = !(*this)[i];
	}

	return result;
}

LogicValue LogicValue::operator&(const LogicValue& otherValue) const
{
	if (this->size() == otherValue.size())
	{
		LogicValue result(this->size());

		for(int i = 0 ; i < this->size() ; i++)
		{
			result[i] = (*this)[i] && otherValue[i];
		}

		return result;
	}
	else
	{
		return getNullValue();
	}
}

LogicValue LogicValue::operator|(const LogicValue& otherValue) const
{
	if (this->size() == otherValue.size())
	{
		LogicValue result(this->size());

		for(int i = 0 ; i < this->size() ; i++)
		{
			result[i] = (*this)[i] || otherValue[i];
		}

		return result;
	}
	else
	{
		return getNullValue();
	}
}

LogicValue LogicValue::operator^(const LogicValue& otherValue) const
{
	if (this->size() == otherValue.size())
	{
		LogicValue result(this->size());

		for(int i = 0 ; i < this->size() ; i++)
		{
			result[i] = (*this)[i] ^ otherValue[i];
		}

		return result;
	}
	else
	{
		return getNullValue();
	}
}

LogicValue LogicValue::operator=(const LogicValue& otherValue)
{
	if (otherValue.isNull() == false)
	{
		this->resize(otherValue.getSize());
	}
	else
	{
		// We are tying to affect null value to this: can't resize to 0 publicly
		((QVector<bool>*)this)->resize(0);
	}

	for(uint i = 0 ; i < this->getSize() ; i++)
	{
		(*this)[i] = otherValue[i];
	}

	return *this;
}

LogicValue LogicValue::operator&=(const LogicValue& otherValue)
{
	LogicValue result = (*this) & otherValue;

	if (!result.isNull())
	{
		*this = result;
	}

	return result;
}

LogicValue LogicValue::operator|=(const LogicValue& otherValue)
{
	LogicValue result = (*this) | otherValue;

	if (!result.isNull())
	{
		*this = result;
	}

	return result;
}

LogicValue LogicValue::operator^=(const LogicValue& otherValue)
{
	LogicValue result = (*this) ^ otherValue;

	if (!result.isNull())
	{
		*this = result;
	}

	return result;
}

// Return value for increment indicates if there is a carry
bool LogicValue::increment()
{
	uint lastI = this->getSize();

	for (int i = 0 ; i < this->size() ; i++)
	{
		if ((*this)[i] == true)
		{
			(*this)[i] = false;
		}
		else
		{
			lastI = i;
			(*this)[i] = true;
			break;
		}
	}

	if (lastI == this->getSize())
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Return value for decrement indicates if value goes below 0
bool LogicValue::decrement()
{
	uint lastI = this->getSize();

	for (int i = 0 ; i < this->size() ; i++)
	{
		if ((*this)[i] == false)
		{
			(*this)[i] = true;
		}
		else
		{
			lastI = i;
			(*this)[i] = false;
			break;
		}
	}

	if (lastI == this->getSize())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool& LogicValue::operator[](uint memberNumber)
{
	static bool garbage = false;
	if (memberNumber >= this->getSize()) return garbage;


	return (*((QVector<bool>*)this))[memberNumber];
}

bool LogicValue::operator[](uint memberNumber) const
{
	if (memberNumber >= this->getSize()) return false;


	return (*((QVector<bool>*)this))[memberNumber];
}

QString LogicValue::toString() const
{
	QString text;

	if (this->isNull())
	{
		text += "(null value)";
	}
	else
	{
		for (bool& b : *((QVector<bool>*)this))
		{
			text = (b?"1":"0") + text;
		}
	}

	return text;
}

int LogicValue::toInt() const
{
	int acc = 0;
	for (int i = 0 ; i < this->size() ; i++)
	{
		if ((*this)[i] == true)
		{
			acc += pow(2, i);
		}
	}
	return acc;
}

LogicValue LogicValue::getSubrange(int msb, int lsb) const
{
	if ( (msb < 0) && (lsb >= 0) ) return LogicValue::getNullValue();
	if (msb >= this->size()) return LogicValue::getNullValue();
	if (lsb >= this->size()) return LogicValue::getNullValue();
	if ( (lsb >= 0) && (lsb > msb) ) return LogicValue::getNullValue();


	if (msb < 0)
	{
		// Get full value
		return *this;
	}
	else if (lsb < 0)
	{
		// Get single bit
		LogicValue result(1);
		result[0] = this->at(msb);

		return result;
	}
	else
	{
		// Get sub-range
		LogicValue result(msb-lsb+1);
		for (int i = 0 ; i <= msb-lsb ; i++)
		{
			result[i] = this->at(lsb+i);
		}

		return result;
	}
}

bool LogicValue::setSubrange(LogicValue value, int msb, int lsb)
{
	if ( (msb < 0) && (lsb >= 0) ) return false;
	if (msb >= this->size()) return false;
	if (lsb >= this->size()) return false;
	if ( (lsb >= 0) && (lsb > msb) ) return false;


	if (msb < 0)
	{
		// Full range affectation
		if (this->size() == value.getSize())
		{
			for (int i = 0 ; i < this->size() ; i++)
			{
				(*this)[i] = value[i];
			}
			return true;
		}
	}
	else if (lsb < 0)
	{
		// Single bit affectation
		if (value.getSize() == 1)
		{
			(*this)[msb] = value[0];
			return true;
		}
	}
	else
	{
		// Sub-range affectation
		if (value.getSize() == (uint)msb-lsb+1)
		{
			for (int i = 0 ; i <= msb-lsb ; i++)
			{
				(*this)[i+lsb] = value[i];
			}
			return true;
		}
	}

	return false;
}
