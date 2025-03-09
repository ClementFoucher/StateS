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

#ifndef SIMULATEDACTIONONVARIABLE_H
#define SIMULATEDACTIONONVARIABLE_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class ActionOnVariable;


class SimulatedActionOnVariable : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SimulatedActionOnVariable(shared_ptr<const ActionOnVariable> sourceAction);

	/////
	// Object functions
public:
	void beginAction();
	void endAction();
	void resetAction();

	bool isActionMemorized() const;

private:
	LogicValue getActionValue() const;
	uint       getActionSize()  const;

	/////
	// Object variables
private:
	componentId_t variableId = nullId;

	ActionOnVariableType_t actionType;

	LogicValue actionValue = LogicValue();
	int        rangeL      = -1;
	int        rangeR      = -1;

	bool isActionActing = false;

};

#endif // SIMULATEDACTIONONVARIABLE_H
