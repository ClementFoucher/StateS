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
#include "fsmgraphictransition.h"

// C++ classes
#include <math.h>
#include <memory>
using namespace std;

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsView>

// States classes
#include "machinemanager.h"
#include "fsmgraphicstate.h"
#include "fsmtransition.h"
#include "fsmgraphictransitionneighborhood.h"
#include "StateS_signal.h"
#include "contextmenu.h"
#include "equation.h"
#include "graphicfsm.h"
#include "fsm.h"
#include "fsmgraphicstate.h"
#include "actionbox.h"


//
// Static elements
//

const qreal FsmGraphicTransition::arrowEndSize = 10;
const qreal FsmGraphicTransition::conditionLineLength = 20;

const QPen FsmGraphicTransition::defaultPen   = QPen(Qt::SolidPattern, 3);
const QPen FsmGraphicTransition::drawingPen   = QPen(QBrush(Qt::blue, Qt::SolidPattern), 3);
const QPen FsmGraphicTransition::underEditPen = QPen(QBrush(Qt::gray, Qt::SolidPattern), 3);
const QPen FsmGraphicTransition::hoverPen     = QPen(QBrush(Qt::blue, Qt::SolidPattern), 3);


QPixmap FsmGraphicTransition::getPixmap(uint size)
{
	QPixmap pixmap(QSize(size, size));
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);

	painter.setPen(FsmGraphicTransition::defaultPen);
	painter.drawLine(0, 0, size, size);
	painter.drawLine(0, 0, size/3, 0);
	painter.drawLine(0, 0, 0, size/3);

	return pixmap;
}

//
// Class object definition
//

FsmGraphicTransition::FsmGraphicTransition(componentId_t logicComponentId) :
    GraphicComponent(logicComponentId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicTransition = fsm->getTransition(this->logicComponentId);
	if (logicTransition == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;


	this->initializeDefaults();
	this->currentPen = &defaultPen;

	this->conditionLineSliderPos = 0.5;
	this->conditionText = new QGraphicsTextItem();
	this->currentMode = Mode_t::standardMode;

	this->sourceStateId = logicTransition->getSourceStateId();
	this->targetStateId = logicTransition->getTargetStateId();

	auto sourceState = graphicFsm->getState(this->sourceStateId);
	auto targetState = graphicFsm->getState(this->targetStateId);
	connect(sourceState, &FsmGraphicState::statePositionChangedEvent, this, &FsmGraphicTransition::transitionNeedsRefreshEventHandler);
	connect(targetState, &FsmGraphicState::statePositionChangedEvent, this, &FsmGraphicTransition::transitionNeedsRefreshEventHandler);

	this->buildChildren();
	this->updateConditionText();
	this->buildRepresentation();
	// Build action box separately as it is not a child of this (scene stacking issues)
	this->actionBox = new ActionBox(logicComponentId);

	this->repositionChildren();
}

FsmGraphicTransition::FsmGraphicTransition(componentId_t sourceStateId, componentId_t targetStateId, const QPointF& dynamicMousePosition) :
	GraphicComponent(nullId)
{
	if ( ( (sourceStateId != nullId) && (targetStateId != nullId) ) ||
		 ( (sourceStateId == nullId) && (targetStateId == nullId) )
	   )
	{
		this->currentMode = Mode_t::errorMode;
		return;
	}

	this->initializeDefaults();
	this->currentPen = &drawingPen;

	if (sourceStateId != nullId)
	{
		this->sourceStateId = sourceStateId;
		this->currentMode = Mode_t::dynamicTargetMode;
	}
	else if (targetStateId != nullId)
	{
		this->targetStateId = targetStateId;
		this->currentMode = Mode_t::dynamicSourceMode;
	}

	this->buildChildren();

	this->dynamicStateId = nullId;
	this->mousePosition = dynamicMousePosition;

	this->buildRepresentation();
}

FsmGraphicTransition::~FsmGraphicTransition()
{
	delete this->conditionText;
	delete this->actionBox;
}

void FsmGraphicTransition::refreshDisplay()
{
	// Clear
	this->clearRepresentation();

	// Rebuild
	this->updateConditionText();
	this->buildRepresentation();
	this->updateSelectionShapeDisplay();
	if (this->actionBox != nullptr)
	{
		this->actionBox->refreshDisplay();
	}

	// Reposition
	this->repositionChildren();
}

componentId_t FsmGraphicTransition::getSourceStateId() const
{
	return this->sourceStateId;
}

componentId_t FsmGraphicTransition::getTargetStateId() const
{
	return this->targetStateId;
}

void FsmGraphicTransition::setConditionLineSliderPosition(qreal position)
{
	if ( (position >= 0) && (position <= 1) )
	{
		this->conditionLineSliderPos = position;
		this->refreshDisplay();
	}
}

qreal FsmGraphicTransition::getConditionLineSliderPosition() const
{
	return this->conditionLineSliderPos;
}

QGraphicsTextItem* FsmGraphicTransition::getConditionText() const
{
	return this->conditionText;
}

QPainterPath FsmGraphicTransition::shape() const
{
	return this->boundingShape;
}

QRectF FsmGraphicTransition::boundingRect() const
{
	return this->boundingShape.boundingRect();
}

ActionBox* FsmGraphicTransition::getActionBox() const
{
	return this->actionBox;
}

void FsmGraphicTransition::setUnderEdit(bool edit)
{
	if (edit == true)
	{
		this->currentPen          = &underEditPen;
		this->currentConditionPen = &underEditPen;
	}
	else
	{
		this->currentPen          = &defaultPen;
		this->currentConditionPen = &defaultPen;
	}

	this->isUnderEdit = edit;
	this->repaint();
}

void FsmGraphicTransition::setDynamicState(componentId_t newDynamicStateId)
{
	if (newDynamicStateId == this->dynamicStateId) return;

	this->dynamicStateId = newDynamicStateId;

	this->refreshDisplay();
}

void FsmGraphicTransition::setMousePosition(const QPointF& mousePos)
{
	this->dynamicStateId = nullId;
	this->mousePosition = mousePos;

	this->refreshDisplay();
}

void FsmGraphicTransition::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	if (this->boundingShape.contains(event->pos()) == false)
	{
		event->ignore();
		return;
	}

	if (this->currentMode != Mode_t::standardMode)
	{
		event->ignore();
		return;
	}

	if (machineManager->getCurrentSimulationMode() != SimulationMode_t::editMode)
	{
		event->ignore();
		return;
	}

	ContextMenu* menu = new ContextMenu();
	menu->addTitle(tr("Transition"));

	menu->addAction(tr("Change source"));
	menu->addAction(tr("Change target"));
	menu->addAction(tr("Edit"));
	menu->addAction(tr("Delete"));
	menu->popup(event->screenPos());

	connect(menu, &QMenu::triggered, this, &FsmGraphicTransition::treatMenu);
}

