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

// Current class header
#include "fsmscene.h"

// Qt classes
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QMessageBox>

// StateS classes
#include "machinemanager.h"
#include "machinebuilder.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "simulatedfsm.h"
#include "graphicfsm.h"
#include "graphicfsmstate.h"
#include "graphicsimulatedfsmstate.h"
#include "graphicfsmtransition.h"
#include "graphicsimulatedfsmtransition.h"
#include "fsmundocommand.h"
#include "actionbox.h"
#include "contextmenu.h"


FsmScene::FsmScene() :
    GenericScene()
{
	connect(this, &QGraphicsScene::selectionChanged, this, &FsmScene::handleSelection);

	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &FsmScene::machineUpdatedEventHandler);

	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
	connect(machineBuilder.get(), &MachineBuilder::changedToolEvent,      this, &FsmScene::toolChangeEventHandler);
	connect(machineBuilder.get(), &MachineBuilder::singleUseToolSelected, this, &FsmScene::singleUseToolChangeEventHandler);

	// Add scene content
	this->displayGraphicMachine();
}

FsmScene::~FsmScene()
{
	// Remove displayed items to avoid scene deleting
	// them, as they belong to a graphic manager
	this->clearScene();
}

void FsmScene::mousePressEvent(QGraphicsSceneMouseEvent* me)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr)
	{
		GenericScene::mousePressEvent(me);
		return;
	}

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr)
	{
		GenericScene::mousePressEvent(me);
		return;
	}

	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
	if (machineBuilder == nullptr)
	{
		GenericScene::mousePressEvent(me);
		return;
	}


	this->transmitMouseEvent = true;

	if (me->button() == Qt::LeftButton)
	{
		if (this->sceneMode == SceneMode_t::addingInitialState)
		{
			// Create state
			auto logicStateId = fsm->addState(true);

			GraphicFsmState* graphicState = graphicFsm->addState(logicStateId, me->scenePos());
			machineManager->notifyMachineEdited();

			// Add graphic state to scene
			this->addState(graphicState, true);

			// Only one initial state in a FSM, switch to regular state tool
			machineBuilder->setTool(MachineBuilderTool_t::state);

			// Transmitting event so that new state is selected
			// and can be moved within the same click
		}
		else if (this->sceneMode == SceneMode_t::addingState)
		{
			// Create state
			auto logicStateId = fsm->addState();

			GraphicFsmState* graphicState = graphicFsm->addState(logicStateId, me->scenePos());
			machineManager->notifyMachineEdited();

			// Add graphic state to scene
			this->addState(graphicState, true);

			// Transmitting event so that new state is selected
			// and can be moved within the same click
		}
		else if ( (this->sceneMode == SceneMode_t::addingTransition) || (this->sceneMode == SceneMode_t::addingTransitionSingleUse) )
		{
			GraphicFsmState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

			if (stateUnderMouse != nullptr)
			{
				if (this->transitionStep == AddTransitionStep_t::waitingForSource)
				{
					this->beginDrawTransition(stateUnderMouse, me->scenePos());
					this->transitionStep = AddTransitionStep_t::waitingForTarget;
				}
				else if (this->transitionStep == AddTransitionStep_t::waitingForTarget)
				{
					// Obtain info from temporary transition then get rid of it
					auto sourceStateId = this->dummyTransition->getSourceStateId();
					auto targetStateId = stateUnderMouse->getLogicComponentId();
					delete this->dummyTransition;
					this->dummyTransition = nullptr;

					// Build new transition
					auto logicTransitionId = fsm->addTransition(sourceStateId, targetStateId);

					GraphicFsmTransition* graphicTransition = graphicFsm->addTransition(logicTransitionId, 0.5);
					machineManager->notifyMachineEdited();

					// Add graphic transition to scene
					this->addTransition(graphicTransition, true);

					if (this->sceneMode == SceneMode_t::addingTransition)
					{
						// Get ready for next transition
						this->transitionStep = AddTransitionStep_t::waitingForSource;
					}
					else // (this->sceneMode == sceneMode_t::addingTransitionSingleUse)
					{
						// Single-use tool: terminate transition adding mode
						machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);
					}
				}

				this->transmitMouseEvent = false;
			}
		}
		else if ( (this->sceneMode == SceneMode_t::editingTransitionSource) || (this->sceneMode == SceneMode_t::editingTransitionTarget) )
		{
			GraphicFsmState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

			if (stateUnderMouse != nullptr)
			{
				auto logicTransition = fsm->getTransition(this->transitionUnderEditId);
				if (logicTransition != nullptr)
				{
					// Just cancel edition if state has not changed
					bool ignore = false;
					if (this->sceneMode == SceneMode_t::editingTransitionSource)
					{
						if (logicTransition->getSourceStateId() == stateUnderMouse->getLogicComponentId())
						{
							ignore = true;
						}
					}
					else // (this->sceneMode == sceneMode_t::editingTransitionTarget)
					{
						if (logicTransition->getTargetStateId() == stateUnderMouse->getLogicComponentId())
						{
							ignore = true;
						}
					}

					if (ignore == false)
					{
						// Get info about current graphic transition
						auto graphicTransition = graphicFsm->getTransition(this->transitionUnderEditId);
						if (graphicTransition != nullptr)
						{
							auto sliderPosition = graphicTransition->getConditionLineSliderPosition();

							// Update logic transition and replace graphic transition
							graphicFsm->removeGraphicComponent(this->transitionUnderEditId);

							if (this->sceneMode == SceneMode_t::editingTransitionTarget)
							{
								fsm->redirectTransition(logicTransition->getId(), logicTransition->getSourceStateId(), stateUnderMouse->getLogicComponentId());
							}
							else
							{
								fsm->redirectTransition(logicTransition->getId(), stateUnderMouse->getLogicComponentId(), logicTransition->getTargetStateId());
							}

							graphicTransition = graphicFsm->addTransition(logicTransition->getId(), sliderPosition);
							this->addTransition(graphicTransition, true);

							machineManager->notifyMachineEdited();
						}
					}
				}

				// Single-use tools: terminate current mode
				machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);

				this->transmitMouseEvent = false;
			}
		}
	}
	else if (me->button() == Qt::RightButton)
	{
		if (this->sceneMode != SceneMode_t::idle)
		{
			this->cancelOngoingAction();
			this->transmitMouseEvent = false;
		}
		else // (this->sceneMode == SceneMode_t::idle)
		{
			if (this->items(me->scenePos(), Qt::IntersectsItemShape, Qt::DescendingOrder).count() == 0)
			{
				// If no tool was selected and there is no item under mouse, display menu.
				// If there is an item under mouse, this is a context menu event on that particular item,
				// in this case just transmit event

				ContextMenu* menu = new ContextMenu();
				menu->addAction(tr("Add state"));
				menu->addAction(tr("Add initial state"));
				menu->popup(me->screenPos());

				this->menuMousePos = me->scenePos();

				connect(menu, &QMenu::triggered, this, &FsmScene::menuAddStateTriggeredEventHandler);

				this->transmitMouseEvent = false;
			}
		}
	}

	if (this->transmitMouseEvent == true)
	{
		GenericScene::mousePressEvent(me);
	}
}

