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

const qreal  FsmGraphicState::radius       = 50;
const QBrush FsmGraphicState::defaultBrush = QBrush(QColor(230,230,230), Qt::SolidPattern);
const QPen   FsmGraphicState::defaultPen   = QPen(Qt::SolidPattern, 3);

qreal FsmGraphicState::getRadius()
{
	return FsmGraphicState::radius;
}

QPixmap FsmGraphicState::getPixmap(uint size, bool isInitial, bool addArrow)
{
	QPixmap pixmap(QSize(size, size));
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);

	painter.setPen(FsmGraphicState::defaultPen);
	painter.setBrush(FsmGraphicState::defaultBrush);
	painter.drawEllipse(QRectF(FsmGraphicState::defaultPen.width()/2, FsmGraphicState::defaultPen.width()/2, size-FsmGraphicState::defaultPen.width(), size-FsmGraphicState::defaultPen.width()));

	if (isInitial)
	{
		qreal space = size/10 + FsmGraphicState::defaultPen.width();
		painter.drawEllipse(QRectF(FsmGraphicState::defaultPen.width()/2 + space, FsmGraphicState::defaultPen.width()/2 + space, size-FsmGraphicState::defaultPen.width() - space*2, size-FsmGraphicState::defaultPen.width() - space*2));
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
	this->setPen(defaultPen);
	this->setBrush(defaultBrush);

	this->setFlag(QGraphicsItem::ItemIsMovable);
	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsFocusable);
	this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
	this->setFlag(QGraphicsItem::ItemClipsToShape);

	this->buildRepresentation();
}

void FsmGraphicState::refreshDisplay()
{
	// Clear
	this->clearRepresentation();

	// Rebuild
	this->buildRepresentation();

	// Update action box
	GraphicActuator::refreshDisplay();
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

	stroker = new QPainterPathStroker(defaultPen);
	path.addEllipse(QRect(-(radius), -(radius), 2*(radius), 2*(radius)));

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

void FsmGraphicState::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete)
	{
		// This call may destroy the current object
		emit this->deleteStateCalledEvent(this->logicComponentId);

		event->accept();
	}
	else if (event->key() == Qt::Key_Menu)
	{
		QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

		QGraphicsView * view = scene()->views()[0];

		QPoint posOnParent = view->mapFromScene(this->scenePos());

		QPoint posOnScreen = view->mapToGlobal(posOnParent);
		contextEvent->setScreenPos(posOnScreen);

		this->contextMenuEvent(contextEvent);

		event->accept();
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

		event->accept();
	}
	else if (event->key() == Qt::Key_F2)
	{
		emit renameStateCalledEvent(this->logicComponentId);

		event->accept();
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

	auto logicState = fsm->getState(this->logicComponentId);
	if (logicState == nullptr) return;

	ContextMenu* menu = new ContextMenu();
	menu->addTitle(tr("State") + " <i>" + logicState->getName() + "</i>");

	if (fsm->getInitialStateId() != logicState->getId())
	{
		menu->addAction(tr("Set initial"));
	}
	menu->addAction(tr("Edit"));
	menu->addAction(tr("Draw transition from this state"));
	menu->addAction(tr("Rename"));
	menu->addAction(tr("Delete"));

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
	if (change == GraphicsItemChange::ItemPositionHasChanged)
	{
		emit statePositionAboutToChangeEvent(this->logicComponentId);

		this->updateActionBoxPosition();
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

	return QGraphicsEllipseItem::itemChange(change, value);
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
}

void FsmGraphicState::clearRepresentation()
{
	qDeleteAll(this->childItems());

	this->selectionShape = nullptr;
	this->actionBoxLine  = nullptr;
	this->stateName      = nullptr;
}

void FsmGraphicState::buildRepresentation()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->logicComponentId);
	if (logicState == nullptr) return;

	this->stateName = new QGraphicsTextItem(this);
	this->stateName->setHtml("<span style=\"color:black;\">" + logicState->getName() + "</span>");

	this->stateName->setPos(-this->stateName->boundingRect().width()/2, -this->stateName->boundingRect().height()/2);

	if (fsm->getInitialStateId() == logicState->getId())
	{
		QGraphicsEllipseItem* insideCircle = new QGraphicsEllipseItem(QRect(-(radius-10), -(radius-10), 2*(radius-10), 2*(radius-10)), this);
		insideCircle->setPen(defaultPen);
	}
}

void FsmGraphicState::updateSelectionShapeDisplay()
{
	if (this->isSelected() == true)
	{
		if (this->selectionShape == nullptr)
		{
			this->selectionShape = new QGraphicsEllipseItem(QRect(-(radius+10), -(radius+10), 2*(radius+10), 2*(radius+10)), this);
			this->selectionShape->setPen(selectionPen);
		}
		else
		{
			this->selectionShape->setVisible(true);
		}
	}
	else if ( (this->isSelected() == false) && (this->selectionShape != nullptr) )
	{
		this->selectionShape->setVisible(false);
	}
}

void FsmGraphicState::updateActionBoxPosition()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->logicComponentId);
	if (logicState == nullptr) return;

	if (logicState->getActions().count() != 0)
	{
		if  (this->actionBoxLine == nullptr)
		{
			this->actionBoxLine = new QGraphicsLineItem(radius, 0, radius + 20, 0, this);
			this->actionBoxLine->setPen(defaultPen);
		}

		auto actionsBoxCenter = this->actionsBox->boundingRect().height() / 2;

		// Positions is expressed in scene coordinates, as action box is not a child of this (scene stacking issues)
		this->actionsBox->setPos(mapToScene(radius + 20, -actionsBoxCenter));
	}
	else
	{
		delete this->actionBoxLine;
		this->actionBoxLine = nullptr;
	}
}
