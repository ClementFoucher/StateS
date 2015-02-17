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
#include "stateeditortab.h"

// Qt classes
#include <QLabel>
#include <QVBoxLayout>

// StateS classes
#include "fsmstate.h"
#include "dynamiclineedit.h"
#include "actioneditor.h"


StateEditorTab::StateEditorTab(FsmState* state, QWidget* parent) :
    ComponentEditorTab(parent)
{
    this->state = state;
    connect(this->state, &MachineActuatorComponent::elementConfigurationChangedEvent, this, &StateEditorTab::updateContent);

    this->setLayout(new QVBoxLayout());

    QLabel* title = new QLabel("<b>" + tr("State editor") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(title);

    QLabel* nameEditTitle = new QLabel(tr("State name"));
    nameEditTitle->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(nameEditTitle);

    textStateName = new DynamicLineEdit(state->getName());
    connect(textStateName, &DynamicLineEdit::newTextAvailable, this, &StateEditorTab::nameChanged);
    this->layout()->addWidget(textStateName);

    actionEditor = new ActionEditor(state, tr("Actions triggered at state activation") + "<br />" + tr("(pulses are maintained while state is active)"));
    this->layout()->addWidget(actionEditor);

    updateLocalContent();
}

void StateEditorTab::changeEditedState(FsmState* newState)
{
    disconnect(this->state, &MachineActuatorComponent::elementConfigurationChangedEvent, this, &StateEditorTab::updateContent);

    this->state = newState;
    actionEditor->changeActuator(state);

    updateContent();

    connect(this->state, &MachineActuatorComponent::elementConfigurationChangedEvent, this, &StateEditorTab::updateContent);
}

void StateEditorTab::setEditName()
{
    textStateName->selectAll();
    textStateName->setFocus();
}

void StateEditorTab::updateContent()
{
    updateLocalContent();
    actionEditor->updateContent();
}

void StateEditorTab::updateLocalContent()
{
    textStateName->setText(state->getName());
}

void StateEditorTab::nameChanged(const QString& name)
{
    if (name != state->getName())
    {
        if (!state->setName(name))
        {
            textStateName->refuseText();
        }
    }
}