void FsmScene::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
	static GraphicFsmState* previousStatePointed = nullptr;

	if ( (this->transitionStep == AddTransitionStep_t::settingSource)    ||
	     (this->transitionStep == AddTransitionStep_t::waitingForTarget) ||
	     (this->sceneMode == SceneMode_t::editingTransitionSource)       ||
	     (this->sceneMode == SceneMode_t::editingTransitionTarget)
	   )
	{
		GraphicFsmState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

		if (stateUnderMouse != nullptr)
		{
			if (stateUnderMouse != previousStatePointed)
			{
				this->dummyTransition->setDynamicState(stateUnderMouse->getLogicComponentId());
				previousStatePointed = stateUnderMouse;
			}
		}
		else
		{
			this->dummyTransition->setMousePosition(me->scenePos());
			previousStatePointed = nullptr;
		}
	}

	// Always transmit mode event as it is required
	// by scene to know cursor position
	GenericScene::mouseMoveEvent(me);
}

void FsmScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* me)
{
	bool transmitEvent = true;

	if (this->transmitMouseEvent == false)
	{
		transmitEvent = false;
		this->transmitMouseEvent = true;
	}

	if (transmitEvent == true)
	{
		GenericScene::mouseReleaseEvent(me);
	}
}

void FsmScene::keyPressEvent(QKeyEvent* ke)
{
	if (ke->key() == Qt::Key_Escape)
	{
		if (this->sceneMode != SceneMode_t::idle)
		{
			this->cancelOngoingAction();
		}
		else // (this->sceneMode == SceneMode_t::idle)
		{
			this->clearSelection();
		}
	}
	else if ( (ke->key() == Qt::Key_Right) ||
	          (ke->key() == Qt::Key_Left)  ||
	          (ke->key() == Qt::Key_Up)    ||
	          (ke->key() == Qt::Key_Down)
	          )
	{
		// Handle move events in scene as they can
		// be dispatched to multiple states.
		if (this->selectedItems().count() == 0)
		{
			// No selected item: ignore event. Event will then be
			// transmitted to view widget which will handle it as view move
			ke->ignore();
		}
		else // At least one item selected
		{
			bool atLeastOneState = false;

			qreal moveSize = 10;
			if ((ke->modifiers() & Qt::ShiftModifier) != 0)
			{
				moveSize = 1;
			}

			// Transmit event to each state in the list
			const auto selectedItems = this->selectedItems();
			for (QGraphicsItem* item : selectedItems)
			{
				GraphicFsmState* state = dynamic_cast<GraphicFsmState*>(item);

				if (state != nullptr)
				{
					atLeastOneState = true;

					QPointF movePoint;
					switch (ke->key())
					{
					case Qt::Key_Left:
						movePoint = QPointF(-moveSize, 0);
						break;
					case Qt::Key_Right:
						movePoint = QPointF(moveSize, 0);
						break;
					case Qt::Key_Up:
						movePoint = QPointF(0, -moveSize);
						break;
					case Qt::Key_Down:
						movePoint = QPointF(0, moveSize);
						break;
					}

					state->setPos(state->pos() + movePoint);
				}
			}

			if (atLeastOneState == false)
			{
				// Same as if there were no selected items at all
				ke->ignore();
			}
		}
	}
	if (ke->key() == Qt::Key_Delete)
	{
		// Allow deleting scene components only in idle state
		if (this->sceneMode == SceneMode_t::idle)
		{
			GenericScene::keyPressEvent(ke);
		}
	}
	else
	{
		// All other events are passed directly to items
		GenericScene::keyPressEvent(ke);
	}
}

void FsmScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* ce)
{
	// Context menu events are only allowed in these two modes
	if ( (this->sceneMode == SceneMode_t::idle) || (this->sceneMode == SceneMode_t::simulating) )
	{
		GenericScene::contextMenuEvent(ce);
	}
}

void FsmScene::updateSimulatioMode(SimulationMode_t newMode)
{
	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
	if (machineBuilder != nullptr)
	{
		machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);

		if (newMode == SimulationMode_t::editMode)
		{
			this->clearScene();

			this->updateSceneMode(SceneMode_t::idle);

			this->displayGraphicMachine();
		}
		else // (newMode == SimulationMode_t::simulateMode)
		{
			// Go back to idle mode first to cancel ongoing editions
			if (this->sceneMode != SceneMode_t::idle)
			{
				this->updateSceneMode(SceneMode_t::idle);
			}

			this->clearScene();

			this->updateSceneMode(SceneMode_t::simulating);

			this->displaySimulatedMachine();
		}
	}
}

void FsmScene::machineUpdatedEventHandler()
{
	delete this->dummyTransition;
	this->dummyTransition = nullptr;

	this->transitionUnderEditId = nullId;
	this->sceneMode = SceneMode_t::idle;
	this->transitionStep = AddTransitionStep_t::notInTransitionAddingMode;

	this->displayGraphicMachine();
}

void FsmScene::toolChangeEventHandler(MachineBuilderTool_t newTool)
{
	switch(newTool)
	{
	case MachineBuilderTool_t::initialState:
		this->updateSceneMode(SceneMode_t::addingInitialState);
		break;
	case MachineBuilderTool_t::state:
		this->updateSceneMode(SceneMode_t::addingState);
		break;
	case MachineBuilderTool_t::transition:
		this->updateSceneMode(SceneMode_t::addingTransition);
		break;
	default:
		this->updateSceneMode(SceneMode_t::idle);
	}
}

