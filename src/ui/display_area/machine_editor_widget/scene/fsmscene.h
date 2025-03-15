/*
 * Copyright © 2014-2025 Clément Foucher
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

// Qt classes
class QAction;

// StateS classes
#include "statestypes.h"
class GraphicFsmTransition;
class GraphicFsmState;


/**
 * @brief The FsmScene class displays the Graphic FSM
 * components, and is capable of editing both the
 * graphic FSM and the logic FSM.
 */
class FsmScene : public GenericScene
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class SceneMode_t
	{
		// General modes
		idle,
		simulating,
		// Adding modes with a Machine Builder tool
		addingInitialState,
		addingState,
		addingTransition,
		// Single-use tools
		editingTransitionSource,
		editingTransitionTarget,
		addingTransitionSingleUse
	};

	enum class AddTransitionStep_t
	{
		notInTransitionAddingMode,
		waitingForSource,
		settingSource,
		waitingForTarget,
		settingTarget
	};

	/////
	// Constructors/destructors
public:
	explicit FsmScene();
	~FsmScene();

	/////
	// Object functions
protected:
	virtual void mousePressEvent  (QGraphicsSceneMouseEvent*       me) override;
	virtual void mouseMoveEvent   (QGraphicsSceneMouseEvent*       me) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*       me) override;
	virtual void keyPressEvent    (QKeyEvent*                      ke) override;
	virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* ce) override;

	virtual void updateSimulatioMode(SimulationMode_t newMode) override;

private slots:
	// Machine manager signals
	void machineUpdatedEventHandler();

	// Tools signals
	void toolChangeEventHandler(MachineBuilderTool_t newTool);
	void singleUseToolChangeEventHandler(MachineBuilderSingleUseTool_t newTool);

	// States signals
	void stateCallsEditEventHandler(componentId_t stateId);
	void stateCallsRenameEventHandler(componentId_t stateId);
	void stateCallsDeleteEventHandler(componentId_t stateId);
	void stateCallsSetInitialStateEventHandler(componentId_t stateId);
	void stateCallsBeginTransitionEventHandler(componentId_t stateId);
	void statePositionAboutToChangeEventHandler(componentId_t stateId);
	void statePositionChangedEventHandler(componentId_t stateId);

	// Transitions signals
	void transitionCallsDynamicSourceEventHandler(componentId_t transitionId);
	void transitionCallsDynamicTargetEventHandler(componentId_t transitionId);
	void transitionCallsEditEventHandler(componentId_t transitionId);
	void transitionCallsDeleteEventHandler(componentId_t transitionId);

	// Others
	void handleSelection();
	void menuAddStateTriggeredEventHandler(QAction* action);

private:
	// Scene mode
	void updateSceneMode(FsmScene::SceneMode_t newMode);

	// Scene clear/build
	void displayGraphicMachine();
	void displaySimulatedMachine();
	void clearScene();

	// Add elements
	void addTransition(GraphicFsmTransition* newTransition, bool connectSignals);
	void addState(GraphicFsmState* newState, bool connectSignals);

	// Transition adding/editings
	void beginDrawTransition(GraphicFsmState* source, const QPointF& currentMousePos = QPointF());
	void cancelOngoingAction();

	// Accessors
	GraphicFsmState* getStateAt(const QPointF& location) const;

	// Helper functions
	QString getUniqueStateName();

	/////
	// Object variables
private:
	// Scene mode
	SceneMode_t sceneMode = SceneMode_t::idle;

	// Transition adding/edition
	AddTransitionStep_t transitionStep = AddTransitionStep_t::notInTransitionAddingMode;
	GraphicFsmTransition* dummyTransition = nullptr;
	componentId_t transitionUnderEditId = nullId;

	// Temporary variables
	QPointF menuMousePos;
	bool transmitMouseEvent;

};

#endif // FSMSCENE_H
