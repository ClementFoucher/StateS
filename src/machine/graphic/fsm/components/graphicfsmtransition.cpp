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
#include "graphicfsmtransition.h"

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsView>

// States classes
#include "machinemanager.h"
#include "fsm.h"
#include "graphicfsm.h"
#include "fsmtransition.h"
#include "graphicfsmstate.h"
#include "graphicfsmtransitionneighborhood.h"
#include "equation.h"
#include "operand.h"
#include "actionbox.h"
#include "contextmenu.h"


//
// Static members
//

const qreal GraphicFsmTransition::arrowEndSize = 10;
const qreal GraphicFsmTransition::conditionLineLength = 20;


QPixmap GraphicFsmTransition::getPixmap(uint size)
{
	QPixmap pixmap(QSize(size, size));
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);

	painter.setPen(GraphicComponent::defaultPen);
	painter.drawLine(0, 0, size, size);
	painter.drawLine(0, 0, size/3, 0);
	painter.drawLine(0, 0, 0, size/3);

	return pixmap;
}

//
// Class object definition
//

GraphicFsmTransition::GraphicFsmTransition(componentId_t logicComponentId) :
	GraphicComponent(logicComponentId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicTransition = fsm->getTransition(this->getLogicComponentId());
	if (logicTransition == nullptr) return;

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;


	// Initialize
	this->initializeDefaults();
	this->currentPen = new QPen(QBrush(GraphicComponent::defaultBorderColor, Qt::SolidPattern), GraphicComponent::defaultLineThickness);
	this->currentMode = Mode_t::standardMode;

	this->sourceStateId = logicTransition->getSourceStateId();
	this->targetStateId = logicTransition->getTargetStateId();

	auto sourceState = graphicFsm->getState(this->sourceStateId);
	auto targetState = graphicFsm->getState(this->targetStateId);
	connect(sourceState, &GraphicFsmState::statePositionChangedEvent, this, &GraphicFsmTransition::connectedStateMovedEventHandler);
	connect(targetState, &GraphicFsmState::statePositionChangedEvent, this, &GraphicFsmTransition::connectedStateMovedEventHandler);

	// Build children items
	this->buildArrowEnd();
	QLineF conditionLineF(0, -GraphicFsmTransition::conditionLineLength/2, 0, GraphicFsmTransition::conditionLineLength/2);
	this->conditionLine = new QGraphicsLineItem(conditionLineF, this);
	this->buildArrowBody();

	this->refreshChildrenItems();

	// Build external items
	this->conditionText = new QGraphicsTextItem();
	this->actionBox = new ActionBox(logicComponentId);

	this->refreshExternalItems();
}

GraphicFsmTransition::GraphicFsmTransition(componentId_t sourceStateId, componentId_t targetStateId, const QPointF& dynamicMousePosition) :
	GraphicComponent(nullId)
{
	if ( ( (sourceStateId != nullId) && (targetStateId != nullId) ) ||
	     ( (sourceStateId == nullId) && (targetStateId == nullId) )
	   ) return;


	// Initialize
	this->initializeDefaults();
	this->currentPen = new QPen(QBrush(GraphicComponent::drawingBorderColor, Qt::SolidPattern), GraphicComponent::defaultLineThickness);

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

	this->mousePosition = dynamicMousePosition;

	// Build children items
	this->buildArrowEnd();
	this->buildArrowBody();

	this->refreshChildrenItems();
}

GraphicFsmTransition::~GraphicFsmTransition()
{
	delete this->conditionText;
	delete this->actionBox;
	delete this->currentPen;
	delete this->currentConditionPen;
}

void GraphicFsmTransition::refreshDisplay()
{
	// Rebuild body
	this->clearArrowBody();
	this->buildArrowBody();

	// Reposition other items
	this->refreshChildrenItems();
	this->refreshExternalItems();
}

componentId_t GraphicFsmTransition::getSourceStateId() const
{
	return this->sourceStateId;
}

componentId_t GraphicFsmTransition::getTargetStateId() const
{
	return this->targetStateId;
}

void GraphicFsmTransition::setConditionLineSliderPosition(qreal position)
{
	if ( (position >= 0) && (position <= 1) )
	{
		this->conditionLineSliderPos = position;
		this->refreshDisplay();
	}
}

