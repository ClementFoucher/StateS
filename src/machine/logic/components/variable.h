/*
 * Copyright © 2014-2025 Clément Foucher
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

#ifndef VARIABLE_H
#define VARIABLE_H

// Parent
#include <machinecomponent.h>

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"


class Variable : public MachineComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Variable(const QString& name);
	explicit Variable(componentId_t id, const QString& name);

	/////
	// Object functions
public:

	///
	// Mutators

	void setName(const QString& newName);
	void resize(uint newSize);
	void setInitialValue(const LogicValue& newInitialValue);

	///
	// Accessors

	QString    getName()         const;
	uint       getSize()         const;
	LogicValue getInitialValue() const;

	/////
	// Signals
signals:
	void variableRenamedEvent();
	void variableResizedEvent();
	void variableInitialValueChangedEvent();

	/////
	// Object variables
private:
	QString    name;
	LogicValue initialValue;

};

#endif // VARIABLE_H
