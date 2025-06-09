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
#include "graphicfsmstate.h"

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QKeyEvent>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "graphicfsmtransition.h"
#include "fsmstate.h"
#include "contextmenu.h"
#include "actionbox.h"


//
// Static members
//

const qreal GraphicFsmState::radius = 50;


qreal GraphicFsmState::getRadius()
{
	return GraphicFsmState::radius;
}

QPixmap GraphicFsmState::getPixmap(uint size, bool isInitial, bool addArrow)
{
	QPixmap pixmap(QSize(size, size));
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);
	int defaultPenWidth = GraphicComponent::defaultPen.width();

	painter.setPen(GraphicComponent::defaultPen);
	painter.setBrush(GraphicComponent::defaultBrush);
	painter.drawEllipse(QRectF(defaultPenWidth/2, defaultPenWidth/2, size-defaultPenWidth, size-defaultPenWidth));

	if (isInitial == true)
	{
		qreal space = size/10 + defaultPenWidth;
		painter.drawEllipse(QRectF(defaultPenWidth/2 + space, defaultPenWidth/2 + space, size-defaultPenWidth - space*2, size-defaultPenWidth - space*2));
	}

	if (addArrow == true)
	{
		painter.drawLine(0, 0, size/3, 0);
		painter.drawLine(0, 0, 0, size/3);
	}

	return pixmap;
}

//
// Class object definition
//

GraphicFsmState::GraphicFsmState(componentId_t logicComponentId) :
	GraphicComponent(logicComponentId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->getLogicComponentId());
	if (logicState == nullptr) return;


	// Initialize
	this->setFlag(QGraphicsItem::ItemIsMovable);
	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsFocusable);
	this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
	this->setFlag(QGraphicsItem::ItemClipsToShape);

	this->setAcceptHoverEvents(true);

	// Build children items
	this->circle = new QGraphicsEllipseItem(-GraphicFsmState::radius, -GraphicFsmState::radius, 2*GraphicFsmState::radius, 2*GraphicFsmState::radius, this);
	this->setBorderColor(defaultBorderColor);
	this->setFillingColor(defaultFillingColor);

	this->refreshChildrenItems();

	// Build external items
	this->actionBox = new ActionBox(logicComponentId, true);

	this->updateActionBoxPosition();
}

GraphicFsmState::~GraphicFsmState()
{
	delete this->actionBox;
}

void GraphicFsmState::refreshDisplay()
{
	this->refreshChildrenItems();
	this->refreshExternalItems();
}

QPainterPath GraphicFsmState::shape() const
{
	// All states share the same shape:
	// store shape after first build then always return the same.
	static QPainterPath path;

	if (path.isEmpty() == true)
	{
		QPainterPathStroker* stroker;

		stroker = new QPainterPathStroker(GraphicComponent::defaultPen);
		path.addEllipse(QRect(-GraphicFsmState::radius, -GraphicFsmState::radius, 2*GraphicFsmState::radius, 2*GraphicFsmState::radius));

		path.setFillRule(Qt::WindingFill);
		path = path.united(stroker->createStroke(path));
		path = path.simplified();
		delete stroker;
	}

	return path;
}

QRectF GraphicFsmState::boundingRect() const
{
	return this->shape().boundingRect();
}

ActionBox* GraphicFsmState::getActionBox() const
{
	return this->actionBox;
}

void GraphicFsmState::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete)
	{
		// This call may destroy the current object
		emit this->deleteStateCalledEvent(this->getLogicComponentId());
	}
	else if (event->key() == Qt::Key_Menu)
	{
		QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

		QGraphicsView* view = scene()->views()[0];

		QPoint posOnParent = view->mapFromScene(this->scenePos());

		QPoint posOnScreen = view->mapToGlobal(posOnParent);
		contextEvent->setScreenPos(posOnScreen);

		this->contextMenuEvent(contextEvent);
	}
	else if (event->key() == Qt::Key_F2)
	{
		emit renameStateCalledEvent(this->getLogicComponentId());
	}
	else
	{
		event->ignore();
	}
}

