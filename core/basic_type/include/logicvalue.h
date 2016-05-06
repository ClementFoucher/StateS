/*
 * Copyright © 2014-2015 Clément Foucher
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

#ifndef LOGICVALUE_H
#define LOGICVALUE_H

// Parent
#include <QVector>


class LogicValue : private QVector<bool>
{
public: // Static

    enum LogicValueErrorEnum{
        unsupported_char = 0,
        resized_to_0     = 1,
        outside_range    = 2
    };

    static LogicValue getValue0(uint size);
    static LogicValue getValue1(uint size);
    static LogicValue getNullValue();
    static LogicValue fromString(const QString& textValue); // Throws StatesException

public:
    explicit LogicValue();
    LogicValue(const LogicValue& stateToCopy);
    explicit LogicValue(uint bitCount, bool initialValue = false);

    void resize(uint newSize); // Throws StatesException
    uint getSize() const;

    LogicValue getValue0() const;
    LogicValue getValue1() const;

    bool isNull() const;

    // Compare operator will return false is used with different size other value
    bool operator==(const LogicValue& otherValue) const;
    bool operator!=(const LogicValue& otherValue) const;

    LogicValue operator!() const;
    // Logic operators result will be null state is used with different size other value
    LogicValue operator&(const LogicValue& otherValue) const;
    LogicValue operator|(const LogicValue& otherValue) const;
    LogicValue operator^(const LogicValue& otherValue) const;

    LogicValue operator=(const LogicValue& otherValue);
    // If used with different size state, value will be unchanged, and call will return null state
    LogicValue operator&=(const LogicValue& otherValue);
    LogicValue operator|=(const LogicValue& otherValue);
    LogicValue operator^=(const LogicValue& otherValue);
    bool increment();

    bool& operator[](uint memberNumber);       // Throws StatesException TODO: find uses
    bool  operator[](uint memberNumber) const; // Throws StatesException TOOD: find uses

    QString toString() const;
};

#endif // LOGICVALUE_H
