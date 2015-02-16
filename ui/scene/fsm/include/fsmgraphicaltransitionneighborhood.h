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

#ifndef FSMGRAPHICALTRANSITIONNEIGHBORHOOD_H
#define FSMGRAPHICALTRANSITIONNEIGHBORHOOD_H

// Parent
#include <QObject>

// Qt classes
#include <QMap>
#include <QGraphicsPathItem>

// StateS classes
class FsmGraphicalTransition;
class FsmGraphicalState;

class FsmGraphicalTransitionNeighborhood : public QObject
{
    Q_OBJECT

public:
    explicit FsmGraphicalTransitionNeighborhood(FsmGraphicalState* source, FsmGraphicalState* target);

    void insertAndNotify(FsmGraphicalTransition* value);
    void removeAndNotify(FsmGraphicalTransition* value);

    FsmGraphicalState* getSource() const;
    int count() const;
    QGraphicsPathItem* buildMyBody(QPen* pen, FsmGraphicalTransition* me, QPointF& curveOrigin, QPointF& curveMiddle, QPointF& curveTarget, qreal& edgeAngle1, qreal& endAngle2, QGraphicsLineItem** conditionLine) const;

    int whatIsMyRank(FsmGraphicalTransition* me) const;

signals:
    void contentChangedEvent();

private:
    int computeTransitionPosition(FsmGraphicalTransition* me) const;

    QMap<int, FsmGraphicalTransition*> map;
    FsmGraphicalState* source = nullptr;
    FsmGraphicalState* target = nullptr;


};

#endif // FSMGRAPHICALTRANSITIONNEIGHBORHOOD_H

