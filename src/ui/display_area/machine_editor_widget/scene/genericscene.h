/*
 * Copyright © 2014-2024 Clément Foucher
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

#ifndef GENERICSCENE_H
#define GENERICSCENE_H

// Parent
#include <QGraphicsScene>

// StateS classes
#include "statestypes.h"


class GenericScene : public QGraphicsScene
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit GenericScene();

	/////
	// Object functions
public:
	QRectF getItemsBoundingRect();
	void recomputeSceneRect();

protected:
	virtual void updateSimulatioMode(SimulationMode_t newMode) = 0;

signals:
	void itemSelectedEvent(componentId_t componentId);
	void editSelectedItemEvent();
	void renameSelectedItemEvent();
	void sceneSimulationModeAboutToChangeEvent();
	void sceneSimulationModeChangedEvent();

private slots:
	void simulationModeChangeEventHandler(SimulationMode_t newMode);

};

#endif // GENERICSCENE_H
