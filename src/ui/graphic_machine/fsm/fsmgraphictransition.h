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

#ifndef FSMGRAPHICTRANSITION_H
#define FSMGRAPHICTRANSITION_H

// Parents
#include "graphicactuator.h"
#include <QGraphicsItemGroup>

// Qt classes
class QAction;

// StateS classes
#include "statestypes.h"


// FsmGraphicTransition has a dynamic behavior.
// When setting dynamic mode, all changes will be stored in a temporary way
// that overseedes the current parameters, but not overrite them. They only
// takes precedence when existing.
// In the end, keepChanges indicates wheter we should keep
// the changes made, and all appropriate actions will be taken wrt. logic FSM,
// or discard changes and delete dynamic parameters.

class FsmGraphicTransition : public GraphicActuator, public QGraphicsItemGroup
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class Mode_t {errorMode, standardMode, dynamicSourceMode, dynamicTargetMode};

	/////
	// Static functions
public:
	static QPixmap getPixmap(uint size);

	/////
	// Static variables
protected:
	static const QPen defaultPen;

private:
	static const qreal arrowEndSize;
	static const qreal middleBarLength;
	static const QPen editPen;
	static const QPen highlightPen;

	/////
	// Constructors/destructors
public:

	// For persistent build of transition, we already have a logic element to represent
	explicit FsmGraphicTransition(componentId_t logicComponentId);

	// This constructor toggles dynamic mode on target. This is used to display a temporary transition when adding/editing.
	explicit FsmGraphicTransition(componentId_t sourceStateId, componentId_t targetStateId, const QPointF& dynamicMousePosition);

	~FsmGraphicTransition();

	/////
	// Object functions
public:
	virtual void refreshDisplay() override;

	componentId_t getSourceStateId() const;
	componentId_t getTargetStateId() const;

	void setConditionLineSliderPosition(qreal position); // Todo: throw exception
	qreal getConditionLineSliderPosition() const;

	QGraphicsTextItem* getConditionText() const;

	virtual QPainterPath shape() const override;
	virtual QRectF boundingRect() const override;

	// Edition related
	void setUnderEdit(bool edit);
	void setDynamicState(componentId_t newDynamicStateId);
	void setMousePosition(const QPointF& mousePos);

signals:
	void editTransitionCalledEvent  (componentId_t transitionId);
	void dynamicSourceCalledEvent   (componentId_t transitionId);
	void dynamicTargetCalledEvent   (componentId_t transitionId);
	void deleteTransitionCalledEvent(componentId_t transitionId);

protected:
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event)          override;
	virtual void keyPressEvent(QKeyEvent* event)                                  override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev)                    override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private slots:
	void transitionNeedsRefreshEventHandler();
	void updateConditionText();
	void treatMenu(QAction* action);

private:
	void clearRepresentation();
	void buildRepresentation();
	void initializeDefaults();
	void buildArrowEnd();
	void repositionArrowEnd(qreal angle, const QPointF& position);
	void rebuildBoundingShape();
	void updateSelectionShapeDisplay();

	virtual void updateActionBoxPosition() override;

	void drawStraightTransition(QPointF currentSourcePoint, QPointF currentTargetPoint);
	void drawAutoTransition(QPointF currentSourcePoint);
	void drawCurvedTransition(QPointF currentSourcePoint, QPointF currentTargetPoint);

	/////
	// Object variables
protected:
	const QPen* currentPen          = nullptr;
	const QPen* currentConditionPen = nullptr;

private:
	// A FSM graphic transition must always have at least a source (may not have a target when drawing)
	componentId_t sourceStateId = 0;
	componentId_t targetStateId = 0;

	// Dynamic mode
	Mode_t currentMode;
	// This will be used if one of the linked state is missing in dynamic mode
	QPointF mousePosition;
	// Dynamic mode holds a temporary state when mouse hovers a state to preview what the result would be if selected
	componentId_t dynamicStateId = 0;

	// Base elements of the arrow
	QGraphicsItem*     arrowBody      = nullptr;
	QGraphicsPathItem* arrowEnd       = nullptr;
	QGraphicsLineItem* conditionLine  = nullptr;
	QGraphicsTextItem* conditionText  = nullptr;
	QGraphicsPathItem* selectionShape = nullptr;

	qreal sceneAngle = 0;

	QPointF autoTransitionConditionPosition;

	QPainterPath boundingShape;

	qreal conditionLineSliderPos;
	QPointF conditionLinePos;

};

#endif // FSMGRAPHICTRANSITION_H

