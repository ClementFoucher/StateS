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

// Current class header
#include "fsmsimulatedstate.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsView>
#include <QKeyEvent>

// StateS classes
#include "machinemanager.h"
#include "fsmstate.h"
#include "contextmenu.h"
#include "fsm.h"
#include "graphicfsm.h"
#include "fsmsimulator.h"


//
// Static elements
//

const QBrush FsmSimulatedState::activeBrush = QBrush(Qt::green, Qt::SolidPattern);


//
// Class object definition
//

FsmSimulatedState::FsmSimulatedState(componentId_t logicComponentId) :
	FsmGraphicState(logicComponentId),
	SimulatedComponent(logicComponentId)
{
	auto graphicFsm = dynamic_pointer_cast<GraphicFsm>(machineManager->getGraphicMachine());
	if (graphicFsm == nullptr) return;

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

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

FsmSimulatedState::~FsmSimulatedState()
{

}

void FsmSimulatedState::refreshDisplay()
{
	if (this->isActive)
	{
		this->setBrush(activeBrush);
	}
	else
	{
		this->setBrush(defaultBrush);
	}

	FsmGraphicState::refreshDisplay();
}

void FsmSimulatedState::setActive(bool active)
{
	this->isActive = active;

	this->refreshDisplay();

	emit this->stateActiveStatusChanged();
}

bool FsmSimulatedState::getIsActive() const
{
	return this->isActive;
}

void FsmSimulatedState::keyPressEvent(QKeyEvent *event)
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

void FsmSimulatedState::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto logicState = fsm->getState(this->logicComponentId);
	if (logicState == nullptr) return;

	ContextMenu* menu = new ContextMenu();
	menu->addTitle(tr("State") + " <i>" + logicState->getName() + "</i>");

	if (this->isActive == false)
	{
		menu->addAction(tr("Set active"));
	}

	if (menu->actions().count() > 1) // > 1 because title is always here
	{
		menu->popup(event->screenPos());

		connect(menu, &QMenu::triggered, this, &FsmSimulatedState::treatMenu);
	}
	else
	{
		delete menu;
	}
}

void FsmSimulatedState::treatMenu(QAction* action)
{
	if (action->text() == tr("Set active"))
	{
		shared_ptr<FsmSimulator> simulator = dynamic_pointer_cast<FsmSimulator>(machineManager->getMachineSimulator());
		if (simulator != nullptr)
		{
			simulator->forceStateActivation(this->logicComponentId);
		}
	}
}