void GraphicFsmState::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->getLogicComponentId());
	if (logicState == nullptr) return;


	ContextMenu* menu = new ContextMenu();
	menu->addTitle(tr("State") + " <i>" + logicState->getName() + "</i>");

	if (fsm->getInitialStateId() != this->getLogicComponentId())
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

		connect(menu, &QMenu::triggered, this, &GraphicFsmState::treatMenu);
	}
	else
	{
		delete menu;
	}
}

QVariant GraphicFsmState::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == GraphicsItemChange::ItemPositionChange)
	{
		emit statePositionAboutToChangeEvent(this->getLogicComponentId());
	}
	if (change == GraphicsItemChange::ItemPositionHasChanged)
	{
		this->updateActionBoxPosition();
		emit statePositionChangedEvent(this->getLogicComponentId());
	}
	else if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedChange)
	{
		// If changing to selected
		if (value.toBool() == true)
		{
			// Refuse selection if there is a transition already selected
			const auto selectedItems = this->scene()->selectedItems();
			for (QGraphicsItem* selectedItem : selectedItems)
			{
				if (dynamic_cast<GraphicFsmTransition*>(selectedItem) != nullptr)
				{
					return (QVariant)false;
				}
			}
		}
	}

	return GraphicComponent::itemChange(change, value);
}

void GraphicFsmState::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	this->setBorderColor(GraphicComponent::hoverBorderColor);
}

void GraphicFsmState::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	this->setBorderColor(GraphicComponent::defaultBorderColor);
}

void GraphicFsmState::setBorderColor(QColor color)
{
	this->circle->setPen(QPen(QBrush(color, Qt::SolidPattern), GraphicComponent::defaultLineThickness));
}

void GraphicFsmState::setFillingColor(QColor color)
{
	this->circle->setBrush(QBrush(color, Qt::SolidPattern));
}

void GraphicFsmState::treatMenu(QAction* action)
{
	if (action->text() == tr("Edit"))
	{
		emit this->editStateCalledEvent(this->getLogicComponentId());
	}
	else if (action->text() == tr("Set initial"))
	{
		emit this->setInitialStateCalledEvent(this->getLogicComponentId());
	}
	else if (action->text() == tr("Rename"))
	{
		emit this->renameStateCalledEvent(this->getLogicComponentId());
	}
	else if (action->text() == tr("Delete"))
	{
		// This call may destroy the current object
		emit this->deleteStateCalledEvent(this->getLogicComponentId());
	}
	else if (action->text() == tr("Draw transition from this state"))
	{
		emit this->beginDrawTransitionFromThisState(this->getLogicComponentId());
	}
}

QAbstractGraphicsShapeItem* GraphicFsmState::buildSelectionShape()
{
	return new QGraphicsEllipseItem(QRect(-(radius+10), -(radius+10), 2*(radius+10), 2*(radius+10)), this);
}

void GraphicFsmState::refreshChildrenItems()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->getLogicComponentId());
	if (logicState == nullptr) return;


	// State name
	if (this->stateName == nullptr)
	{
		this->stateName = new QGraphicsTextItem(this);
	}

	this->stateName->setHtml("<span style=\"color:black;\">" + logicState->getName() + "</span>");
	this->stateName->setPos(-this->stateName->boundingRect().width()/2, -this->stateName->boundingRect().height()/2);

	// Inner circle
	if (fsm->getInitialStateId() == this->getLogicComponentId())
	{
		if (this->innerCircle == nullptr)
		{
			this->innerCircle = new QGraphicsEllipseItem(QRect(-(GraphicFsmState::radius-10), -(GraphicFsmState::radius-10), 2*(GraphicFsmState::radius-10), 2*(GraphicFsmState::radius-10)), this);
			this->innerCircle->setPen(GraphicComponent::defaultPen);
		}
	}
	else
	{
		delete this->innerCircle;
		this->innerCircle = nullptr;
	}
}

void GraphicFsmState::refreshExternalItems()
{
	if (this->actionBox != nullptr)
	{
		this->actionBox->refreshDisplay();
		this->updateActionBoxPosition();
	}
}

void GraphicFsmState::updateActionBoxPosition()
{
	if (this->actionBox == nullptr) return;


	auto actionsBoxCenter = this->actionBox->getHeight() / 2;

	// Position is expressed in scene coordinates, as action box is not a child of this (scene stacking issues)
	this->actionBox->setPos(mapToScene(GraphicFsmState::radius, -actionsBoxCenter));
}
