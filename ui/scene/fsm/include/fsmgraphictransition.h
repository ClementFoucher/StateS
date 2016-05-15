/*
 * Copyright © 2014-2015 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QAction;

// StateS classes
#include "machine.h"
class FsmGraphicState;
class FsmTransition;
class FsmGraphicTransitionNeighborhood;

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

public:
    enum class mode{initMode, standardMode, dynamicSourceMode, dynamicTargetMode};

    // Static
public:
    static QPixmap getPixmap(uint size);

private:
    // Private constructor for handling common initialization elements
    explicit FsmGraphicTransition();

public:
    // This is static build of transition, we already have a logic element to represent
    explicit FsmGraphicTransition(shared_ptr<FsmTransition> logicTransition);
    // This constructor toggles dynamic mode on target. This is used when drawing a transition graphically
    explicit FsmGraphicTransition(FsmGraphicState* source, const QPointF& dynamicMousePosition);
    ~FsmGraphicTransition();

    void setLogicTransition(shared_ptr<FsmTransition> transition);
    shared_ptr<FsmTransition> getLogicTransition() const;

    bool setSourceState(FsmGraphicState* newSource);
    bool setTargetState(FsmGraphicState* newTarget);

    FsmGraphicState* getSource() const;
    FsmGraphicState* getTarget() const;

    void setMousePosition(const QPointF& mousePos);
    void setTargetMousePosition(const QPointF& newTarget);

    bool setDynamicSourceMode(const QPointF& mousePosition);
    bool setDynamicTargetMode(const QPointF& mousePosition);
    bool endDynamicMode(bool keepChanges);

    shared_ptr<FsmGraphicTransitionNeighborhood> helloIMYourNewNeighbor();

    void setConditionLineSliderPosition(qreal position); // Todo: throw exception
    qreal getConditionLineSliderPosition();

    QGraphicsTextItem* getConditionText() const;
    QPainterPath shape() const override;
    QRectF boundingRect() const override;

signals:
    void editCalledEvent(shared_ptr<FsmTransition>);
    void dynamicSourceCalledEvent(FsmGraphicTransition*);
    void dynamicTargetCalledEvent(FsmGraphicTransition*);
    void graphicTransitionEdited();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private slots:
    void updateDisplay();
    void updateText();
    void treatMenu(QAction* action);
    void machineModeChangedEventHandler(Machine::mode);

private:
    void finishInitialize();
    void rebuildArrowEnd();
    void rebuildBoundingShape();
    void updateSelectionShapeDisplay();

    void checkNeighboors();
    void setNeighbors(shared_ptr<FsmGraphicTransitionNeighborhood> neighborhood);
    void quitNeighboorhood();  // Ohhh... So sad

    //
    weak_ptr<FsmTransition> logicTransition;

    // A FSM graphic transition must always have at least a source (may not have target when first drawing)
    FsmGraphicState* source = nullptr;
    FsmGraphicState* target = nullptr;

    // Dynamic mode
    mode currentMode;
    // This will be used if one of the linked state is missing in dynamic mode
    QPointF mousePosition;
    // Dynamic mode holds a temporary state when mouse hovers a state to preview what would be the result if selected
    FsmGraphicState* dynamicState = nullptr;

    // Base elements of the arrow
    QGraphicsItem*      arrowBody     = nullptr;
    QGraphicsItem*      arrowEnd      = nullptr;
    QGraphicsLineItem*  conditionLine = nullptr;
    QGraphicsTextItem*  conditionText = nullptr;
//    QGraphicsRectItem*  selectionBox  = nullptr; // Manually deal with selection box as Qt has trouble displaying it with complex shapes
    QGraphicsPathItem* selectionShape  = nullptr;
    qreal sceneAngle = 0;

    // This list is shared by all graphic transitions that have same {source, parent} couple (either direction)
    shared_ptr<FsmGraphicTransitionNeighborhood> neighbors;

    QPen* currentPen = nullptr;

    // To avoid redrawing when state moves: useless
    bool autoTransitionNeedsRedraw = true;
    QPointF autoTransitionConditionPosition;

    QPainterPath boundingShape;

    qreal conditionLineSliderPos;

private:
    // These static items will depend on configuation later
    static qreal arrowEndSize;
    static qreal middleBarLength;
    static QPen standardPen;
    static QPen editPen;
    static QPen selectionPen;
    static QPen inactivePen;
    static QPen activePen;
};

#endif // FSMGRAPHICTRANSITION_H

