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
#include "stateeditortab.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>

// StateS classes
#include "machinemanager.h"
#include "fsmstate.h"
#include "selfmanageddynamiclineeditor.h"
#include "actioneditor.h"
#include "fsm.h"
#include "fsmstaterenameundocommand.h"


StateEditorTab::StateEditorTab(componentId_t stateId, QWidget* parent) :
	ComponentEditorTab(parent)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto state = fsm->getState(stateId);
	if (state == nullptr) return;


	////
	// Build object

	this->stateId = stateId;
	connect(state.get(), &FsmState::stateRenamedEvent, this, &StateEditorTab::updateContent);

	////
	// Visual rendering

	//
	// Title

	QLabel* title = new QLabel("<b>" + tr("State editor") + "</b>");
	title->setAlignment(Qt::AlignCenter);

	//
	// State name

	this->textStateName = new SelfManagedDynamicLineEditor(state->getName());
	connect(this->textStateName, &SelfManagedDynamicLineEditor::newTextAvailableEvent, this, &StateEditorTab::nameTextChangedEventHandler);
	connect(this->textStateName, &SelfManagedDynamicLineEditor::userCancelEvent,       this, &StateEditorTab::updateContent);

	// Package in a group
	auto nameEditGroup = new QGroupBox(tr("State name"));
	auto nameEditLayout = new QVBoxLayout(nameEditGroup);
	nameEditLayout->addWidget(this->textStateName);

	//
	// State actions

	ActionEditor* actionEditor = new ActionEditor(stateId);

	// Package in a group
	auto actionGroup = new QGroupBox(tr("Actions triggered on state activation"));
	auto actionLayout = new QVBoxLayout(actionGroup);
	actionLayout->addWidget(actionEditor);

	//
	// Build complete rendering

	auto* mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(title);
	mainLayout->addWidget(nameEditGroup);
	mainLayout->addWidget(actionGroup);

	////
	// Fill content

	this->updateContent();
}

void StateEditorTab::setEditName()
{
	this->textStateName->selectAll();
	this->textStateName->setFocus();
}

/**
 * @brief StateEditorTab::mousePressEvent
 * Used to allow validation of name wherever we click,
 * otherwise clicks inside this widget won't validate input.
 */
void StateEditorTab::mousePressEvent(QMouseEvent* e)
{
	this->textStateName->clearFocus();

	ComponentEditorTab::mousePressEvent(e);
}

void StateEditorTab::updateContent()
{
	this->textStateName->resetView();

	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto state = fsm->getState(this->stateId);
	if (state == nullptr) return;


	this->textStateName->setText(state->getName());
}

void StateEditorTab::nameTextChangedEventHandler(const QString& name)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto state = fsm->getState(this->stateId);
	if (state == nullptr) return;

	if (name == state->getName()) return; // Must be checked because setting focus triggers this event


	QString previousName = state->getName();
	bool result = fsm->renameState(this->stateId, name);
	if (result == true)
	{
		auto undoCommand = new FsmStateRenameUndoCommand(state->getId(), previousName);
		machineManager->notifyMachineEdited(undoCommand);
	}
	else
	{
		this->textStateName->setErroneous(true);
	}
}
