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

// Current class header
#include "graphicmachine.h"

// StateS
#include "graphiccomponent.h"
#include "graphicsimulatedcomponent.h"


GraphicMachine::~GraphicMachine()
{
	auto simulatedGraphicComponents = this->simulatedComponentsMap.values();
	for (auto& graphicComponent : simulatedGraphicComponents)
	{
		delete graphicComponent;
	}

	auto graphicComponents = this->componentsMap.values();
	for (auto& graphicComponent : graphicComponents)
	{
		delete graphicComponent;
	}
}

void GraphicMachine::clearSimulation()
{
	auto graphicSimulatedComponents = this->simulatedComponentsMap.values();

	for (auto& component : graphicSimulatedComponents)
	{
		delete component;
	}

	this->simulatedComponentsMap.clear();
}

GraphicComponent* GraphicMachine::getGraphicComponent(ComponentId componentId) const
{
	if (this->componentsMap.contains(componentId))
	{
		return this->componentsMap[componentId];
	}
	else
	{
		return nullptr;
	}
}

GraphicSimulatedComponent* GraphicMachine::getSimulatedGraphicComponent(ComponentId componentId) const
{
	if (this->simulatedComponentsMap.contains(componentId))
	{
		return this->simulatedComponentsMap[componentId];
	}
	else
	{
		return nullptr;
	}
}

void GraphicMachine::removeGraphicComponent(ComponentId id)
{
	if (this->componentsMap.contains(id))
	{
		this->componentsMap[id]->deleteLater();
		this->componentsMap.remove(id);
	}
}

void GraphicMachine::addComponent(GraphicComponent* graphicComponent)
{
	auto logicComponentId = graphicComponent->getLogicComponentId();
	this->componentsMap[logicComponentId] = graphicComponent;
}

void GraphicMachine::addSimulatedComponent(GraphicSimulatedComponent* simulatedGraphicComponent)
{
	auto graphicComponent = dynamic_cast<GraphicComponent*>(simulatedGraphicComponent);
	if (graphicComponent == nullptr) return;


	auto logicComponentId = graphicComponent->getLogicComponentId();
	this->simulatedComponentsMap[logicComponentId] = simulatedGraphicComponent;
}

const QList<GraphicComponent*> GraphicMachine::getGraphicComponents() const
{
	return this->componentsMap.values();
}
