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

// Current class header
#include "fsmscene.h"

// Qt classes
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>

// Debug
#include <QDebug>

// StateS classes
#include "machinemanager.h"
#include "fsmgraphicstate.h"
#include "fsmgraphictransition.h"
#include "fsm.h"
#include "fsmstate.h"
#include "fsmtransition.h"
#include "contextmenu.h"


FsmScene::FsmScene(shared_ptr<MachineManager> machineManager) :
    GenericScene()
{
	this->machineManager = machineManager;

	this->sceneMode = sceneMode_t::idle;

	shared_ptr<Fsm> fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	shared_ptr<MachineBuilder> machineBuilder = machineManager->getMachineBuilder();

	connect(this,                 &QGraphicsScene::selectionChanged, this, &FsmScene::handleSelection);
	connect(fsm.get(),            &Fsm::simulationModeChangedEvent,  this, &FsmScene::simulationModeChangeEventHandler);
	connect(machineBuilder.get(), &MachineBuilder::changedToolEvent, this, &FsmScene::toolChangeEventHandler);

	this->build();
}

void FsmScene::build()
{
	shared_ptr<Fsm> fsm = this->getFsm();
	for (shared_ptr<FsmState> state : fsm->getStates())
	{
		this->addState(state->getGraphicRepresentation());
	}

	for (shared_ptr<FsmTransition> transition : fsm->getTransitions())
	{
		this->addTransition(transition->getGraphicRepresentation());
	}
}

FsmScene::~FsmScene()
{
	// We have to delete transitions first, so handle
	// item deletion manually

	QList<FsmGraphicTransition*> transitions;
	QList<FsmGraphicState*> states;

	foreach(QGraphicsItem* item, items())
	{
		FsmGraphicTransition* currentTransition = dynamic_cast<FsmGraphicTransition*>(item);

		if (currentTransition != nullptr)
			transitions.append(currentTransition);
	}

	foreach(QGraphicsItem* item, items())
	{
		FsmGraphicState* currentState = dynamic_cast<FsmGraphicState*>(item);

		if (currentState!= nullptr)
			states.append(currentState);
	}

	qDeleteAll(transitions);
	qDeleteAll(states);
}

void FsmScene::beginDrawTransition(FsmGraphicState* source, const QPointF& currentMousePos)
{
	shared_ptr<Machine> l_machine = this->getFsm();
	if (l_machine != nullptr)
	{
		if (!currentMousePos.isNull())
		{
			this->currentTransition = new FsmGraphicTransition(source, currentMousePos);
		}
		else
		{
			// Compute mouse pos wrt. scene
			QGraphicsView* currentView = this->views().constFirst();
			QPointF sceneMousePos = currentView->mapToScene(currentView->mapFromGlobal(QCursor::pos()));
			this->currentTransition = new FsmGraphicTransition(source, sceneMousePos);
		}

		this->addTransition(this->currentTransition);
	}
}

void FsmScene::toolChangeEventHandler(MachineBuilder::tool newTool)
{
	if (newTool == MachineBuilder::tool::initial_state)
	{
		this->upateSceneMode(sceneMode_t::addingInitialState);
	}
	else if (newTool == MachineBuilder::tool::state)
	{
		this->upateSceneMode(sceneMode_t::addingState);
	}
	else if (newTool == MachineBuilder::tool::transition)
	{
		this->upateSceneMode(sceneMode_t::addingTransition);
	}
	else
	{
		this->upateSceneMode(sceneMode_t::idle);
	}
}

