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

// Current class header
#include "signal.h"

Signal::Signal(const QString& name, const LogicValue& initialValue, const LogicValue& currentValue, bool isConstant) :
    initialValue(initialValue),
    currentValue(currentValue)
{
    this->name = name;
    this->isConstant = isConstant;

    // Link specific events signals to general events
    connect(this, &Signal::signalRenamedEvent,             this, &Signal::signalStaticConfigurationChangedEvent);
    connect(this, &Signal::signalResizedEvent,             this, &Signal::signalStaticConfigurationChangedEvent);
    connect(this, &Signal::SignalinitialValueChangedEvent, this, &Signal::signalStaticConfigurationChangedEvent);

    // This event also impacts dynamic values
    connect(this, &Signal::signalResizedEvent, this, &Signal::signalDynamicStateChangedEvent);
}

Signal::Signal(const QString &name, const LogicValue& initialValue, bool isConstant) :
    Signal(name, initialValue, initialValue, isConstant)
{
}

Signal::Signal(const QString &name, uint bitCount) :
    Signal(name, LogicValue(bitCount), LogicValue(bitCount), false)
{
}

Signal::Signal(const QString &name, bool initialValue, bool isConstant) :
    Signal(name, LogicValue(1, initialValue), LogicValue(1, initialValue), isConstant)
{
}

Signal::~Signal()
{
    emit signalDeletedEvent();
}

QString Signal::getName() const
{
    return name;
}

void Signal::setName(const QString& value)
{
    name = value;
    emit signalRenamedEvent();
}

uint Signal::getSize() const
{
    return currentValue.getSize();
}

bool Signal::resize(uint newSize)
{
    if (newSize != 0)
    {
        this->currentValue.resize(newSize);
        this->initialValue.resize(newSize);

        emit signalResizedEvent();
        return true;
    }
    else
        return false;
}

QString Signal::getText(bool colored) const
{
    if (!colored)
    {
        return this->name;
    }
    else
    {
        if (this->isTrue())
            return "<font color=\"green\">" + this->name + "</font>";
        else if (this->isFalse())
            return "<font color=\"red\">"   + this->name + "</font>";
        else
            return "<font color=\"blue\">"  + this->name + "</font>";
    }
}

bool Signal::setCurrentValue(const LogicValue &value)
{
    if ( (!this->isConstant) && (value.getSize() == this->getSize()) )
    {
        this->currentValue = value;
        emit signalDynamicStateChangedEvent();

        return true;
    }
    else
    {
        return false;
    }
}

LogicValue Signal::getCurrentValue() const
{
    return this->currentValue;
}

LogicValue Signal::getInitialValue() const
{
    return this->initialValue;
}

bool Signal::setInitialValue(const LogicValue &newInitialValue)
{
    if (this->getSize() == newInitialValue.getSize())
    {
        this->initialValue = newInitialValue;

        if (this->isConstant)
            this->currentValue = this->initialValue;

        emit SignalinitialValueChangedEvent();

        return true;
    }
    else
    {
        return false;
    }
}

void Signal::reinitialize()
{
    this->currentValue = this->initialValue;
    emit signalDynamicStateChangedEvent();
}

bool Signal::resetValue()
{
    return setCurrentValue(LogicValue::getValue0(this->getSize()));
}

bool Signal::set()
{
    return setCurrentValue(LogicValue::getValue1(1));
}

bool Signal::isAllZeros() const
{
    return this->getCurrentValue().isAllZeros(); // Use get current value to allow polymorphism
}

bool Signal::isAllOnes() const
{
    return this->getCurrentValue().isAllOnes(); // Use get current value to allow polymorphism
}

// True or false concept here only apply to one bit signals
// A larger signal will be be neither true nor false
bool Signal::isTrue() const
{
    if (this->getSize() == 1 && this->isAllOnes())
        return true;
    else
        return false;
}

bool Signal::isFalse() const
{
    if (this->getSize() == 1 && this->isAllZeros())
        return true;
    else
        return false;
}
bool Signal::getIsConstant() const
{
    return isConstant;
}
