/*
 * Copyright © 2023-2026 Clément Foucher
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

// Parent
#include "graphicmachine.h"

// Qt
#include "QHash"

// StateS
class GraphicAttributes;
class GraphicFsmState;
class GraphicFsmTransition;
class GenericScene;
class GraphicFsmTransitionNeighborhood;
class GraphicSimulatedFsmState;
class GraphicSimulatedFsmTransition;


class GraphicFsm : public GraphicMachine
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicFsm() = default;

	/////
	// Object functions
public:
	virtual void build(std::shared_ptr<GraphicAttributes> graphicAttributes) override;
	virtual void buildSimulation() override;

	virtual std::shared_ptr<GraphicAttributes> getGraphicAttributes() const override;
	virtual GenericScene* getGraphicScene() const override;

	virtual void removeGraphicComponent(ComponentId id) override;
	virtual void forceRefreshSimulatedDisplay() override;

	GraphicFsmState*      addState     (ComponentId logicStateId,      QPointF position);
	GraphicFsmTransition* addTransition(ComponentId logicTransitionId, qreal sliderPos);

	const QList<GraphicFsmState*>      getStates()      const;
	const QList<GraphicFsmTransition*> getTransitions() const;

	GraphicFsmState*      getState(ComponentId id)      const;
	GraphicFsmTransition* getTransition(ComponentId id) const;

	GraphicSimulatedFsmState*      getSimulatedState(ComponentId id)      const;
	GraphicSimulatedFsmTransition* getSimulatedTransition(ComponentId id) const;

	// Neighborhood
	int getTransitionRank(ComponentId transitionId) const;
	std::shared_ptr<GraphicFsmTransitionNeighborhood> getTransitionNeighborhood(ComponentId transitionId) const;

private:
	void buildStates(std::shared_ptr<GraphicAttributes> configuration);
	void buildTransitions(std::shared_ptr<GraphicAttributes> configuration);

	void addTransitionToNeighborhood(ComponentId transitionId);
	void removeTransitionFromNeighborhood(ComponentId transitionId);

	/////
	// Object variables
private:
	// Neighborhoods are stored for each pair of FsmState
	// First key is the lowest state ID, second key the other state ID.
	QHash<ComponentId, QHash<ComponentId, std::shared_ptr<GraphicFsmTransitionNeighborhood>>> neighborhoods;

};

#endif // GRAPHICFSM_H
