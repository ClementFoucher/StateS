/*
 * Copyright © 2016 Clément Foucher
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
#ifndef ACTIONONSIGNAL_H
#define ACTIONONSIGNAL_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "logicvalue.h"
class Signal;


/**
 * @brief The ActionOnSignal class manages actions on signals.
 *
 * The range parameters are valid when >= 0.
 * If only left range is valid, the action acts on a single bit.
 * If both are valid, the action acts on the [rangeL..rangeR] sub-vector.
 *
 * If signal is size 1 or action acts on a single bit
 *   => action value is always implicit and assign type is illegal.
 * Else (vector signal with whole range or sub-range action)
 *   => action value is implicit for set and reset types, explicit for others.
 *
 * For implicit action values, actionValue is null, but the
 * value can still be obtained using public function getActionValue().
 *
 * Action only has a weak link to associated signal.
 * If signal is deleted, Action should be deleted too.
 * But in the meantime, action just doesn't react to any
 * external sollicitation: setters are ignored, action
 * value getter returns null value, action size getter
 * returns 0 and other getters return meaningless values.
 * Begin and end actions have no effect.
 */
class ActionOnSignal : public QObject
{
	Q_OBJECT

public:
	enum ActionOnSignalErrorEnum
	{
		illegal_type              = 0,
		illegal_value             = 1,
		illegal_range             = 2,
		action_value_is_read_only = 3
	};

public:
	enum class action_types { activeOnState, pulse, set, reset, assign };

public:
	explicit ActionOnSignal(shared_ptr<Signal> signal, action_types actionType, LogicValue actionValue = LogicValue::getNullValue(),
	                        int rangeL = -1, int rangeR = -1, QObject* parent = nullptr);

	void setActionType (action_types newType); // Throws StatesException
	void setActionValue(LogicValue newValue);  // Throws StatesException
	void setActionRange(int newRangeL, int newRangeR, LogicValue newValue = LogicValue::getNullValue()); // Throws StatesException

	shared_ptr<Signal> getSignalActedOn()      const;
	action_types       getActionType()         const;
	LogicValue         getActionValue()        const;
	int                getActionRangeL()       const;
	int                getActionRangeR()       const;
	uint               getActionSize()         const;
	bool               isActionValueEditable() const;

	void beginAction();
	void endAction();

signals:
	void actionChangedEvent();

private slots:
	void signalResizedEventHandler();

private:
	bool checkIfRangeFitsSignal(int rangeL, int rangeR) const;

private:
	weak_ptr<Signal> signal;

	action_types actionType;
	LogicValue   actionValue;
	int          rangeL;
	int          rangeR;

	bool isActionActing = false;

};

#endif // ACTIONONSIGNAL_H
