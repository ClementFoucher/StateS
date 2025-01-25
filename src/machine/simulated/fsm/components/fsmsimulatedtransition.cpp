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
#include "fsmsimulatedtransition.h"

// C++ classes
#include <math.h>
#include <memory>
using namespace std;

// Qt classes
#include <QPen>

// States classes
#include "machinemanager.h"
#include "fsmtransition.h"
#include "variable.h"
#include "fsm.h"
#include "fsmsimulator.h"
#include "statesexception.h"
#include "exceptiontypes.h"
#include "fsmsimulatedstate.h"


//
// Static elements
//

const QPen FsmSimulatedTransition::activePen   = QPen(QBrush(QColor(0, 0xB0, 0), Qt::SolidPattern), 3);
const QPen FsmSimulatedTransition::inactivePen = QPen(QBrush(Qt::red,            Qt::SolidPattern), 3);


//
// Class object definition
//

FsmSimulatedTransition::FsmSimulatedTransition(componentId_t logicComponentId) :
    FsmGraphicTransition(logicComponentId),
    SimulatedActuatorComponent(logicComponentId)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto simulatedFsm = dynamic_pointer_cast<FsmSimulator>(machineManager->getMachineSimulator());
	if (simulatedFsm == nullptr) return;

	auto logicTransition = fsm->getTransition(this->logicComponentId);
	if (logicTransition == nullptr) return;

	auto sourceState = dynamic_cast<FsmSimulatedState*>(simulatedFsm->getComponent(logicTransition->getSourceStateId()));
	if (sourceState == nullptr) return;

	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::ItemIsFocusable,  false);
	this->setFlag(QGraphicsItem::ItemClipsToShape, false);

	this->setAcceptHoverEvents(false);

	auto condition = logicTransition->getCondition();
	if (condition != nullptr) // nullptr is still a valid condition
	{
		connect(condition.get(), &Variable::variableDynamicStateChangedEvent, this, &FsmSimulatedTransition::refreshDisplay);
	}

	connect(sourceState, &FsmSimulatedState::stateActiveStatusChanged, this, &FsmSimulatedTransition::refreshDisplay);
}

FsmSimulatedTransition::~FsmSimulatedTransition()
{

}

void FsmSimulatedTransition::refreshDisplay()
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto fsmSimulator = dynamic_pointer_cast<FsmSimulator>(machineManager->getMachineSimulator());
	if (fsmSimulator == nullptr) return;

	auto logicTransition = fsm->getTransition(this->logicComponentId);
	if (logicTransition == nullptr) return;

	auto sourceState = dynamic_cast<FsmSimulatedState*>(fsmSimulator->getComponent(logicTransition->getSourceStateId()));
	if (sourceState == nullptr) return;


	//
	// Condition pen
	shared_ptr<Variable> condition = logicTransition->getCondition();
	if (condition != nullptr)
	{
		try
		{
			if (condition->isTrue())
			{
				this->currentConditionPen = &activePen;
			}
			else
			{
				this->currentConditionPen = &inactivePen;
			}
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == SignalError_t::signal_is_not_bool) )
			{
				// Condition is incorrect, considered false
				this->currentConditionPen = &inactivePen;
			}
			else
			{
				throw;
			}
		}
	}
	else
	{
		// Missing condition is implicitly 1
		this->currentConditionPen = &activePen;
	}

	//
	// Main pen
	if (sourceState->getIsActive() == true)
	{
		this->currentPen = this->currentConditionPen;
	}
	else
	{
		this->currentPen = &FsmGraphicTransition::defaultPen;
	}

	FsmGraphicTransition::refreshDisplay();
}