void FsmScene::mousePressEvent(QGraphicsSceneMouseEvent* me)
{
	bool transmitEvent = true;
	shared_ptr<Fsm> l_machine = this->getFsm();

	if (l_machine != nullptr)
	{
		shared_ptr<MachineBuilder> machineBuilder = this->machineManager->getMachineBuilder();
		if (machineBuilder != nullptr)
		{
			if (me->button() == Qt::LeftButton)
			{
				if (this->sceneMode == sceneMode_t::addingInitialState)
				{
					// Create logic state
					shared_ptr<FsmState> logicState = l_machine->addState(me->scenePos(), true);

					// Add graphic state
					this->addState(logicState->getGraphicRepresentation());

					// Only one initial state in a FSM, switch to regular state tool
					machineBuilder->setTool(MachineBuilder::tool::state);

					// Transmitting event so that new state is selected
					// and can be moved within the same click
				}
				else if (this->sceneMode == sceneMode_t::addingState)
				{
					// Create logic state
					shared_ptr<FsmState> logicState = l_machine->addState(me->scenePos());

					// Add graphic state
					this->addState(logicState->getGraphicRepresentation());

					// Transmitting event so that new state is selected
					// and can be moved within the same click
				}
				else if ( (this->sceneMode == sceneMode_t::addingTransition) || (this->sceneMode == sceneMode_t::addingTransitionSingleUse) )
				{
					FsmGraphicState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

					if (stateUnderMouse != nullptr)
					{
						if (this->transitionStep == addTransitionStep_t::waitingForSource)
						{
							this->beginDrawTransition(stateUnderMouse, me->scenePos());
							this->transitionStep = addTransitionStep_t::settingSource;
							transmitEvent = false;
						}
						else if (this->transitionStep == addTransitionStep_t::waitingForTarget)
						{
							this->currentTransition->setTargetState(stateUnderMouse);
							this->currentTransition->endDynamicMode(true);

							shared_ptr<FsmState> source = this->currentTransition->getSource()->getLogicState();
							shared_ptr<FsmState> target = this->currentTransition->getTarget()->getLogicState();

							shared_ptr<FsmTransition> logicTransition = l_machine->addTransition(source, target, this->currentTransition);

							this->currentTransition = nullptr;
							machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::none);

							this->transitionStep = addTransitionStep_t::settingTarget;
							transmitEvent = false;
						}
					}
				}
				else if ( (this->sceneMode == sceneMode_t::editingTransitionSource) || (this->sceneMode == sceneMode_t::editingTransitionTarget) )
				{
					FsmGraphicState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

					if (stateUnderMouse != nullptr)
					{
						this->currentTransition->endDynamicMode(true);

						machineBuilder->setTool(MachineBuilder::tool::none);
						this->upateSceneMode(sceneMode_t::goingBackToIdle);
						this->currentTransition = nullptr;

						transmitEvent = false;
					}
				}
			}
			else if (me->button() == Qt::RightButton)
			{
				if ( (this->sceneMode == sceneMode_t::editingTransitionSource) || (this->sceneMode == sceneMode_t::editingTransitionTarget) )
				{
					// Cancel transition edition
					machineBuilder->setTool(MachineBuilder::tool::none);
					this->upateSceneMode(sceneMode_t::goingBackToIdle);
					this->currentTransition->endDynamicMode(false);
					this->currentTransition = nullptr;

					transmitEvent = false;
				}
				else if (this->sceneMode == sceneMode_t::addingTransition)
				{
					if (this->transitionStep == addTransitionStep_t::waitingForSource)
					{
						// Ge rid of tool
						machineBuilder->setTool(MachineBuilder::tool::none);
						this->upateSceneMode(sceneMode_t::goingBackToIdle);
						delete this->currentTransition;
						this->currentTransition = nullptr;
					}
					else
					{
						// Cancel transition insertion
						this->cancelDrawTransition();
					}
					transmitEvent = false;
				}
				else if  (this->sceneMode == sceneMode_t::addingTransitionSingleUse)
				{
					this->cancelDrawTransition();
					this->upateSceneMode(sceneMode_t::goingBackToIdle);
					transmitEvent = false;
				}
				else if (machineBuilder->getTool() != MachineBuilder::tool::none)
				{
					machineBuilder->setTool(MachineBuilder::tool::none);
					this->upateSceneMode(sceneMode_t::goingBackToIdle);

					transmitEvent = false;
				}
				// Else display a context menu (except if there is an item under mouse:
				// this is a context menu event on that particular item, in this case just transmit event)
				else if (this->items(me->scenePos(), Qt::IntersectsItemShape, Qt::DescendingOrder).count() == 0)
				{
					// Display context menu
					if (this->sceneMode != sceneMode_t::simulating) // Adding states is not allowed in simulation mode
					{
						ContextMenu* menu = new ContextMenu();
						menu->addAction(tr("Add state"));
						menu->addAction(tr("Add initial state"));
						menu->popup(me->screenPos());

						this->mousePos = me->scenePos();

						connect(menu, &QMenu::triggered, this, &FsmScene::treatMenu);

						transmitEvent = false;
					}
				}
			}
		}

		if (transmitEvent)
		{
			GenericScene::mousePressEvent(me);
		}
	}
}

