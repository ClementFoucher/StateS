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

#ifndef MACHINEACTUATORCOMPONENT_H
#define MACHINEACTUATORCOMPONENT_H

// Parent
#include "machinecomponent.h"

// Stdlib
#include <memory>

// StateS
class ActionOnVariable;
class Variable;


/**
 * @brief The MachineActuatorComponent class represents
 * a machine component that can act on variables.
 *
 * It manages a ranked list of actions and is able to
 * remove an action when the associated variable is deleted.
 */
class MachineActuatorComponent : public MachineComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineActuatorComponent() = default;
	explicit MachineActuatorComponent(ComponentId id) : MachineComponent(id) {}

	/////
	// Object functions
public:
	std::shared_ptr<ActionOnVariable> addAction(ComponentId variableId);
	void addAction(std::shared_ptr<ActionOnVariable> action, std::shared_ptr<Variable> variable); // Add action when machine is still being parsed
	void removeAction(uint actionRank);
	std::shared_ptr<ActionOnVariable> getAction(uint actionRank) const;
	const QList<std::shared_ptr<ActionOnVariable>> getActions() const;

	void changeActionRank(uint oldActionRank, uint newActionRank);

	virtual uint getAllowedActionTypes() const = 0;

private slots:
	void variableDeletedEventHandler(ComponentId deletedVariableId);
	void variableInActionListModifiedEventHandler();

private:
	void addActionInternal(std::shared_ptr<ActionOnVariable> action, std::shared_ptr<Variable> variable);

	/////
	// Signals
signals:
	void actionFixedEvent();

	/////
	// Object variables
private:
	QList<std::shared_ptr<ActionOnVariable>> actionList;

};

#endif // MACHINEACTUATORCOMPONENT_H