void FsmGraphicTransition::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Menu)
	{
		QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

		QGraphicsView* view = scene()->views()[0];

		QPoint posOnParent = view->mapFromScene(this->scenePos());

		QPoint posOnScreen = view->mapToGlobal(posOnParent);
		contextEvent->setScreenPos(posOnScreen);

		this->contextMenuEvent(contextEvent);
	}
	else if (event->key() == Qt::Key_Delete)
	{
		emit this->deleteTransitionCalledEvent(this->logicComponentId);
	}
	else
	{
		event->ignore();
	}
}

void FsmGraphicTransition::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
	// We do not need to handle release and other events as mousePressEvent is either:
	// => passed to parent class, so other events will be so
	// => ignored, so no other event will be trigered
	if (this->boundingShape.contains(ev->pos()))
		QGraphicsItemGroup::mousePressEvent(ev);
	else
		ev->ignore();
}

QVariant FsmGraphicTransition::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
	if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedChange)
	{
		// If changing to selected
		if (value.toBool() == true)
		{
			// Refuse selection if there are other item(s) already selected
			if (this->scene()->selectedItems().count() != 0)
				return (QVariant)false;
		}
	}
	else if (change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged)
	{
		this->updateSelectionShapeDisplay();
	}

	return QGraphicsItemGroup::itemChange(change, value);
}

void FsmGraphicTransition::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	// Temporarity override current color
	this->currentPen          = &FsmGraphicTransition::hoverPen;
	this->currentConditionPen = &FsmGraphicTransition::hoverPen;

	this->repaint();
}

void FsmGraphicTransition::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	// Restore color depending on edit mode
	this->setUnderEdit(this->isUnderEdit);
}

void FsmGraphicTransition::transitionNeedsRefreshEventHandler()
{
	this->refreshDisplay();
}

