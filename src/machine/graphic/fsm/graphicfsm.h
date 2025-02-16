/*
 * Copyright © 2023-2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRAPHICFSM_H
#define GRAPHICFSM_H

// Parent class
#include "graphicmachine.h"

// Qt classes
#include "QHash"

// StateS classes
#include "statestypes.h"
class GraphicAttributes;
class FsmGraphicState;
class FsmGraphicTransition;
class GenericScene;
class FsmGraphicTransitionNeighborhood;


class GraphicFsm : public GraphicMachine
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicFsm();

	/////
	// Object functions
public:
	virtual void build(shared_ptr<GraphicAttributes> graphicAttributes) override;

	virtual shared_ptr<GraphicAttributes> getGraphicAttributes() const override;
	virtual GenericScene* getGraphicScene() const override;

	virtual void removeGraphicComponent(componentId_t id) override;

	FsmGraphicState*      addState     (componentId_t logicStateId,      QPointF position);
	FsmGraphicTransition* addTransition(componentId_t logicTransitionId, qreal sliderPos);

	const QList<FsmGraphicState*>      getStates()      const;
	const QList<FsmGraphicTransition*> getTransitions() const;

	FsmGraphicState*      getState(componentId_t id)      const;
	FsmGraphicTransition* getTransition(componentId_t id) const;

	// Neighborhood
	int getTransitionRank(componentId_t transitionId) const;
	shared_ptr<FsmGraphicTransitionNeighborhood> getTransitionNeighborhood(componentId_t transitionId) const;

private:
	void buildStates(shared_ptr<GraphicAttributes> configuration);
	void buildTransitions(shared_ptr<GraphicAttributes> configuration);

	void addTransitionToNeighborhood(componentId_t transitionId);
	void removeTransitionFromNeighborhood(componentId_t transitionId);

	/////
	// Object variables
private:
	// Neighborhoods are stored for each pair of FsmState
	// First key is the lowest state ID, second key the other state ID.
	QHash<componentId_t, QHash<componentId_t, shared_ptr<FsmGraphicTransitionNeighborhood>>> neighborhoods;

};

#endif // GRAPHICFSM_H
