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

#ifndef MACHINEVALUE_H
#define MACHINEVALUE_H

// Stdlib
#include <variant>

// Qt
#include <QtTypes>

// StateS
#include "booleanvalue.h"
#include "bitvectorvalue.h"
#include "naturalvalue.h"
#include "relativevalue.h"


/**
 * @brief The MachineValue class is a container for other
 *        values types. It also provides a "null type" for
 *        values that are invalid or not initialized.
 */
class MachineValue
{

	/////
	// Type declarations
public:
	enum class Type_t { nullType, boolean, bitVector, natural, relative };

	/////
	// Static functions
public:
	static MachineValue fromType(Type_t type);

	/////
	// Constructors/destructors
public:
	MachineValue() : value{BooleanValue{}}, valueType{Type_t::nullType} {}

	MachineValue(const MachineValue&  value) : value{value.value}, valueType{value.valueType} {}
	MachineValue(const MachineValue&& value) : value{value.value}, valueType{value.valueType} {}

	MachineValue(const BooleanValue&   value) : value{value}, valueType{Type_t::boolean  } {}
	MachineValue(const BitVectorValue& value) : value{value}, valueType{Type_t::bitVector} {}
	MachineValue(const NaturalValue&   value) : value{value}, valueType{Type_t::natural  } {}
	MachineValue(const RelativeValue&  value) : value{value}, valueType{Type_t::relative } {}

	MachineValue(const BooleanValue&&   value) : value{value}, valueType{Type_t::boolean  } {}
	MachineValue(const BitVectorValue&& value) : value{value}, valueType{Type_t::bitVector} {}
	MachineValue(const NaturalValue&&   value) : value{value}, valueType{Type_t::natural  } {}
	MachineValue(const RelativeValue&&  value) : value{value}, valueType{Type_t::relative } {}

	/////
	// Object functions
public:
	QString toDisplayString() const;
	QString toRawString()     const;

	bool isNull() const;

	Type_t getType() const;

	BooleanValue   getBooleanValue()   const;
	BitVectorValue getBitVectorValue() const;
	NaturalValue   getNaturalValue()   const;
	RelativeValue  getRelativeValue()  const;

	MachineValue operator=(const MachineValue&  otherValue);
	MachineValue operator=(const MachineValue&& otherValue);

	bool operator==(const MachineValue& otherValue) const;
	bool operator!=(const MachineValue& otherValue) const;

	bool operator>(const MachineValue& otherValue) const;
	bool operator<(const MachineValue& otherValue) const;
	bool operator>=(const MachineValue& otherValue) const;
	bool operator<=(const MachineValue& otherValue) const;

	/////
	// Object variables
private:
	std::variant<BooleanValue, BitVectorValue, NaturalValue, RelativeValue> value;
	Type_t valueType;

};

#endif // MACHINEVALUE_H