void FsmScene::mouseMoveEvent(QGraphicsSceneMouseEvent* me)
{
	bool transmitEvent = true;
	static FsmGraphicState* previousStatePointed = nullptr;

	if ( (this->transitionStep == addTransitionStep_t::settingSource) ||
	     (this->transitionStep == addTransitionStep_t::waitingForTarget) ||
	     (this->sceneMode == sceneMode_t::editingTransitionSource) ||
	     (this->sceneMode == sceneMode_t::editingTransitionTarget)
	   )
	{
		FsmGraphicState* stateUnderMouse = getStateAt(QPointF(me->scenePos()));

		if (stateUnderMouse != nullptr)
		{
			if ( (this->transitionStep == addTransitionStep_t::settingSource) ||
			     (this->transitionStep == addTransitionStep_t::waitingForTarget) ||
			     (this->sceneMode == sceneMode_t::editingTransitionTarget)
			   )
			{
				if (stateUnderMouse != previousStatePointed)
				{
					this->currentTransition->setTargetState(stateUnderMouse);
					previousStatePointed = stateUnderMouse;
				}
			}
			else
			{

				if (stateUnderMouse != previousStatePointed)
				{
					this->currentTransition->setSourceState(stateUnderMouse);
					previousStatePointed = stateUnderMouse;
				}
			}
		}
		else
		{
			this->currentTransition->setMousePosition(me->scenePos());
			previousStatePointed = nullptr;
		}

		transmitEvent = false;
	}
	else if (this->sceneMode == sceneMode_t::goingBackToIdle)
	{
		transmitEvent = false;
	}

	if (transmitEvent)
	{
		GenericScene::mouseMoveEvent(me);
	}
}

void FsmScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* me)
{
	bool transmitEvent = true;

	shared_ptr<MachineBuilder> machineBuilder = this->machineManager->getMachineBuilder();
	if (machineBuilder != nullptr)
	{
		if ( (this->sceneMode == sceneMode_t::addingTransition) || (this->sceneMode == sceneMode_t::addingTransitionSingleUse) )
		{
			if (this->transitionStep == addTransitionStep_t::settingSource)
			{
				this->transitionStep = addTransitionStep_t::waitingForTarget;
				transmitEvent = false;
			}
			else if (this->transitionStep == addTransitionStep_t::settingTarget)
			{
				this->transitionStep = addTransitionStep_t::waitingForSource;
				if (this->sceneMode == sceneMode_t::addingTransitionSingleUse)
				{
					this->upateSceneMode(sceneMode_t::idle);
				}
				transmitEvent = false;
			}
		}
		else if (this->sceneMode == sceneMode_t::goingBackToIdle)
		{
			this->upateSceneMode(sceneMode_t::idle);
			transmitEvent = false;
		}

		if (transmitEvent)
			GenericScene::mouseReleaseEvent(me);

		// TODO: better place for this call?
		updateSceneRect();
	}
}

void FsmScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* me)
{
	// TODO : update
	bool transmitEvent = true;

	if (this->sceneMode == sceneMode_t::addingTransition)
	{
		transmitEvent = false;
	}
	else if (this->sceneMode == sceneMode_t::goingBackToIdle)
	{
		transmitEvent = false;
	}

	if (transmitEvent)
	{
		GenericScene::mouseReleaseEvent(me);
	}
}

void FsmScene::keyPressEvent(QKeyEvent* ke)
{
	if (ke->key() == Qt::Key_Escape)
	{
		shared_ptr<MachineBuilder> machineBuilder = this->machineManager->getMachineBuilder();

		if  (machineBuilder != nullptr)
		{
			if (this->sceneMode == sceneMode_t::addingTransition)
			{
				this->upateSceneMode(sceneMode_t::idle);
			}
			if (this->sceneMode == sceneMode_t::addingTransitionSingleUse)
			{
				this->upateSceneMode(sceneMode_t::idle);
			}
			else if ( (this->sceneMode == sceneMode_t::editingTransitionSource) || (this->sceneMode == sceneMode_t::editingTransitionTarget) )
			{
				// Cancel transition edition
				this->upateSceneMode(sceneMode_t::idle);
				this->currentTransition->endDynamicMode(false);
				this->currentTransition = nullptr;
			}
			else
			{
				this->clearSelection();
			}
		}
	}
	else if ( (ke->key() == Qt::Key_Right) ||
	          (ke->key() == Qt::Key_Left)  ||
	          (ke->key() == Qt::Key_Up)    ||
	          (ke->key() == Qt::Key_Down)
	          )
	{
		if (this->selectedItems().count() == 0)
		{
			// Ignore event => transmitted to view widget
			// which will handle it as view move
			ke->ignore();
		}
		else
		{
			bool atLeastOneState = false;
			// Transmit event to each state in the list
			foreach (QGraphicsItem* item, this->selectedItems())
			{
				FsmGraphicState* state = dynamic_cast<FsmGraphicState*>(item);

				if (state != nullptr)
				{
					atLeastOneState = true;
					state->keyPressEvent(ke);
				}
			}

			if (!atLeastOneState)
			{
				// Same as if there were no selected items at all
				ke->ignore();
			}
		}
	}
	if (ke->key() == Qt::Key_Delete)
	{
		// Allow deleting scene components only in idle state
		if (this->sceneMode == sceneMode_t::idle)
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
	// Do not show menus while a transition in a dynamic mode
	if ( (this->sceneMode == sceneMode_t::idle) || (this->sceneMode == sceneMode_t::simulating) )
	{
		GenericScene::contextMenuEvent(ce);
	}
}

void FsmScene::transitionCallsDynamicSourceEventHandler(FsmGraphicTransition* transition)
{
	shared_ptr<MachineBuilder> machineBuilder = this->machineManager->getMachineBuilder();
	if (machineBuilder != nullptr)
	{
		this->upateSceneMode(sceneMode_t::editingTransitionSource);
		machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::editTransitionSource);
		this->currentTransition = transition;

		clearSelection();
		transition->setSelected(true);

		this->currentTransition->setDynamicSourceMode(QPointF());
	}
}

