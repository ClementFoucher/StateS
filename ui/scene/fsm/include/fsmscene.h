/*
 * Copyright © 2014 Clément Foucher
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

#ifndef FSMSCENE_H
#define FSMSCENE_H

#include "genericscene.h"

#include "resourcesbar.h"

class FsmGraphicalTransition;
class FsmGraphicalState;
class Fsm;
class FsmState;
class FsmTransition;

class FsmScene : public GenericScene
{
    Q_OBJECT

public:
    explicit FsmScene(ResourcesBar* resources, Fsm* machine);
    ~FsmScene() override;

    void setDisplaySize(const QSize& newSize) override;

    void simulationModeChanged() override;

    ResourcesBar::mode getMode() const;

    void beginDrawTransition(FsmGraphicalState* source, const QPointF& currentMousePos);

protected slots:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private slots:
    void handleSelection();
    void stateCallsEdit(FsmState* state);
    void stateCallsRename(FsmState* state);
    void treatMenu(QAction*);

    void updateSceneRect();

    void transitionCallsDynamicSource(FsmGraphicalTransition* transition);
    void transitionCallsDynamicTarget(FsmGraphicalTransition* transition);
    void transitionCallsEdit(FsmTransition* transition);

private:
    FsmGraphicalState* getStateAt(const QPointF& location) const;
    FsmGraphicalState* addState(FsmState *logicState, QPointF location);
    void addTransition(FsmGraphicalTransition* newTransition);

    Fsm* machine = nullptr;

    bool isDrawingTransition = false;
    bool isEditingTransitionSource = false;
    bool isEditingTransitionTarget = false;
    FsmGraphicalTransition * currentTransition = nullptr;

    QPointF mousePos;

    QSize displaySize;

};

#endif // FSMSCENE_H
