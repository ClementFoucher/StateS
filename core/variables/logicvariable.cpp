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

#include "logicvariable.h"


LogicVariable* LogicVariable::constant0 = new LogicVariable(false);
LogicVariable* LogicVariable::constant1 = new LogicVariable(true);

LogicVariable::LogicVariable(const QString& name)
{
    this->name = name;
}

LogicVariable::LogicVariable()
{
    this->name = "¹²³";
}

LogicVariable::LogicVariable(bool active)
{
    if (active)
    {
        this->name = "1";

        this->currentState = true;

        this->isConstant = true;
    }
    else
    {
        this->name = "0";

        this->currentState = false;

        this->isConstant = true;
    }
}


LogicVariable::~LogicVariable()
{
    emit deletedEvent(this);
}

void LogicVariable::updateState()
{
    if (this->isPulse)
    {
        this->currentState = false;
        this->isPulse = false;

        emit stateChangedEvent();
    }
}

QString LogicVariable::getText() const
{
    return name;
}

QString LogicVariable::getName() const
{
    return name;
}

void LogicVariable::setName(const QString& value)
{
    name = value;

    emit renamedEvent();
}

bool LogicVariable::getCurrentState() const
{
    return currentState;
}

bool LogicVariable::isActive() const
{
    return getCurrentState();
}

bool LogicVariable::isInactive() const
{
    return !getCurrentState();
}

void LogicVariable::setCurrentState(bool value, bool pulse)
{
    if (! isConstant)
    {
        this->isPulse = pulse;
        this->currentState = value;

        emit stateChangedEvent();
    }
}
