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
#include "graphicfsmtransition.h"
#include "fsmstate.h"
#include "contextmenu.h"
#include "fsm.h"
#include "actionbox.h"


//
// Static elements
//

const qreal GraphicFsmState::radius = 50;

const QBrush GraphicFsmState::defaultBrush = QBrush(QColor(230,230,230), Qt::SolidPattern);

const QPen GraphicFsmState::defaultPen = QPen(Qt::SolidPattern, 3);
const QPen GraphicFsmState::hoverPen   = QPen(QBrush(Qt::blue, Qt::SolidPattern), 3);

qreal GraphicFsmState::getRadius()
{
	return GraphicFsmState::radius;
}

QPixmap GraphicFsmState::getPixmap(uint size, bool isInitial, bool addArrow)
{
	QPixmap pixmap(QSize(size, size));
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);

	painter.setPen(GraphicFsmState::defaultPen);
	painter.setBrush(GraphicFsmState::defaultBrush);
	painter.drawEllipse(QRectF(GraphicFsmState::defaultPen.width()/2, GraphicFsmState::defaultPen.width()/2, size-GraphicFsmState::defaultPen.width(), size-GraphicFsmState::defaultPen.width()));

	if (isInitial)
	{
		qreal space = size/10 + GraphicFsmState::defaultPen.width();
		painter.drawEllipse(QRectF(GraphicFsmState::defaultPen.width()/2 + space, GraphicFsmState::defaultPen.width()/2 + space, size-GraphicFsmState::defaultPen.width() - space*2, size-GraphicFsmState::defaultPen.width() - space*2));
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

GraphicFsmState::GraphicFsmState(componentId_t logicComponentId) :
    GraphicComponent(logicComponentId),
    QGraphicsEllipseItem(-radius, -radius, 2*radius, 2*radius)
{
	this->setPen(defaultPen);
	this->setBrush(defaultBrush);

	this->setFlag(QGraphicsItem::ItemIsMovable);
	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsFocusable);
	this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
	this->setFlag(QGraphicsItem::ItemClipsToShape);

	this->setAcceptHoverEvents(true);

	this->buildRepresentation();
	// Build action box separately as it is not a child of this (scene stacking issues)
	this->actionBox = new ActionBox(logicComponentId, true);
	this->updateActionBoxPosition();
}

GraphicFsmState::~GraphicFsmState()
{
	delete this->actionBox;
}

void GraphicFsmState::refreshDisplay()
{
	// Clear
	this->clearRepresentation();

	// Rebuild
	this->buildRepresentation();
	this->updateSelectionShapeDisplay();
	this->actionBox->refreshDisplay();

	// Reposition
	this->updateActionBoxPosition();
}

QPainterPath GraphicFsmState::shape() const
{
	// All states share the same shape:
	// store shape after building and always return the same.
	static QPainterPath path;

	if (path.isEmpty())
	{
		QPainterPathStroker* stroker;

		stroker = new QPainterPathStroker(defaultPen);
		path.addEllipse(QRect(-(radius), -(radius), 2*(radius), 2*(radius)));

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

void GraphicFsmState::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	// This disables the automatic selection shape.
	// Thanks to Stephen Chu on StackOverflow for the trick.
	QStyleOptionGraphicsItem myOption(*option);
	myOption.state &= ~QStyle::State_Selected;
	QGraphicsEllipseItem::paint(painter, &myOption, widget);
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
	else if (event->key() == Qt::Key_F2)
	{
		emit renameStateCalledEvent(this->logicComponentId);
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
		emit statePositionAboutToChangeEvent(this->logicComponentId);
	}
	if (change == GraphicsItemChange::ItemPositionHasChanged)
	{
		this->updateActionBoxPosition();
		emit statePositionChangedEvent(this->logicComponentId);
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

void GraphicFsmState::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	this->setPen(GraphicFsmState::hoverPen);
}

void GraphicFsmState::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	this->setPen(GraphicFsmState::defaultPen);
}

void GraphicFsmState::treatMenu(QAction* action)
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

void GraphicFsmState::clearRepresentation()
{
	qDeleteAll(this->childItems());

	this->selectionShape = nullptr;
	this->stateName      = nullptr;
}

void GraphicFsmState::buildRepresentation()
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

void GraphicFsmState::updateSelectionShapeDisplay()
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

void GraphicFsmState::updateActionBoxPosition()
{
	auto actionsBoxCenter = this->actionBox->getHeight() / 2;

	// Position is expressed in scene coordinates, as action box is not a child of this (scene stacking issues)
	this->actionBox->setPos(mapToScene(radius, -actionsBoxCenter));
}
