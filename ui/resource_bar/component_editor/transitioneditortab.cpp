/*
 * Copyright © 2014 Clément Foucher
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

// StateS classes
#include "fsmtransition.h"
#include "fsm.h"
#include "equation.h"
#include "equationeditor.h"
#include "contextmenu.h"
#include "actioneditor.h"


TransitionEditorTab::TransitionEditorTab(FsmTransition* transition, QWidget* parent) :
    ComponentEditorTab(parent)
{
    this->transition = transition;
    connect(this->transition, &MachineActuatorComponent::elementConfigurationChangedEvent, this, &TransitionEditorTab::updateContent);

    this->layout = new QGridLayout(this);

    QLabel* title = new QLabel("<b>" + tr("Transition editor") + "</b>");
    title->setAlignment(Qt::AlignCenter);
    this->layout->addWidget(title, 0, 0, 1, 2);

    QLabel* contitionTitle = new QLabel(tr("Condition"));
    contitionTitle->setAlignment(Qt::AlignCenter);
    this->layout->addWidget(contitionTitle, 1, 0, 1, 2);

    if (transition->getCondition() != nullptr)
        this->conditionText = new QLabel(transition->getCondition()->getText());
    else
        this->conditionText = new QLabel("1");
    this->layout->addWidget(this->conditionText, 2, 0, 1, 2);

    // Row 3 is reserved for warning message (dynamically insterted)

    this->buttonSetCondition = new QPushButton(tr("Set condition"));
    connect(this->buttonSetCondition, &QAbstractButton::clicked, this, &TransitionEditorTab::setCondition);
    this->layout->addWidget(this->buttonSetCondition, 4, 0, 1, 1);

    this->buttonClearCondition = new QPushButton(tr("Clear condition"));
    connect(this->buttonClearCondition, &QAbstractButton::clicked, this, &TransitionEditorTab::clearCondition);
    this->layout->addWidget(this->buttonClearCondition, 4, 1, 1, 1);

    this->actionEditor = new ActionEditor(transition, tr("Actions triggered when transition is crossed"));
    this->layout->addWidget(this->actionEditor, 5, 0, 1, 2);

    updateLocalContent();
}

void TransitionEditorTab::updateContent()
{
    updateLocalContent();

    actionEditor->updateContent();
}

void TransitionEditorTab::updateLocalContent()
{
    if (transition->getCondition() != nullptr)
    {
        conditionText->setText(transition->getCondition()->getText());

        if (transition->getCondition()->getSize() != 1)
        {
            delete this->conditionWarningText;

            if (transition->getCondition()->getSize() == 0)
            {
                this->conditionWarningText = new QLabel("<font color=red>" +
                                                        tr("Warning: equation representing condition is not valid.") +
                                                        "<br />" + tr("Thus, the current transition will never be crossed.") +
                                                        "</font>"
                                                        );
            }
            else
            {
                this->conditionWarningText = new QLabel("<font color=red>" +
                                                        tr("Warning: equation representing condition is size") + " " + QString::number(transition->getCondition()->getSize()) +
                                                        "<br />" + tr("Conditions must be size one to allow being treated as booleans.") +
                                                        "<br />" + tr("Thus, the current transition will never be crossed.") +
                                                        "</font>"
                                                        );
            }

            this->conditionWarningText->setAlignment(Qt::AlignCenter);
            this->layout->addWidget(conditionWarningText, 3, 0, 1, 2);

        }
        else
        {
            if (this->conditionWarningText != nullptr)
            {
                delete this->conditionWarningText;
                this->conditionWarningText = nullptr;
            }
        }
    }
    else
    {
        conditionText->setText("1");
    }
}

void TransitionEditorTab::changeEditedTransition(FsmTransition* transition)
{
    disconnect(this->transition, &MachineActuatorComponent::elementConfigurationChangedEvent, this, &TransitionEditorTab::updateContent);

    this->transition = transition;
    actionEditor->changeActuator(transition);

    updateContent();

    connect(this->transition, &MachineActuatorComponent::elementConfigurationChangedEvent, this, &TransitionEditorTab::updateContent);
}

void TransitionEditorTab::setCondition()
{
    if (transition->getOwningMachine()->getReadableSignals().count() != 0)
    {
        EquationEditor* eqEdit;

        if (!(transition->getCondition() == nullptr)) //->isSystemConstantOne())
            eqEdit = new EquationEditor(transition->getOwningMachine(), transition->getCondition());
        else
            eqEdit = new EquationEditor(transition->getOwningMachine(), nullptr);

        int result = eqEdit->exec();

        if (result == QDialog::DialogCode::Accepted)
        {
            Signal* tmp = eqEdit->getResultEquation();

            transition->setCondition(tmp);
        }

        delete eqEdit;
    }
    else
    {
        ContextMenu* menu = ContextMenu::createErrorMenu(tr("No compatible signal!"));
        menu->popup(buttonSetCondition->mapToGlobal(QPoint(buttonSetCondition->width(), -menu->sizeHint().height())));
    }

}

void TransitionEditorTab::clearCondition()
{
    transition->clearCondition();
}

void TransitionEditorTab::treatMenuSetCondition(QAction* action)
{
    foreach (Signal* currentVariable, transition->getOwningMachine()->getReadableSignals())
    {
        if (currentVariable->getName() == action->text())
        {
            transition->setCondition(currentVariable);
            break;
        }
    }
}
