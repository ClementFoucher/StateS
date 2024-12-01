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

// Current class header
#include "graphicvectortimeline.h"

// Qt classes
#include <QPainter>


GraphicVectorTimeLine::GraphicVectorTimeLine(uint eventDelay, const LogicValue& initialValue, QWidget* parent) :
	GraphicTimeLine(eventDelay, parent)
{
	this->mode = DisplayMode_t::vector;
	this->reset(initialValue);
}

GraphicVectorTimeLine::GraphicVectorTimeLine(uint eventDelay, const QString& initialState, QWidget* parent) :
	GraphicTimeLine(eventDelay, parent)
{
	this->mode = DisplayMode_t::state;
	this->reset(initialState);
}

void GraphicVectorTimeLine::addPoint(const LogicValue& newValue)
{
	auto previousValue = this->values.last();
	this->values.append(newValue);

	bool valueChanged = (newValue != previousValue) ? true : false;
	this->buildPoly(valueChanged);

	this->update();
}

void GraphicVectorTimeLine::addPoint(const QString& newState)
{
	auto previousState = this->states.last();
	this->states.append(newState);

	bool stateChanged = (newState != previousState) ? true : false;
	this->buildPoly(stateChanged);

	this->update();
}

void GraphicVectorTimeLine::updateLastPoint(const LogicValue& value)
{
	// If no change to do, return
	if (this->values.last() == value)
		return;

	// Else change last point
	if (this->values.count() > 1)
	{
		this->removeLastPoint();
		this->addPoint(value);
	}
	else
	{
		// Replace first point
		this->reset(value);
	}
}

void GraphicVectorTimeLine::updateLastPoint(const QString& state)
{
	// If no change to do, return
	if (this->states.last() == state)
		return;

	// Else change last point
	if (this->states.count() > 1)
	{
		this->removeLastPoint();
		this->addPoint(state);
	}
	else
	{
		// Replace first point
		this->reset(state);
	}
}

void GraphicVectorTimeLine::reset(const LogicValue& initialValue)
{
	this->values.clear();
	this->values.append(initialValue);

	this->resetPoly();
	this->update();
}

void GraphicVectorTimeLine::reset(const QString& initialState)
{
	this->states.clear();
	this->states.append(initialState);

	this->resetPoly();
	this->update();
}

void GraphicVectorTimeLine::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	// First line
	for (int i = 0 ; i < timeLinePoly1.count()-1 ; i++)
	{
		QPoint source;
		QPoint target;

		source.setX(timeLinePoly1[i].x()*stepLength + stepLength/2);
		source.setY( (timeLinePoly1[i].y() == 1)?5:this->height()-5 );

		target.setX(timeLinePoly1[i+1].x()*stepLength + stepLength/2);
		target.setY( (timeLinePoly1[i+1].y() == 1)?5:this->height()-5 );

		painter.drawLine(source, target);
	}

	// Second line
	for (int i = 0 ; i < timeLinePoly2.count()-1 ; i++)
	{
		QPoint source;
		QPoint target;

		source.setX(timeLinePoly2[i].x()*stepLength + stepLength/2);
		source.setY( (timeLinePoly2[i].y() == 1)?5:this->height()-5 );

		target.setX(timeLinePoly2[i+1].x()*stepLength + stepLength/2);
		target.setY( (timeLinePoly2[i+1].y() == 1)?5:this->height()-5 );

		painter.drawLine(source, target);
	}

	// Value
	int oneCycleWidth = this->stepLength*this->pointsPerCycle;
	int x = 0;
	int width = oneCycleWidth/4;
	bool firstChangeDone = false;

	int iterations;
	switch (this->mode)
	{
	case DisplayMode_t::vector:
		iterations = this->values.count();
		break;
	case DisplayMode_t::state:
		iterations = this->states.count();
		break;
	break;
	}
	for (int i = 0 ; i < iterations ; i++)
	{
		bool doDisplay = true;

		if (i != iterations-1)
		{
			bool valueChanged;
			switch (this->mode)
			{
			case DisplayMode_t::vector:
				{
					auto currentValue = this->values[i];
					auto nextValue    = this->values[i+1];
					valueChanged = (currentValue == nextValue) ? false : true;
				}
				break;
			case DisplayMode_t::state:
				{
					auto currentValue = this->states[i];
					auto nextValue    = this->states[i+1];
					valueChanged = (currentValue == nextValue) ? false : true;
				}
				break;
			}

			if (valueChanged == false)
			{
				doDisplay = false;
				width += oneCycleWidth;
			}
		}

		if (doDisplay == true)
		{
			if (firstChangeDone == false)
			{
				firstChangeDone = true;
				x += oneCycleWidth/5;
			}

			QString text;
			switch (this->mode)
			{
			case DisplayMode_t::vector:
				text = QString::number(this->values[i].toInt());
				break;
			case DisplayMode_t::state:
				text = this->states[i];
				break;
			}
			QRectF rectangle(x, 0, width, this->height());
			painter.drawText(rectangle, text, QTextOption(Qt::AlignCenter));

			x += width;
			width = oneCycleWidth;
		}
	}
}

