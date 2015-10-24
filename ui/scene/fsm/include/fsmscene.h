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

#ifndef FSMSCENE_H
#define FSMSCENE_H

// Parent
#include "genericscene.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QAction;

// To access enums
#include "machine.h"

// StateS classes
class FsmGraphicalTransition;
class FsmGraphicalState;
class Fsm;
class FsmState;
class FsmTransition;
class ResourceBar;


class FsmScene : public GenericScene
{
    Q_OBJECT

private:
    enum sceneMode_e
    {
        idle,
        drawingTransition,
        editingTransitionSource,
        editingTransitionTarget,
        simulating
    };

public:
    explicit FsmScene(shared_ptr<Fsm> machine, ResourceBar* resourceBar);
    ~FsmScene();

    void setDisplaySize(const QSize& newSize) override;

    void beginDrawTransition(FsmGraphicalState* source, const QPointF& currentMousePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private slots:
    void simulationModeChanged(Machine::mode newMode);

    void handleSelection();
    void stateCallsEditEventHandler(shared_ptr<FsmState> state);
    void stateCallsRenameEventHandler(shared_ptr<FsmState> state);
    void treatMenu(QAction*);

    void updateSceneRect();

    void transitionCallsDynamicSourceEventHandler(FsmGraphicalTransition* transition);
    void transitionCallsDynamicTargetEventHandler(FsmGraphicalTransition* transition);
    void transitionCallsEditEventHandler(shared_ptr<FsmTransition> transition);

private:
    bool cancellingAction = false;
    bool askedForMenu = false;
    FsmGraphicalState* getStateAt(const QPointF& location) const;
    FsmGraphicalState* addState(shared_ptr<FsmState> logicState, QPointF location);
    void addTransition(FsmGraphicalTransition* newTransition);

    shared_ptr<Fsm> machine;
    ResourceBar* resources;

    sceneMode_e sceneMode;
    //bool isDrawingTransition = false;
    //bool isEditingTransitionSource = false;
    //bool isEditingTransitionTarget = false;
    //bool isSimulating = false;
    FsmGraphicalTransition* currentTransition = nullptr;

    QPointF mousePos;

    QSize displaySize;

};

#endif // FSMSCENE_H
