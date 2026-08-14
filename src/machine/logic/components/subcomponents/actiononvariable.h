/*
 * Copyright © 2016-2026 Clément Foucher
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

// Stdlib
#include <memory>
using namespace std;

// StateS
#include "componentid.h"
#include "machinevalue.h"
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
	// Type declarations
public:
	enum class Type_t : uint
	{
		none       = 0x0,
		continuous = 0x1,
		pulse      = 0x2,
		set        = 0x4,
		reset      = 0x8,
		assign     = 0x10,
		increment  = 0x20,
		decrement  = 0x40
	};

	/////
	// Static functions
public:
	static QString getActionTypeText(ActionOnVariable::Type_t type);
	static QIcon   getActionTypeIcon(ActionOnVariable::Type_t type, bool isDown = false);

	/////
	// Constructors/destructors
public:
	explicit ActionOnVariable(ComponentId variableId, uint actuatorAllowedActions);
	explicit ActionOnVariable(shared_ptr<Variable> variable, uint actuatorAllowedActions, ActionOnVariable::Type_t actionType, MachineValue actionValue, int rangeL, int rangeR); // Build an action on variable when machine is still being parsed

	/////
	// Object functions
public:
	void checkAndFixAction();

	void setActionType (ActionOnVariable::Type_t newType);
	void setActionValue(MachineValue newValue);
	void setActionRange(int newRangeL, int newRangeR);

	ComponentId              getVariableActedOnId()  const;
	ActionOnVariable::Type_t getActionType()         const;
	MachineValue             getActionValue()        const;
	int                      getActionRangeL()       const;
	int                      getActionRangeR()       const;
	bool                     isActionValueEditable() const;

	uint getAllowedActionTypes() const;

	QString getCurrentActionTypeText() const;
	QIcon   getCurrentActionTypeIcon() const;

private slots:
	void variableTypeChangedEventHandler();
	void variableMemorizedStateChangedEventHandler();
	void variableInitialValueChangedEventHandler();

private:
	void connectSignals(shared_ptr<Variable> variable);

	bool checkIfRangeFitsVariable(int rangeL, int rangeR) const;

	void checkAndFixActionRange();
	void checkAndFixActionType();
	void checkAndFixActionValue();

	void setDefaultActionValue();

	MachineValue::Type_t getExpectedActionType() const;
	uint getExpectedBitVectorActionSize() const;

	/////
	// Signals
signals:
	void actionChangedEvent(); // Emitted when action changes whatever the cause
	void actionFixedEvent();   // Emitted when action had to be adapted due to its related variable changing

	/////
	// Object variables
private:
	ComponentId variableId = nullId;
	uint actuatorAllowedActions = static_cast<uint>(ActionOnVariable::Type_t::none);

	ActionOnVariable::Type_t actionType;
	MachineValue actionValue;
	int rangeL = -1;
	int rangeR = -1;

};

#endif // ACTIONONVARIABLE_H