qreal GraphicFsmTransition::getConditionLineSliderPosition() const
{
	return this->conditionLineSliderPos;
}

QGraphicsTextItem* GraphicFsmTransition::getConditionText() const
{
	return this->conditionText;
}

QPainterPath GraphicFsmTransition::shape() const
{
	return this->boundingShape;
}

QRectF GraphicFsmTransition::boundingRect() const
{
	return this->boundingShape.boundingRect();
}

ActionBox* GraphicFsmTransition::getActionBox() const
{
	return this->actionBox;
}

void GraphicFsmTransition::setUnderEdit(bool edit)
{
	if (edit == true)
	{
		this->setArrowColor(GraphicComponent::underEditBorderColor);
		this->setConditionColor(GraphicComponent::underEditBorderColor);
	}
	else
	{
		this->setArrowColor(GraphicComponent::defaultBorderColor);
		this->setConditionColor(GraphicComponent::defaultBorderColor);
	}

	this->isUnderEdit = edit;
	this->repaint();
}

void GraphicFsmTransition::setDynamicState(componentId_t newDynamicStateId)
{
	if (newDynamicStateId == this->dynamicStateId) return;

	this->dynamicStateId = newDynamicStateId;

	this->refreshDisplay();
}

void GraphicFsmTransition::setMousePosition(const QPointF& mousePos)
{
	this->dynamicStateId = nullId;
	this->mousePosition = mousePos;

	this->refreshDisplay();
}

void GraphicFsmTransition::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
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

	connect(menu, &QMenu::triggered, this, &GraphicFsmTransition::treatMenu);
}

void GraphicFsmTransition::keyPressEvent(QKeyEvent* event)
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
		emit this->deleteTransitionCalledEvent(this->getLogicComponentId());
	}
	else
	{
		event->ignore();
	}
}

QVariant GraphicFsmTransition::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
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

	return GraphicComponent::itemChange(change, value);
}

void GraphicFsmTransition::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	if (this->isUnderEdit == false)
	{
		this->setArrowColor(GraphicComponent::hoverBorderColor);
		this->setConditionColor(GraphicComponent::hoverBorderColor);
	}
}

void GraphicFsmTransition::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	if (this->isUnderEdit == false)
	{
		this->setArrowColor(GraphicComponent::defaultBorderColor);
		this->setConditionColor(GraphicComponent::defaultBorderColor);
	}
}

void GraphicFsmTransition::setArrowColor(QColor color)
{
	delete this->currentPen;
	this->currentPen = new QPen(QBrush(color, Qt::SolidPattern), GraphicComponent::defaultLineThickness);

	this->repaint();
}

void GraphicFsmTransition::setConditionColor(QColor color)
{
	delete this->currentConditionPen;
	this->currentConditionPen = new QPen(QBrush(color, Qt::SolidPattern), GraphicComponent::defaultLineThickness);

	this->repaint();
}

void GraphicFsmTransition::connectedStateMovedEventHandler()
{
	this->refreshDisplay();
}

void GraphicFsmTransition::updateConditionText()
{
	// Ignore this call for dummy transitions
	if (this->currentMode != Mode_t::standardMode) return;

	if (this->conditionText == nullptr) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicTransition = fsm->getTransition(this->getLogicComponentId());
	if (logicTransition == nullptr) return;


	auto condition = logicTransition->getCondition();
	if (condition == nullptr)
	{
		// An empty condition is always true
		this->conditionText->setHtml("<div style=\"background-color:#E8E8E8; color:#000000;\">1</div>");
	}
	else if (condition->getOperatorType() != OperatorType_t::identity)
	{
		this->conditionText->setHtml("<div style=\"background-color:#E8E8E8;\">" + condition->getColoredText() + "</div>");
	}
	else // (condition->getOperatorType() == OperatorType_t::identity)
	{
		// Identity represents either a variable or a constant condition
		auto operand = condition->getOperand(0);
		if (operand == nullptr) return;


		this->conditionText->setHtml("<div style=\"background-color:#E8E8E8;color:black;\">" + operand->getText() + "</div>");
	}
}

