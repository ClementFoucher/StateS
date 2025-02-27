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

#ifndef FSMGRAPHICTRANSITIONNEIGHBORHOOD_H
#define FSMGRAPHICTRANSITIONNEIGHBORHOOD_H

// Parent
#include <QObject>

// Qt classes
#include <QMap>

// StateS classes
class FsmGraphicTransition;
class FsmGraphicState;


class FsmGraphicTransitionNeighborhood : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FsmGraphicTransitionNeighborhood(FsmGraphicState* source, FsmGraphicState* target);

	/////
	// Object functions
public:
	void insertTransition(FsmGraphicTransition* transition);
	void removeTransition(FsmGraphicTransition* transition);

	const QList<FsmGraphicTransition*> getTransitions() const;

	FsmGraphicState* getSource() const;
	FsmGraphicState* getTarget() const;

	int count() const;

	int getTransitionNumber(FsmGraphicTransition* transition) const;

	/////
	// Object variables
private:
	QMap<int, FsmGraphicTransition*> map;
	FsmGraphicState* source = nullptr;
	FsmGraphicState* target = nullptr;

};

#endif // FSMGRAPHICTRANSITIONNEIGHBORHOOD_H
