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

#include <QLabel>

#include "transitioneditortab.h"

#include "fsmtransition.h"
#include "fsm.h"
#include "io.h"
#include "input.h"
#include "logicequation.h"
#include "equationeditor.h"
#include "contextmenu.h"
#include "signalactionslist.h"

TransitionEditorTab::TransitionEditorTab(FsmTransition* transition, QWidget* parent) :
    EditorTab(parent)
{
    this->transition = transition;
    connect(transition, SIGNAL(elementConfigurationChanged()), this, SLOT(updateContent()));

    QLabel* title = new QLabel("<b>" + tr("Transition editor") + "</b>");
    title->setAlignment(Qt::AlignCenter);

    QLabel* contitionTitle = new QLabel(tr("Condition"));
    contitionTitle->setAlignment(Qt::AlignCenter);
    conditionText = new QLabel(transition->getCondition()->getText());

    buttonSetCondition = new QPushButton(tr("Set condition"));
    connect(buttonSetCondition, SIGNAL(clicked()), this, SLOT(setCondition()));

    buttonClearCondition = new QPushButton(tr("Clear condition"));
    connect(buttonClearCondition, SIGNAL(clicked()), this, SLOT(clearCondition()));

    QLabel* actionListTitle = new QLabel(tr("Actions triggered when transition is crossed"));
    actionListTitle->setAlignment(Qt::AlignCenter);
    actionList = new QTableWidget(0, 2);
    actionList->setHorizontalHeaderItem(0, new QTableWidgetItem());
    actionList->setHorizontalHeaderItem(1, new QTableWidgetItem());
    actionList->horizontalHeaderItem(0)->setText(tr("Type"));
    actionList->horizontalHeaderItem(1)->setText(tr("Signal"));
    actionList->setSelectionBehavior(QAbstractItemView::SelectRows);


    buttonAddAction = new QPushButton(tr("Add action"));
    connect(buttonAddAction, SIGNAL(clicked()), this, SLOT(addAction()));
    buttonRemoveAction = new QPushButton(tr("Remove action"));
    connect(buttonRemoveAction, SIGNAL(clicked()), this, SLOT(removeAction()));

    layout = new QGridLayout(this);

    layout->addWidget(title, 0, 1, 1, 2);
    layout->addWidget(contitionTitle, 1, 1, 1, 2);
    layout->addWidget(conditionText, 2, 1, 1, 2);
    layout->addWidget(buttonSetCondition, 3, 1, 1, 1);
    layout->addWidget(buttonClearCondition, 3, 2, 1, 1);
    layout->addWidget(actionListTitle, 4, 1, 1, 2);
    layout->addWidget(actionList, 5, 1, 1, 2);
    layout->addWidget(buttonAddAction, 6, 1, 1, 1);
    layout->addWidget(buttonRemoveAction, 6, 2, 1, 1);

    updateContent();
}

TransitionEditorTab::~TransitionEditorTab()
{

}

void TransitionEditorTab::updateContent()
{
    conditionText->setText(transition->getCondition()->getText());

    actionList->clearContents();
    actionList->setRowCount(0);
    foreach(LogicVariable* var, transition->getActions())
    {
        actionList->insertRow(actionList->rowCount());

        actionList->setCellWidget(actionList->rowCount()-1, 0, new SignalActionsList(transition, var));
        actionList->setItem(actionList->rowCount()-1, 1, new QTableWidgetItem(var->getName()));
    }
}

void TransitionEditorTab::changeEditedTransition(FsmTransition* transition)
{
    disconnect(transition, SIGNAL(elementConfigurationChanged()), this, SLOT(updateContent()));

    this->transition = transition;

    connect(transition, SIGNAL(elementConfigurationChanged()), this, SLOT(updateContent()));

    updateContent();
}

void TransitionEditorTab::setCondition()
{
    if (transition->getOwningMachine()->getReadableVariables().count() != 0)
    {
        EquationEditor* eqEdit = new EquationEditor(transition->getOwningMachine(), transition->getCondition());
        int result = eqEdit->exec();

        if (result == QDialog::DialogCode::Accepted)
        {
            LogicVariable* tmp = eqEdit->getResultEquation();

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
    foreach (LogicVariable* currentVariable, transition->getOwningMachine()->getReadableVariables())
    {
        if (currentVariable->getName() == action->text())
        {
            transition->setCondition(currentVariable);
            break;
        }
    }
}


void TransitionEditorTab::addAction()
{
    ContextMenu* menu;

    QList<LogicVariable*> availableActions;

    foreach(LogicVariable* var, transition->getOwningMachine()->getWrittableVariables())
    {
        if (!transition->getActions().contains(var))
            availableActions.append(var);
    }

    if (availableActions.count() != 0)
    {
        menu = new ContextMenu();
        menu->setListStyle();

        foreach(LogicVariable* var, availableActions)
        {
            menu->addAction(var->getName());
        }

        connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(treatMenuAdd(QAction*)));
    }
    else
    {
        menu = ContextMenu::createErrorMenu(tr("No compatible signal!"));
    }

    menu->popup(buttonAddAction->mapToGlobal(QPoint(buttonAddAction->width(), -menu->sizeHint().height())));

}

void TransitionEditorTab::removeAction()
{
    QList<QTableWidgetItem*> selection = actionList->selectedItems();

    foreach (QTableWidgetItem* currentItem, selection)
    {
        if (currentItem->column() == 1)
        {
            transition->removeAction(currentItem->text());
        }
    }
}

void TransitionEditorTab::treatMenuAdd(QAction* action)
{
    transition->addAction(action->text());
}

