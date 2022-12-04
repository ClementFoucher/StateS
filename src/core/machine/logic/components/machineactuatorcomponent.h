/*
 * Copyright © 2014-2016 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class ActionOnSignal;
class Signal;


/**
 * @brief The MachineActuatorComponent class represents
 * a machine component with action capabilities.
 *
 * It manages a ranked list of actions and is able to
 * remove an action when the associated signal is deleted.
 *
 * A MachineActuatorComponent triggers the actions activation
 * and deactivation.
 */
class MachineActuatorComponent : public MachineComponent
{
	Q_OBJECT

public:
	typedef enum
	{
		none          = 0x0,
		activeOnState = 0x1,
		pulse         = 0x2,
		set           = 0x4,
		reset         = 0x8,
		assign        = 0x10
	} allowed_action_types;

	enum MachineActuatorComponentErrorEnum
	{
		out_of_range = 0
	};

public:
	explicit MachineActuatorComponent(shared_ptr<Machine> owningMachine);

	QList<shared_ptr<ActionOnSignal>> getActions() const;
	shared_ptr<ActionOnSignal> getAction(uint actionRank) const; // Throws StatesException

	shared_ptr<ActionOnSignal> addAction(shared_ptr<Signal> signal);
	void removeAction(uint actionRank); // Throws StatesException
	void changeActionRank(uint oldActionRank, uint newActionRank); // Throws StatesException

	void activateActions();
	void deactivateActions();

	virtual uint getAllowedActionTypes() const = 0;

signals:
	void actionListChangedEvent();

private slots:
	void cleanActionList();

private:
	QList<shared_ptr<ActionOnSignal>> actionList;

};

#endif // MACHINEACTUATORCOMPONENT_H
