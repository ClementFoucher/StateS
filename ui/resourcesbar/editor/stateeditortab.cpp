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
#include <QMenu>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QList>

#include <QDebug>

#include "stateeditortab.h"

#include "fsmstate.h"
#include "logicvariable.h"
#include "fsm.h"
#include "io.h"
#include "dynamiclineedit.h"

StateEditorTab::StateEditorTab(FsmState* state, QWidget* parent) :
    EditorTab(parent)
{
    this->state = state;
    connect(state, SIGNAL(stateConfigurationChanged()), this, SLOT(updateContent()));

    QLabel* title = new QLabel("<b>" + tr("State editor") + "</b>");
    title->setAlignment(Qt::AlignCenter);

    QLabel* nameEditTitle = new QLabel(tr("State name"));
    nameEditTitle->setAlignment(Qt::AlignCenter);
    textStateName = new DynamicLineEdit(state->getName());
    connect(textStateName, SIGNAL(newTextAvailable(QString)), this, SLOT(nameChanged(QString)));

    QLabel* actionListTitle = new QLabel(tr("Actions triggered when state is active"));
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
    layout->addWidget(nameEditTitle, 1, 1, 1, 2);
    layout->addWidget(textStateName, 2, 1, 1, 2);
    layout->addWidget(actionListTitle, 3, 1, 1, 2);
    layout->addWidget(actionList, 4, 1, 1, 2);
    layout->addWidget(buttonAddAction, 5, 1, 1, 1);
    layout->addWidget(buttonRemoveAction, 5, 2, 1, 1);

    updateContent();
}

StateEditorTab::~StateEditorTab()
{
    disconnect(state, SIGNAL(stateConfigurationChanged()), this, SLOT(updateContent()));
}

void StateEditorTab::changeEditedState(FsmState* newState)
{
    disconnect(state, SIGNAL(stateConfigurationChanged()), this, SLOT(updateContent()));

    this->state = newState;
    updateContent();

    connect(newState, SIGNAL(stateConfigurationChanged()), this, SLOT(updateContent()));
}

void StateEditorTab::setEditName()
{
    textStateName->selectAll();
    textStateName->setFocus();
}

void StateEditorTab::updateContent()
{
    textStateName->setText(state->getName());

    actionList->clearContents();
    actionList->setRowCount(0);
    foreach(LogicVariable* var, state->getActions())
    {
        actionList->insertRow(actionList->rowCount());
        actionList->setItem(actionList->rowCount()-1, 1, new QTableWidgetItem(var->getName()));
    }
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

void StateEditorTab::addAction()
{
    QMenu *menu = new QMenu();

    QList<LogicVariable*> availableActions = state->getOwningMachine()->getWrittableVariables();

    if (availableActions.count() != 0)
    {
        menu->setStyleSheet( QString( "background-color: lightgrey; border: 3px double"));

        foreach(LogicVariable* var, availableActions)
        {
            if (!state->getActions().contains(var))
                menu->addAction(var->getName());
        }

        connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(treatMenuAdd(QAction*)));
    }
    else
    {
        menu->addAction("No compatible signal!");

        menu[0].setStyleSheet( QString( "background-color: lightgrey; border: 3px solid red; color: red"));

    }

    menu->popup(buttonAddAction->mapToGlobal(QPoint(buttonAddAction->width(), -menu->sizeHint().height())));

}

void StateEditorTab::removeAction()
{
    QList<QTableWidgetItem*> selection = actionList->selectedItems();

    foreach (QTableWidgetItem* currentItem, selection)
    {
        if (currentItem->column() == 1)
        {
            state->removeAction(currentItem->text());
        }
    }
}

void StateEditorTab::treatMenuAdd(QAction* action)
{
    state->addAction(action->text());
}
