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

#ifndef FSMGRAPHICALTRANSITION_H
#define FSMGRAPHICALTRANSITION_H

// Parents
#include <QObject>
#include <QGraphicsItemGroup>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QAction;

// StateS classes
class FsmGraphicalState;
class FsmTransition;
class FsmGraphicalTransitionNeighborhood;

// FsmGraphicalTransition has a dynamic behavior.
// When setting dynamic mode, all changes will be stored in a temporary way
// that overseedes the current parameters, but not overrite them. They only
// takes precedence when existing.
// In the end, keepChanges indicates wheter we should keep
// the changes made, and all appropriate actions will be taken wrt. logical FSM,
// or discard changes and delete dynamic parameters.

class FsmGraphicalTransition : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT

public:
    enum class mode{initMode, standardMode, dynamicSourceMode, dynamicTargetMode};

    // Static
public:
    static QPixmap getPixmap(uint size);

private:
    // Private constructor for handling common initialization elements
    explicit FsmGraphicalTransition();

public:
    // This is static build of transition, we already have a logic element to represent
    explicit FsmGraphicalTransition(shared_ptr<FsmTransition> logicTransition);
    // This constructor toggles dynamic mode on target. This is used when drawing a transition graphically
    explicit FsmGraphicalTransition(FsmGraphicalState* source, const QPointF& dynamicMousePosition);
    ~FsmGraphicalTransition();

    void setLogicalTransition(shared_ptr<FsmTransition> transition);
    shared_ptr<FsmTransition> getLogicalTransition() const;

    bool setSourceState(FsmGraphicalState* newSource);
    bool setTargetState(FsmGraphicalState* newTarget);

    FsmGraphicalState* getSource() const;
    FsmGraphicalState* getTarget() const;

    void setMousePosition(const QPointF& mousePos);
    void setTargetMousePosition(const QPointF& newTarget);

    bool setDynamicSourceMode(const QPointF& mousePosition);
    bool setDynamicTargetMode(const QPointF& mousePosition);
    bool endDynamicMode(bool keepChanges);

    void treatSelectionBox();

    FsmGraphicalTransitionNeighborhood* helloIMYourNewNeighbor();

    QGraphicsTextItem  * getConditionText() const;
    QGraphicsItemGroup * getActionsBox()    const;

signals:
    void editCalledEvent(shared_ptr<FsmTransition>);
    void dynamicSourceCalledEvent(FsmGraphicalTransition*);
    void dynamicTargetCalledEvent(FsmGraphicalTransition*);

protected slots:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateDisplay();
    void updateText();
    void treatMenu(QAction* action);

private:
    void finishInitialize();
    void rebuildArrowEnd();

    void checkNeighboors();
    void setNeighbors(FsmGraphicalTransitionNeighborhood* neighborhood);
    void quitNeighboorhood();  // Ohhh... So sad

    //
    weak_ptr<FsmTransition> logicalTransition;

    // A FSM graphical transition must always have at least a source (may not have target when first drawing)
    FsmGraphicalState* source = nullptr;
    FsmGraphicalState* target = nullptr;

    // Dynamic mode
    mode currentMode;
    // This will be used if one of the linked state is missing in dynamic mode
    QPointF mousePosition;
    // Dynamic mode holds a temporary state when mouse hovers a state to preview what would be the result if selected
    FsmGraphicalState* dynamicState = nullptr;

    // Base elements of the arrow
    QGraphicsItem*      arrowBody = nullptr;
    QGraphicsItem*      arrowEnd = nullptr;
    QGraphicsTextItem*  conditionText = nullptr;
    QGraphicsItemGroup* actionsBox   = nullptr;
    QGraphicsLineItem*  conditionLine = nullptr;
    QGraphicsRectItem*  selectionBox = nullptr; // Manually deal with selection bow as Qt has trouble displaying it with complex shapes
    qreal sceneAngle = 0;

    // This list will be shared by all graphical transitions that have same {source, parent} couple (either direction)
    FsmGraphicalTransitionNeighborhood* neighbors = nullptr;

    QPen* currentPen  = nullptr;

    bool autoTransitionNeedsRedraw = true;
    QPointF arcMiddle;

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

#endif // FSMGRAPHICALTRANSITION_H

