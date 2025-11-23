/*
 * Copyright © 2020-2025 Clément Foucher
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
#include "fsmdrawingtoolbar.h"

// StateS classes
#include "machinemanager.h"
#include "machinebuilder.h"
#include "graphicfsmstate.h"
#include "graphicfsmtransition.h"


FsmDrawingToolBar::FsmDrawingToolBar(QWidget* parent) :
	DrawingToolBar(parent)
{
	// Build actions
	this->actionMouse = new QAction(this);
	//this->actionMouse->setIcon(QIcon());
	this->actionMouse->setText(tr("Mouse"));
	this->actionMouse->setToolTip(tr("Deselect any building tool"));
	this->actionMouse->setCheckable(true);
	this->actionMouse->setChecked(true);
	this->actionMouse->setEnabled(false);

	this->actionAddInitialState = new QAction(this);
	this->actionAddInitialState->setIcon(QIcon(GraphicFsmState::getPixmap(128, true)));
	this->actionAddInitialState->setText(tr("Add initial state"));
	this->actionAddInitialState->setCheckable(true);

	this->actionAddState = new QAction(this);
	this->actionAddState->setIcon(QIcon(GraphicFsmState::getPixmap(128)));
	this->actionAddState->setText(tr("Add state"));
	this->actionAddState->setCheckable(true);

	this->actionAddTransition = new QAction(this);
	this->actionAddTransition->setIcon(QIcon(GraphicFsmTransition::getPixmap(128)));
	this->actionAddTransition->setText(tr("Add transition"));
	this->actionAddTransition->setCheckable(true);

	// Add actions
	this->beginAddActions();
	this->addAction(this->actionMouse);
	this->addSeparator();
	this->addAction(this->actionAddInitialState);
	this->addAction(this->actionAddState);
	this->addAction(this->actionAddTransition);
	this->endAddActions();

	// Connect actions
	connect(this->actionMouse,           &QAction::triggered, this, &FsmDrawingToolBar::mouseToolRequestedEvent);
	connect(this->actionAddInitialState, &QAction::triggered, this, &FsmDrawingToolBar::initialStateToolRequestedEvent);
	connect(this->actionAddState,        &QAction::triggered, this, &FsmDrawingToolBar::stateToolRequestedEvent);
	connect(this->actionAddTransition,   &QAction::triggered, this, &FsmDrawingToolBar::transitionToolRequestedEvent);
}

bool FsmDrawingToolBar::toolChangedEventHandler(MachineBuilderTool_t newTool)
{
	bool result = false;

	switch (newTool)
	{
	case MachineBuilderTool_t::state:
		this->actionMouse->          setChecked(false);
		this->actionAddInitialState->setChecked(false);
		this->actionAddState->       setChecked(true);
		this->actionAddTransition->  setChecked(false);
		this->actionMouse->          setEnabled(true);

		result = true;
		break;

	case MachineBuilderTool_t::transition:
		this->actionMouse->          setChecked(false);
		this->actionAddInitialState->setChecked(false);
		this->actionAddState->       setChecked(false);
		this->actionAddTransition->  setChecked(true);
		this->actionMouse->          setEnabled(true);

		result = true;
		break;

	case MachineBuilderTool_t::initialState:
		this->actionMouse->          setChecked(false);
		this->actionAddInitialState->setChecked(true);
		this->actionAddState->       setChecked(false);
		this->actionAddTransition->  setChecked(false);
		this->actionMouse->          setEnabled(true);

		result = true;
		break;

	case MachineBuilderTool_t::none:
		this->actionMouse->          setChecked(true);
		this->actionAddInitialState->setChecked(false);
		this->actionAddState->       setChecked(false);
		this->actionAddTransition->  setChecked(false);
		this->actionMouse->          setEnabled(false);

		result =  true;
		break;
	}

	return result;
}

void FsmDrawingToolBar::mouseToolRequestedEvent(bool)
{
	shared_ptr<MachineBuilder> l_machineBuilder = machineManager->getMachineBuilder();
	if (l_machineBuilder == nullptr) return;

	l_machineBuilder->setTool(MachineBuilderTool_t::none);
}

void FsmDrawingToolBar::initialStateToolRequestedEvent(bool activated)
{
	shared_ptr<MachineBuilder> l_machineBuilder = machineManager->getMachineBuilder();
	if (l_machineBuilder == nullptr) return;

	if (activated)
		l_machineBuilder->setTool(MachineBuilderTool_t::initialState);
	else
		l_machineBuilder->setTool(MachineBuilderTool_t::none);
}

void FsmDrawingToolBar::stateToolRequestedEvent(bool activated)
{
	shared_ptr<MachineBuilder> l_machineBuilder = machineManager->getMachineBuilder();
	if (l_machineBuilder == nullptr) return;

	if (activated)
		l_machineBuilder->setTool(MachineBuilderTool_t::state);
	else
		l_machineBuilder->setTool(MachineBuilderTool_t::none);
}

void FsmDrawingToolBar::transitionToolRequestedEvent(bool activated)
{
	shared_ptr<MachineBuilder> l_machineBuilder = machineManager->getMachineBuilder();
	if (l_machineBuilder == nullptr) return;

	if (activated)
		l_machineBuilder->setTool(MachineBuilderTool_t::transition);
	else
		l_machineBuilder->setTool(MachineBuilderTool_t::none);
}
