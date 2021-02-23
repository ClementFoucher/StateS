/*
 * Copyright © 2014-2021 Clément Foucher
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

#ifndef FSMSCENE_H
#define FSMSCENE_H

// Parent
#include "genericscene.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QAction;

// To access enums
#include "machine.h"
#include "machinebuilder.h"

// StateS classes
class MachineManager;
class FsmGraphicTransition;
class FsmGraphicState;
class Fsm;
class FsmState;
class FsmTransition;


/**
 * @brief The FsmScene class is a FSM graphic representation.
 * It is deeply linked to the Fsm object, which can't be replaced:
 * Object must be deleted and recreated if the machine changes.
 */
class FsmScene : public GenericScene
{
	Q_OBJECT

private:
	enum sceneMode_e
	{
		idle,
		goingBackToIdle,
		askedForMenu,
		drawingTransition,
		editingTransitionSource,
		editingTransitionTarget,
		simulating
	};

public:
	explicit FsmScene(shared_ptr<MachineManager> machineManager);
	~FsmScene();

	virtual void setDisplaySize(const QSize& newSize) override;

	void beginDrawTransition(FsmGraphicState* source, const QPointF& currentMousePos);

protected:
	void mousePressEvent      (QGraphicsSceneMouseEvent*)       override;
	void mouseMoveEvent       (QGraphicsSceneMouseEvent*)       override;
	void mouseReleaseEvent    (QGraphicsSceneMouseEvent*)       override;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)       override;
	void contextMenuEvent     (QGraphicsSceneContextMenuEvent*) override;
	void keyPressEvent        (QKeyEvent*)                      override;

private slots:
	void simulationModeChanged(Machine::simulation_mode newMode);

	void handleSelection();
	void stateCallsEditEventHandler(shared_ptr<FsmState> state);
	void stateCallsRenameEventHandler(shared_ptr<FsmState> state);
	void treatMenu(QAction*);
	void menuHiding();
	void changedToolEventHandler(MachineBuilder::tool);
	void changedSingleUseToolEventHandler(MachineBuilder::singleUseTool);

	void updateSceneRect();

	void transitionCallsDynamicSourceEventHandler(FsmGraphicTransition* transition);
	void transitionCallsDynamicTargetEventHandler(FsmGraphicTransition* transition);
	void transitionCallsEditEventHandler(shared_ptr<FsmTransition> transition);

private:
	FsmGraphicState* getStateAt(const QPointF& location) const;
	FsmGraphicState* addState(FsmGraphicState* newState);
	void addTransition(FsmGraphicTransition* newTransition);
	void build();

	void cancelDrawTransition();

	shared_ptr<Fsm> getFsm() const;

private:
	shared_ptr<MachineManager> machineManager;

	sceneMode_e sceneMode;
	FsmGraphicTransition* currentTransition = nullptr;

	QPointF mousePos;

	QSize displaySize;
};

#endif // FSMSCENE_H