void FsmGraphicTransition::updateConditionText()
{
	// Ignore this call for dummy transactions
	if (this->currentMode != Mode_t::standardMode) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicTransition = fsm->getTransition(this->logicComponentId);

	if (logicTransition != nullptr)
	{
		if (logicTransition->getCondition() == nullptr)
		{
			this->conditionText->setHtml("<div style=\"background-color:#E8E8E8; color:#000000;\">1</div>");
		}
		else
		{
			shared_ptr<Equation> equationCondition = dynamic_pointer_cast<Equation>(logicTransition->getCondition());

			if (equationCondition != nullptr)
			{
				this->conditionText->setHtml("<div style=\"background-color:#E8E8E8;\">" + equationCondition->getColoredText() + "</div>");
			}
			else
			{
				this->conditionText->setHtml("<div style=\"background-color:#E8E8E8;color:black;\">" + logicTransition->getCondition()->getText() + "</div>");
			}
		}
	}
	else
	{
		this->conditionText->setHtml("<div style=\"background-color:#E8E8E8;\">1</div>");
	}
}

void FsmGraphicTransition::treatMenu(QAction* action)
{
	if (action->text() == tr("Change source"))
	{
		emit this->dynamicSourceCalledEvent(this->logicComponentId);
	}
	else if (action->text() == tr("Change target"))
	{
		emit this->dynamicTargetCalledEvent(this->logicComponentId);
	}
	if (action->text() == tr("Edit"))
	{
		emit this->editTransitionCalledEvent(this->logicComponentId);
	}
	else if (action->text() == tr("Delete"))
	{
		emit this->deleteTransitionCalledEvent(this->logicComponentId);
	}
}

void FsmGraphicTransition::clearRepresentation()
{
	delete this->arrowBody;
	this->arrowBody = nullptr;

	delete this->selectionShape;
	this->selectionShape = nullptr;
}

void FsmGraphicTransition::buildRepresentation()
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	//
	// Determine source and target points depending on mode.
	// For ends that are connected to a state, center is used as a first approximation
	QPointF currentSourcePoint;
	QPointF currentTargetPoint;
	componentId_t currentSourceStateId = nullId;
	componentId_t currentTargetStateId = nullId;
	auto currentSourceState = graphicFsm->getState(this->sourceStateId);
	auto currentTargetState = graphicFsm->getState(this->targetStateId);

	if (this->currentMode == Mode_t::standardMode)
	{
		currentSourcePoint = currentSourceState->scenePos();
		currentTargetPoint = currentTargetState->scenePos();

		currentSourceStateId = this->sourceStateId;
		currentTargetStateId = this->targetStateId;
	}
	else if (this->currentMode == Mode_t::dynamicSourceMode)
	{
		if (this->dynamicStateId != nullId)
		{
			auto dynamicState = graphicFsm->getState(this->dynamicStateId);
			currentSourcePoint = dynamicState->scenePos();
			currentSourceStateId = this->dynamicStateId;
		}
		else
		{
			currentSourcePoint = this->mousePosition;
			currentSourceStateId = nullId;
		}

		currentTargetPoint = currentTargetState->scenePos();
		currentTargetStateId = this->targetStateId;
	}
	else if (this->currentMode == Mode_t::dynamicTargetMode)
	{
		currentSourcePoint = currentSourceState->scenePos();
		currentSourceStateId = this->sourceStateId;

		if (this->dynamicStateId != nullId)
		{
			auto dynamicState = graphicFsm->getState(this->dynamicStateId);
			currentTargetPoint = dynamicState->scenePos();
			currentTargetStateId = this->dynamicStateId;
		}
		else
		{
			currentTargetPoint = this->mousePosition;
			currentTargetStateId = nullId;
		}
	}

	//
	// Redraw arrow body
	if (this->logicComponentId == nullId)
	{
		// Currently adding or editing a transition
		if (currentSourceStateId != currentTargetStateId)
		{
			this->drawStraightTransition(currentSourcePoint, currentTargetPoint);
		}
		else
		{
			this->drawAutoTransition(currentSourcePoint);
		}
	}
	else
	{
		auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
		if (graphicFsm == nullptr) return;

		if ( (graphicFsm->getTransitionRank(this->logicComponentId) == nullId) && (currentSourceStateId != currentTargetStateId) )
		{
			this->drawStraightTransition(currentSourcePoint, currentTargetPoint);
		}
		else if (currentSourceStateId == currentTargetStateId)
		{
			this->drawAutoTransition(currentSourcePoint);
		}
		else
		{
			this->drawCurvedTransition(currentSourcePoint, currentTargetPoint);
		}

		this->rebuildBoundingShape();
	}

	this->repaint();
}

