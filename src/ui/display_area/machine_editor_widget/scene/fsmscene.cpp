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

// Current class header
#include "fsmscene.h"

// Qt
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QMessageBox>

// StateS
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
#include "fsmstatemoveundocommand.h"
#include "actionbox.h"
#include "contextmenu.h"


FsmScene::FsmScene() :
	GenericScene()
{
	connect(this, &QGraphicsScene::selectionChanged, this, &FsmScene::handleSelection);

	connect(machineManager.get(), &MachineManager::machineUpdatedEvent,       this, &FsmScene::machineUpdatedEventHandler);
	connect(machineManager.get(), &MachineManager::interfaceModeChangedEvent, this, &FsmScene::interfaceModeChangedEventHandler);

	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
	connect(machineBuilder.get(), &MachineBuilder::changedToolEvent, this, &FsmScene::toolChangeEventHandler);

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
	if (machineManager->getCurrentInterfaceMode() != InterfaceMode_t::editMode)
	{
		// We only handle mouse events when in edit mode
		GenericScene::mousePressEvent(me);
		return;
	}

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
		if (this->sceneEditionMode == SceneEditionMode_t::addingInitialState)
		{
			// Machine is about to be edited
			machineManager->notifyMachineAboutToBeDiffEdited();

			// Create logic state & update FSM
			auto logicStateId = fsm->addState(this->getUniqueStateName());
			fsm->setInitialState(logicStateId);

			// Create graphic state
			auto graphicState = graphicFsm->addState(logicStateId, me->scenePos());

			// Add graphic state to scene
			this->addState(graphicState, true);

			// Only one initial state in a FSM, switch to regular state tool
			machineBuilder->setTool(MachineBuilderTool_t::state);

			// Machine has been edited
			machineManager->notifyMachineEdited();

			// Transmitting event so that new state is selected
			// and can be moved within the same click
		}
		else if (this->sceneEditionMode == SceneEditionMode_t::addingState)
		{
			// Machine is about to be edited
			machineManager->notifyMachineAboutToBeDiffEdited();

			// Create logic state
			auto logicStateId = fsm->addState(this->getUniqueStateName());

			// Create graphic state
			auto graphicState = graphicFsm->addState(logicStateId, me->scenePos());

			// Add graphic state to scene
			this->addState(graphicState, true);

			// Machine has been edited
			machineManager->notifyMachineEdited();

			// Transmitting event so that new state is selected
			// and can be moved within the same click
		}
		else if ( (this->sceneEditionMode == SceneEditionMode_t::addingTransition) || (this->sceneEditionMode == SceneEditionMode_t::addingTransitionSingleUse) )
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

					// Machine is about to be edited
					machineManager->notifyMachineAboutToBeDiffEdited();

					// Create logic transition
					auto logicTransitionId = fsm->addTransition(sourceStateId, targetStateId);

					// Create graphic transition
					auto graphicTransition = graphicFsm->addTransition(logicTransitionId, 0.5);

					// Add graphic transition to scene
					this->addTransition(graphicTransition, true);

					// Update mode or tool
					if (this->sceneEditionMode == SceneEditionMode_t::addingTransition)
					{
						// Get ready for next transition
						this->transitionStep = AddTransitionStep_t::waitingForSource;
					}
					else // (this->sceneEditionMode == SceneEditionMode_t::addingTransitionSingleUse)
					{
						this->updateSceneEditionMode(SceneEditionMode_t::idle);
						// Single-use tool: terminate transition adding mode
						machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);
					}

					// Machine has been edited
					machineManager->notifyMachineEdited();
				}

				this->transmitMouseEvent = false;
			}
		}
		else if ( (this->sceneEditionMode == SceneEditionMode_t::editingTransitionSource) || (this->sceneEditionMode == SceneEditionMode_t::editingTransitionTarget) )
		{
			GraphicFsmState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

			if (stateUnderMouse != nullptr)
			{
				auto logicTransition = fsm->getTransition(this->transitionUnderEditId);
				if (logicTransition != nullptr)
				{
					// Just cancel edition if state has not changed
					bool ignore = false;
					if (this->sceneEditionMode == SceneEditionMode_t::editingTransitionSource)
					{
						if (logicTransition->getSourceStateId() == stateUnderMouse->getLogicComponentId())
						{
							this->cancelOngoingEdition();
							ignore = true;
						}
					}
					else // (this->sceneEditionMode == SceneEditionMode_t::editingTransitionTarget)
					{
						if (logicTransition->getTargetStateId() == stateUnderMouse->getLogicComponentId())
						{
							this->cancelOngoingEdition();
							ignore = true;
						}
					}

					if (ignore == false)
					{
						// Get info about current graphic transition
						auto graphicTransition = graphicFsm->getTransition(this->transitionUnderEditId);
						if (graphicTransition != nullptr)
						{
							// Machine is about to be edited
							machineManager->notifyMachineAboutToBeDiffEdited();

							// Remember slider position
							auto sliderPosition = graphicTransition->getConditionLineSliderPosition();

							// Update logic transition and replace graphic transition
							// It is easier to just delete and rebuild the graphic transition
							// so that nighborhood is automatically updated
							graphicFsm->removeGraphicComponent(this->transitionUnderEditId);

							if (this->sceneEditionMode == SceneEditionMode_t::editingTransitionTarget)
							{
								fsm->redirectTransition(logicTransition->getId(), logicTransition->getSourceStateId(), stateUnderMouse->getLogicComponentId());
							}
							else
							{
								fsm->redirectTransition(logicTransition->getId(), stateUnderMouse->getLogicComponentId(), logicTransition->getTargetStateId());
							}

							graphicTransition = graphicFsm->addTransition(logicTransition->getId(), sliderPosition);
							this->addTransition(graphicTransition, true);

							// Machine has been edited
							QString undoDescription = "TRANSITION_REDIRECT__" + QString::number(logicTransition->getId());
							machineManager->notifyMachineEdited(undoDescription);
						}
					}
				}

				// Single-use tools: terminate current mode
				this->updateSceneEditionMode(SceneEditionMode_t::idle);
				machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);

				this->transmitMouseEvent = false;
			}
		}
	}
	else if (me->button() == Qt::RightButton)
	{
		switch (this->sceneEditionMode)
		{
		case SceneEditionMode_t::editingTransitionSource:
		case SceneEditionMode_t::editingTransitionTarget:
		case SceneEditionMode_t::addingTransitionSingleUse:
			// For single-use tools, get back to idle mode (which
			// cancels the ongoing edtion) and get rid of the tool
			this->updateSceneEditionMode(SceneEditionMode_t::idle);
			machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);

			this->transmitMouseEvent = false;
			break;
		case SceneEditionMode_t::addingInitialState:
		case SceneEditionMode_t::addingState:
			// For state adding, get rid of the tool (this will update scene mode indirectly)
			machineBuilder->setTool(MachineBuilderTool_t::none);

			this->transmitMouseEvent = false;
			break;
		case SceneEditionMode_t::addingTransition:
			// Depending on the step, get rid of tool or get back to initial step
			if (this->transitionStep == AddTransitionStep_t::waitingForSource)
			{
				// Get rid of tool (this will update scene mode indirectly)
				machineBuilder->setTool(MachineBuilderTool_t::none);
			}
			else
			{
				// Cancel current transition insertion but stay in adding mode
				this->cancelOngoingEdition();
				this->transitionStep = AddTransitionStep_t::waitingForSource;
			}

			this->transmitMouseEvent = false;
			break;
		case SceneEditionMode_t::idle:
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
			break;
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

	if ( (this->transitionStep == AddTransitionStep_t::settingSource)            ||
	     (this->transitionStep == AddTransitionStep_t::waitingForTarget)         ||
	     (this->sceneEditionMode == SceneEditionMode_t::editingTransitionSource) ||
	     (this->sceneEditionMode == SceneEditionMode_t::editingTransitionTarget)
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
	if (this->transmitMouseEvent == false)
	{
		this->transmitMouseEvent = true;
	}
	else
	{
		GenericScene::mouseReleaseEvent(me);
	}
}

void FsmScene::keyPressEvent(QKeyEvent* ke)
{
	if (ke->key() == Qt::Key_Escape)
	{
		shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
		if (machineBuilder == nullptr) return;


		switch (this->sceneEditionMode)
		{
		case SceneEditionMode_t::editingTransitionSource:
		case SceneEditionMode_t::editingTransitionTarget:
		case SceneEditionMode_t::addingTransitionSingleUse:
			// For single-use tools, get back to idle mode (which
			// cancels the ongoing edtion) and get rid of the tool
			this->updateSceneEditionMode(SceneEditionMode_t::idle);
			machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::none);
			break;
		case SceneEditionMode_t::addingInitialState:
		case SceneEditionMode_t::addingState:
			// For state adding, get rid of the tool (this will update scene mode indirectly)
			machineBuilder->setTool(MachineBuilderTool_t::none);
			break;
		case SceneEditionMode_t::addingTransition:
			// Depending on the step, get rid of tool or get back to initial step
			if (this->transitionStep == AddTransitionStep_t::waitingForSource)
			{
				// Get rid of tool (this will update scene mode indirectly)
				machineBuilder->setTool(MachineBuilderTool_t::none);
			}
			else
			{
				// Cancel current transition insertion but stay in adding mode
				this->cancelOngoingEdition();
				this->transitionStep = AddTransitionStep_t::waitingForSource;
			}
			break;
		case SceneEditionMode_t::idle:
			this->clearSelection();
			break;
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
	else if (ke->key() == Qt::Key_Delete)
	{
		// Only allow deleting scene components when in idle mode
		if (this->sceneEditionMode == SceneEditionMode_t::idle)
		{
			auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
			if (fsm == nullptr) return;


			const auto selectedItems = this->selectedItems();
			if (selectedItems.count() > 0)
			{
				QList<GraphicFsmTransition*> selectedTransitions;
				QList<GraphicFsmState*>      selectedStates;

				bool atLeastOneItemToDelete = false;
				for (QGraphicsItem* item : selectedItems)
				{
					auto transition = dynamic_cast<GraphicFsmTransition*>(item);
					auto state      = dynamic_cast<GraphicFsmState*>     (item);
					if (transition != nullptr)
					{
						selectedTransitions.append(transition);
						atLeastOneItemToDelete = true;
					}
					else if (state != nullptr)
					{
						selectedStates.append(state);
						atLeastOneItemToDelete = true;
					}
				}

				// Machine is about to be edited
				machineManager->notifyMachineAboutToBeDiffEdited();

				// Delete selected items
				for (auto& transition : selectedTransitions)
				{
					fsm->removeTransition(transition->getLogicComponentId());
				}
				for (auto& state : selectedStates)
				{
					fsm->removeState(state->getLogicComponentId());
				}

				if (atLeastOneItemToDelete == true)
				{
					// Machine has been edited
					machineManager->notifyMachineEdited();
				}
			}
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
	switch (machineManager->getCurrentInterfaceMode())
	{
	case InterfaceMode_t::editMode:
		if (this->sceneEditionMode == SceneEditionMode_t::idle)
		{
			// In edit mode, context menu is only allowed
			// when there is no ongoing action
			GenericScene::contextMenuEvent(ce);
		}
		else
		{
			ce->ignore();
		}
		break;
	case InterfaceMode_t::simulateMode:
		GenericScene::contextMenuEvent(ce);
		break;
	case InterfaceMode_t::verifyMode:
		// No context menu allowed in this mode
		ce->ignore();
		break;
	}
}

void FsmScene::interfaceModeChangedEventHandler(InterfaceMode_t newMode)
{
	static bool sceneIsInSimulateMode = false;

	// Go back to idle mode before changing interface mode
	if (this->sceneEditionMode != SceneEditionMode_t::idle)
	{
		this->updateSceneEditionMode(SceneEditionMode_t::idle);
	}

	// Get rid of any tool on mode change
	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();
	if (machineBuilder != nullptr)
	{
		machineBuilder->resetTool();
	}

	switch (newMode)
	{
	case InterfaceMode_t::editMode:
		if (sceneIsInSimulateMode == true)
		{
			emit this->requestSaveViewEvent();
			this->clearScene();
			this->displayGraphicMachine();
			emit this->requestRestoreViewEvent();

			sceneIsInSimulateMode = false;
		}
		break;
	case InterfaceMode_t::simulateMode:
		emit this->requestSaveViewEvent();
		this->clearScene();
		this->displaySimulatedMachine();
		emit this->requestRestoreViewEvent();

		sceneIsInSimulateMode = true;
		break;
	case InterfaceMode_t::verifyMode:
		// Nothing to do
		break;
	}
}

void FsmScene::machineUpdatedEventHandler()
{
	this->updateSceneEditionMode(SceneEditionMode_t::idle);
	this->displayGraphicMachine();
}

void FsmScene::toolChangeEventHandler(MachineBuilderTool_t newTool)
{
	switch (newTool)
	{
	case MachineBuilderTool_t::initialState:
		this->updateSceneEditionMode(SceneEditionMode_t::addingInitialState);
		break;
	case MachineBuilderTool_t::state:
		this->updateSceneEditionMode(SceneEditionMode_t::addingState);
		break;
	case MachineBuilderTool_t::transition:
		this->updateSceneEditionMode(SceneEditionMode_t::addingTransition);
		break;
	case MachineBuilderTool_t::none:
		this->updateSceneEditionMode(SceneEditionMode_t::idle);
		break;
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
	emit this->editSelectedItemEvent();
}

void FsmScene::stateCallsRenameEventHandler(componentId_t stateId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto graphicState = graphicFsm->getState(stateId);
	if (graphicState == nullptr) return;


	this->clearSelection();
	graphicState->setSelected(true);
	emit this->renameSelectedItemEvent();
}

void FsmScene::stateCallsDeleteEventHandler(componentId_t stateId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Remove state
	fsm->removeState(stateId);

	// Machine has been edited
	machineManager->notifyMachineEdited();
}

void FsmScene::stateCallsSetInitialStateEventHandler(componentId_t stateId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Set state as initial
	fsm->setInitialState(stateId);

	// Machine has been edited
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


	this->updateSceneEditionMode(SceneEditionMode_t::addingTransitionSingleUse);

	this->beginDrawTransition(graphicState);

	machineBuilder->setSingleUseTool(MachineBuilderSingleUseTool_t::drawTransitionFromScene);
}

void FsmScene::statePositionAboutToChangeEventHandler(componentId_t stateId)
{
	// Machine has been edited
	QString undoDescription = "STATE_MOVE_" + this->selectionDescription;
	auto undoCommand = new FsmStateMoveUndoCommand(undoDescription, stateId);
	machineManager->notifyMachineEdited(undoCommand);
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


	this->updateSceneEditionMode(SceneEditionMode_t::editingTransitionSource);

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


	this->updateSceneEditionMode(SceneEditionMode_t::editingTransitionTarget);

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
	emit this->editSelectedItemEvent();
}

void FsmScene::transitionCallsDeleteEventHandler(componentId_t transitionId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Remove transition
	fsm->removeTransition(transitionId);

	// Machine has been edited
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
			emit this->itemSelectedEvent(currentState->getLogicComponentId());
		}
		else
		{
			GraphicFsmTransition* currentTransition = dynamic_cast< GraphicFsmTransition* >(this->selectedItems().at(0));
			if (currentTransition!= nullptr)
			{
				emit this->itemSelectedEvent(currentTransition->getLogicComponentId());
			}
		}
	}
	else
	{
		emit this->itemSelectedEvent(nullId);
	}

	// Build a string representing state selection
	this->selectionDescription.clear();

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;


	for (const auto& item : this->selectedItems())
	{
		auto graphicState = dynamic_cast<const GraphicFsmState*>(item);
		if (graphicState == nullptr) continue;

		auto logicState = fsm->getState(graphicState->getLogicComponentId());
		if (logicState == nullptr) continue;


		this->selectionDescription += "_" + logicState->getName();
	}
}

void FsmScene::menuAddStateTriggeredEventHandler(QAction* action)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	auto logicStateId = nullId;
	if (action->text() == tr("Add state"))
	{
		// Create logic state
		logicStateId = fsm->addState(this->getUniqueStateName());
	}
	else if (action->text() == tr("Add initial state"))
	{
		// Create logic state & update FSM
		logicStateId = fsm->addState(this->getUniqueStateName());
		fsm->setInitialState(logicStateId);
	}

	if (logicStateId != nullId)
	{
		// Create graphic state
		GraphicFsmState* graphicState = graphicFsm->addState(logicStateId, this->menuMousePos);

		// Update scene
		this->addState(graphicState, true);

		// Machine has been edited
		machineManager->notifyMachineEdited();
	}
}

void FsmScene::updateSceneEditionMode(FsmScene::SceneEditionMode_t newMode)
{
	// Cancel ongoing editions before changing mode
	this->cancelOngoingEdition();

	// Update edit mode
	this->sceneEditionMode = newMode;

	// Set transition mode step
	if (newMode == SceneEditionMode_t::addingTransition)
	{
		this->transitionStep = AddTransitionStep_t::waitingForSource;
	}
	else if (newMode == SceneEditionMode_t::addingTransitionSingleUse)
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
			if (machineManager->getCurrentInterfaceMode() == InterfaceMode_t::editMode)
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
			if (machineManager->getCurrentInterfaceMode() == InterfaceMode_t::editMode)
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

void FsmScene::cancelOngoingEdition()
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;


	// Get rid of dummy transition if there is one
	delete this->dummyTransition;
	this->dummyTransition = nullptr;

	// If there is a transition being edited, cancel highlighting
	if (this->transitionUnderEditId != nullId)
	{
		auto transition = graphicFsm->getTransition(this->transitionUnderEditId);
		if (transition != nullptr)
		{
			transition->setUnderEdit(false);
		}
		this->transitionUnderEditId = nullId;
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

QString FsmScene::getUniqueStateName()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return QString();


	QString baseName = tr("State");
	QString currentName = baseName + " #0";
	uint i = 0;

	bool nameIsValid = false;
	while (nameIsValid == false)
	{
		nameIsValid = true;
		for (auto stateId : fsm->getAllStatesIds())
		{
			auto state = fsm->getState(stateId);
			if (state->getName() == currentName)
			{
				i++;
				currentName = baseName + " #" + QString::number(i);
				nameIsValid = false;
				break;
			}
		}
	}

	return currentName;
}
