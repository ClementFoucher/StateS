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

#ifndef VARIABLETIMELINE_H
#define VARIABLETIMELINE_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Variable;
class Clock;
class GraphicTimeLine;


class VariableTimeline : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit VariableTimeline(uint delay, shared_ptr<Variable> signal, shared_ptr<Clock> clock, QWidget* parent = nullptr);

	/////
	// Object functions
private slots:
	void clockEventHandler();
	void resetEventHandler();
	void updateCurrentValue();

	/////
	// Object variables
private:
	weak_ptr<Variable> signal;

	QList<GraphicTimeLine*> signalLineDisplay;

};

#endif // VARIABLETIMELINE_H