void FsmScene::singleUseToolChangeEventHandler(MachineBuilderSingleUseTool_t newTool)
{
	switch(newTool)
	{
	case MachineBuilderSingleUseTool_t::drawTransitionFromScene:
		this->updateSceneMode(SceneMode_t::addingTransitionSingleUse);
		break;
	case MachineBuilderSingleUseTool_t::editTransitionSource:
		this->updateSceneMode(SceneMode_t::editingTransitionSource);
		break;
	case MachineBuilderSingleUseTool_t::editTransitionTarget:
		this->updateSceneMode(SceneMode_t::editingTransitionTarget);
		break;
	default:
		this->updateSceneMode(SceneMode_t::idle);
	}
}

void FsmScene::stateCallsEditEventHandler(componentId_t stateId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto graphicState = graphicFsm->getState(stateId);
	if (graphicState == nullptr) return;


	this->clearSelection();
	graphicState->setSelected(true);
	emit editSelectedItemEvent();
}

void FsmScene::stateCallsRenameEventHandler(componentId_t stateId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto graphicState = graphicFsm->getState(stateId);
	if (graphicState == nullptr) return;


	this->clearSelection();
	graphicState->setSelected(true);
	emit renameSelectedItemEvent();
}

void FsmScene::stateCallsDeleteEventHandler(componentId_t stateId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(stateId);
	if (logicState == nullptr) return;

	bool doDelete = false;
	int linkedTransitions = logicState->getOutgoingTransitionsIds().count() + logicState->getIncomingTransitionsIds().count();
	if (linkedTransitions != 0)
	{
		QString messageText = tr("Delete current state?") + "<br />";
		if (linkedTransitions == 1)
			messageText += tr("The connected transition will be deleted");
		else
			messageText += tr("All") + " " + QString::number(linkedTransitions) + " " + tr("connected transitions will be deleted");

		QMessageBox::StandardButton reply = QMessageBox::question(this->views().at(0), tr("User confirmation required"), messageText, QMessageBox::Ok | QMessageBox::Cancel);

		if (reply == QMessageBox::StandardButton::Ok)
		{
			doDelete = true;
		}
	}
	else
	{
		doDelete = true;
	}

	if (doDelete == true)
	{
		fsm->removeState(stateId);
		machineManager->notifyMachineEdited();
	}
}

void FsmScene::stateCallsSetInitialStateEventHandler(componentId_t stateId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	fsm->setInitialState(stateId);
	machineManager->notifyMachineEdited();
}

void FsmScene::stateCallsBeginTransitionEventHandler(componentId_t stateId)
{
	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
	if (machineBuilder == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto graphicState = graphicFsm->getState(stateId);
	if (graphicState == nullptr) return;

	this->beginDrawTransition(graphicState);

	machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::drawTransitionFromScene);
}

void FsmScene::statePositionAboutToChangeEventHandler(componentId_t stateId)
{
	static bool inMacro = false;
	static uint statesRemaining;

	if (inMacro == false)
	{
		// Count the number of selected states
		uint selectedStatesCount = 0;
		const auto selectedItems = this->selectedItems();
		for (QGraphicsItem* item : selectedItems)
		{
			if (dynamic_cast<GraphicFsmState*>(item) != nullptr)
			{
				selectedStatesCount++;
			}
		}
		if (selectedStatesCount > 1)
		{
			// If multiple states selected, create an undo macro
			machineManager->beginUndoMacro(tr("Multiple states moved"));
			statesRemaining = selectedStatesCount;
			inMacro = true;
		}
	}

	auto undoCommand = new FsmUndoCommand(UndoCommandId_t::fsmUndoStateMoveId, stateId);
	machineManager->notifyMachineEdited(undoCommand);

	if (inMacro == true)
	{
		statesRemaining--;

		if (statesRemaining == 0)
		{
			machineManager->endUndoMacro();
			inMacro = false;
		}
	}
}

void FsmScene::statePositionChangedEventHandler(componentId_t)
{
	this->recomputeSceneRect();
}

void FsmScene::transitionCallsDynamicSourceEventHandler(componentId_t transitionId)
{
	auto machineBuilder = machineManager->getMachineBuilder();
	if (machineBuilder == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto transition = graphicFsm->getTransition(transitionId);
	if (transition == nullptr) return;

	QGraphicsView* currentView = this->views().constFirst();
	QPointF sceneMousePos = currentView->mapToScene(currentView->mapFromGlobal(QCursor::pos()));
	this->dummyTransition = new GraphicFsmTransition(nullId, transition->getTargetStateId(), sceneMousePos);
	this->transitionUnderEditId = transition->getLogicComponentId();
	this->addTransition(this->dummyTransition, false);

	this->clearSelection();
	transition->setSelected(true);
	transition->setUnderEdit(true);

	machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::editTransitionSource);
}

