/*
 * Copyright © 2025 Clément Foucher
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
#include "graphicsimulatedfsmstate.h"

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QKeyEvent>

// StateS classes
#include "machinemanager.h"
#include "graphicfsm.h"
#include "simulatedfsm.h"
#include "simulatedfsmstate.h"
#include "contextmenu.h"


GraphicSimulatedFsmState::GraphicSimulatedFsmState(componentId_t logicComponentId) :
	GraphicFsmState(logicComponentId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto graphicState = graphicFsm->getState(logicComponentId);
	if (graphicState == nullptr) return;


	this->setPos(graphicState->pos());

	this->setFlag(QGraphicsItem::ItemIsMovable,                 false);
	this->setFlag(QGraphicsItem::ItemIsSelectable,              false);
	this->setFlag(QGraphicsItem::ItemIsFocusable,               false);
	this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges, false);
	this->setFlag(QGraphicsItem::ItemClipsToShape,              false);

	this->setAcceptHoverEvents(false);
}

void GraphicSimulatedFsmState::refreshSimulatedDisplay()
{
	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto simulatedState = simulatedFsm->getSimulatedState(this->getLogicComponentId());
	if (simulatedState == nullptr) return;


	if (simulatedState->getIsActive() == true)
	{
		this->setFillingColor(GraphicSimulatedComponent::simuActiveFillingColor);
	}
	else
	{
		this->setFillingColor(GraphicComponent::defaultFillingColor);
	}

	emit this->componentRefreshedEvent();
}

void GraphicSimulatedFsmState::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Menu)
	{
		QGraphicsSceneContextMenuEvent* contextEvent = new QGraphicsSceneContextMenuEvent(QEvent::KeyPress);

		QGraphicsView* view = scene()->views()[0];

		QPoint posOnParent = view->mapFromScene(this->scenePos());

		QPoint posOnScreen = view->mapToGlobal(posOnParent);
		contextEvent->setScreenPos(posOnScreen);

		this->contextMenuEvent(contextEvent);

		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void GraphicSimulatedFsmState::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
	if (simulatedFsm == nullptr) return;

	auto simulatedState = simulatedFsm->getSimulatedState(this->getLogicComponentId());
	if (simulatedState == nullptr) return;


	ContextMenu* menu = new ContextMenu();
	menu->addTitle(tr("State") + " <i>" + simulatedState->getName() + "</i>");

	if (simulatedState->getIsActive() == false)
	{
		menu->addAction(tr("Set active"));
	}

	if (menu->actions().count() > 1) // > 1 because title is always here
	{
		menu->popup(event->screenPos());

		connect(menu, &QMenu::triggered, this, &GraphicSimulatedFsmState::menuSetActiveTriggeredEventHandler);
	}
	else
	{
		delete menu;
	}
}

void GraphicSimulatedFsmState::menuSetActiveTriggeredEventHandler(QAction* action)
{
	if (action->text() == tr("Set active"))
	{
		auto simulatedFsm = dynamic_pointer_cast<SimulatedFsm>(machineManager->getSimulatedMachine());
		if (simulatedFsm == nullptr) return;


		simulatedFsm->forceStateActivation(this->getLogicComponentId());
	}
}