void FsmScene::transitionCallsDynamicTargetEventHandler(FsmGraphicTransition* transition)
{
	shared_ptr<MachineBuilder> machineBuilder = this->machineManager->getMachineBuilder();
	if (machineBuilder != nullptr)
	{
		this->upateSceneMode(sceneMode_t::editingTransitionTarget);
		machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::editTransitionTarget);
		this->currentTransition = transition;

		clearSelection();
		transition->setSelected(true);

		this->currentTransition->setDynamicTargetMode(QPointF());
	}
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
		FsmGraphicState* currentState = dynamic_cast< FsmGraphicState* >(this->selectedItems().at(0));
		if (currentState != nullptr)
		{
			emit itemSelectedEvent(currentState->getLogicState());
		}
		else
		{
			FsmGraphicTransition* currentTransition = dynamic_cast< FsmGraphicTransition* >(this->selectedItems().at(0));
			if (currentTransition!= nullptr)
			{
				emit itemSelectedEvent(currentTransition->getLogicTransition());
			}
		}
	}
	else
	{
		emit itemSelectedEvent(nullptr);
	}
}

FsmGraphicState* FsmScene::getStateAt(const QPointF& location) const
{
	QList<QGraphicsItem*> itemsAtThisPoint = this->items(location, Qt::IntersectsItemShape, Qt::DescendingOrder);
	// Warning: if using transform on view, the upper line should be adapted!

	foreach (QGraphicsItem* item, itemsAtThisPoint)
	{
		// Select the topmost visible state
		FsmGraphicState* currentItem = dynamic_cast<FsmGraphicState*> (item);

		if ( currentItem != nullptr)
		{
			return currentItem;
		}
	}

	return nullptr;
}

void FsmScene::simulationModeChangeEventHandler(Machine::simulation_mode newMode)
{
	shared_ptr<MachineBuilder> machineBuilder = this->machineManager->getMachineBuilder();
	if (machineBuilder != nullptr)
	{
		if (newMode == Machine::simulation_mode::editMode)
		{
			this->handleSelection();
			this->upateSceneMode(sceneMode_t::idle);
			machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::none);
		}
		else if (newMode == Machine::simulation_mode::simulateMode)
		{
			this->upateSceneMode(sceneMode_t::simulating);
			machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::none);
		}
	}
}

void FsmScene::upateSceneMode(FsmScene::sceneMode_t newMode)
{
	if ( ( (this->sceneMode == sceneMode_t::addingTransition) || (this->sceneMode == sceneMode_t::addingTransitionSingleUse) )
	     && (newMode != sceneMode_t::addingTransition)
	   )
	{
		this->cancelDrawTransition();
		this->transitionStep = addTransitionStep_t::notInTransitionAddingMode;
	}

	this->sceneMode = newMode;

	switch(this->sceneMode)
	{
	case sceneMode_t::addingInitialState:
	case sceneMode_t::addingState:
		emit this->updateCursorEvent(SceneWidget::mouseCursor_t::state);
		break;
	case sceneMode_t::addingTransition:
		this->transitionStep = addTransitionStep_t::waitingForSource;
	case sceneMode_t::addingTransitionSingleUse:
	case sceneMode_t::editingTransitionSource:
	case sceneMode_t::editingTransitionTarget:
		emit this->updateCursorEvent(SceneWidget::mouseCursor_t::transition);
		break;
	case sceneMode_t::goingBackToIdle:
	case sceneMode_t::idle:
	case sceneMode_t::simulating:
		emit this->updateCursorEvent(SceneWidget::mouseCursor_t::none);
		break;
	}
}

