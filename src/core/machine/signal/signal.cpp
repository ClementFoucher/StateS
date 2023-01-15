/*
 * Copyright © 2014-2023 Clément Foucher
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
#include "exceptiontypes.h"


Signal::Signal(const QString &name, uint size) // Throws StatesException
{
	if (size == 0)
		throw StatesException("Signal", SignalError_t::building_zero_sized, "Signal size set to 0");

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
		throw StatesException("Signal", SignalError_t::signal_resized_to_0, "Trying to resize signal with size 0");

	this->currentValue.resize(newSize); // Throws StatesException - size checked - ignored
	this->initialValue.resize(newSize); // Throws StatesException - size checked - ignored

	emit signalResizedEvent();
}

QString Signal::getText() const
{
	return this->getColoredText(false);
}

QString Signal::getColoredText(bool activeColored) const
{
	if (!activeColored)
	{
		return "<span style=\"color:black;\">" + this->name + "</span>";
	}
	else
	{
		if (this->getSize() == 1)
		{
			if (this->isTrue()) // Throws StatesException - Size checked - ignored
				return "<span style=\"color:green;\">" + this->name + "</span>";
			else // (this->isFalse())
				return "<span style=\"color:red;\">"   + this->name + "</span>";
		}
		else
			return "<span style=\"color:blue;\">"  + this->name + "</span>";
	}
}

void Signal::setCurrentValue(const LogicValue& value) // Throws StatesException
{
	// We have to make sure we use this call fuction,
	// and not the overriding ones:
	Signal::setCurrentValueSubRange(value, -1, -1); // Throws StatesException - Propagted
}

void Signal::setCurrentValueSubRange(const LogicValue& value, int rangeL, int rangeR) // Throws StatesException
{
	bool setOk = false;

	if (rangeL < 0)
	{
		// Full range affectation
		if (this->getSize() == value.getSize())
		{
			this->currentValue = value;
			setOk = true;
		}
	}
	else if (rangeR < 0)
	{
		// Single bit affectation
		if ( (value.getSize() == 1) && ((uint)rangeL < this->getSize()) )
		{
			this->currentValue[rangeL] = value[0];
			setOk = true;
		}
	}
	else
	{
		// Sub-range affectation
		if ( ((uint)rangeL < this->getSize()) && (rangeL > rangeR) && (value.getSize() == (uint)rangeR+rangeL+1) )
		{
			for (int i = rangeR ; i <= rangeL ; i++)
			{
				this->currentValue[i] = value[i-rangeR];
			}
			setOk = true;
		}
	}

	if (setOk == true)
	{
		emit signalDynamicStateChangedEvent();
	}
	else
	{
		throw StatesException("Signal", SignalError_t::size_mismatch, "Trying to set initial value with value whom size does not match signal size or specified rank");
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

void Signal::setInitialValue(const LogicValue& newInitialValue) // Throws StatesException
{
	if (this->getSize() == newInitialValue.getSize())
	{
		this->initialValue = newInitialValue;

		emit SignalInitialValueChangedEvent();
	}
	else
	{
		throw StatesException("Signal", SignalError_t::size_mismatch, "Trying to set initial value with value whom size does not match signal size");
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
		if (this->currentValue == LogicValue::getValue1(1))
			return true;
		else
			return false;
	}
	else
	{
		throw StatesException("Signal", SignalError_t::signal_is_not_bool, "Asking for boolean value on non 1-sized signal");
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
		throw StatesException("Signal", SignalError_t::signal_is_not_bool, "Asking for boolean value on non 1-sized signal");
	}
}