void FsmScene::transitionCallsDynamicTargetEventHandler(componentId_t transitionId)
{
	auto machineBuilder = machineManager->getMachineBuilder();
	if (machineBuilder == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto transition = graphicFsm->getTransition(transitionId);
	if (transition == nullptr) return;


	QGraphicsView* currentView = this->views().constFirst();
	QPointF sceneMousePos = currentView->mapToScene(currentView->mapFromGlobal(QCursor::pos()));
	this->dummyTransition = new GraphicFsmTransition(transition->getSourceStateId(), nullId, sceneMousePos);
	this->transitionUnderEditId = transition->getLogicComponentId();
	this->addTransition(this->dummyTransition, false);

	this->clearSelection();
	transition->setSelected(true);
	transition->setUnderEdit(true);

	machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::editTransitionTarget);
}

void FsmScene::transitionCallsEditEventHandler(componentId_t transitionId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto transition = graphicFsm->getTransition(transitionId);
	if (transition == nullptr) return;


	this->clearSelection();
	transition->setSelected(true);
	emit editSelectedItemEvent();
}

void FsmScene::transitionCallsDeleteEventHandler(componentId_t transitionId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	fsm->removeTransition(transitionId);
	machineManager->notifyMachineEdited();
}

void FsmScene::handleSelection()
{
	// Set focus for context event
	if (this->selectedItems().count() == 1)
	{
		this->selectedItems().at(0)->setFocus();
	}
	else
	{
		this->clearFocus();
	}

	// Updates resource panel selected item
	if (this->selectedItems().count() == 1)
	{
		GraphicFsmState* currentState = dynamic_cast< GraphicFsmState* >(this->selectedItems().at(0));
		if (currentState != nullptr)
		{
			emit itemSelectedEvent(currentState->getLogicComponentId());
		}
		else
		{
			GraphicFsmTransition* currentTransition = dynamic_cast< GraphicFsmTransition* >(this->selectedItems().at(0));
			if (currentTransition!= nullptr)
			{
				emit itemSelectedEvent(currentTransition->getLogicComponentId());
			}
		}
	}
	else
	{
		emit itemSelectedEvent(nullId);
	}
}

void FsmScene::menuAddStateTriggeredEventHandler(QAction* action)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;


	auto logicStateId = nullId;
	if (action->text() == tr("Add state"))
	{
		logicStateId = fsm->addState();
	}
	else if (action->text() == tr("Add initial state"))
	{
		logicStateId = fsm->addState(true);
	}

	if (logicStateId != nullId)
	{
		GraphicFsmState* graphicState = graphicFsm->addState(logicStateId, this->menuMousePos);
		machineManager->notifyMachineEdited();

		this->addState(graphicState, true);
	}
}

void FsmScene::updateSceneMode(FsmScene::SceneMode_t newMode)
{
	// Cancel ongoing editions before changing mode
	if ( (this->sceneMode == SceneMode_t::editingTransitionSource)   ||
	     (this->sceneMode == SceneMode_t::editingTransitionTarget)   ||
	     (this->sceneMode == SceneMode_t::addingTransitionSingleUse) ||
	     (this->sceneMode == SceneMode_t::addingTransition)          )
	{
		delete this->dummyTransition;
		this->dummyTransition = nullptr;

		if ( (this->sceneMode == SceneMode_t::editingTransitionSource) ||
		     (this->sceneMode == SceneMode_t::editingTransitionTarget) )
		{
			auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
			if (graphicFsm == nullptr) return;

			auto graphicTransition = graphicFsm->getTransition(this->transitionUnderEditId);
			if (graphicTransition == nullptr) return;

			graphicTransition->setUnderEdit(false);
		}
	}

	this->sceneMode = newMode;

	// Set transition mode step
	if (newMode == SceneMode_t::addingTransition)
	{
		this->transitionStep = AddTransitionStep_t::waitingForSource;
	}
	else if (newMode == SceneMode_t::addingTransitionSingleUse)
	{
		this->transitionStep = AddTransitionStep_t::waitingForTarget;
	}
	else
	{
		this->transitionStep = AddTransitionStep_t::notInTransitionAddingMode;
	}
}