void FsmGraphicTransition::initializeDefaults()
{
	this->currentPen          = &defaultPen;
	this->currentConditionPen = &defaultPen;

	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsFocusable);
	this->setFlag(QGraphicsItem::ItemClipsToShape);

	this->setAcceptHoverEvents(true);
}

void FsmGraphicTransition::buildChildren()
{
	//
	// Arrow end

	// Initially, arrow will point to north-west
	QPainterPath arrowPath;
	arrowPath.lineTo(this->arrowEndSize, 0);
	arrowPath.moveTo(0, 0);
	arrowPath.lineTo(0, this->arrowEndSize);

	this->arrowEnd = new QGraphicsPathItem(arrowPath, this);

	//
	// Condition line

	if (this->currentMode == Mode_t::standardMode)
	{
		QLineF conditionLineF(0, -conditionLineLength/2, 0, conditionLineLength/2);
		this->conditionLine = new QGraphicsLineItem(conditionLineF, this);
	}
}

void FsmGraphicTransition::repositionChildren()
{
	this->arrowEnd->setRotation(this->arrowEndAngle);
	this->arrowEnd->setPos(this->arrowEndPosition);

	if (this->conditionLine != nullptr)
	{
		this->conditionLine->setRotation(this->conditionLineAngle);
		this->conditionLine->setPos(this->conditionLinePos);
	}

	if (this->conditionText != nullptr)
	{
		this->conditionText->setPos(mapToScene(this->conditionLinePos) + QPointF(0, 5));
	}

	if ( (this->actionBox != nullptr) && (this->conditionText != nullptr) )
	{
		this->actionBox->setPos(mapToScene(this->conditionLinePos + this->conditionText->boundingRect().bottomLeft() + QPointF(0, 5)));
	}
}

void FsmGraphicTransition::repaint()
{
	this->arrowEnd->setPen(*this->currentPen);

	if (this->conditionLine != nullptr)
	{
		this->conditionLine->setPen(*this->currentConditionPen);
	}

	QGraphicsLineItem* line = dynamic_cast<QGraphicsLineItem*>(this->arrowBody);
	if (line != nullptr)
	{
		line->setPen(*this->currentPen);
		return;
	}

	QGraphicsPathItem* arcItem = dynamic_cast<QGraphicsPathItem*>(this->arrowBody);
	if (arcItem != nullptr)
	{
		arcItem->setPen(*this->currentPen);
		return;
	}
}

