/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIMULATEDVARIABLE_H
#define SIMULATEDVARIABLE_H

// Parent
#include "simulatedcomponent.h"

// StateS classes
#include "logicvalue.h"


class SimulatedVariable : public SimulatedComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedVariable(componentId_t componentId);

	/////
	// Object functions
public:

	///
	// Mutators

	void setCurrentValue(const LogicValue& value);
	void setCurrentValueSubRange(const LogicValue& value, int rangeL, int rangeR);
	void reinitialize();

	///
	// Accessors

	QString    getName()         const;
	uint       getSize()         const;
	LogicValue getInitialValue() const;
	LogicValue getCurrentValue() const;
	bool       getMemorized()    const;

	/////
	// Signals
signals:
	void variableCurrentValueChangedEvent();

	/////
	// Object variables
protected:
	QString    name;
	LogicValue initialValue;
	LogicValue currentValue;
	bool       memorized;

};

#endif // SIMULATEDVARIABLE_H