void FsmScene::displayGraphicMachine()
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;


	QList<GraphicFsmState*> states = graphicFsm->getStates();
	for (GraphicFsmState* graphicState : states)
	{
		this->addState(graphicState, true);
	}

	QList<GraphicFsmTransition*> transitions = graphicFsm->getTransitions();
	for (GraphicFsmTransition* graphicTransition : transitions)
	{
		this->addTransition(graphicTransition, true);
	}
}

void FsmScene::displaySimulatedMachine()
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	for (auto& stateId : fsm->getAllStatesIds())
	{
		auto simulatedState = graphicFsm->getSimulatedState(stateId);
		if (simulatedState == nullptr) continue;


		this->addState(simulatedState, false);
	}

	for (auto& transitionId : fsm->getAllTransitionsIds())
	{
		auto simulatedTransition = graphicFsm->getSimulatedTransition(transitionId);
		if (simulatedTransition == nullptr) continue;


		this->addTransition(simulatedTransition, false);
	}
}

void FsmScene::clearScene()
{
	const auto displayedItems = this->items();
	for (auto item : displayedItems)
	{
		auto transitionItem = dynamic_cast<GraphicFsmTransition*>(item);
		if (transitionItem != nullptr)
		{
			if (this->sceneMode != SceneMode_t::simulating)
			{
				disconnect(transitionItem, &GraphicFsmTransition::dynamicSourceCalledEvent,    this, &FsmScene::transitionCallsDynamicSourceEventHandler);
				disconnect(transitionItem, &GraphicFsmTransition::dynamicTargetCalledEvent,    this, &FsmScene::transitionCallsDynamicTargetEventHandler);
				disconnect(transitionItem, &GraphicFsmTransition::editTransitionCalledEvent,   this, &FsmScene::transitionCallsEditEventHandler);
				disconnect(transitionItem, &GraphicFsmTransition::deleteTransitionCalledEvent, this, &FsmScene::transitionCallsDeleteEventHandler);
			}

			auto condition = transitionItem->getConditionText();
			if (condition != nullptr)
			{
				this->removeItem(condition);
			}

			auto actions = transitionItem->getActionBox();
			if (actions != nullptr)
			{
				this->removeItem(actions);
			}

			this->removeItem(transitionItem);
		}

		auto stateItem = dynamic_cast<GraphicFsmState*>(item);
		if (stateItem != nullptr)
		{
			if (this->sceneMode != SceneMode_t::simulating)
			{
				disconnect(stateItem, &GraphicFsmState::editStateCalledEvent,             this, &FsmScene::stateCallsEditEventHandler);
				disconnect(stateItem, &GraphicFsmState::renameStateCalledEvent,           this, &FsmScene::stateCallsRenameEventHandler);
				disconnect(stateItem, &GraphicFsmState::deleteStateCalledEvent,           this, &FsmScene::stateCallsDeleteEventHandler);
				disconnect(stateItem, &GraphicFsmState::setInitialStateCalledEvent,       this, &FsmScene::stateCallsSetInitialStateEventHandler);
				disconnect(stateItem, &GraphicFsmState::beginDrawTransitionFromThisState, this, &FsmScene::stateCallsBeginTransitionEventHandler);
				disconnect(stateItem, &GraphicFsmState::statePositionAboutToChangeEvent,  this, &FsmScene::statePositionAboutToChangeEventHandler);
				disconnect(stateItem, &GraphicFsmState::statePositionChangedEvent,        this, &FsmScene::statePositionChangedEventHandler);
			}

			auto actions = stateItem->getActionBox();
			if (actions != nullptr)
			{
				this->removeItem(actions);
			}

			this->removeItem(stateItem);
		}
	}

	this->recomputeSceneRect();
}

void FsmScene::addTransition(GraphicFsmTransition* newTransition, bool connectSignals)
{
	this->addItem(newTransition);
	newTransition->setZValue(2);

	QGraphicsTextItem* condition = newTransition->getConditionText();
	if (condition != nullptr)
	{
		this->addItem(condition);
		condition->setZValue(3);
	}

	QGraphicsItemGroup* actionsBox = newTransition->getActionBox();
	if (actionsBox != nullptr)
	{
		this->addItem(actionsBox);
		actionsBox->setZValue(3);
	}

	if (connectSignals ==  true)
	{
		connect(newTransition, &GraphicFsmTransition::dynamicSourceCalledEvent,    this, &FsmScene::transitionCallsDynamicSourceEventHandler);
		connect(newTransition, &GraphicFsmTransition::dynamicTargetCalledEvent,    this, &FsmScene::transitionCallsDynamicTargetEventHandler);
		connect(newTransition, &GraphicFsmTransition::editTransitionCalledEvent,   this, &FsmScene::transitionCallsEditEventHandler);
		connect(newTransition, &GraphicFsmTransition::deleteTransitionCalledEvent, this, &FsmScene::transitionCallsDeleteEventHandler);
	}

	this->recomputeSceneRect();
}