void FsmGraphicTransition::rebuildBoundingShape()
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	this->prepareGeometryChange();

	QGraphicsLineItem* arrowBodyStraightLine = dynamic_cast<QGraphicsLineItem*>(this->arrowBody);

	QPainterPath path;
	QTransform t;

	if (arrowBodyStraightLine != nullptr)
	{
		QLineF straightLine = arrowBodyStraightLine->line();

		QRectF boundRect(0, -conditionLineLength/2, straightLine.length(), conditionLineLength);

		path.addRect(boundRect);
		path.angleAtPercent(arrowBodyStraightLine->rotation());

		t.rotate(-straightLine.angle());
	}
	else if (this->sourceStateId == this->targetStateId)
	{
		// Auto-transition
		QPainterPath arrowPath = ((QGraphicsPathItem*)arrowBody)->path();

		qreal outterScale = (arrowPath.boundingRect().height() + 3*conditionLineLength/4) / arrowPath.boundingRect().height();
		qreal innerScale  = (arrowPath.boundingRect().height() - 3*conditionLineLength/4) / arrowPath.boundingRect().height();

		QTransform expand;
		expand.scale(outterScale, outterScale);
		QPainterPath outterPath = expand.map(arrowPath);

		QTransform shrink;
		shrink.scale(innerScale, innerScale);
		QPainterPath innerPath = shrink.map(arrowPath);

		outterPath.translate(0, conditionLineLength/2);
		innerPath.translate(0, -conditionLineLength/2);

		innerPath = innerPath.toReversed();

		path.moveTo(outterPath.pointAtPercent(0));
		path.connectPath(outterPath);
		path.lineTo(innerPath.pointAtPercent(0));
		path.connectPath(innerPath);

		path.closeSubpath();
	}
	else
	{
		// Arc transiton
		auto neighborhood = graphicFsm->getTransitionNeighborhood(this->logicComponentId);

		QPainterPath arrowPath = ((QGraphicsPathItem*)this->arrowBody)->path();

		qreal outterScale = (arrowPath.boundingRect().width() + conditionLineLength/2) / arrowPath.boundingRect().width();
		qreal innerScale  = (arrowPath.boundingRect().width() - conditionLineLength/2) / arrowPath.boundingRect().width();

		QTransform expand;
		expand.scale(outterScale, outterScale);
		QPainterPath outterPath = expand.map(arrowPath);

		QTransform shrink;
		shrink.scale(innerScale, innerScale);
		QPainterPath innerPath = shrink.map(arrowPath);

		if (graphicFsm->getTransitionRank(this->logicComponentId) > 0)
		{
			outterPath.translate(-(outterPath.boundingRect().width()-arrowPath.boundingRect().width())/2, conditionLineLength/2);
			innerPath.translate((arrowPath.boundingRect().width()-innerPath.boundingRect().width())/2, -conditionLineLength/2);
		}
		else
		{
			outterPath.translate(-(outterPath.boundingRect().width()-arrowPath.boundingRect().width())/2, -conditionLineLength/2);
			innerPath.translate((arrowPath.boundingRect().width()-innerPath.boundingRect().width())/2, conditionLineLength/2);
		}

		innerPath = innerPath.toReversed();

		path.moveTo(outterPath.pointAtPercent(0));
		path.connectPath(outterPath);
		path.lineTo(innerPath.pointAtPercent(0));
		path.connectPath(innerPath);

		path.closeSubpath();

		t.rotate(-this->sceneAngle);
	}

	this->boundingShape = t.map(path);
}

