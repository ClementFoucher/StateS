/*
 * Copyright © 2024 Clément Foucher
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

#ifndef GRAPHICVECTORTIMELINE_H
#define GRAPHICVECTORTIMELINE_H

// Parent
#include "graphictimeline.h"

// Qt classes
#include <QPolygon>
#include <QVector>

// StateS classes
#include "logicvalue.h"


class GraphicVectorTimeLine : public GraphicTimeLine
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicVectorTimeLine(uint eventDelay, const LogicValue& initialValue, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void addPoint(const LogicValue& newValue);
	void updateLastPoint(const LogicValue& state);
	void reset(const LogicValue& initialValue);

protected:
	virtual void paintEvent(QPaintEvent*) override;

private:
	void removeLastPoint();

	/////
	// Object variables
private:
	QPolygon timeLinePoly1;
	QPolygon timeLinePoly2;
	QVector<LogicValue> values;

};

#endif // GRAPHICVECTORTIMELINE_H
