/*
 * Copyright © 2024-2025 Clément Foucher
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

#ifndef GRAPHICTIMELINE_H
#define GRAPHICTIMELINE_H

// Parent
#include <QWidget>


class GraphicTimeLine : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicTimeLine(uint eventDelay, QWidget* parent = nullptr);

	/////
	// Object variables
protected:
	uint stepLength     = 5;
	uint pointsPerCycle = 4;
	uint eventDelay     = 0;

};

#endif // GRAPHICTIMELINE_H
