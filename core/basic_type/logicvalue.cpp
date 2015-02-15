/*
 * Copyright © 2014 Clément Foucher
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

#include "logicvalue.h"

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

    foreach(QChar c, textValue)
    {
        if (c == '0')
            realValue.prepend(false);
        else if (c == '1')
            realValue.prepend(true);
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

void LogicValue::resize(uint newSize)
{
    ((QVector<bool>*)this)->resize(newSize);
}

uint LogicValue::getSize() const
{
    return this->size();
}

bool LogicValue::isAllZeros() const
{
    foreach (bool b, *((QVector<bool>*)this)) {
        if (b)
            return false;
    }

    return true;
}

bool LogicValue::isAllOnes() const
{
    foreach (bool b, *((QVector<bool>*)this)) {
        if (!b)
            return false;
    }

    return true;
}

LogicValue LogicValue::getValue0()
{
    return LogicValue(this->size(), false);
}

LogicValue LogicValue::getValue1()
{
    return LogicValue(this->size(), true);
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
            result[i] = (*this)[i] & otherValue[i];
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
            result[i] = (*this)[i] | otherValue[i];
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
    this->resize(otherValue.getSize());

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

bool& LogicValue::operator[](uint memberNumber)
{
    if (memberNumber < this->getSize())
        return (*((QVector<bool>*)this))[memberNumber];
    else
        return this->foo;
}

bool LogicValue::operator[](uint memberNumber) const
{
    if (memberNumber < this->getSize())
        return (*((QVector<bool>*)this))[memberNumber];
    else
        return false;
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
