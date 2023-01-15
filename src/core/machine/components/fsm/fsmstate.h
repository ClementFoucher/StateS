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

#ifndef FSMSTATE_H
#define FSMSTATE_H

// Parent
#include "fsmcomponent.h"

// Qt classes
#include <QList>

// StateS classes
#include "statestypes.h"


class FsmState : public FsmComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmState(const QString& name);
	explicit FsmState(componentId_t id, const QString& name);

	/////
	// Object functions
public:
	void setName(const QString& value);
	QString getName() const;

	void addOutgoingTransitionId(componentId_t transitionId);
	void removeOutgoingTransitionId(componentId_t transitionId);
	const QList<componentId_t> getOutgoingTransitionsIds() const;

	void addIncomingTransitionId(componentId_t transitionId);
	void removeIncomingTransitionId(componentId_t transitionId);
	const QList<componentId_t> getIncomingTransitionsIds() const;

	virtual uint getAllowedActionTypes() const override;

	// Simuation status
	void setActive(bool value);
	bool getIsActive() const;

signals:
	void stateRenamedEvent();

	/////
	// Object variables
private:
	QString name;

	QList<componentId_t> inputTransitionsIds;
	QList<componentId_t> outputTransitionsIds;

	bool isActive = false;

};

#endif // FSMSTATE_H
