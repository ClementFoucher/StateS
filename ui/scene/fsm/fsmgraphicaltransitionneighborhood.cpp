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

#include "fsmgraphicaltransitionneighborhood.h"

#include "fsmgraphicalstate.h"
#include "fsmgraphicaltransition.h"

FsmGraphicalTransitionNeighborhood::FsmGraphicalTransitionNeighborhood(FsmGraphicalState* source, FsmGraphicalState* target)
{
    this->source = source;
    this->target = target;
}

FsmGraphicalState* FsmGraphicalTransitionNeighborhood::getSource() const
{
    return source;
}

void FsmGraphicalTransitionNeighborhood::insertAndNotify(FsmGraphicalTransition* value)
{
    map.insert(map.count(), value);

    emit contentChangedEvent();
}

void FsmGraphicalTransitionNeighborhood::removeAndNotify(FsmGraphicalTransition* value)
{
    int removedValueKey = map.key(value);
    map.remove(removedValueKey);

    // Set all following transition to go down one position to keep a consistent key system
    for (int currentKey = removedValueKey+1 ; currentKey <= map.lastKey() ; currentKey++)
    {
        map.insert(currentKey-1, map[currentKey]);
        map.remove(currentKey);
    }

    emit contentChangedEvent();
}

int FsmGraphicalTransitionNeighborhood::count() const
{
    return map.count();
}


int FsmGraphicalTransitionNeighborhood::whatIsMyRank(FsmGraphicalTransition* me) const
{
    return map.key(me);
}

int FsmGraphicalTransitionNeighborhood::computeTransitionPosition(FsmGraphicalTransition* me) const
{
    // Keys are:            0,  1, 2,  3, 4,  5, etc.
    // Associate ranks are: 1, -1, 2, -2, 3, -3, etc.

    // This is then: order = (key/2 + 1)*(-1)^(key)
    int rank = (pow(-1, map.key(me)) * (map.key(me)/2 + 1));
    // If there is an odd number of mates, allow rank to begin from 0 (straight line)
    if ((map.count() % 2) == 1)
        // To do so, decrement positive kays => 1, -1, 2, -2, 3 will become 0, -1, 1, -2, 2
        if (rank > 0) rank--;

    return rank;
}


QGraphicsPathItem* FsmGraphicalTransitionNeighborhood::buildMyBody(QPen* pen, FsmGraphicalTransition* me, QPointF& deltaCurveOrigin, QPointF& curveMiddle, QPointF& curveTarget, qreal& edgeAngle1, qreal& edgeAngle2) const
{
    //
    // Create a first system as:
    // - Origin (0, 0) is in neighborhood origin
    // - x is a line passing through both ends states' centers with (0, 1) in target center
    // - Y is X's normal, with same measures
    QLineF initialSystemXVector(QPointF(0, 0), target->scenePos() - source->scenePos());
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
    cPointYTranslationVector.setLength(computeTransitionPosition(me) * 150);
    QPointF sceneSystemCPoint(sceneSystemXVector.p2()/2 + cPointYTranslationVector.p2());

    // Use initial C point to calculate new coordinates system
    // Deltas indicates curve start/end positions wrt. local system (start/ends on states perimeter)
    QLineF deltaSystemOriginVector(QPointF(0,0), sceneSystemCPoint);
    deltaSystemOriginVector.setLength(FsmGraphicalState::getRadius());

    QLineF deltaTargetVector(QPointF(0,0), sceneSystemCPoint - sceneSystemXVector.p2());
    deltaTargetVector.setLength(FsmGraphicalState::getRadius());

    // Yes, many coordinates systems.
    QLineF deltaSystemXVector(deltaSystemOriginVector.p2(), sceneSystemXVector.p2() + deltaTargetVector.p2());
    deltaSystemXVector.translate(-deltaSystemXVector.p1());
    // Update C point position in delta coordinates system
    QPointF deltaSystemCPoint = sceneSystemCPoint - deltaSystemOriginVector.p2();


    QPainterPath path;
    path.quadTo(deltaSystemCPoint, deltaSystemXVector.p2());

    QGraphicsPathItem* curve = new QGraphicsPathItem(path, me);
    curve->setPen(*pen);

    QLineF middleVector(QPointF(0,0),
                        QPointF(deltaSystemXVector.p2().x()/2,
                                deltaSystemCPoint.y()/2));

    QLineF edgeAngle1Line(deltaSystemCPoint, deltaSystemXVector.p2());
    QLineF edgeAngle2Line(deltaSystemCPoint, deltaSystemXVector.p1());

    // Display condition
    QLineF conditionLine = sceneSystemYVector;
    conditionLine.setLength(20);
    QGraphicsLineItem *conditionLineDisplay = new QGraphicsLineItem(conditionLine, curve);

    QPointF conditionPosition(middleVector.p2());
    conditionPosition.setY(conditionPosition.y() - conditionLine.length()/2);
    conditionLineDisplay->setPos(conditionPosition);
    conditionLineDisplay->setPen(*pen);

    // Drawing in a horizontal coordinates, then rotate.
    // This is probably too much, but it was easier to reprensent in my mind ;)
    curve->setTransform(QTransform().rotate(-initialSystemXVector.angle()));

    middleVector.setAngle(middleVector.angle()+initialSystemXVector.angle());
    deltaSystemOriginVector.setAngle(deltaSystemOriginVector.angle()+initialSystemXVector.angle());

    QLineF actualTarget(QPointF(0, 0), deltaSystemXVector.p2());
    actualTarget.setAngle(actualTarget.angle() + initialSystemXVector.angle());

    edgeAngle1Line.setAngle(edgeAngle1Line.angle() + initialSystemXVector.angle());
    edgeAngle2Line.setAngle(edgeAngle2Line.angle() + initialSystemXVector.angle());

    // Delta curve origin is the position of curve starting point wrt. source state center
    deltaCurveOrigin = deltaSystemOriginVector.p2();
    // Curve middle is the curve middle point in the horizontal coordinates system which originates at curve origin
    curveMiddle = middleVector.p2();
    // Curve target is the curve last point in the horizontal coordinates system which originates at curve origin
    curveTarget = actualTarget.p2();
    // Curve angles wrt. states perimeters
    edgeAngle1 = edgeAngle1Line.angle();
    edgeAngle2 = edgeAngle2Line.angle();

    return curve;
}
