/*
 * Copyright © 2014-2015 Clément Foucher
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

// StateS classes
#include "actioneditor.h"
#include "conditioneditor.h"
#include "fsmtransition.h"


TransitionEditorTab::TransitionEditorTab(shared_ptr<FsmTransition> transition, QWidget* parent) :
    ComponentEditorTab(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);

	QLabel* title = new QLabel("<b>" + tr("Transition editor") + "</b>");
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);

	ConditionEditor* conditionEditor = new ConditionEditor(transition);
	layout->addWidget(conditionEditor);

	ActionEditor* actionEditor = new ActionEditor(transition, tr("Actions triggered when transition is crossed:"));
	layout->addWidget(actionEditor);
}