void FsmGraphicTransition::updateSelectionShapeDisplay()
{
	if (this->isSelected() == true)
	{
		if (this->selectionShape == nullptr)
		{
			this->selectionShape = new QGraphicsPathItem(this->boundingShape, this);
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

void FsmGraphicTransition::drawStraightTransition(QPointF currentSourcePoint, QPointF currentTargetPoint)
{
	//
	// Build arrow body

	QLineF straightLine(QPointF(0, 0), currentTargetPoint - currentSourcePoint);

	// Set line appropriate size (reduce current size by correct number of state radius to point on states border)
	if ( ((this->currentMode == Mode_t::dynamicSourceMode) || (this->currentMode == Mode_t::dynamicTargetMode)) && (this->dynamicStateId == nullId) )
	{
		// Reduce line length to match space between state center and mouse position
		straightLine.setLength(straightLine.length() - FsmGraphicState::getRadius());
	}
	else
	{
		// Reduce line length to match space between states
		straightLine.setLength(straightLine.length() - 2*FsmGraphicState::getRadius());
	}

	// Create line graphic representation
	QGraphicsLineItem* line = new QGraphicsLineItem(this);
	line->setLine(straightLine);
	this->arrowBody = line;

	// If source is a state, line should be translated to begin on state border
	if ( !((this->currentMode == Mode_t::dynamicSourceMode) && (this->dynamicStateId == nullId)) )
	{
		// Translation vector is based on straight line (same vector) and normalized to required translation length
		QLineF translationVector(straightLine);
		translationVector.setLength(FsmGraphicState::getRadius());

		currentSourcePoint += translationVector.p2();
	}

	//
	// Compute condition line position (only in standard mode, not displayed in other modes)

	if (this->currentMode == Mode_t::standardMode)
	{
		this->conditionLinePos   = QPointF(straightLine.p2()*this->conditionLineSliderPos);
		this->conditionLineAngle = -straightLine.angle();
	}

	//
	// Compute arrow end position

	// Current target position is calculated based on source point translated by line
	currentTargetPoint = currentSourcePoint + straightLine.p2();

	// Compute arrow end rotation based on line angle
	this->arrowEndAngle = 135-straightLine.angle();
	// Set arrow position to be at target side
	// Positions are intended in scene coordinates. Relocate wrt. this item coordinates: (0, 0) is at source point position
	this->arrowEndPosition = currentTargetPoint - currentSourcePoint;

	//
	// Position the whole transition

	this->setPos(currentSourcePoint);
}

void FsmGraphicTransition::drawAutoTransition(QPointF currentSourcePoint)
{
	//
	// Build arrow body

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	uint rank;
	if (graphicFsm != nullptr)
	{
		auto neighborhood = graphicFsm->getTransitionNeighborhood(this->logicComponentId);
		if (neighborhood != nullptr)
		{
			rank = neighborhood->getTransitionNumber(this);
		}
		else
		{
			rank = 0;
		}
	}
	else
	{
		rank = 0;
	}

	QLineF stateCenterToArcStartVector(0, 0, 1, -1);
	stateCenterToArcStartVector.setLength(FsmGraphicState::getRadius());

	QLineF stateCenterToArcEndVector(0, 0, -1, -1);
	stateCenterToArcEndVector.setLength(FsmGraphicState::getRadius());

	qreal unitaryArcWidth  = 2*FsmGraphicState::getRadius();
	qreal unitaryArcHeight = 2*FsmGraphicState::getRadius();

	QLineF stateCenterToUnitaryArcCenterVector(0, 0, 0, 2*stateCenterToArcStartVector.p2().y());

	qreal arcWidth  = unitaryArcWidth  * pow (1.2, rank);
	qreal arcHeight = unitaryArcHeight * pow (1.4, rank);

	QLineF stateCenterToArcCenterVector(0, 0, 0, stateCenterToUnitaryArcCenterVector.p2().y() - (arcHeight-unitaryArcHeight)/2);

	QRectF arcRect(-arcWidth/2, stateCenterToArcCenterVector.p2().y()-arcHeight/2, arcWidth, arcHeight);

	QPainterPath arc;
	arc.moveTo(stateCenterToArcStartVector.p2());

	qreal arcStartAngle = -45;
	qreal arcTotalAngle = 360 - 2* (90 + arcStartAngle);

	arc.arcTo(arcRect, arcStartAngle, arcTotalAngle);

	arc.lineTo(stateCenterToArcEndVector.p2());

	QGraphicsPathItem* arcItem = new QGraphicsPathItem(arc, this);
	this->arrowBody = arcItem;

	//
	// Compute condition line position (only in standard mode, not displayed in other modes)

	if (this->currentMode == Mode_t::standardMode)
	{
		this->conditionLinePos   = arc.pointAtPercent(this->conditionLineSliderPos);
		this->conditionLineAngle = -arc.angleAtPercent(this->conditionLineSliderPos);
	}

	//
	// Compute arrow end position

	this->arrowEndAngle = 180;
	this->arrowEndPosition = stateCenterToArcEndVector.p2();

	//
	// Position the whole transition

	this->setPos(currentSourcePoint);
}

void FsmGraphicTransition::drawCurvedTransition(QPointF currentSourcePoint, QPointF currentTargetPoint)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto neighborhood = graphicFsm->getTransitionNeighborhood(this->logicComponentId);

	// Take neighbors in consideration to draw a curve
	// All neighbors will have a curved body, which curve intensity
	// depends on their rank in neighborhood

	// Get actual source and target in mates's coordinate system
	auto sourceStateId = neighborhood->getSource()->getLogicComponentId();
	if ( ((this->currentMode == Mode_t::standardMode)      && (sourceStateId != this->sourceStateId)) ||
	     ((this->currentMode == Mode_t::dynamicTargetMode) && (sourceStateId != this->sourceStateId)) ||
	     ((this->currentMode == Mode_t::dynamicSourceMode) && (sourceStateId != this->dynamicStateId))
	   )
	{
		QPointF temp = currentTargetPoint;
		currentTargetPoint = currentSourcePoint;
		currentSourcePoint = temp;
	}

	//
	// Build arrow body

	//
	// Create a first system as:
	// - Origin (0, 0) is in neighborhood origin
	// - x is a line passing through both ends states' centers with (0, 1) in target center
	// - Y is X's normal, with same measures
	QLineF initialSystemXVector(QPointF(0, 0), neighborhood->getTarget()->scenePos() - neighborhood->getSource()->scenePos());
	QLineF initialSystemYVector = initialSystemXVector.normalVector();

	//
	// Based on this system measures, create a curve in scene
	// coordinates system. It will be rotated later.

	QLineF sceneSystemXVector(0, 0, initialSystemXVector.length(), 0);
	QLineF sceneSystemYVector(0, 0, 0, initialSystemYVector.length());

	//
	// Then, restain the coordinates system to begin transition
	// edges on state perimeter.

	// Calculate initial C bezier point
	QLineF cPointYTranslationVector(sceneSystemYVector);
	cPointYTranslationVector.setLength(graphicFsm->getTransitionRank(this->logicComponentId) * 150);
	QPointF sceneSystemCPoint(sceneSystemXVector.p2()/2 + cPointYTranslationVector.p2());

	// Use initial C point to calculate new coordinates system
	// Deltas indicates curve start/end positions wrt. local system (start/ends on states perimeter)
	QLineF deltaSystemOriginVector(QPointF(0,0), sceneSystemCPoint);
	deltaSystemOriginVector.setLength(FsmGraphicState::getRadius());

	QLineF deltaTargetVector(QPointF(0,0), sceneSystemCPoint - sceneSystemXVector.p2());
	deltaTargetVector.setLength(FsmGraphicState::getRadius());

	// Yes, many coordinates systems.
	QLineF deltaSystemXVector(deltaSystemOriginVector.p2(), sceneSystemXVector.p2() + deltaTargetVector.p2());
	deltaSystemXVector.translate(-deltaSystemXVector.p1());
	// Update C point position in delta coordinates system
	QPointF deltaSystemCPoint = sceneSystemCPoint - deltaSystemOriginVector.p2();

	QPainterPath path;
	path.quadTo(deltaSystemCPoint, deltaSystemXVector.p2());

	QGraphicsPathItem* curve = new QGraphicsPathItem(path, this);

	// Drawing in a horizontal coordinates, then rotate.
	// This is probably too much, but it was easier to reprensent in my mind ;)
	curve->setTransform(QTransform().rotate(-initialSystemXVector.angle()));

	deltaSystemOriginVector.setAngle(deltaSystemOriginVector.angle()+initialSystemXVector.angle());

	QLineF actualTarget(QPointF(0, 0), deltaSystemXVector.p2());
	actualTarget.setAngle(actualTarget.angle() + initialSystemXVector.angle());

	// Delta curve origin is the position of curve starting point wrt. source state center
	QPointF deltaCurveOrigin = deltaSystemOriginVector.p2();

	// Curve target is the curve last point in the horizontal coordinates system which originates at curve origin
	QPointF curveTarget = actualTarget.p2();

	// Compute scene angle
	this->sceneAngle = QLineF(QPointF(0,0), curveTarget).angle();

	this->arrowBody = curve;

	//
	// Compute condition line position (only in standard mode, not displayed in other modes)

	if (this->currentMode == Mode_t::standardMode)
	{
		QLineF middleVector;
		qreal  angleAtPos;
		if (sourceStateId == this->sourceStateId)
		{
			middleVector = QLineF(QPointF(0,0), path.pointAtPercent(this->conditionLineSliderPos));
			angleAtPos   = path.angleAtPercent(this->conditionLineSliderPos);
		}
		else
		{
			middleVector = QLineF(QPointF(0,0), path.pointAtPercent(1-this->conditionLineSliderPos));
			angleAtPos   = path.angleAtPercent(1-this->conditionLineSliderPos);
		}
		middleVector.setAngle(middleVector.angle() + initialSystemXVector.angle());

		this->conditionLinePos   = middleVector.p2();
		this->conditionLineAngle = -(angleAtPos + this->sceneAngle);
	}

	//
	// Compute arrow end position

	if (((this->currentMode == Mode_t::standardMode)      && (sourceStateId == this->sourceStateId)) ||
	    ((this->currentMode == Mode_t::dynamicTargetMode) && (sourceStateId == this->sourceStateId)) ||
	    ((this->currentMode == Mode_t::dynamicSourceMode) && (sourceStateId == this->dynamicStateId)))
	{
		QLineF edgeAngleLine(deltaSystemCPoint, deltaSystemXVector.p2());
		this->arrowEndAngle = 135-(edgeAngleLine.angle() + initialSystemXVector.angle());
		this->arrowEndPosition = curveTarget;
	}
	else
	{
		QLineF edgeAngleLine(deltaSystemCPoint, deltaSystemXVector.p1());
		this->arrowEndAngle = 135-(edgeAngleLine.angle() + initialSystemXVector.angle());
		this->arrowEndPosition = QPointF(0, 0);
	}

	//
	// Position the whole transition

	this->setPos(currentSourcePoint+deltaCurveOrigin);
}
