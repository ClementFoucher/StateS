/*
 * Copyright © 2023 Clément Foucher
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

#ifndef FSMSIMULATEDSTATE_H
#define FSMSIMULATEDSTATE_H

// Parents
#include "fsmgraphicstate.h"
#include "simulatedactuatorcomponent.h"

// Qt classes
class QAction;

// StateS classes
#include "statestypes.h"


class FsmSimulatedState : public FsmGraphicState, public SimulatedActuatorComponent
{
	Q_OBJECT

	/////
	// Static variables
private:
	static const QBrush activeBrush;

	/////
	// Constructors/destructors
public:
	explicit FsmSimulatedState(componentId_t logicComponentId);
	virtual ~FsmSimulatedState() override;

	/////
	// Object functions
public:
	virtual void refreshDisplay() override;

	void setActive(bool active);
	bool getIsActive() const;

signals:
	void stateActiveStatusChanged();

protected:
	virtual void keyPressEvent(QKeyEvent* event)                         override;
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private slots:
	void treatMenu(QAction* action);

	/////
	// Object variables
private:
	bool isActive = false;

};

#endif // FSMSIMULATEDSTATE_H
