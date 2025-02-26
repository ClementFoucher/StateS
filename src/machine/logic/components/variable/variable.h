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
#include <QObject>

// StateS classes
#include "logicvalue.h"


class Variable : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Variable(const QString& name);

	~Variable();

	/////
	// Object functions
public:
	QString getName() const;
	void setName(const QString& value); // TODO: check variable name here and throw StatesException

	uint getSize() const;
	void resize(uint newSize); // Throws StatesException

	LogicValue getInitialValue() const;
	void setInitialValue(const LogicValue& newInitialValue); // Throws StatesException
	void reinitialize();

	void setCurrentValue(const LogicValue& value); // Throws StatesException
	void setCurrentValueSubRange(const LogicValue& value, int rangeL, int rangeR); // Throws StatesException
	LogicValue getCurrentValue() const;

	void notifyVariableAboutToBeDeleted();

	/////
	// Signals
signals:
	void variableInitialValueChangedEvent();
	void variableCurrentValueChangedEvent();
	void variableRenamedEvent();
	void variableResizedEvent();
	void variableDeletedEvent();
	void variableAboutToBeDeletedEvent(); // Used to notify operands as they hold a shared_ptr

	/////
	// Object variables
private:
	QString name;
	LogicValue initialValue;

	// Simulation
	LogicValue currentValue;

};

#endif // VARIABLE_H
