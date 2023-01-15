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

// Current class header
#include "fsmgraphicstate.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QStyleOptionGraphicsItem>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "fsmgraphictransition.h"
#include "fsmstate.h"
#include "contextmenu.h"
#include "fsm.h"


//
// Static elements
//

const qreal FsmGraphicState::radius = 50;
QBrush FsmGraphicState::inactiveBrush = QBrush(QColor(230,230,230), Qt::SolidPattern);
QBrush FsmGraphicState::activeBrush = QBrush(Qt::green, Qt::SolidPattern);
QPen FsmGraphicState::pen = QPen(Qt::SolidPattern, 3);

qreal FsmGraphicState::getRadius()
{
	return radius;
}

QPixmap FsmGraphicState::getPixmap(uint size, bool isInitial, bool addArrow)
{
	QPixmap pixmap(QSize(size, size));
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);

	painter.setPen(FsmGraphicState::pen);
	painter.setBrush(FsmGraphicState::inactiveBrush);
	painter.drawEllipse(QRectF(FsmGraphicState::pen.width()/2, FsmGraphicState::pen.width()/2, size-FsmGraphicState::pen.width(), size-FsmGraphicState::pen.width()));

	if (isInitial)
	{
		qreal space = size/10 + FsmGraphicState::pen.width();
		painter.drawEllipse(QRectF(FsmGraphicState::pen.width()/2 + space, FsmGraphicState::pen.width()/2 + space, size-FsmGraphicState::pen.width() - space*2, size-FsmGraphicState::pen.width() - space*2));
	}

	if (addArrow)
	{
		painter.drawLine(0, 0, size/3, 0);
		painter.drawLine(0, 0, 0, size/3);
	}

	return pixmap;
}

//
// Class object definition
//

FsmGraphicState::FsmGraphicState(componentId_t logicComponentId) :
    GraphicActuator(logicComponentId),
    QGraphicsEllipseItem(-radius, -radius, 2*radius, 2*radius)
{
	this->setPen(pen);

	this->setFlag(QGraphicsItem::ItemIsMovable);
	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsFocusable);
	this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
	this->setFlag(QGraphicsItem::ItemClipsToShape);

	this->rebuildRepresentation();

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	auto logicState = fsm->getState(this->logicComponentId);
	connect(logicState.get(), &MachineComponent::componentSimulatedStateChangedEvent, this, &FsmGraphicState::componentUpdatedEventHandler);

	connect(machineManager.get(), &MachineManager::simulationModeChangedEvent, this, &FsmGraphicState::machineModeChangedEventHandler);
}

void FsmGraphicState::refreshDisplay()
{
	this->rebuildRepresentation();
}

/**
 * @brief Move even is inhibited until the state
 * is actually placed on the scene. This function
 * enables it afterwhat.
 */
void FsmGraphicState::enableMoveEvent()
{
	this->moveEventEnabled = true;
}

void FsmGraphicState::moveState(Direction_t direction, bool smallMove)
{
	qreal moveSize = 10;
	if (smallMove == true)
	{
		moveSize = 1;
	}

	QPointF movePoint;
	switch (direction)
	{
	case Direction_t::left:
		movePoint = QPointF(-moveSize, 0);
		break;
	case Direction_t::right:
		movePoint = QPointF(moveSize, 0);
		break;
	case Direction_t::up:
		movePoint = QPointF(0, -moveSize);
		break;
	case Direction_t::down:
		movePoint = QPointF(0, moveSize);
		break;
	}

	this->setPos(this->pos() + movePoint);
}

QPainterPath FsmGraphicState::shape() const
{
	QPainterPath path;
	QPainterPathStroker* stroker;

	if (this->selectionShape != nullptr)
	{
		stroker = new QPainterPathStroker(selectionPen);
		path.addEllipse(QRect(-(radius+10), -(radius+10), 2*(radius+10), 2*(radius+10)));
	}
	else
	{
		stroker = new QPainterPathStroker(pen);
		path.addEllipse(QRect(-(radius), -(radius), 2*(radius), 2*(radius)));
	}

	path.setFillRule(Qt::WindingFill);
	path = path.united(stroker->createStroke(path));
	path = path.simplified();
	delete stroker;
	return path;
}

QRectF FsmGraphicState::boundingRect() const
{
	return this->shape().boundingRect();
}

void FsmGraphicState::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	// This disables the automatic selection shape.
	// Thanks to Stephen Chu on StackOverflow for the trick.
	QStyleOptionGraphicsItem myOption(*option);
	myOption.state &= ~QStyle::State_Selected;
	QGraphicsEllipseItem::paint(painter, &myOption, widget);
}

void FsmGraphicState::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Delete)
	{
		// This call may destroy the current object
		emit this->deleteStateCalledEvent(this->logicComponentId);
	}
	else if (event->key() == Qt::Key_Menu)
	{
		QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

		QGraphicsView * view = scene()->views()[0];

		QPoint posOnParent = view->mapFromScene(this->scenePos());

		QPoint posOnScreen = view->mapToGlobal(posOnParent);
		contextEvent->setScreenPos(posOnScreen);

		this->contextMenuEvent(contextEvent);
	}
	else if ( (event->key() == Qt::Key_Right) ||
	          (event->key() == Qt::Key_Left)  ||
	          (event->key() == Qt::Key_Up)    ||
	          (event->key() == Qt::Key_Down)
	        )
	{
		Direction_t direction;
		switch (event->key())
		{
		case Qt::Key_Left:
			direction = Direction_t::left;
			break;
		case Qt::Key_Right:
			direction = Direction_t::right;
			break;
		case Qt::Key_Up:
			direction = Direction_t::up;
			break;
		case Qt::Key_Down:
			direction = Direction_t::down;
			break;
		}

		bool smallMove = false;
		if ((event->modifiers() & Qt::ShiftModifier) != 0)
		{
			smallMove = true;
		}

		this->moveState(direction, smallMove);
	}
	else if (event->key() == Qt::Key_F2)
	{
		emit renameStateCalledEvent(this->logicComponentId);
	}
	else
	{
		event->ignore();
	}
}

