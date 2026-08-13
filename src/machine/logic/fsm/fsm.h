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

#ifndef FSM_H
#define FSM_H

// Parent
#include "machine.h"

// Stdlib
#include <memory>
using namespace std;

// StateS
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

	ComponentId addState     (const QString& name, ComponentId id = nullId);
	ComponentId addTransition(ComponentId sourceStateId, ComponentId targetStateId, ComponentId id = nullId);

	void removeState     (ComponentId stateId);
	void removeTransition(ComponentId transitionId);

	shared_ptr<FsmState>      getState     (ComponentId stateId)      const;
	shared_ptr<FsmTransition> getTransition(ComponentId transitionId) const;

	const QList<ComponentId> getAllStatesIds()      const;
	const QList<ComponentId> getAllTransitionsIds() const;

	///
	// Components edition functions

	bool renameState(ComponentId stateId, const QString& newName);

	void redirectTransition(ComponentId transitionId, ComponentId newSourceStateId, ComponentId newTargetStateId);

	///
	// Initial state managemment

	void setInitialState(ComponentId stateId);
	ComponentId getInitialStateId() const;

	/////
	// Object variables
private:
	//FSM characteristic
	ComponentId initialStateId = nullId;

	// Maintain lists of states and transitions to allow fast access
	QList<ComponentId> states;
	QList<ComponentId> transitions;

};

#endif // FSM_H
