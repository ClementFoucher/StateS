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

#ifndef FSM_H
#define FSM_H

// Parent
#include "machine.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class FsmState;
class FsmTransition;


class Fsm : public Machine
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Fsm() = default;

	// Pseudo-constructor to process post-loading actions
	virtual void finalizeLoading() override;

	/////
	// Object functions
public:

	///
	// Components accessors and mutators

	componentId_t addState     (const QString& name, componentId_t id = nullId);
	componentId_t addTransition(componentId_t sourceStateId, componentId_t targetStateId, componentId_t id = nullId);

	void removeState     (componentId_t stateId);
	void removeTransition(componentId_t transitionId);

	shared_ptr<FsmState>      getState     (componentId_t stateId)      const;
	shared_ptr<FsmTransition> getTransition(componentId_t transitionId) const;

	const QList<componentId_t> getAllStatesIds()      const;
	const QList<componentId_t> getAllTransitionsIds() const;

	///
	// Components edition functions

	bool renameState(componentId_t stateId, const QString& newName);

	void redirectTransition(componentId_t transitionId, componentId_t newSourceStateId, componentId_t newTargetStateId);

	///
	// Initial state managemment

	void setInitialState(componentId_t stateId);
	componentId_t getInitialStateId() const;

	/////
	// Object variables
private:
	//FSM characteristic
	componentId_t initialStateId = nullId;

	// Maintain lists of states and transitions to allow fast access
	QList<componentId_t> states;
	QList<componentId_t> transitions;

};

#endif // FSM_H
