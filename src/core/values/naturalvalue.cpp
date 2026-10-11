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
#include "naturalvalue.h"

// Qt
#include <QString>

// StateS
#include "relativevalue.h"


NaturalValue NaturalValue::fromRawString(const QString& textValue)
{
	bool ok;
	uint value = textValue.toUInt(&ok);

	if (ok == true)
	{
		return NaturalValue{value};
	}

	// Default to 0 when incorrect text
	return NaturalValue{};
}

int NaturalValue::maxValue()
{
	// This value is used to define the limits in
	// a QIntValidator that only allows ints.
	// Just consider the max value is max()/2...
	return static_cast<int>(std::numeric_limits<unsigned int>::max()/2);
}

NaturalValue::NaturalValue(const NaturalValue& valueToCopy)
{
	this->value = valueToCopy.value;
}

NaturalValue::NaturalValue(const NaturalValue&& valueToCopy)
{
	this->value = valueToCopy.value;
}

QString NaturalValue::toDisplayString() const
{
	return this->toRawString();
}

QString NaturalValue::toRawString() const
{
	return QString::number(this->value);
}

NaturalValue::operator unsigned int() const
{
	return this->value;
}

bool NaturalValue::operator==(const NaturalValue& otherValue) const
{
	return this->value == otherValue.value;
}

bool NaturalValue::operator!=(const NaturalValue& otherValue) const
{
	return !(*this == otherValue);
}

bool NaturalValue::operator==(const RelativeValue& otherValue) const
{
	return this->value == otherValue.value;
}

bool NaturalValue::operator!=(const RelativeValue& otherValue) const
{
	return !(*this == otherValue);
}

bool NaturalValue::operator>(const NaturalValue& otherValue) const
{
	return this->value > otherValue.value;
}

bool NaturalValue::operator<(const NaturalValue& otherValue) const
{
	return this->value < otherValue.value;
}

bool NaturalValue::operator>=(const NaturalValue& otherValue) const
{
	return this->value >= otherValue.value;
}

bool NaturalValue::operator<=(const NaturalValue& otherValue) const
{
	return this->value <= otherValue.value;
}

bool NaturalValue::operator>(const RelativeValue& otherValue) const
{
	return static_cast<int>(this->value) > otherValue.value;
}

bool NaturalValue::operator<(const RelativeValue& otherValue) const
{
	return static_cast<int>(this->value) > otherValue.value;
}

bool NaturalValue::operator>=(const RelativeValue& otherValue) const
{
	return static_cast<int>(this->value) > otherValue.value;
}

bool NaturalValue::operator<=(const RelativeValue& otherValue) const
{
	return static_cast<int>(this->value) > otherValue.value;
}

NaturalValue NaturalValue::operator+(const NaturalValue& otherValue) const
{
	return NaturalValue{this->value + otherValue.value};
}

NaturalValue NaturalValue::operator-(const NaturalValue& otherValue) const
{
	int newValue = this->value - otherValue.value;
	if (newValue < 0)
	{
		newValue = 0;
	}
	return NaturalValue{static_cast<unsigned int>(newValue)};
}

RelativeValue NaturalValue::operator+(const RelativeValue& otherValue) const
{
	return RelativeValue{static_cast<int>(this->value) + otherValue.value};
}

RelativeValue NaturalValue::operator-(const RelativeValue& otherValue) const
{
	return RelativeValue{static_cast<int>(this->value) - otherValue.value};
}

NaturalValue NaturalValue::operator=(const NaturalValue& otherValue)
{
	this->value = otherValue.value;

	return *this;
}
