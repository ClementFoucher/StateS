/*
 * Copyright © 2016-2025 Clément Foucher
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

#ifndef ACTIONONVARIABLE_H
#define ACTIONONVARIABLE_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class Variable;


/**
 * @brief The ActionOnVariable class manages actions on variables.
 *
 * The range parameters are valid when >= 0.
 * If only left range is valid, the action acts on a single bit.
 * If both are valid, the action acts on the [rangeL..rangeR] sub-vector.
 *
 * If variable is size 1 or action acts on a single bit
 *   => action value is always implicit and assign type is illegal.
 * Else (vector variable with whole range or sub-range action)
 *   => action value is implicit for set and reset types, explicit for others.
 *
 * For implicit action values, actionValue is null, but the
 * value can still be obtained using public function getActionValue().
 *
 * If variable is deleted, Action should be deleted too.
 * But in the meantime, action just doesn't react to any
 * external sollicitation: setters are ignored, action
 * value getter returns null value, action size getter
 * returns 0 and other getters return meaningless values.
 * Begin and end actions have no effect.
 */
class ActionOnVariable : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionOnVariable(componentId_t variableId, ActionOnVariableType_t actionType);
	explicit ActionOnVariable(shared_ptr<Variable> variable, ActionOnVariableType_t actionType, LogicValue actionValue, int rangeL, int rangeR); // Build an action on variable when machine is still being parsed

	/////
	// Object functions
public:
	void setActionType (ActionOnVariableType_t newType);
	void setActionValue(LogicValue newValue);
	void setActionRange(int newRangeL, int newRangeR);

	componentId_t          getVariableActedOnId()  const;
	ActionOnVariableType_t getActionType()         const;
	LogicValue             getActionValue()        const;
	int                    getActionRangeL()       const;
	int                    getActionRangeR()       const;
	uint                   getActionSize()         const;
	bool                   isActionValueEditable() const;
	bool                   isActionMemorized()     const;

	void checkActionValue();

	void beginAction();
	void endAction();

signals:
	void actionChangedEvent();

private slots:
	void variableResizedEventHandler();

private:
	bool checkIfRangeFitsVariable(int rangeL, int rangeR) const;
	void initialize(shared_ptr<Variable> variable, ActionOnVariableType_t actionType);

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

#endif // ACTIONONVARIABLE_H
