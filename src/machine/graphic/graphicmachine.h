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

#ifndef GRAPHICMACHINE_H
#define GRAPHICMACHINE_H

// Parent
#include <QObject>

// Stdlib
#include <memory>

// Qt
#include <QMap>

// StateS
#include "componentid.h"
class GraphicComponent;
class GraphicSimulatedComponent;
class GraphicAttributes;
class GenericScene;


class GraphicMachine : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GraphicMachine() = default;
	~GraphicMachine();

	/////
	// Object functions
public:

	// Graphic objects creation can't be done in constructor, as their
	// creation requires access to the graphic machine itself.
	virtual void build(std::shared_ptr<GraphicAttributes> graphicAttributes) = 0;
	virtual void buildSimulation() = 0;
	void clearSimulation();

	// To produce a graphic scene depending on the specialized type of machine.
	virtual GenericScene* getGraphicScene() const = 0;

	// Extract graphic attributes from the current graphic objects
	virtual std::shared_ptr<GraphicAttributes> getGraphicAttributes() const = 0;

	GraphicComponent* getGraphicComponent(ComponentId componentId) const;
	GraphicSimulatedComponent* getSimulatedGraphicComponent(ComponentId componentId) const;

	virtual void removeGraphicComponent(ComponentId id);
	virtual void forceRefreshSimulatedDisplay() = 0;

protected:
	void addComponent(GraphicComponent* graphicComponent);
	void addSimulatedComponent(GraphicSimulatedComponent* simulatedGraphicComponent);
	const QList<GraphicComponent*> getGraphicComponents() const;

	/////
	// Object variables
private:
	QMap<ComponentId, GraphicComponent*> componentsMap;
	QMap<ComponentId, GraphicSimulatedComponent*> simulatedComponentsMap;

};

#endif // GRAPHICMACHINE_H
