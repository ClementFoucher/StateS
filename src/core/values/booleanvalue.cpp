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
#include "booleanvalue.h"

// Qt
#include <QString>


BooleanValue BooleanValue::fromRawString(const QString& textValue)
{
	if (textValue == "FALSE")
	{
		return BooleanValue::falseValue();
	}
	else if (textValue == "TRUE")
	{
		return BooleanValue::trueValue();
	}

	// Default to false when incorrect text
	return BooleanValue::falseValue();
}

BooleanValue BooleanValue::trueValue()
{
	return BooleanValue{true};
}

BooleanValue BooleanValue::falseValue()
{
	return BooleanValue{false};
}

BooleanValue::BooleanValue(const BooleanValue& valueToCopy)
{
	this->value = valueToCopy.value;
}

BooleanValue::BooleanValue(const BooleanValue&& valueToCopy)
{
	this->value = valueToCopy.value;
}

bool BooleanValue::operator==(const BooleanValue& otherValue) const
{
	return this->value == otherValue.value;
}

bool BooleanValue::operator!=(const BooleanValue& otherValue) const
{
	return !(*this == otherValue);
}

BooleanValue BooleanValue::operator!() const
{
	return BooleanValue{!this->value};
}

BooleanValue BooleanValue::operator&(const BooleanValue& otherValue) const
{
	return BooleanValue{this->value && otherValue.value};
}

BooleanValue BooleanValue::operator|(const BooleanValue& otherValue) const
{
	return BooleanValue{this->value || otherValue.value};
}

BooleanValue BooleanValue::operator^(const BooleanValue& otherValue) const
{
	return BooleanValue{this->value != otherValue.value};
}

BooleanValue BooleanValue::operator=(const BooleanValue& otherValue)
{
	this->value = otherValue.value;

	return *this;
}

BooleanValue BooleanValue::operator&=(const BooleanValue& otherValue)
{
	this->value = this->value && otherValue.value;

	return *this;
}

BooleanValue BooleanValue::operator|=(const BooleanValue& otherValue)
{
	this->value = this->value || otherValue.value;

	return *this;
}

BooleanValue BooleanValue::operator^=(const BooleanValue& otherValue)
{
	this->value = this->value != otherValue.value;

	return *this;
}

QString BooleanValue::toDisplayString() const
{
	if (this->value == true)
	{
		return tr("True");
	}
	else
	{
		return tr("False");
	}
}

QString BooleanValue::toRawString() const
{
	if (this->value == true)
	{
		return "TRUE";
	}
	else
	{
		return "FALSE";
	}
}

BooleanValue::operator bool() const
{
	return this->value;
}