void GraphicVectorTimeLine::buildPoly(bool valueChanged)
{
	for (uint i = 0 ; i < this->pointsPerCycle ; i++)
	{
		QPoint lastPosition = timeLinePoly1.last();

		if ( (i == 0) && (valueChanged == true) )
		{
			if (lastPosition.y() == 0)
			{
				timeLinePoly1.append(QPoint(lastPosition.x() + 1, 1));
				timeLinePoly2.append(QPoint(lastPosition.x() + 1, 0));
			}
			else
			{
				timeLinePoly1.append(QPoint(lastPosition.x() + 1, 0));
				timeLinePoly2.append(QPoint(lastPosition.x() + 1, 1));
			}
		}
		else
		{
			if (lastPosition.y() == 0)
			{
				timeLinePoly1.append(QPoint(lastPosition.x() + 1, 0));
				timeLinePoly2.append(QPoint(lastPosition.x() + 1, 1));
			}
			else
			{
				timeLinePoly1.append(QPoint(lastPosition.x() + 1, 1));
				timeLinePoly2.append(QPoint(lastPosition.x() + 1, 0));
			}
		}
	}

	this->setMinimumWidth(timeLinePoly1.last().x()*this->stepLength + 5*this->stepLength);
	this->setMaximumWidth(timeLinePoly1.last().x()*this->stepLength + 5*this->stepLength);
}

void GraphicVectorTimeLine::resetPoly()
{
	this->timeLinePoly1.clear();
	this->timeLinePoly2.clear();
	// Starting point of graphic vector: not a real point
	this->timeLinePoly1.append(QPoint(0, 0));
	this->timeLinePoly2.append(QPoint(0, 1));
	// Actual initial point
	this->timeLinePoly1.append(QPoint(1, 0));
	this->timeLinePoly2.append(QPoint(1, 1));

	this->setMinimumWidth(5*this->stepLength);
	this->setMaximumWidth(5*this->stepLength);
}

void GraphicVectorTimeLine::removeLastPoint()
{
	int values;
	switch (this->mode)
	{
	case DisplayMode_t::vector:
		values = this->values.count();
		break;
	case DisplayMode_t::state:
		values = this->states.count();
		break;
	}

	if (values > 1) // First point can't be handled: we need an initial value
	{
		for (uint i = 0 ; i < this->pointsPerCycle ; i++)
		{
			this->timeLinePoly1.removeLast();
			this->timeLinePoly2.removeLast();
		}
		switch (this->mode)
		{
		case DisplayMode_t::vector:
			this->values.removeLast();
			break;
		case DisplayMode_t::state:
			this->states.removeLast();
			break;
		}
	}
}
