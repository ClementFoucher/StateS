/*
 * Copyright © 2014-2017 Clément Foucher
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

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsView>

// States classes
#include "fsmgraphicstate.h"
#include "fsmstate.h"
#include "machine.h"
#include "fsmtransition.h"
#include "fsmgraphictransitionneighborhood.h"
#include "StateS_signal.h"
#include "contextmenu.h"
#include "fsm.h"
#include "statesexception.h"
#include "equation.h"


qreal FsmGraphicTransition::arrowEndSize = 10;
qreal FsmGraphicTransition::middleBarLength = 20;
QPen FsmGraphicTransition::standardPen = QPen(Qt::SolidPattern, 3);
QPen FsmGraphicTransition::editPen = QPen(QBrush(Qt::red, Qt::SolidPattern), 3);
QPen FsmGraphicTransition::activePen = QPen(QBrush(Qt::darkGreen, Qt::SolidPattern), 3);
QPen FsmGraphicTransition::inactivePen = QPen(QBrush(Qt::red, Qt::SolidPattern), 3);

QPixmap FsmGraphicTransition::getPixmap(uint size)
{
	QPixmap pixmap(QSize(size, size));
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);

	painter.setPen(FsmGraphicTransition::standardPen);
	painter.drawLine(0, 0, size, size);
	painter.drawLine(0, 0, size/3, 0);
	painter.drawLine(0, 0, 0, size/3);

	return pixmap;
}

FsmGraphicTransition::FsmGraphicTransition()
{
	this->currentPen = &standardPen;
	this->conditionLineSliderPos = 0.5;

	this->conditionText = new QGraphicsTextItem();

	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsFocusable);
	this->setFlag(QGraphicsItem::ItemClipsToShape);
}

FsmGraphicTransition::FsmGraphicTransition(FsmGraphicState* source, const QPointF& dynamicMousePosition) :
    FsmGraphicTransition()
{
	this->currentMode = mode::initMode;
	this->setSourceState(source);
	this->setDynamicTargetMode(dynamicMousePosition);
	// We are now in dynamic target mode

	this->finishInitialize();
}

FsmGraphicTransition::FsmGraphicTransition(shared_ptr<FsmTransition> logicTransition) :
    FsmGraphicTransition()
{
	this->setLogicTransition(logicTransition);

	shared_ptr<Fsm> machine = logicTransition->getOwningFsm();
	connect(machine.get(), &Fsm::simulationModeChangedEvent, this, &FsmGraphicTransition::machineModeChangedEventHandler);

	currentMode = mode::initMode;
	this->setSourceState(logicTransition->getSource()->getGraphicRepresentation());
	this->setTargetState(logicTransition->getTarget()->getGraphicRepresentation());
	currentMode = mode::standardMode;

	this->checkNeighbors();
	this->finishInitialize();
}

void FsmGraphicTransition::finishInitialize()
{
	this->rebuildArrowEnd();
	this->updateText();
	this->updateDisplay();
}

FsmGraphicTransition::~FsmGraphicTransition()
{
	this->quitNeighborhood();

	delete conditionText;

	shared_ptr<FsmTransition> l_logicTransition = this->getLogicTransition();

	if (l_logicTransition != nullptr)
	{
		disconnect(l_logicTransition.get(), &MachineComponent::componentNeedsGraphicUpdateEvent,    this, &FsmGraphicTransition::updateText);
		disconnect(l_logicTransition.get(), &MachineComponent::componentSimulatedStateChangedEvent, this, &FsmGraphicTransition::updateText);

		shared_ptr<Fsm> machine = l_logicTransition->getOwningFsm();
		if (machine != nullptr)
		{
			disconnect(machine.get(), &Fsm::simulationModeChangedEvent, this, &FsmGraphicTransition::machineModeChangedEventHandler);
		}
	}
}

void FsmGraphicTransition::setLogicTransition(shared_ptr<FsmTransition> transition)
{
	if (this->getLogicTransition() == nullptr)
	{
		this->setLogicActuator(dynamic_pointer_cast<MachineActuatorComponent>(transition)); // Throws StatesException - ignored as we checked nullness

		connect(transition.get(), &MachineComponent::componentNeedsGraphicUpdateEvent,    this, &FsmGraphicTransition::updateText);
		connect(transition.get(), &MachineComponent::componentSimulatedStateChangedEvent, this, &FsmGraphicTransition::updateText);

		this->currentMode = mode::initMode;
		this->setSourceState(transition->getSource()->getGraphicRepresentation());
		this->setTargetState(transition->getTarget()->getGraphicRepresentation());
		this->currentMode = mode::standardMode;

		this->refreshNeighborhood();
		this->finishInitialize();
	}
}

shared_ptr<FsmTransition> FsmGraphicTransition::getLogicTransition() const
{
	return dynamic_pointer_cast<FsmTransition>(this->getLogicActuator());
}

bool FsmGraphicTransition::setSourceState(FsmGraphicState* newSource)
{
	this->autoTransitionNeedsRedraw = true;

	if (this->currentMode == mode::initMode)
	{
		this->source = newSource;

		// Connect to new state
		connect(this->source, &FsmGraphicState::stateMovedEvent, this, &FsmGraphicTransition::updateDisplay);

		return true;
	}
	else if (this->currentMode == mode::standardMode)
	{
		if (newSource == nullptr) // We can't change source for nullptr
			return false;
		else if (newSource == this->source)
			return true;
		else
		{
			// Disconnect existing signal
			// If we were a single-state transition, do not disconnect as we still have target connected!
			if (this->source != this->target)
				disconnect(this->source, &FsmGraphicState::stateMovedEvent, this, &FsmGraphicTransition::updateDisplay);

			// If dynamic target state exists, we are already in the right neighborhood
			if (this->dynamicState != newSource)
				this->quitNeighborhood();

			this->source = newSource;

			// Connect to new state
			connect(this->source, &FsmGraphicState::stateMovedEvent, this, &FsmGraphicTransition::updateDisplay);

			// If dynamic target state exists, we are already in the right neighborhood
			if (this->dynamicState != newSource)
			{
				this->checkNeighbors();
				this->updateDisplay();
			}

			return true;
		}
	}
	else if (this->currentMode == mode::dynamicSourceMode)
	{
		if (newSource == this->dynamicState)
			return true;
		else
		{
			this->quitNeighborhood();

			this->dynamicState = newSource;

			this->checkNeighbors();
			this->updateDisplay();

			return true;
		}
	}
	else
		return false;

}

bool FsmGraphicTransition::setTargetState(FsmGraphicState* newTarget)
{
	this->autoTransitionNeedsRedraw = true;

	if (this->currentMode == mode::initMode)
	{
		this->target = newTarget;

		// Connect to new state
		connect(this->target, &FsmGraphicState::stateMovedEvent, this, &FsmGraphicTransition::updateDisplay);

		return true;
	}
	else if (this->currentMode == mode::standardMode)
	{
		if (newTarget == nullptr) // We can't change target for nullptr
			return false;
		else if (newTarget == this->target)
			return true;
		else
		{
			// Disconnect existing signal, if existing (state can be created without target graphically)
			if (this->target != nullptr)
			{
				// If we were a single-state transition, do not disconnect as we still have source connected!
				if (this->source != this->target)
					disconnect(this->target, &FsmGraphicState::stateMovedEvent, this, &FsmGraphicTransition::updateDisplay);
			}

			// If dynamic target state exists, we are already in the right neighborhood
			if (this->dynamicState != newTarget)
				this->quitNeighborhood();


			this->target = newTarget;

			// Connect to new state
			connect(this->target, &FsmGraphicState::stateMovedEvent, this, &FsmGraphicTransition::updateDisplay);

			// If dynamic target state exists, we are already in the right neighborhood
			if (this->dynamicState != newTarget)
			{
				this->checkNeighbors();
				this->updateDisplay();
			}

			return true;
		}
	}
	else if (this->currentMode == mode::dynamicTargetMode)
	{
		if (newTarget == this->dynamicState)
			return true;
		else
		{
			this->quitNeighborhood();
			this->dynamicState = newTarget;
			this->checkNeighbors();
			this->updateDisplay();

			return true;
		}
	}
	else
		return false;
}

void FsmGraphicTransition::setMousePosition(const QPointF& mousePos)
{
	this->dynamicState = nullptr;
	this->mousePosition = mousePos;

	this->quitNeighborhood();
	this->updateDisplay();
}

FsmGraphicState* FsmGraphicTransition::getSource() const
{
	return this->source;
}

FsmGraphicState* FsmGraphicTransition::getTarget() const
{
	return this->target;
}

void FsmGraphicTransition::rebuildArrowEnd()
{
	delete this->arrowEnd;
	this->arrowEnd = new QGraphicsItemGroup(this);

	//QPainterPath arrowPath;
	QGraphicsLineItem* arrowEnd1 = new QGraphicsLineItem(this->arrowEnd);
	QGraphicsLineItem* arrowEnd2 = new QGraphicsLineItem(this->arrowEnd);

	arrowEnd1->setPen(*this->currentPen);
	arrowEnd2->setPen(*this->currentPen);

	// Original arrow point north-west
	arrowEnd1->setLine(0, 0, this->arrowEndSize, 0);
	arrowEnd2->setLine(0, 0, 0, this->arrowEndSize);
}

void FsmGraphicTransition::rebuildBoundingShape()
{
	this->prepareGeometryChange();

	QGraphicsLineItem* arrowBodyStraightLine = dynamic_cast<QGraphicsLineItem*>(this->arrowBody);
	QGraphicsPathItem* arrowBodyPath = dynamic_cast<QGraphicsPathItem*>(this->arrowBody);

	QPainterPath path;
	QTransform t;

	if (arrowBodyStraightLine != nullptr)
	{
		QLineF straightLine = arrowBodyStraightLine->line();

		QRectF boundRect(0, -middleBarLength/2, straightLine.length(), middleBarLength);

		path.addRect(boundRect);
		path.angleAtPercent(arrowBodyStraightLine->rotation());

		t.rotate(-straightLine.angle());
	}
	else if (this->neighbors == nullptr)
	{
		// Auto-transition
		QPainterPath arrowPath = ((QGraphicsPathItem*)arrowBody)->path();

		qreal outterScale = (arrowPath.boundingRect().height() + 3*middleBarLength/4) / arrowPath.boundingRect().height();
		qreal innerScale  = (arrowPath.boundingRect().height() - 3*middleBarLength/4) / arrowPath.boundingRect().height();

		QTransform expand;
		expand.scale(outterScale, outterScale);
		QPainterPath outterPath = expand.map(arrowPath);

		QTransform shrink;
		shrink.scale(innerScale, innerScale);
		QPainterPath innerPath = shrink.map(arrowPath);

		outterPath.translate(0, middleBarLength/2);
		innerPath.translate(0, -middleBarLength/2);

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

		// Special case: the path can actually be a straight line
		if ( (this->neighbors != nullptr) && (this->neighbors->count() % 2 == 1) && (this->neighbors->whatIsMyRank(this) == 0) )
		{
			QLineF straightLine = QLineF(QPointF(0, 0), arrowBodyPath->boundingRect().bottomRight());
			QRectF boundRect(0, -middleBarLength/2, straightLine.length(), middleBarLength);

			path.addRect(boundRect);
		}
		else
		{
			QPainterPath arrowPath = ((QGraphicsPathItem*)this->arrowBody)->path();

			qreal outterScale = (arrowPath.boundingRect().width() + middleBarLength/2) / arrowPath.boundingRect().width();
			qreal innerScale  = (arrowPath.boundingRect().width() - middleBarLength/2) / arrowPath.boundingRect().width();

			QTransform expand;
			expand.scale(outterScale, outterScale);
			QPainterPath outterPath = expand.map(arrowPath);

			QTransform shrink;
			shrink.scale(innerScale, innerScale);
			QPainterPath innerPath = shrink.map(arrowPath);

			if (this->neighbors->computeTransitionPosition(this) > 0)
			{
				outterPath.translate(-(outterPath.boundingRect().width()-arrowPath.boundingRect().width())/2, middleBarLength/2);
				innerPath.translate((arrowPath.boundingRect().width()-innerPath.boundingRect().width())/2, -middleBarLength/2);
			}
			else
			{
				outterPath.translate(-(outterPath.boundingRect().width()-arrowPath.boundingRect().width())/2, -middleBarLength/2);
				innerPath.translate((arrowPath.boundingRect().width()-innerPath.boundingRect().width())/2, middleBarLength/2);
			}

			innerPath = innerPath.toReversed();

			path.moveTo(outterPath.pointAtPercent(0));
			path.connectPath(outterPath);
			path.lineTo(innerPath.pointAtPercent(0));
			path.connectPath(innerPath);

			path.closeSubpath();
		}

		t.rotate(-this->sceneAngle);
	}

	this->boundingShape = t.map(path);

	this->update();
}

void FsmGraphicTransition::updateSelectionShapeDisplay()
{
	delete this->selectionShape;
	this->selectionShape = nullptr;

	if (this->isSelected())
	{
		this->selectionShape = new QGraphicsPathItem(this->boundingShape, this);
		this->selectionShape->setPen(selectionPen);
	}
}

void FsmGraphicTransition::updateText()
{
	shared_ptr<FsmTransition> l_logicTransition = this->getLogicTransition();

	//
	// Condition

	// Should also make background semi-transparent... (we could avoid color?)
	if (l_logicTransition != nullptr)
	{
		Machine::simulation_mode currentMode = l_logicTransition->getOwningFsm()->getCurrentSimulationMode();

		if ( (scene() != nullptr) && (currentMode == Machine::simulation_mode::simulateMode) )
		{
			if (l_logicTransition->getCondition() == nullptr)
				conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");
			else
			{
				shared_ptr<Equation> equationCondition = dynamic_pointer_cast<Equation>(l_logicTransition->getCondition());

				if (equationCondition != nullptr)
					this->conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + equationCondition->getColoredText(true, true) + "</div>");
				else
					this->conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + l_logicTransition->getCondition()->getColoredText(true) + "</div>");
			}

			if (conditionLine != nullptr)
			{
				shared_ptr<Signal> condition = l_logicTransition->getCondition();
				if (condition != nullptr)
				{
					try
					{
						if (condition->isTrue())
							this->conditionLine->setPen(activePen);
						else
							this->conditionLine->setPen(inactivePen);
					}
					catch (const StatesException& e)
					{
						if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::signal_is_not_bool) )
						{
							// Condition is incorrect, considered false
							this->conditionLine->setPen(inactivePen);
						}
						else
							throw;
					}
				}
				else
				{
					// Missing condition is implicitly 1
					this->conditionLine->setPen(activePen);
				}
			}
		}
		else
		{
			if (l_logicTransition->getCondition() == nullptr)
				this->conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");
			else
			{
				shared_ptr<Equation> equationCondition = dynamic_pointer_cast<Equation>(l_logicTransition->getCondition());

				if (equationCondition != nullptr)
					this->conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + equationCondition->getColoredText(false, true) + "</div>");
				else
					this->conditionText->setHtml("<div style='background-color:#E8E8E8;'>" + l_logicTransition->getCondition()->getText() + "</div>");
			}
			if (this->conditionLine != nullptr)
				this->conditionLine->setPen(standardPen);
		}
	}
	else
		this->conditionText->setHtml("<div style='background-color:#E8E8E8;'>1</div>");

	//
	// Actions

	this->buildActionsBox(*this->currentPen, false);
}

void FsmGraphicTransition::updateDisplay()
{
	//
	// First deal with mates as this function can be called by a neighborhood change
	if (this->neighbors != nullptr)
	{
		if (this->neighbors->count() == 1)
		{
			this->neighbors.reset();
		}
	}

	//
	// Determine source and target points depending on mode.
	// For ends that are connected to a state, center is used as a first approximation
	QPointF currentSourcePoint;
	QPointF currentTargetPoint;
	FsmGraphicState* currentSourceState = nullptr;
	FsmGraphicState* currentTargetState = nullptr;

	if (this->currentMode == mode::standardMode)
	{
		currentSourcePoint = this->source->scenePos();
		currentTargetPoint = this->target->scenePos();

		currentSourceState = this->source;
		currentTargetState = this->target;
	}
	else if (this->currentMode == mode::dynamicSourceMode)
	{
		if (this->dynamicState != nullptr)
		{
			currentSourcePoint = this->dynamicState->scenePos();
			currentSourceState = this->dynamicState;
		}
		else
		{
			currentSourcePoint = this->mousePosition;
			currentSourceState = nullptr;
		}

		currentTargetPoint = this->target->scenePos();
		currentTargetState = this->target;
	}
	else if (this->currentMode == mode::dynamicTargetMode)
	{
		currentSourcePoint = this->source->scenePos();
		currentSourceState = this->source;

		if (this->dynamicState != nullptr)
		{
			currentTargetPoint = this->dynamicState->scenePos();
			currentTargetState = this->dynamicState;
		}
		else
		{
			currentTargetPoint = this->mousePosition;
			currentTargetState = nullptr;
		}
	}

	//
	// Redraw arrow body
	QPointF conditionLinePos;

	if ( (this->neighbors == nullptr) && (currentSourceState != currentTargetState) )
	{
		delete this->conditionLine;
		delete this->arrowBody;

		this->conditionLine = nullptr;
		this->arrowBody     = nullptr;

		// If no mates, draw a straight line

		QLineF straightLine(QPointF(0, 0), currentTargetPoint - currentSourcePoint);

		// Set line appropriate size (reduce current size by correct number of state radius to point on states border)
		if ( ((this->currentMode == mode::dynamicSourceMode) || (this->currentMode == mode::dynamicTargetMode)) && (this->dynamicState == nullptr) )
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
		//   line->setFlag(QGraphicsItem::ItemIsSelectable);
		line->setPen(*this->currentPen);
		line->setLine(straightLine);
		this->arrowBody = line;

		// Display condition
		QLineF conditionLineF = straightLine.normalVector();
		conditionLineF.setLength(middleBarLength);
		this->conditionLine = new QGraphicsLineItem(conditionLineF, this);
		this->conditionLine->setPen(*this->currentPen);
		// Determine position
		conditionLinePos = QPointF(straightLine.p2()*this->conditionLineSliderPos);
		this->conditionLine->setPos(conditionLinePos - conditionLineF.p2()/2);

		//
		// Update positions with actual ones used for construction

		// If source is a state, line should be translated to begin on state border
		if ( !((this->currentMode == mode::dynamicSourceMode) && (this->dynamicState == nullptr)) )
		{
			// Translation vector is based on straight line (same vector) and normalized to required translation length
			QLineF translationVector(straightLine);
			translationVector.setLength(FsmGraphicState::getRadius());

			currentSourcePoint += translationVector.p2();
		}

		// Current target position is calculated based on source point translated by line
		currentTargetPoint = currentSourcePoint + straightLine.p2();

		// Compute arrow end rotation based on line angle
		this->arrowEnd->setRotation(135-straightLine.angle());
		// Set arrow position to be at target side
		this->arrowEnd->setPos(currentTargetPoint - currentSourcePoint); // Positions are intended in scene coordinates. Relocate wrt. this item coordinates: (0, 0) is at source point position

		this->setPos(currentSourcePoint);
	}
	else if (currentSourceState == currentTargetState)
	{
		// Auto-transition

		if (this->autoTransitionNeedsRedraw)
		{
			delete this->conditionLine;
			delete this->arrowBody;

			this->conditionLine = nullptr;
			this->arrowBody     = nullptr;

			uint rank;
			if (this->neighbors != nullptr)
				rank = this->neighbors->whatIsMyRank(this);
			else
				rank = 0;

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
			arcItem->setPen(*this->currentPen);
			this->arrowBody = arcItem;

			QLineF conditionLineF(0, -middleBarLength/2, 0, 1);
			conditionLineF.setLength(middleBarLength);
			this->conditionLine = new QGraphicsLineItem(conditionLineF, arcItem);
			this->conditionLine->setPen(*this->currentPen);

			this->autoTransitionConditionPosition = arc.pointAtPercent(this->conditionLineSliderPos);
			this->conditionLine->setPos(this->autoTransitionConditionPosition);
			this->conditionLine->setRotation(-arc.angleAtPercent(this->conditionLineSliderPos));

			this->arrowEnd->setPos(stateCenterToArcEndVector.p2());
			this->arrowEnd->setRotation(180);

			this->autoTransitionNeedsRedraw = false;
		}

		conditionLinePos = this->autoTransitionConditionPosition;
		this->setPos(currentSourcePoint);
	}
	else
	{
		delete this->conditionLine;
		delete this->arrowBody;

		this->conditionLine = nullptr;
		this->arrowBody     = nullptr;

		// Take mates in consideration to draw a curve
		// All mates will have a curved body, which curve intensity
		// depends on their rank in mates

		// Get actual source and target in mates's coordinate system
		if ( ((this->currentMode == mode::standardMode)      && (this->neighbors->getSource() != this->source)) ||
		     ((this->currentMode == mode::dynamicTargetMode) && (this->neighbors->getSource() != this->source)) ||
		     ((this->currentMode == mode::dynamicSourceMode) && (this->neighbors->getSource() != this->dynamicState))
		   )
		{
			QPointF temp = currentTargetPoint;
			currentTargetPoint = currentSourcePoint;
			currentSourcePoint = temp;
		}

		// Build arrow body

		QPointF deltaCurveOrigin;
		QPointF curveTarget;
		qreal endAngle1;
		qreal endAngle2;
		this->arrowBody = this->neighbors->buildMyBody(this->currentPen, this, deltaCurveOrigin, conditionLinePos, curveTarget, endAngle1, endAngle2, &this->conditionLine, this->conditionLineSliderPos);

		this->sceneAngle = QLineF(QPointF(0,0), curveTarget).angle();

		// Place arrow end on correct side

		if (((this->currentMode == mode::standardMode)      && (this->neighbors->getSource() == this->source)) ||
		    ((this->currentMode == mode::dynamicTargetMode) && (this->neighbors->getSource() == this->source)) ||
		    ((this->currentMode == mode::dynamicSourceMode) && (this->neighbors->getSource() == this->dynamicState)))
		{
			this->arrowEnd->setPos(curveTarget);
			this->arrowEnd->setRotation(135-endAngle1);
		}
		else
		{
			this->arrowEnd->setPos(0, 0);
			this->arrowEnd->setRotation(135-endAngle2);
		}

		this->setPos(currentSourcePoint+deltaCurveOrigin);

		// Must be done after scene mapping

	}

	this->conditionText->setPos(mapToScene(conditionLinePos) + QPointF(0, 5));

	QGraphicsItemGroup* actionBox = this->getActionsBox();
	if (actionBox != nullptr)
		actionBox->setPos(mapToScene(conditionLinePos + this->conditionText->boundingRect().bottomLeft() + QPointF(0, 5)));

	this->rebuildBoundingShape();
	this->updateSelectionShapeDisplay();
}

bool FsmGraphicTransition::setDynamicSourceMode(const QPointF& mousePosition)
{
	if (this->currentMode != mode::standardMode)
		return false;
	else
	{
		this->currentMode = mode::dynamicSourceMode;
		this->currentPen = &editPen;
		rebuildArrowEnd();

		setMousePosition(mousePosition);

		return true;
	}
}

bool FsmGraphicTransition::setDynamicTargetMode(const QPointF& mousePosition)
{
	if ( (this->currentMode != mode::standardMode) && (this->currentMode != mode::initMode) )
		return false;
	else
	{
		this->currentMode = mode::dynamicTargetMode;
		this->currentPen = &editPen;
		rebuildArrowEnd();

		setMousePosition(mousePosition);

		return true;
	}
}

bool FsmGraphicTransition::endDynamicMode(bool keepChanges)
{
	shared_ptr<FsmTransition> l_logicTransition = this->getLogicTransition();

	this->autoTransitionNeedsRedraw = true;

	if (keepChanges)
	{
		if (this->dynamicState == nullptr)
		{
			return false;
		}
		else if (this->currentMode == mode::dynamicSourceMode)
		{
			this->currentMode = mode::standardMode;
			this->setSourceState(this->dynamicState);
			if (l_logicTransition != nullptr)
			{
				shared_ptr<Fsm> fsm = l_logicTransition->getOwningFsm();
				fsm->redirectTransition(l_logicTransition, this->dynamicState->getLogicState(), l_logicTransition->getTarget());
			}
		}
		else if (currentMode == mode::dynamicTargetMode)
		{
			this->currentMode = mode::standardMode;
			this->setTargetState(dynamicState);
			if (l_logicTransition != nullptr)
			{
				shared_ptr<Fsm> fsm = l_logicTransition->getOwningFsm();
				fsm->redirectTransition(l_logicTransition, l_logicTransition->getSource(), this->dynamicState->getLogicState());
			}
		}
	}

	this->dynamicState = nullptr;
	if (!keepChanges)
	{
		this->currentMode = mode::standardMode;
		this->refreshNeighborhood();
	}

	this->currentPen = &standardPen;

	rebuildArrowEnd();
	updateDisplay();

	return true;
}

void FsmGraphicTransition::checkNeighbors()
{
	// Algo is as follows:
	// Check all outcoming transitions from both ends' states (except me)
	// If none => all right, no precautions needed
	// If 1 => initiate a mates secrete society
	// If more than one => join mates secrete society

	FsmGraphicState* actualSourceState;
	FsmGraphicState* actualTargetState;

	if (this->currentMode == mode::dynamicSourceMode)
	{
		actualSourceState = this->dynamicState;
		actualTargetState = this->target;
	}
	else if (this->currentMode == mode::dynamicTargetMode)
	{
		actualSourceState = this->source;
		actualTargetState = this->dynamicState;
	}
	else
	{
		actualSourceState = this->source;
		actualTargetState = this->target;
	}

	shared_ptr<FsmTransition> newFriend;

	foreach (shared_ptr<FsmTransition> neighboor, actualSourceState->getLogicState()->getOutgoingTransitions())
	{
		if ( (neighboor->getGraphicRepresentation() != nullptr) && (neighboor->getGraphicRepresentation() != this) ) // Needed, as in dynamic mode I still belong to states!
		{
			if (neighboor->getTarget() == actualTargetState->getLogicState())
			{
				newFriend = neighboor;
				break;
			}
		}
	}

	if (newFriend == nullptr)
	{
		foreach (shared_ptr<FsmTransition> neighboor, actualTargetState->getLogicState()->getOutgoingTransitions())
		{
			if ( (neighboor->getGraphicRepresentation() != nullptr) && (neighboor->getGraphicRepresentation() != this) ) // Needed, as in dynamic mode I still belong to states!
			{
				if (neighboor->getTarget() == actualSourceState->getLogicState())
				{
					newFriend= neighboor;
					break;
				}
			}
		}
	}

	if (newFriend != nullptr)
	{
		setNeighborhood(newFriend->getGraphicRepresentation()->helloIMYourNewNeighbor());
	}
}

shared_ptr<FsmGraphicTransitionNeighborhood> FsmGraphicTransition::helloIMYourNewNeighbor()
{
	if (this->neighbors == nullptr)
	{
		shared_ptr<FsmGraphicTransitionNeighborhood> newNeighborhood(new FsmGraphicTransitionNeighborhood(this->source, this->target));
		this->setNeighborhood(newNeighborhood);
		// Redraw useless (harmful?) as there is currenly only me in mates.
		// Redraw will be automatic on mate joining neighborhood
	}

	return this->neighbors;
}

void FsmGraphicTransition::setConditionLineSliderPosition(qreal position)
{
	if ( (position >= 0) && (position <= 1) )
	{
		this->conditionLineSliderPos = position;
		this->autoTransitionNeedsRedraw = true;
		emit transitionSliderPositionChangedEvent();
		this->updateDisplay();
	}
}

qreal FsmGraphicTransition::getConditionLineSliderPosition() const
{
	return this->conditionLineSliderPos;
}

void FsmGraphicTransition::setNeighborhood(shared_ptr<FsmGraphicTransitionNeighborhood> neighborhood)
{
	// Should not happen if already having one.
	// Maybe we should test it?
	this->neighbors = neighborhood;

	neighborhood->insertAndNotify(this);
	connect(this->neighbors.get(), &FsmGraphicTransitionNeighborhood::contentChangedEvent, this, &FsmGraphicTransition::updateDisplay);

	this->updateSelectionShapeDisplay();
}

void FsmGraphicTransition::quitNeighborhood()
{
	// Algo is as follows:
	// If no mates => nothing to do!
	// Else quit neighborhood. This will notify remaning members,
	// and if only one remaining, it will disolve neighborhood

	if (this->neighbors != nullptr)
	{
		disconnect(this->neighbors.get(), &FsmGraphicTransitionNeighborhood::contentChangedEvent, this, &FsmGraphicTransition::updateDisplay);
		this->neighbors->removeAndNotify(this);
		this->neighbors = nullptr;
	}
}

void FsmGraphicTransition::refreshNeighborhood()
{
	this->quitNeighborhood();
	this->checkNeighbors();
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

void FsmGraphicTransition::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	if (this->currentMode == mode::standardMode)
	{
		ContextMenu* menu = new ContextMenu();
		menu->addTitle(tr("Transition"));

		menu->addAction(tr("Change source"));
		menu->addAction(tr("Change target"));
		menu->addAction(tr("Edit"));
		menu->addAction(tr("Delete"));
		menu->popup(event->screenPos());

		connect(menu, &QMenu::triggered, this, &FsmGraphicTransition::treatMenu);
	}
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
		shared_ptr<FsmTransition> l_logicTransition = this->getLogicTransition();
		shared_ptr<Fsm> fsm = l_logicTransition->getOwningFsm();

		if (l_logicTransition != nullptr)
		{
			fsm->removeTransition(l_logicTransition);
		}
	}
	else
	{
		event->ignore();
	}
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

	return QGraphicsItem::itemChange(change, value);
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

void FsmGraphicTransition::treatMenu(QAction* action)
{
	if (action->text() == tr("Change source"))
		emit dynamicSourceCalledEvent(this);
	else if (action->text() == tr("Change target"))
		emit dynamicTargetCalledEvent(this);
	if (action->text() == tr("Edit"))
		emit editCalledEvent(this->getLogicTransition());
	else if (action->text() == tr("Delete"))
	{
		// This call will destroy the current object as consequence of the logic object destruction

		shared_ptr<FsmTransition> l_logicTransition = this->getLogicTransition();
		shared_ptr<Fsm> fsm = l_logicTransition->getOwningFsm();

		if (l_logicTransition != nullptr)
		{
			fsm->removeTransition(l_logicTransition);
		}
	}
}

void FsmGraphicTransition::machineModeChangedEventHandler(Machine::simulation_mode)
{
	this->updateText();
}
