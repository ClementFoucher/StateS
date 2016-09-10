/*
 * Copyright © 2014-2016 Clément Foucher
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
#include "constant.h"

// StateS classes
#include "statesexception.h"


Constant::Constant(const QString& name) :
    Signal(name)
{
}

Constant::Constant(const QString& name, uint size) : // Throws StatesException
    Signal(name, size) // Throws StatesException: propagated
{
}

void Constant::setInitialValue(const LogicValue& newInitialValue) // Throws StatesException
{
    Signal::setInitialValue(newInitialValue); // Throws StatesException - Propagated
    Signal::setCurrentValue(newInitialValue); // Throws StatesException - Propagated
}

void Constant::setCurrentValue(const LogicValue&) // Throws StatesException
{
    throw StatesException("Constant", change_current_requested, "Trying to affect a current value to a constant");
}

void Constant::setCurrentValueSubRange(const LogicValue&, int, int) // Throws StatesException
{
    throw StatesException("Constant", change_current_requested, "Trying to affect a current value to a constant");
}
