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

#ifndef FSMGRAPHICTRANSITIONNEIGHBORHOOD_H
#define FSMGRAPHICTRANSITIONNEIGHBORHOOD_H

// Parent
#include <QObject>

// Qt classes
#include <QMap>
class QGraphicsPathItem;
class QGraphicsLineItem;

// StateS classes
class FsmGraphicTransition;
class FsmGraphicState;


class FsmGraphicTransitionNeighborhood : public QObject
{
	Q_OBJECT

public:
	explicit FsmGraphicTransitionNeighborhood(FsmGraphicState* source, FsmGraphicState* target);

	void insertAndNotify(FsmGraphicTransition* value);
	void removeAndNotify(FsmGraphicTransition* value);

	FsmGraphicState* getSource() const;
	int count() const;
	QGraphicsPathItem* buildMyBody(QPen* pen, FsmGraphicTransition* me, QPointF& curveOrigin, QPointF& conditionLinePos, QPointF& curveTarget, qreal& edgeAngle1, qreal& endAngle2, QGraphicsLineItem** conditionLine, qreal conditionLineSliderPos) const;

	int whatIsMyRank(FsmGraphicTransition* me) const;
	int computeTransitionPosition(FsmGraphicTransition* me) const;

signals:
	void contentChangedEvent();

private:

	QMap<int, FsmGraphicTransition*> map;
	FsmGraphicState* source = nullptr;
	FsmGraphicState* target = nullptr;


};

#endif // FSMGRAPHICTRANSITIONNEIGHBORHOOD_H

