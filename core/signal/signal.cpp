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
#include "StateS_signal.h"

// StateS classes
#include "statesexception.h"


Signal::Signal(const QString &name, uint size) // Throws StatesException
{
    if (size == 0)
        throw StatesException("Signal", building_zero_sized, "Signal size set to 0");

    this->name = name;
    this->initialValue = LogicValue::getValue0(size);
    this->currentValue = this->initialValue;

    // Link specific events signals to general events
    connect(this, &Signal::signalRenamedEvent,             this, &Signal::signalStaticConfigurationChangedEvent);
    connect(this, &Signal::signalResizedEvent,             this, &Signal::signalStaticConfigurationChangedEvent);
    connect(this, &Signal::SignalInitialValueChangedEvent, this, &Signal::signalStaticConfigurationChangedEvent);

    // This event also impacts dynamic values
    connect(this, &Signal::signalResizedEvent, this, &Signal::signalDynamicStateChangedEvent);
}

Signal::Signal(const QString& name) :
    Signal(name, 1) // Size to 1 => no exception to catch - ignored
{

}

Signal::~Signal()
{
    emit signalDeletedEvent();
}

QString Signal::getName() const
{
    return this->name;
}

void Signal::setName(const QString& value)
{
    this->name = value;
    emit signalRenamedEvent();
}

uint Signal::getSize() const
{
    return this->currentValue.getSize();
}

void Signal::resize(uint newSize) // Throws StatesException
{
    if (newSize == 0)
        throw StatesException("Signal", resized_to_0, "Trying to resize signal with size 0");

    this->currentValue.resize(newSize); // Throws StatesException - size checked - ignored
    this->initialValue.resize(newSize); // Throws StatesException - size checked - ignored

    emit signalResizedEvent(this->shared_from_this()); // Clear to use shared_from_this: this function can't be called in constructor
}

QString Signal::getText() const
{
    return this->getColoredText(false);
}

QString Signal::getColoredText(bool activeColored) const
{
    if (!activeColored)
    {
        return this->name;
    }
    else
    {
        if (this->getSize() == 1)
        {
            if (this->isTrue()) // Throws StatesException - Size checked - ignored
                return "<font color=\"green\">" + this->name + "</font>";
            else // (this->isFalse())
                return "<font color=\"red\">"   + this->name + "</font>";
        }
        else
            return "<font color=\"blue\">"  + this->name + "</font>";
    }
}

void Signal::setCurrentValue(const LogicValue& value) // Throws StatesException
{
    if (value.getSize() == this->getSize())
    {
        this->currentValue = value;

        emit signalDynamicStateChangedEvent();
    }
    else
    {
        throw StatesException("Signal", size_mismatch, "Trying to set initial value with value whom size does not match signal size");
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

void Signal::setInitialValue(const LogicValue &newInitialValue) // Throws StatesException
{
    if (this->getSize() == newInitialValue.getSize())
    {
        this->initialValue = newInitialValue;

        emit SignalInitialValueChangedEvent();
    }
    else
    {
        throw StatesException("Signal", size_mismatch, "Trying to set initial value with value whom size does not match signal size");
    }
}

void Signal::reinitialize()
{
    this->currentValue = this->initialValue;
    emit signalDynamicStateChangedEvent();
}

void Signal::resetValue()
{
    setCurrentValue(LogicValue::getValue0(this->getSize())); // Throws StatesException - Size determined from actual size - ignored
}

void Signal::set()
{
    setCurrentValue(LogicValue::getValue1(this->getSize())); // Throws StatesException - Size determined from actual size - ignored
}

// True or false concept here only apply to one bit signals
// A larger signal will neither be true nor false
bool Signal::isTrue() const // Throws StatesException
{
    if (this->getSize() == 1)
    {
        if (this->currentValue[0] == 1)
            return true;
        else
            return false;
    }
    else
    {
        throw StatesException("Signal", signal_is_not_bool, "Asking for boolean value on non 1-sized signal");
    }
}

bool Signal::isFalse() const // Throws StatesException
{
    if (this->getSize() == 1)
    {
        if (this->currentValue[0] == 0)
            return true;
        else
            return false;
    }
    else
    {
        throw StatesException("Signal", signal_is_not_bool, "Asking for boolean value on non 1-sized signal");
    }
}
