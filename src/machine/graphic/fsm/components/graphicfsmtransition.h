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

#ifndef GRAPHICFSMTRANSITION_H
#define GRAPHICFSMTRANSITION_H

// Parent
#include "graphiccomponent.h"

// Qt classes
class QAction;

// StateS classes
#include "statestypes.h"
class ActionBox;


class GraphicFsmTransition : public GraphicComponent
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
private:
	static const qreal arrowEndSize;
	static const qreal conditionLineLength;

	/////
	// Constructors/destructors
public:

	// For persistent build of transition, we already have a logic element to represent
	explicit GraphicFsmTransition(componentId_t logicComponentId);

	// This constructor toggles dynamic mode on target. This is used to display a temporary transition when adding/editing.
	explicit GraphicFsmTransition(componentId_t sourceStateId, componentId_t targetStateId, const QPointF& dynamicMousePosition);

	virtual ~GraphicFsmTransition();

	/////
	// Object functions
public:
	virtual void refreshDisplay() override;

	componentId_t getSourceStateId() const;
	componentId_t getTargetStateId() const;

	void setConditionLineSliderPosition(qreal position);
	qreal getConditionLineSliderPosition() const;

	QGraphicsTextItem* getConditionText() const;

	virtual QPainterPath shape() const override;
	virtual QRectF boundingRect() const override;

	ActionBox* getActionBox() const;

	// Edition related
	void setUnderEdit(bool edit);
	void setDynamicState(componentId_t newDynamicStateId);
	void setMousePosition(const QPointF& mousePos);

protected:
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event)          override;
	virtual void keyPressEvent(QKeyEvent* event)                                  override;
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event)                 override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event)                 override;

	void setArrowColor(QColor color);
	void setConditionColor(QColor color);

private slots:
	void connectedStateMovedEventHandler();
	void updateConditionText();
	void treatMenu(QAction* action);

private:
	virtual QAbstractGraphicsShapeItem* buildSelectionShape() override;

	void clearArrowBody();

	void buildArrowBody();
	void buildArrowEnd();

	void refreshChildrenItems();
	void refreshExternalItems();

	void initializeDefaults();
	void repaint();
	void rebuildBoundingShape();

	void drawStraightTransition(QPointF currentSourcePoint, QPointF currentTargetPoint);
	void drawAutoTransition(QPointF currentSourcePoint);
	void drawCurvedTransition(QPointF currentSourcePoint, QPointF currentTargetPoint);

	/////
	// Signals
signals:
	void editTransitionCalledEvent  (componentId_t transitionId);
	void dynamicSourceCalledEvent   (componentId_t transitionId);
	void dynamicTargetCalledEvent   (componentId_t transitionId);
	void deleteTransitionCalledEvent(componentId_t transitionId);

	/////
	// Object variables
private:
	QPen* currentPen          = nullptr;
	QPen* currentConditionPen = nullptr;

	// A FSM graphic transition must always have at least a source (may not have a target when drawing)
	componentId_t sourceStateId = nullId;
	componentId_t targetStateId = nullId;

	// Dynamic mode
	Mode_t currentMode = Mode_t::errorMode;
	// This will be used if one of the linked state is missing in dynamic mode
	QPointF mousePosition;
	// Dynamic mode holds a temporary state when mouse hovers a state to preview what the result would be if selected
	componentId_t dynamicStateId = nullId;
	bool isUnderEdit = false;

	// Children items
	QGraphicsItem*     arrowBody     = nullptr;
	QGraphicsPathItem* arrowEnd      = nullptr;
	QGraphicsLineItem* conditionLine = nullptr;

	// External items (not children to avoid stacking issues)
	QGraphicsTextItem* conditionText = nullptr;
	ActionBox*         actionBox     = nullptr;

	// Parameters for components
	qreal sceneAngle = 0;

	QPointF conditionLinePos;
	QPointF arrowEndPosition;
	qreal   conditionLineAngle;
	qreal   arrowEndAngle;

	// Retain bounding shape for selection
	QPainterPath boundingShape;

	// Position of the condition line in %
	qreal conditionLineSliderPos = 0.5;

};

#endif // GRAPHICFSMTRANSITION_H