void GraphicFsmTransition::treatMenu(QAction* action)
{
	if (action->text() == tr("Change source"))
	{
		emit this->dynamicSourceCalledEvent(this->getLogicComponentId());
	}
	else if (action->text() == tr("Change target"))
	{
		emit this->dynamicTargetCalledEvent(this->getLogicComponentId());
	}
	if (action->text() == tr("Edit"))
	{
		emit this->editTransitionCalledEvent(this->getLogicComponentId());
	}
	else if (action->text() == tr("Delete"))
	{
		emit this->deleteTransitionCalledEvent(this->getLogicComponentId());
	}
}

QAbstractGraphicsShapeItem* GraphicFsmTransition::buildSelectionShape()
{
	return new QGraphicsPathItem(this->boundingShape, this);
}

void GraphicFsmTransition::clearArrowBody()
{
	delete this->arrowBody;
	this->arrowBody = nullptr;

	this->clearSelectionShape();
}

void GraphicFsmTransition::buildArrowBody()
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
	if (this->getLogicComponentId() == nullId)
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

		if ( (graphicFsm->getTransitionRank(this->getLogicComponentId()) == 0) && (currentSourceStateId != currentTargetStateId) )
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

void GraphicFsmTransition::buildArrowEnd()
{
	// Initially, arrow will point to north-west
	QPainterPath arrowPath;
	arrowPath.lineTo(GraphicFsmTransition::arrowEndSize, 0);
	arrowPath.moveTo(0, 0);
	arrowPath.lineTo(0, GraphicFsmTransition::arrowEndSize);

	this->arrowEnd = new QGraphicsPathItem(arrowPath, this);
}

void GraphicFsmTransition::refreshChildrenItems()
{
	this->arrowEnd->setRotation(this->arrowEndAngle);
	this->arrowEnd->setPos(this->arrowEndPosition);

	if (this->conditionLine != nullptr)
	{
		this->conditionLine->setRotation(this->conditionLineAngle);
		this->conditionLine->setPos(this->conditionLinePos);
	}
}

void GraphicFsmTransition::refreshExternalItems()
{
	if (this->conditionText != nullptr)
	{
		this->updateConditionText();
		this->conditionText->setPos(mapToScene(this->conditionLinePos) + QPointF(0, 5));
	}

	if (this->actionBox != nullptr)
	{
		this->actionBox->refreshDisplay();
		if (this->conditionText != nullptr)
		{
			this->actionBox->setPos(mapToScene(this->conditionLinePos + this->conditionText->boundingRect().bottomLeft() + QPointF(0, 5)));
		}
	}
}

void GraphicFsmTransition::initializeDefaults()
{
	this->currentConditionPen = new QPen(QBrush(GraphicComponent::defaultBorderColor, Qt::SolidPattern), GraphicComponent::defaultLineThickness);

	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsFocusable);
	this->setFlag(QGraphicsItem::ItemClipsToShape);

	this->setAcceptHoverEvents(true);
}

void GraphicFsmTransition::repaint()
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

