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
#include "relativevalue.h"

// Qt
#include <QString>

// StateS
#include "naturalvalue.h"


RelativeValue RelativeValue::fromRawString(const QString& textValue)
{
	bool ok;
	int value = textValue.toInt(&ok);

	if (ok == true)
	{
		return RelativeValue{value};
	}

	// Default to 0 when incorrect text
	return RelativeValue{};
}

int RelativeValue::minValue()
{
	return std::numeric_limits<int>::min();
}

int RelativeValue::maxValue()
{
	return std::numeric_limits<int>::max();
}

RelativeValue::RelativeValue(const RelativeValue& valueToCopy)
{
	this->value = valueToCopy.value;
}

RelativeValue::RelativeValue(const RelativeValue&& valueToCopy)
{
	this->value = valueToCopy.value;
}

QString RelativeValue::toDisplayString() const
{
	return this->toRawString();
}

QString RelativeValue::toRawString() const
{
	return QString::number(this->value);
}

RelativeValue::operator int() const
{
	return this->value;
}

bool RelativeValue::operator==(const RelativeValue& otherValue) const
{
	return this->value == otherValue.value;
}

bool RelativeValue::operator!=(const RelativeValue& otherValue) const
{
	return !(*this == otherValue);
}

bool RelativeValue::operator==(const NaturalValue& otherValue) const
{
	return this->value == otherValue.value;
}

bool RelativeValue::operator!=(const NaturalValue& otherValue) const
{
	return !(*this == otherValue);
}

RelativeValue RelativeValue::operator=(const RelativeValue& otherValue)
{
	this->value = otherValue.value;

	return *this;
}
