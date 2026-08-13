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

#ifndef BOOLEANVALUE_H
#define BOOLEANVALUE_H

// Parent
#include <QObject>


class BooleanValue : public QObject
{
	Q_OBJECT;

	/////
	// Static functions
public:
	static BooleanValue fromRawString(const QString& textValue);

	static BooleanValue trueValue();
	static BooleanValue falseValue();

	/////
	// Constructors/destructors
public:
	explicit BooleanValue() : value{false} {}
	explicit BooleanValue(bool value) : value{value} {}

	BooleanValue(const BooleanValue& valueToCopy);
	BooleanValue(const BooleanValue&& valueToCopy);

	/////
	// Object functions
public:
	QString toDisplayString() const;
	QString toRawString()     const;

	// Operator overloading
	operator bool() const;

	bool operator==(const BooleanValue& otherValue) const;
	bool operator!=(const BooleanValue& otherValue) const;

	BooleanValue operator!() const;
	BooleanValue operator&(const BooleanValue& otherValue) const;
	BooleanValue operator|(const BooleanValue& otherValue) const;
	BooleanValue operator^(const BooleanValue& otherValue) const;

	BooleanValue operator=(const BooleanValue& otherValue);
	BooleanValue operator&=(const BooleanValue& otherValue);
	BooleanValue operator|=(const BooleanValue& otherValue);
	BooleanValue operator^=(const BooleanValue& otherValue);

	/////
	// Object variables
private:
	bool value;

};

#endif // BOOLEANVALUE_H