void FsmScene::stateCallsEditEventHandler(shared_ptr<FsmState> state)
{
	this->clearSelection();
	state->getGraphicRepresentation()->setSelected(true);
	emit editSelectedItemEvent();
}

void FsmScene::stateCallsRenameEventHandler(shared_ptr<FsmState> state)
{
	this->clearSelection();
	state->getGraphicRepresentation()->setSelected(true);
	emit renameSelectedItemEvent();
}

void FsmScene::treatMenu(QAction* action)
{
	shared_ptr<Fsm> l_machine = this->getFsm();

	if (l_machine != nullptr)
	{
		this->upateSceneMode(sceneMode_t::idle); // Useless ?
		if (action->text() == tr("Add state"))
		{
			shared_ptr<FsmState> logicState = l_machine->addState(this->mousePos);

			this->addState(logicState->getGraphicRepresentation());
		}
		else if (action->text() == tr("Add initial state"))
		{
			shared_ptr<FsmState> logicState = l_machine->addState(this->mousePos, true);

			this->addState(logicState->getGraphicRepresentation());
		}
	}
}

void FsmScene::stateCallsBeginTransitionEventHandler(FsmGraphicState* sourceState)
{
	shared_ptr<MachineBuilder> machineBuilder = this->machineManager->getMachineBuilder();
	if (machineBuilder != nullptr)
	{
		machineBuilder->setSingleUseTool(MachineBuilder::singleUseTool::drawTransitionFromScene);
		this->beginDrawTransition(sourceState);
		// Go straight to target edition as we already know the source
		this->upateSceneMode(sceneMode_t::addingTransitionSingleUse);
		this->transitionStep = addTransitionStep_t::waitingForTarget;
	}
}

void FsmScene::transitionCallsEditEventHandler(shared_ptr<FsmTransition> transition)
{
	this->clearSelection();
	transition->getGraphicRepresentation()->setSelected(true);
	emit editSelectedItemEvent();
}

FsmGraphicState* FsmScene::addState(FsmGraphicState* newState)
{
	this->addItem(newState);
	newState->setZValue(1);

	QGraphicsItemGroup* actionsBox = newState->getActionsBox();
	this->addItem(actionsBox);
	actionsBox->setZValue(3);

	connect(newState, &FsmGraphicState::editStateCalledEvent,             this, &FsmScene::stateCallsEditEventHandler);
	connect(newState, &FsmGraphicState::renameStateCalledEvent,           this, &FsmScene::stateCallsRenameEventHandler);
	connect(newState, &FsmGraphicState::beginDrawTransitionFromThisState, this, &FsmScene::stateCallsBeginTransitionEventHandler);

	newState->enableMoveEvent();

	return newState;
}

void FsmScene::addTransition(FsmGraphicTransition* newTransition)
{
	connect(newTransition, &FsmGraphicTransition::dynamicSourceCalledEvent, this, &FsmScene::transitionCallsDynamicSourceEventHandler);
	connect(newTransition, &FsmGraphicTransition::dynamicTargetCalledEvent, this, &FsmScene::transitionCallsDynamicTargetEventHandler);
	connect(newTransition, &FsmGraphicTransition::editCalledEvent,          this, &FsmScene::transitionCallsEditEventHandler);

	this->addItem(newTransition);
	newTransition->setZValue(2);

	QGraphicsTextItem* condition = newTransition->getConditionText();
	this->addItem(condition);
	condition->setZValue(3);

	QGraphicsItemGroup* actionsBox = newTransition->getActionsBox();
	this->addItem(actionsBox);
	actionsBox->setZValue(3);
}

void FsmScene::cancelDrawTransition()
{
	if (this->currentTransition != nullptr)
	{
		shared_ptr<Fsm> l_machine = this->getFsm();

		if (l_machine != nullptr)
		{
			if (this->sceneMode == sceneMode_t::addingTransition)
			{
				delete this->currentTransition;
				this->currentTransition = nullptr;

				this->transitionStep = addTransitionStep_t::waitingForSource;
			}
			else if (this->sceneMode == sceneMode_t::addingTransitionSingleUse)
			{
				delete this->currentTransition;
				this->currentTransition = nullptr;
			}
			else if ( (this->sceneMode == sceneMode_t::editingTransitionSource) || (this->sceneMode == sceneMode_t::editingTransitionTarget) )
			{
				this->currentTransition->endDynamicMode(false);
				this->currentTransition = nullptr;

				this->upateSceneMode(sceneMode_t::idle);
			}
		}
	}
}

shared_ptr<Fsm> FsmScene::getFsm() const
{
	return dynamic_pointer_cast<Fsm>(this->machineManager->getMachine());
}
