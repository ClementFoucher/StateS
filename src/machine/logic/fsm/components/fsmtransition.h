/*
 * Copyright © 2014-2023 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
class Equation;


class FsmTransition : public MachineActuatorComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmTransition(componentId_t sourceStateId, componentId_t targetStateId);
	explicit FsmTransition(componentId_t id, componentId_t sourceStateId, componentId_t targetStateId);

	/////
	// Object functions
public:
	void setSourceStateId(componentId_t sourceStateId);
	componentId_t getSourceStateId() const;

	void setTargetStateId(componentId_t targetStateId);
	componentId_t getTargetStateId() const;

	void setCondition(shared_ptr<Signal> signalNewCondition);
	void clearCondition();
	shared_ptr<Signal> getCondition() const;

	virtual uint getAllowedActionTypes() const override;

signals:
	void conditionChangedEvent();

private slots:
	void conditionChangedEventHandler();

	/////
	// Object variables
private:
	componentId_t sourceStateId;
	componentId_t targetStateId;

	shared_ptr<Equation> condition;

};

#endif // FSMTRANSITION_H
