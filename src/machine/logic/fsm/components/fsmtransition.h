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

#ifndef FSMTRANSITION_H
#define FSMTRANSITION_H

// Parent
#include "machineactuatorcomponent.h"

// Stdlib
#include <memory>
using namespace std;

// StateS
class Equation;


class FsmTransition : public MachineActuatorComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmTransition(ComponentId sourceStateId, ComponentId targetStateId);
	explicit FsmTransition(ComponentId id, ComponentId sourceStateId, ComponentId targetStateId);

	/////
	// Object functions
public:
	void setSourceStateId(ComponentId sourceStateId);
	ComponentId getSourceStateId() const;

	void setTargetStateId(ComponentId targetStateId);
	ComponentId getTargetStateId() const;

	void setCondition(shared_ptr<Equation> newCondition);
	void clearCondition();
	shared_ptr<Equation> getCondition() const;

	virtual uint getAllowedActionTypes() const override;

private slots:
	void conditionChangedEventHandler();
	void conditionInvalidatedEventHandler();

	/////
	// Signals
signals:
	void conditionChangedEvent();

	/////
	// Object variables
private:
	ComponentId sourceStateId;
	ComponentId targetStateId;

	shared_ptr<Equation> condition;

};

#endif // FSMTRANSITION_H
