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

// StateS classes
#include "statestypes.h"
class GraphicTimeLine;


class VariableTimeline : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit VariableTimeline(uint delay, componentId_t variableId, QWidget* parent = nullptr);

	/////
	// Object functions
private slots:
	void doStepEventHandler();
	void resetEventHandler();
	void updateCurrentValue();

	/////
	// Object variables
private:
	componentId_t variableId = nullId;

	QList<GraphicTimeLine*> variableLineDisplay;

};

#endif // VARIABLETIMELINE_H
