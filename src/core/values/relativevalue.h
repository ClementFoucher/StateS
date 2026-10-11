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

#ifndef RELATIVEVALUE_H
#define RELATIVEVALUE_H

// Qt
class QString;

// StateS
class NaturalValue;


class RelativeValue
{
	friend class NaturalValue;

	/////
	// Static functions
public:
	static RelativeValue fromRawString(const QString& textValue);

	static int minValue();
	static int maxValue();

	/////
	// Constructors/destructors
public:
	explicit RelativeValue() : value{0} {}
	explicit RelativeValue(int value) : value{value} {}

	RelativeValue(const RelativeValue&  valueToCopy);
	RelativeValue(const RelativeValue&& valueToCopy);

	/////
	// Object functions
public:
	QString toDisplayString() const;
	QString toRawString()     const;

	// Operator overloading
	operator int() const;

	bool operator==(const RelativeValue& otherValue) const;
	bool operator!=(const RelativeValue& otherValue) const;

	bool operator==(const NaturalValue& otherValue) const;
	bool operator!=(const NaturalValue& otherValue) const;

	bool operator>(const RelativeValue& otherValue) const;
	bool operator<(const RelativeValue& otherValue) const;
	bool operator>=(const RelativeValue& otherValue) const;
	bool operator<=(const RelativeValue& otherValue) const;

	bool operator>(const NaturalValue& otherValue) const;
	bool operator<(const NaturalValue& otherValue) const;
	bool operator>=(const NaturalValue& otherValue) const;
	bool operator<=(const NaturalValue& otherValue) const;

	RelativeValue operator=(const RelativeValue& otherValue);

	/////
	// Object variables
private:
	// We are stuck to int to remain compatible with Qt's functions
	int value;

};

#endif // RELATIVEVALUE_H
