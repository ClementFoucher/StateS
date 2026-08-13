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

#ifndef FSMSTATE_H
#define FSMSTATE_H

// Parent
#include "machineactuatorcomponent.h"

// Qt
#include <QList>


class FsmState : public MachineActuatorComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmState(const QString& name);
	explicit FsmState(ComponentId id, const QString& name);

	/////
	// Object functions
public:
	void setName(const QString& value);
	QString getName() const;

	void addOutgoingTransitionId(ComponentId transitionId);
	void removeOutgoingTransitionId(ComponentId transitionId);
	const QList<ComponentId> getOutgoingTransitionsIds() const;

	void addIncomingTransitionId(ComponentId transitionId);
	void removeIncomingTransitionId(ComponentId transitionId);
	const QList<ComponentId> getIncomingTransitionsIds() const;

	virtual uint getAllowedActionTypes() const override;

signals:
	void stateRenamedEvent();

	/////
	// Object variables
private:
	QString name;

	// Linked transitions lists only for quick access,
	// they are not actual part of the state
	QList<ComponentId> inputTransitionsIds;
	QList<ComponentId> outputTransitionsIds;

};

#endif // FSMSTATE_H