void FsmGraphicState::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto l_logicState = fsm->getState(this->logicComponentId);
	if (l_logicState == nullptr) return;

	ContextMenu* menu = new ContextMenu();
	menu->addTitle(tr("State") + " <i>" + l_logicState->getName() + "</i>");

	auto currentMode = machineManager->getCurrentSimulationMode();

	if (currentMode == SimulationMode_t::editMode )
	{
		if (fsm->getInitialStateId() != l_logicState->getId())
		{
			menu->addAction(tr("Set initial"));
		}
		menu->addAction(tr("Edit"));
		menu->addAction(tr("Draw transition from this state"));
		menu->addAction(tr("Rename"));
		menu->addAction(tr("Delete"));
	}
	else if (currentMode == SimulationMode_t::simulateMode )
	{
		if (! l_logicState->getIsActive())
		{
			menu->addAction(tr("Set active"));
		}
	}

	if (menu->actions().count() > 1) // > 1 because title is always here
	{
		menu->popup(event->screenPos());

		connect(menu, &QMenu::triggered, this, &FsmGraphicState::treatMenu);
	}
	else
	{
		delete menu;
	}
}

QVariant FsmGraphicState::itemChange(GraphicsItemChange change, const QVariant& value)
{
	// Inform connected transitions we are moving
	if (change == GraphicsItemChange::ItemPositionHasChanged)
	{
		if (this->moveEventEnabled)
		{
			emit statePositionAboutToChangeEvent(this->logicComponentId);
		}
	}
	else if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedChange)
	{
		// If changing to selected
		if (value.toBool() == true)
		{
			// Refuse selection if there is a transition already selected
			foreach(QGraphicsItem* selectedItem, this->scene()->selectedItems())
			{
				if (dynamic_cast<FsmGraphicTransition*>(selectedItem) != nullptr)
					return (QVariant)false;
			}
		}
	}
	else if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged)
	{
		this->updateSelectionShapeDisplay();
	}

	// Reposition action box
	if (scene() != nullptr)
	{
		QGraphicsItemGroup* actionBox = this->getActionsBox();
		if (actionBox != nullptr)
			actionBox->setPos(mapToScene(radius + 20, 0));
	}

	return QGraphicsEllipseItem::itemChange(change, value);
}

void FsmGraphicState::componentUpdatedEventHandler()
{
	this->rebuildRepresentation();
}

void FsmGraphicState::treatMenu(QAction* action)
{
	if (action->text() == tr("Edit"))
	{
		emit this->editStateCalledEvent(this->logicComponentId);
	}
	else if (action->text() == tr("Set initial"))
	{
		emit this->setInitialStateCalledEvent(this->logicComponentId);
	}
	else if (action->text() == tr("Rename"))
	{
		emit this->renameStateCalledEvent(this->logicComponentId);
	}
	else if (action->text() == tr("Delete"))
	{
		// This call may destroy the current object
		emit this->deleteStateCalledEvent(this->logicComponentId);
	}
	else if (action->text() == tr("Draw transition from this state"))
	{
		emit this->beginDrawTransitionFromThisState(this->logicComponentId);
	}
	else if (action->text() == tr("Set active"))
	{
		auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
		if (fsm == nullptr) return;

		auto l_logicState = fsm->getState(this->logicComponentId);
		if (l_logicState == nullptr) return;

		fsm->forceStateActivation(l_logicState);
	}

}

void FsmGraphicState::machineModeChangedEventHandler(SimulationMode_t)
{
	this->rebuildRepresentation();
}

void FsmGraphicState::rebuildRepresentation()
{
	// Clear all child items
	qDeleteAll(this->childItems());
	this->selectionShape = nullptr;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto l_logicState = fsm->getState(this->logicComponentId);
	if (l_logicState == nullptr) return;

	if (l_logicState->getIsActive())
		this->setBrush(activeBrush);
	else
		this->setBrush(inactiveBrush);

	stateName = new QGraphicsTextItem(this);
	stateName->setHtml("<span style=\"color:black;\">" + l_logicState->getName() + "</span>");

	stateName->setPos(-stateName->boundingRect().width()/2, -stateName->boundingRect().height()/2);

	if (fsm->getInitialStateId() == l_logicState->getId())
	{
		QGraphicsEllipseItem* insideCircle = new QGraphicsEllipseItem(QRect(-(radius-10), -(radius-10), 2*(radius-10), 2*(radius-10)), this);
		insideCircle->setPen(pen);
	}

	this->buildActionsBox(pen, true);
	QGraphicsItemGroup* actionsBox = this->getActionsBox();
	if (actionsBox != nullptr)
	{
		actionsBox->setPos(mapToScene(radius + 20,0)); // Positions must be expressed wrt. scene, ast this is not a child of this (scene stacking issues)
	}

	this->updateSelectionShapeDisplay();
}

void FsmGraphicState::updateSelectionShapeDisplay()
{
	if ( (this->isSelected()) && (this->selectionShape == nullptr) )
	{
		this->selectionShape = new QGraphicsEllipseItem(QRect(-(radius+10), -(radius+10), 2*(radius+10), 2*(radius+10)), this);
		this->selectionShape->setPen(selectionPen);
	}
	else if ( (!this->isSelected()) && (this->selectionShape != nullptr) )
	{
		delete this->selectionShape;
		this->selectionShape = nullptr;
	}
}
