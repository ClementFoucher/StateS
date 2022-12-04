/*
 * Copyright © 2014-2020 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QList>

// StateS classes
class FsmTransition;
class FsmGraphicState;


class FsmState : public FsmComponent, public enable_shared_from_this<FsmState>
{
	Q_OBJECT

public:
	explicit FsmState(shared_ptr<Fsm> parent, const QString& name, QPointF location);
	~FsmState();

	// Logic structure
	void setName(const QString& value);
	QString getName() const;

	void addOutgoingTransition(shared_ptr<FsmTransition> transition);
	void removeOutgoingTransition(shared_ptr<FsmTransition> transition);
	const QList<shared_ptr<FsmTransition>> getOutgoingTransitions() const;

	void addIncomingTransition(shared_ptr<FsmTransition> transition);
	void removeIncomingTransition(shared_ptr<FsmTransition> transition);
	const QList<shared_ptr<FsmTransition>> getIncomingTransitions() const;

	bool isInitial() const;
	void notifyInitialStatusChanged();

	virtual uint getAllowedActionTypes() const override;

	// Simuation status
	void setActive(bool value);
	bool getIsActive() const;

	// Graphic management
	FsmGraphicState* getGraphicRepresentation();

signals:
	void stateRenamedEvent();
	void stateSimulatedStateChangedEvent();
	void statePositionChangedEvent(shared_ptr<FsmState> me);

private slots:
	void graphicRepresentationDeletedEventHandler();
	void graphicRepresentationMovedEventHandler();

private:
	QList<weak_ptr<FsmTransition>> inputTransitions;
	QList<weak_ptr<FsmTransition>> outputTransitions;

	FsmGraphicState* graphicRepresentation = nullptr;
	QString name;

	bool isActive = false;
};

#endif // FSMSTATE_H
