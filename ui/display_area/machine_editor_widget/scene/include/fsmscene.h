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
 * It is deeply linked to the Fsm object, which must be provided
 * at creation, and can't be replaced.
 */
class FsmScene : public GenericScene
{
	Q_OBJECT

private:
	enum class sceneMode_t
	{
		// General modes
		idle,
		simulating,
		// Special mode to handle click events TODO: remove
		goingBackToIdle,
		// Edition of existing elements
		editingTransitionSource,
		editingTransitionTarget,
		// Adding modes with a tool
		addingInitialState,
		addingState,
		addingTransition,
		// Single-use tools
		addingTransitionSingleUse
	};

	enum class addTransitionStep_t
	{
		notInTransitionAddingMode,
		waitingForSource,
		settingSource,
		waitingForTarget,
		settingTarget
	};

public:
	explicit FsmScene(shared_ptr<MachineManager> machineManager);
	~FsmScene();

protected:
	void mousePressEvent      (QGraphicsSceneMouseEvent*       me) override;
	void mouseMoveEvent       (QGraphicsSceneMouseEvent*       me) override;
	void mouseReleaseEvent    (QGraphicsSceneMouseEvent*       me) override;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*       me) override;
	void keyPressEvent        (QKeyEvent*                      ke) override;
	void contextMenuEvent     (QGraphicsSceneContextMenuEvent* ce) override;

private slots:
	void simulationModeChangeEventHandler(Machine::simulation_mode newMode);
	void toolChangeEventHandler(MachineBuilder::tool newTool);

	void stateCallsEditEventHandler(shared_ptr<FsmState> state);
	void stateCallsRenameEventHandler(shared_ptr<FsmState> state);
	void stateCallsBeginTransitionEventHandler(FsmGraphicState* sourceState);

	void transitionCallsDynamicSourceEventHandler(FsmGraphicTransition* transition);
	void transitionCallsDynamicTargetEventHandler(FsmGraphicTransition* transition);
	void transitionCallsEditEventHandler(shared_ptr<FsmTransition> transition);

	void handleSelection();
	void treatMenu(QAction* action);
	void upateSceneMode(sceneMode_t newMode);

private:
	void build();

	void beginDrawTransition(FsmGraphicState* source, const QPointF& currentMousePos = QPointF());
	void cancelDrawTransition();
	void addTransition(FsmGraphicTransition* newTransition);

	FsmGraphicState* addState(FsmGraphicState* newState);
	FsmGraphicState* getStateAt(const QPointF& location) const;

	shared_ptr<Fsm> getFsm() const;

private:
	shared_ptr<MachineManager> machineManager;

	sceneMode_t sceneMode;
	addTransitionStep_t transitionStep = addTransitionStep_t::notInTransitionAddingMode;

	FsmGraphicTransition* currentTransition = nullptr;
	QPointF mousePos;
};

#endif // FSMSCENE_H
