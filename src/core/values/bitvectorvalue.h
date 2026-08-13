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

#ifndef BITVECTORVALUE_H
#define BITVECTORVALUE_H

// Parent
#include <QObject>


class BitVectorValue : public QObject
{
	Q_OBJECT;

	/////
	// Static functions
public:
	static BitVectorValue fromRawString(const QString& textValue);

	static BitVectorValue allOnes(uint size);
	static BitVectorValue allZeros(uint size);

	static uint8_t maxSize();

	/////
	// Constructors/destructors
public:
	explicit BitVectorValue() : value{0}, vectorSize{2} {}
	explicit BitVectorValue(uint64_t value, uint size);

	BitVectorValue(const BitVectorValue&  otherValue) : value{otherValue.value}, vectorSize{otherValue.vectorSize} {}
	BitVectorValue(const BitVectorValue&& otherValue) : value{otherValue.value}, vectorSize{otherValue.vectorSize} {}

	/////
	// Object functions
public:
	QString  toDisplayString() const;
	QString  toRawString()     const;
	uint64_t toInt()           const;

	void resize(uint newSize);
	uint getSize() const;

	bool increment();
	bool decrement();

	QString toString() const;

	BitVectorValue getSubrange(uint msb, uint lsb) const;
	bool getBit(int rank) const;
	bool setSubrange(BitVectorValue value, int msb, int lsb);
	void setBit(int rank, bool value);

	// Operator overloading

	// Compare operator will return false is used with different size other value
	bool operator==(const BitVectorValue& otherValue) const;
	bool operator!=(const BitVectorValue& otherValue) const;

	BitVectorValue operator!() const;
	BitVectorValue operator~() const;
	// Logic operators result will be null state is used with different size other value
	BitVectorValue operator&(const BitVectorValue& otherValue) const;
	BitVectorValue operator|(const BitVectorValue& otherValue) const;
	BitVectorValue operator^(const BitVectorValue& otherValue) const;

	BitVectorValue operator=(const BitVectorValue&  otherValue);
	BitVectorValue operator=(const BitVectorValue&& otherValue);
	// If used with different size state, value will be unchanged, and call will return null state
	BitVectorValue operator&=(const BitVectorValue& otherValue);
	BitVectorValue operator|=(const BitVectorValue& otherValue);
	BitVectorValue operator^=(const BitVectorValue& otherValue);

	bool operator[](uint memberNumber) const;

private:
	uint64_t maxValue() const;

	/////
	// Object variables
private:
	uint64_t value;
	uint8_t  vectorSize;
};

#endif // BITVECTORVALUE_H
