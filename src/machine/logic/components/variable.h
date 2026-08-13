/*
 * Copyright © 2014-2026 Clément Foucher
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

// StateS
#include "machinevalue.h"


class Variable : public MachineComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Variable(const QString& name, MachineValue::Type_t type) :
	    MachineComponent(),
	    name{name}, initialValue{MachineValue::fromType(type)}, memorized{false}
	{}

	explicit Variable(ComponentId id, const QString& name, MachineValue::Type_t type) :
	    MachineComponent(id),
	    name{name}, initialValue{MachineValue::fromType(type)}, memorized{false}
	{}

	/////
	// Object functions
public:

	///
	// Mutators

	void setName(const QString& newName);
	void setType(MachineValue::Type_t newType);
	void setInitialValue(MachineValue newInitialValue);
	void setMemorized(bool memorized);

	///
	// Accessors

	QString                   getName()         const;
	MachineValue::Type_t getType()         const;
	MachineValue              getInitialValue() const;
	bool                      getMemorized()    const;

	/////
	// Signals
signals:
	void variableRenamedEvent();
	void variableTypeChangedEvent();
	void variableInitialValueChangedEvent();
	void variableMemorizedStateChangedEvent();

	/////
	// Object variables
private:
	QString      name;
	MachineValue initialValue;
	bool         memorized;

};

#endif // VARIABLE_H