void FsmScene::addState(GraphicFsmState* newState, bool connectSignals)
{
	this->addItem(newState);
	newState->setZValue(1);

	QGraphicsItemGroup* actionsBox = newState->getActionBox();
	this->addItem(actionsBox);
	actionsBox->setZValue(3);

	if (connectSignals == true)
	{
		connect(newState, &GraphicFsmState::editStateCalledEvent,             this, &FsmScene::stateCallsEditEventHandler);
		connect(newState, &GraphicFsmState::renameStateCalledEvent,           this, &FsmScene::stateCallsRenameEventHandler);
		connect(newState, &GraphicFsmState::deleteStateCalledEvent,           this, &FsmScene::stateCallsDeleteEventHandler);
		connect(newState, &GraphicFsmState::setInitialStateCalledEvent,       this, &FsmScene::stateCallsSetInitialStateEventHandler);
		connect(newState, &GraphicFsmState::beginDrawTransitionFromThisState, this, &FsmScene::stateCallsBeginTransitionEventHandler);
		connect(newState, &GraphicFsmState::statePositionAboutToChangeEvent,  this, &FsmScene::statePositionAboutToChangeEventHandler);
		connect(newState, &GraphicFsmState::statePositionChangedEvent,        this, &FsmScene::statePositionChangedEventHandler);
	}

	this->recomputeSceneRect();
}

void FsmScene::beginDrawTransition(GraphicFsmState* source, const QPointF& currentMousePos)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	if (!currentMousePos.isNull())
	{
		this->dummyTransition = new GraphicFsmTransition(source->getLogicComponentId(), nullId, currentMousePos);
	}
	else
	{
		// Compute mouse pos wrt. scene
		QGraphicsView* currentView = this->views().constFirst();
		QPointF sceneMousePos = currentView->mapToScene(currentView->mapFromGlobal(QCursor::pos()));
		this->dummyTransition = new GraphicFsmTransition(source->getLogicComponentId(), nullId, sceneMousePos);
	}

	this->addTransition(this->dummyTransition, false);
}

void FsmScene::cancelOngoingAction()
{
	auto machineBuilder = machineManager->getMachineBuilder();
	if (machineBuilder == nullptr) return;


	switch (this->sceneMode)
	{
	case SceneMode_t::editingTransitionSource:
	case SceneMode_t::editingTransitionTarget:
	case SceneMode_t::addingTransitionSingleUse:
		machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);
		break;
	case SceneMode_t::addingTransition:
		if (this->transitionStep == AddTransitionStep_t::waitingForSource)
		{
			// Get rid of tool
			machineBuilder->setTool(MachineBuilderTool_t::none);
		}
		else
		{
			// Cancel current transition insertion but stay in adding mode
			delete this->dummyTransition;
			this->dummyTransition = nullptr;

			this->transitionStep = AddTransitionStep_t::waitingForSource;
		}
		break;
	case SceneMode_t::addingInitialState:
	case SceneMode_t::addingState:
		machineBuilder->setTool(MachineBuilderTool_t::none);
		break;
	case SceneMode_t::idle:
	case SceneMode_t::simulating:
		// Nothing to do for these modes
		break;
	}
}

GraphicFsmState* FsmScene::getStateAt(const QPointF& location) const
{
	const QList<QGraphicsItem*> itemsAtThisPoint = this->items(location, Qt::IntersectsItemShape, Qt::DescendingOrder);
	// Warning: if using transform on view, the upper line should be adapted!

	for (QGraphicsItem* item : itemsAtThisPoint)
	{
		// Select the topmost visible state
		GraphicFsmState* currentItem = dynamic_cast<GraphicFsmState*> (item);

		if ( currentItem != nullptr)
		{
			return currentItem;
		}
	}

	return nullptr;
}
