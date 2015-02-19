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
#include "conditioneditor.h"

// Qt classes
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

// StateS classes
#include "fsmtransition.h"
#include "truthtabledisplay.h"
#include "equation.h"
#include "truthtable.h"
#include "machine.h"
#include "equationeditor.h"
#include "contextmenu.h"


ConditionEditor::ConditionEditor(FsmTransition *transition, QWidget* parent) :
    QWidget(parent)
{
    this->transition = transition;
    connect(this->transition, &FsmTransition::conditionChanged, this, &ConditionEditor::updateContent);

    this->layout = new QGridLayout(this);

    QLabel* conditionTitle = new QLabel(tr("Condition"));
    conditionTitle->setAlignment(Qt::AlignCenter);
    this->layout->addWidget(conditionTitle, 0, 0, 1, 2);

    this->conditionText = new QLabel("...");
    this->layout->addWidget(this->conditionText, 1, 0, 1, 2);

    this->buttonSetCondition = new QPushButton(tr("Set condition"));
    connect(this->buttonSetCondition, &QAbstractButton::clicked, this, &ConditionEditor::editCondition);
    this->layout->addWidget(this->buttonSetCondition, 2, 0, 1, 1);

    QPushButton* buttonClearCondition = new QPushButton(tr("Clear condition"));
    connect(buttonClearCondition, &QAbstractButton::clicked, this, &ConditionEditor::clearCondition);
    this->layout->addWidget(buttonClearCondition, 2, 1, 1, 1);

    this->buttonExpandTruthTable = new QPushButton(tr("Display truth table"));
    connect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);
    this->layout->addWidget(this->buttonExpandTruthTable, 3, 0, 1, 2);

    this->updateContent();
}

ConditionEditor::~ConditionEditor()
{
    delete truthTable;
    delete truthTableDisplay; // In case not shown
}

void ConditionEditor::updateTransition(FsmTransition* newTransition)
{
    disconnect(this->transition, &FsmTransition::conditionChanged, this, &ConditionEditor::updateContent);
    this->transition = newTransition;
    connect(this->transition, &FsmTransition::conditionChanged, this, &ConditionEditor::updateContent);

    this->updateContent();
}

void ConditionEditor::expandTruthTable()
{
    if (!this->isTruthTableDisplayed())
    {
        disconnect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);

        if (this->truthTableDisplay != nullptr)
        {
            this->truthTableDisplay->setVisible(true);
        }
        else
        {
            Equation* equation = dynamic_cast<Equation*>(this->transition->getCondition());

            if (equation != nullptr)
            {
                this->truthTable = new TruthTable(equation);
                this->truthTableDisplay = new TruthTableDisplay(this->truthTable);
                this->layout->addWidget(this->truthTableDisplay, 4, 0, 1, 2);
            }
            else
            {
                // TODO
            }
        }

        this->buttonExpandTruthTable->setText(tr("Collapse truth table"));

        connect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::collapseTruthTable);

    }
}

void ConditionEditor::collapseTruthTable()
{
    disconnect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::collapseTruthTable);

    this->buttonExpandTruthTable->setText(tr("Display truth table"));
    this->truthTableDisplay->setVisible(false);

    connect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);
}

bool ConditionEditor::isTruthTableDisplayed()
{
    if ( (this->truthTable != nullptr) && (this->truthTableDisplay->isVisible()) )
         return true;
    else return false;
}

void ConditionEditor::updateContent()
{
    delete this->conditionWarningText;
    this->conditionWarningText = nullptr;

    bool truthTableDisplayed = this->isTruthTableDisplayed();
    if (truthTableDisplayed)
        this->collapseTruthTable();

    delete this->truthTableDisplay;
    this->truthTableDisplay = nullptr;
    delete this->truthTable;
    this->truthTable = nullptr;

    this->buttonExpandTruthTable->setEnabled(false);

    if (this->transition->getCondition() != nullptr)
    {
        this->conditionText->setText(transition->getCondition()->getText());

        if (this->transition->getCondition()->getSize() == 1)
        {
            Equation* equationCondition = dynamic_cast<Equation*>(this->transition->getCondition());

            if (equationCondition != nullptr)
            {
                this->buttonExpandTruthTable->setEnabled(true);

                if (truthTableDisplayed)
                    this->expandTruthTable();
            }
        }
        else
        {
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
    }
    else
    {
        this->conditionText->setText("1");
    }
}

void ConditionEditor::editCondition()
{
    if (transition->getOwningMachine()->getReadableSignals().count() != 0)
    {
        EquationEditor* eqEdit;

        if (!(transition->getCondition() == nullptr))
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

void ConditionEditor::clearCondition()
{
    transition->clearCondition();
}

void ConditionEditor::treatMenuSetCondition(QAction* action)
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
