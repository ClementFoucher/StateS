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
#include "fsm.h"


StateEditorTab::StateEditorTab(shared_ptr<FsmState> state, QWidget* parent) :
    ComponentEditorTab(parent)
{
    this->state = state;
    connect(state.get(), &FsmState::stateRenamedEvent, this, &StateEditorTab::updateContent);

    this->setLayout(new QVBoxLayout());

    QLabel* title = new QLabel("<b>" + tr("State editor") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(title);

    QLabel* nameEditTitle = new QLabel(tr("State name"));
    nameEditTitle->setAlignment(Qt::AlignCenter);
    this->layout()->addWidget(nameEditTitle);

    textStateName = new DynamicLineEdit(state->getName(), true);
    connect(textStateName, &DynamicLineEdit::newTextAvailableEvent, this, &StateEditorTab::nameChangedEventHandler);
    connect(textStateName, &DynamicLineEdit::userCancelEvent, this, &StateEditorTab::updateContent);
    this->layout()->addWidget(textStateName);

    actionEditor = new ActionEditor(state, tr("Actions triggered at state activation") + "<br />" + tr("(pulses are maintained while state is active)"));
    this->layout()->addWidget(actionEditor);

    updateContent();
}

void StateEditorTab::changeEditedState(shared_ptr<FsmState> newState)
{
    disconnect(this->state.lock().get(), &FsmState::stateRenamedEvent, this, &StateEditorTab::updateContent);

    this->state = newState;
    actionEditor->changeActuator(newState);

    updateContent();

    connect(newState.get(), &FsmState::stateRenamedEvent, this, &StateEditorTab::updateContent);
}

void StateEditorTab::setEditName()
{
    textStateName->selectAll();
    textStateName->setFocus();
}

/**
 * @brief StateEditorTab::mousePressEvent
 * Used to allow validation of name wherever we click,
 * otherwise clicks inside this widget won't validate input.
 */
void StateEditorTab::mousePressEvent(QMouseEvent*)
{
    this->textStateName->clearFocus();
}

void StateEditorTab::updateContent()
{
    textStateName->resetView();
    textStateName->setText(this->state.lock()->getName());
}

void StateEditorTab::nameChangedEventHandler(const QString& name)
{
    shared_ptr<FsmState> state = this->state.lock();

    if (name != state->getName())
    {
        if ( !(state->getOwningFsm()->renameState(state, name)) )
        {
            textStateName->markAsErroneous();
        }
    }
}
