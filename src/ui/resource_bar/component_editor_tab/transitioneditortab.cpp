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
#include "transitioneditortab.h"

// Qt classes
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>

// StateS classes
#include "machinemanager.h"
#include "fsm.h"
#include "actioneditor.h"
#include "conditioneditor.h"


TransitionEditorTab::TransitionEditorTab(componentId_t transitionId, QWidget* parent) :
	ComponentEditorTab(parent)
{
	auto fsm = dynamic_pointer_cast<Fsm>(machineManager->getMachine());
	if (fsm == nullptr) return;

	auto transition = fsm->getTransition(transitionId);
	if (transition == nullptr) return;


	//
	// Title

	QLabel* title = new QLabel("<b>" + tr("Transition editor") + "</b>");
	title->setAlignment(Qt::AlignCenter);

	//
	// Transition condition

	ConditionEditor* conditionEditor = new ConditionEditor(transitionId);

	// Package in a group
	auto conditionGroup = new QGroupBox(tr("Condition"));
	auto conditionLayout = new QVBoxLayout(conditionGroup);
	conditionLayout->addWidget(conditionEditor);

	//
	// Transition actions

	ActionEditor* actionEditor = new ActionEditor(transitionId);

	// Package in a group
	auto actionGroup = new QGroupBox(tr("Actions triggered when transition is crossed"));
	auto actionLayout = new QVBoxLayout(actionGroup);
	actionLayout->addWidget(actionEditor);

	//
	// Build complete rendering

	auto* mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(title);
	mainLayout->addWidget(conditionGroup);
	mainLayout->addWidget(actionGroup);
}
