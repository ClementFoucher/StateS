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
 * Depending on action type and size, the action value can be
 * explicit or implicit. Implicit values are provided by this class
 * and can not be edited.
 */
class ActionOnVariable : public QObject
{
	Q_OBJECT

	/////
	// Static functions
public:
	static QString getActionTypeText(ActionOnVariableType_t type);
	static QIcon   getActionTypeIcon(ActionOnVariableType_t type, bool isDown = false);

	/////
	// Constructors/destructors
public:
	explicit ActionOnVariable(componentId_t variableId, uint actuatorAllowedActions);
	explicit ActionOnVariable(shared_ptr<Variable> variable, uint actuatorAllowedActions, ActionOnVariableType_t actionType, LogicValue actionValue, int rangeL, int rangeR); // Build an action on variable when machine is still being parsed

	/////
	// Object functions
public:
	void checkAndFixAction();

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

	uint getAllowedActionTypes() const;

	QString getCurrentActionTypeText() const;
	QIcon   getCurrentActionTypeIcon() const;

private slots:
	void variableResizedEventHandler();
	void variableMemorizedStateChangedEventHandler();
	void variableInitialValueChangedEventHandler();

private:
	void connectSignals(shared_ptr<Variable> variable);

	bool checkIfRangeFitsVariable(int rangeL, int rangeR) const;

	void checkAndFixActionRange();
	void checkAndFixActionType();
	void checkAndFixActionValue();

	/////
	// Signals
signals:
	void actionChangedEvent();

	/////
	// Object variables
private:
	componentId_t variableId = nullId;
	uint actuatorAllowedActions = (uint)ActionOnVariableType_t::none;

	ActionOnVariableType_t actionType;
	LogicValue actionValue;
	int rangeL = -1;
	int rangeR = -1;

};

#endif // ACTIONONVARIABLE_H