void GraphicFsmTransition::rebuildBoundingShape()
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

		QRectF boundRect(0, -GraphicFsmTransition::conditionLineLength/2, straightLine.length(), GraphicFsmTransition::conditionLineLength);

		path.addRect(boundRect);
		path.angleAtPercent(arrowBodyStraightLine->rotation());

		t.rotate(-straightLine.angle());
	}
	else if (this->sourceStateId == this->targetStateId)
	{
		// Auto-transition
		QPainterPath arrowPath = ((QGraphicsPathItem*)arrowBody)->path();

		qreal outterScale = (arrowPath.boundingRect().height() + 3*GraphicFsmTransition::conditionLineLength/4) / arrowPath.boundingRect().height();
		qreal innerScale  = (arrowPath.boundingRect().height() - 3*GraphicFsmTransition::conditionLineLength/4) / arrowPath.boundingRect().height();

		QTransform expand;
		expand.scale(outterScale, outterScale);
		QPainterPath outterPath = expand.map(arrowPath);

		QTransform shrink;
		shrink.scale(innerScale, innerScale);
		QPainterPath innerPath = shrink.map(arrowPath);

		outterPath.translate(0, GraphicFsmTransition::conditionLineLength/2);
		innerPath.translate(0, -GraphicFsmTransition::conditionLineLength/2);

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
		auto neighborhood = graphicFsm->getTransitionNeighborhood(this->getLogicComponentId());

		QPainterPath arrowPath = ((QGraphicsPathItem*)this->arrowBody)->path();

		qreal outterScale = (arrowPath.boundingRect().width() + GraphicFsmTransition::conditionLineLength/2) / arrowPath.boundingRect().width();
		qreal innerScale  = (arrowPath.boundingRect().width() - GraphicFsmTransition::conditionLineLength/2) / arrowPath.boundingRect().width();

		QTransform expand;
		expand.scale(outterScale, outterScale);
		QPainterPath outterPath = expand.map(arrowPath);

		QTransform shrink;
		shrink.scale(innerScale, innerScale);
		QPainterPath innerPath = shrink.map(arrowPath);

		if (graphicFsm->getTransitionRank(this->getLogicComponentId()) > 0)
		{
			outterPath.translate(-(outterPath.boundingRect().width()-arrowPath.boundingRect().width())/2, GraphicFsmTransition::conditionLineLength/2);
			innerPath.translate((arrowPath.boundingRect().width()-innerPath.boundingRect().width())/2, -GraphicFsmTransition::conditionLineLength/2);
		}
		else
		{
			outterPath.translate(-(outterPath.boundingRect().width()-arrowPath.boundingRect().width())/2, -GraphicFsmTransition::conditionLineLength/2);
			innerPath.translate((arrowPath.boundingRect().width()-innerPath.boundingRect().width())/2, GraphicFsmTransition::conditionLineLength/2);
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

void GraphicFsmTransition::drawStraightTransition(QPointF currentSourcePoint, QPointF currentTargetPoint)
{
	//
	// Build arrow body

	QLineF straightLine(QPointF(0, 0), currentTargetPoint - currentSourcePoint);

	// Set line appropriate size (reduce current size by correct number of state radius to point on states border)
	if ( ((this->currentMode == Mode_t::dynamicSourceMode) || (this->currentMode == Mode_t::dynamicTargetMode)) && (this->dynamicStateId == nullId) )
	{
		// Reduce line length to match space between state center and mouse position
		straightLine.setLength(straightLine.length() - GraphicFsmState::getRadius());
	}
	else
	{
		// Reduce line length to match space between states
		straightLine.setLength(straightLine.length() - 2*GraphicFsmState::getRadius());
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
		translationVector.setLength(GraphicFsmState::getRadius());

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

void GraphicFsmTransition::drawAutoTransition(QPointF currentSourcePoint)
{
	//
	// Build arrow body

	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	uint rank;
	if (graphicFsm != nullptr)
	{
		auto neighborhood = graphicFsm->getTransitionNeighborhood(this->getLogicComponentId());
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
	stateCenterToArcStartVector.setLength(GraphicFsmState::getRadius());

	QLineF stateCenterToArcEndVector(0, 0, -1, -1);
	stateCenterToArcEndVector.setLength(GraphicFsmState::getRadius());

	qreal unitaryArcWidth  = 2*GraphicFsmState::getRadius();
	qreal unitaryArcHeight = 2*GraphicFsmState::getRadius();

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

void GraphicFsmTransition::drawCurvedTransition(QPointF currentSourcePoint, QPointF currentTargetPoint)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto neighborhood = graphicFsm->getTransitionNeighborhood(this->getLogicComponentId());

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
	cPointYTranslationVector.setLength(graphicFsm->getTransitionRank(this->getLogicComponentId()) * 150);
	QPointF sceneSystemCPoint(sceneSystemXVector.p2()/2 + cPointYTranslationVector.p2());

	// Use initial C point to calculate new coordinates system
	// Deltas indicates curve start/end positions wrt. local system (start/ends on states perimeter)
	QLineF deltaSystemOriginVector(QPointF(0,0), sceneSystemCPoint);
	deltaSystemOriginVector.setLength(GraphicFsmState::getRadius());

	QLineF deltaTargetVector(QPointF(0,0), sceneSystemCPoint - sceneSystemXVector.p2());
	deltaTargetVector.setLength(GraphicFsmState::getRadius());

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
