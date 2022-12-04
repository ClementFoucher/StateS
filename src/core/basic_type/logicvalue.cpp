/*
 * Copyright © 2014-2016 Clément Foucher
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

// StateS classes
#include "statesexception.h"


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

LogicValue LogicValue::fromString(const QString &textValue) // Throws StatesException
{
	LogicValue realValue;

	foreach(QChar c, textValue)
	{
		if (c == '0')
			realValue.prepend(false);
		else if (c == '1')
			realValue.prepend(true);
		else
			throw StatesException("LogicValue", unsupported_char, "Unsupported character in string");
	}

	return realValue;
}

LogicValue::LogicValue() :
    QVector<bool>()
{
}

LogicValue::LogicValue(const LogicValue& stateToCopy) :
    LogicValue(stateToCopy.getSize())
{
	// Force deep copy
	for (uint i = 0 ; i < stateToCopy.getSize() ; i++)
		(*this)[i] = stateToCopy[i];
}

LogicValue::LogicValue(uint bitCount, bool initialValue) :
    QVector<bool>(bitCount, initialValue)
{
}

void LogicValue::resize(uint newSize) // Throws StatesException
{
	if (newSize != 0)
	{
		((QVector<bool>*)this)->resize(newSize);
	}
	else
	{
		throw StatesException("LogicValue", resized_to_0, "Trying to resize to 0-sized vector");
	}
}

uint LogicValue::getSize() const
{
	return this->size();
}

bool LogicValue::isNull() const
{
	if (this->size() == 0)
		return true;
	else
		return false;
}

bool LogicValue::operator==(const LogicValue& otherValue) const
{
	if (this->size() == otherValue.size())
	{
		for (int i = 0 ; i < this->size() ; i++)
		{
			if ((*this)[i] != otherValue[i])
				return false;
		}

		return true;
	}
	else
		return false;
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
		return getNullValue();
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
		return getNullValue();
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
		return getNullValue();
}

LogicValue LogicValue::operator=(const LogicValue& otherValue)
{
	try
	{
		this->resize(otherValue.getSize()); // Throws StatesException
		for(uint i = 0 ; i < this->getSize() ; i++)
		{
			(*this)[i] = otherValue[i];
		}
	}
	catch (const StatesException& e)
	{
		if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueErrorEnum::resized_to_0) )
		{
			// We are tying to affect null value to this: can't resize to 0 publicly
			((QVector<bool>*)this)->resize(0);
		}
		else
			throw;
	}

	return *this;
}

LogicValue LogicValue::operator&=(const LogicValue& otherValue)
{
	LogicValue result = (*this) & otherValue;

	if (!result.isNull())
		*this = result;

	return result;
}

LogicValue LogicValue::operator|=(const LogicValue& otherValue)
{
	LogicValue result = (*this) | otherValue;

	if (!result.isNull())
		*this = result;

	return result;
}

LogicValue LogicValue::operator^=(const LogicValue& otherValue)
{
	LogicValue result = (*this) ^ otherValue;

	if (!result.isNull())
		*this = result;

	return result;
}

// Return value for increment indicates if there is a carry
bool LogicValue::increment()
{
	uint lastI = this->getSize();

	for (int i = 0 ; i < this->size() ; i++)
	{
		if ((*this)[i] == true)
			(*this)[i] = false;
		else
		{
			lastI = i;
			(*this)[i] = true;
			break;
		}
	}

	if (lastI == this->getSize())
		return true;
	else
		return false;
}

bool& LogicValue::operator[](uint memberNumber) // Throws StatesException
{
	if (memberNumber < this->getSize())
		return (*((QVector<bool>*)this))[memberNumber];
	else
		throw StatesException("LogicValue", outside_range, "Outside range access");
}

bool LogicValue::operator[](uint memberNumber) const // Throws StatesException
{
	if (memberNumber < this->getSize())
		return (*((QVector<bool>*)this))[memberNumber];
	else
		throw StatesException("LogicValue", outside_range, "Outside range access");
}

QString LogicValue::toString() const
{
	QString text;

	if (this->isNull())
		text += "(null value)";
	else
	{
		foreach (bool b, *((QVector<bool>*)this))
		{
			text = (b?"1":"0") + text;
		}
	}

	return text;
}
