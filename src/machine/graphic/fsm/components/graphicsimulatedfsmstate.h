/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRAPHICSIMULATEDFSMSTATE_H
#define GRAPHICSIMULATEDFSMSTATE_H

// Parents
#include "graphicfsmstate.h"
#include "graphicsimulatedcomponent.h"

// StateS classes
#include "statestypes.h"


class GraphicSimulatedFsmState : public GraphicFsmState, public GraphicSimulatedComponent
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicSimulatedFsmState(componentId_t logicComponentId);

	/////
	// Object functions
public:
	virtual void refreshSimulatedDisplay() override;

protected:
	virtual void keyPressEvent(QKeyEvent* event)                         override;
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private slots:
	void menuSetActiveTriggeredEventHandler(QAction* action);

	/////
	// Signals
signals:
	void componentRefreshedEvent();

};

#endif // GRAPHICSIMULATEDFSMSTATE_H
