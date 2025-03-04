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

#ifndef GRAPHICFSMTRANSITIONNEIGHBORHOOD_H
#define GRAPHICFSMTRANSITIONNEIGHBORHOOD_H

// Parent
#include <QObject>

// Qt classes
#include <QMap>

// StateS classes
class GraphicFsmTransition;
class GraphicFsmState;


class GraphicFsmTransitionNeighborhood : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicFsmTransitionNeighborhood(GraphicFsmState* source, GraphicFsmState* target);

	/////
	// Object functions
public:
	void insertTransition(GraphicFsmTransition* transition);
	void removeTransition(GraphicFsmTransition* transition);

	const QList<GraphicFsmTransition*> getTransitions() const;

	GraphicFsmState* getSource() const;
	GraphicFsmState* getTarget() const;

	int count() const;

	int getTransitionNumber(GraphicFsmTransition* transition) const;

	/////
	// Object variables
private:
	QMap<int, GraphicFsmTransition*> map;
	GraphicFsmState* source = nullptr;
	GraphicFsmState* target = nullptr;

};

#endif // GRAPHICFSMTRANSITIONNEIGHBORHOOD_H
