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
#include "actioneditor.h"

// Qt classes
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>

// StateS classes
#include "machineactuatorcomponent.h"
#include "actionlisteditor.h"
#include "contextmenu.h"
#include "machine.h"
#include "signal.h"


ActionEditor::ActionEditor(shared_ptr<MachineActuatorComponent> actuator, QString title, QWidget* parent) :
    QWidget(parent)
{
    this->actuator = actuator;
    connect(actuator.get(), &MachineActuatorComponent::actionListChangedEvent, this, &ActionEditor::updateContent);

    QGridLayout* layout = new QGridLayout(this);

    if (title.size() != 0)
    {
        QLabel* actionListTitle = new QLabel(title);
        actionListTitle->setAlignment(Qt::AlignCenter);
        actionListTitle->setWordWrap(true);
        layout->addWidget(actionListTitle, 0, 0, 1, 2);
    }

    actionList = new QTableWidget(0, 3);
    actionList->setHorizontalHeaderItem(0, new QTableWidgetItem());
    actionList->setHorizontalHeaderItem(1, new QTableWidgetItem());
    actionList->setHorizontalHeaderItem(2, new QTableWidgetItem());
    actionList->horizontalHeaderItem(0)->setText(tr("Type"));
    actionList->horizontalHeaderItem(1)->setText(tr("Signal"));
    actionList->horizontalHeaderItem(2)->setText(tr("Value"));
    actionList->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(actionList, 1, 0, 1, 2);

    buttonAddAction = new QPushButton(tr("Add action"));
    connect(buttonAddAction, &QAbstractButton::clicked, this, &ActionEditor::addAction);
    layout->addWidget(buttonAddAction, 2, 0, 1, 1);

    buttonRemoveAction = new QPushButton(tr("Remove action"));
    connect(buttonRemoveAction, &QAbstractButton::clicked, this, &ActionEditor::removeAction);
    layout->addWidget(buttonRemoveAction, 2, 1, 1, 1);

    connect(actionList, &QTableWidget::itemChanged, this, &ActionEditor::itemValueChangedEventHandler);

    this->updateContent();
}

void ActionEditor::changeActuator(shared_ptr<MachineActuatorComponent> actuator)
{
    disconnect(this->actuator.lock().get(), &MachineActuatorComponent::actionListChangedEvent, this, &ActionEditor::updateContent);

    this->actuator = actuator;
    this->updateContent();

    connect(actuator.get(), &MachineActuatorComponent::actionListChangedEvent, this, &ActionEditor::updateContent);
}

void ActionEditor::updateContent()
{
    disconnect(actionList, &QTableWidget::itemChanged, this, &ActionEditor::itemValueChangedEventHandler);

    actionList->clearContents();
    actionList->setRowCount(0);
    tableItemsMapping.clear();

    shared_ptr<MachineActuatorComponent> actuator = this->actuator.lock();

    if (actuator != nullptr)
    {
        foreach(shared_ptr<Signal> sig, actuator->getActions())
        {
            actionList->insertRow(actionList->rowCount());

            actionList->setCellWidget(actionList->rowCount()-1, 0, new ActionListEditor(actuator, sig));

            QTableWidgetItem* currentTableWidget = new QTableWidgetItem(sig->getName());
            currentTableWidget->setFlags(currentTableWidget->flags() ^ Qt::ItemIsEditable);
            actionList->setItem(actionList->rowCount()-1, 1, currentTableWidget);

            LogicValue actionValue = actuator->getActionValue(sig);


            if (actuator->getActionType(sig) == MachineActuatorComponent::action_types::activeOnState)
            {
                currentTableWidget = new QTableWidgetItem("1");
                currentTableWidget->setFlags(0);
                actionList->setItem(actionList->rowCount()-1, 2, currentTableWidget);
            }
            else if (actuator->getActionType(sig) == MachineActuatorComponent::action_types::pulse)
            {
                currentTableWidget = new QTableWidgetItem("1");
                currentTableWidget->setFlags(0);
                actionList->setItem(actionList->rowCount()-1, 2, currentTableWidget);
            }
            else if (actuator->getActionType(sig) == MachineActuatorComponent::action_types::set)
            {
                currentTableWidget = new QTableWidgetItem("1");
                currentTableWidget->setFlags(0);
                actionList->setItem(actionList->rowCount()-1, 2, currentTableWidget);
            }
            else if (actuator->getActionType(sig) == MachineActuatorComponent::action_types::reset)
            {
                currentTableWidget = new QTableWidgetItem(LogicValue::getValue0(sig->getSize()).toString());
                currentTableWidget->setFlags(0);
                actionList->setItem(actionList->rowCount()-1, 2, currentTableWidget);
            }
            else if (actuator->getActionType(sig) == MachineActuatorComponent::action_types::assign)
            {
                currentTableWidget = new QTableWidgetItem(actionValue.toString());
                currentTableWidget->setFlags(currentTableWidget->flags() | Qt::ItemIsEditable);
                tableItemsMapping[currentTableWidget] = sig;
                actionList->setItem(actionList->rowCount()-1, 2, currentTableWidget);
            }
        }
    }

    connect(actionList, &QTableWidget::itemChanged, this, &ActionEditor::itemValueChangedEventHandler);
}

void ActionEditor::addAction()
{
    ContextMenu* menu;

    QList<shared_ptr<Signal>> availableActions;

    shared_ptr<MachineActuatorComponent> actuator = this->actuator.lock();

    if (actuator != nullptr)
    {
        shared_ptr<Machine> owningMachine = actuator->getOwningMachine();

        if (owningMachine != nullptr)
        {
            foreach(shared_ptr<Signal> var, owningMachine->getWrittableSignals())
            {
                if (!actuator->getActions().contains(var))
                    availableActions.append(var);
            }

            if (availableActions.count() != 0)
            {
                menu = new ContextMenu();
                menu->setListStyle();

                foreach(shared_ptr<Signal> var, availableActions)
                {
                    menu->addAction(var->getName());
                }

                connect(menu, &QMenu::triggered, this, &ActionEditor::treatMenuAdd);
            }
            else
            {
                menu = ContextMenu::createErrorMenu(tr("No compatible signal!"));
            }

            menu->popup(buttonAddAction->mapToGlobal(QPoint(buttonAddAction->width(), -menu->sizeHint().height())));
        }
    }
    else
    {
        this->updateContent();
    }
}

void ActionEditor::removeAction()
{
    shared_ptr<MachineActuatorComponent> actuator = this->actuator.lock();

    if (actuator != nullptr)
    {
        QList<QTableWidgetItem*> selection = actionList->selectedItems();

        foreach (QTableWidgetItem* currentItem, selection)
        {
            if (currentItem->column() == 1)
            {
                actuator->removeActionByName(currentItem->text());
            }
        }
    }
    else
    {
        this->updateContent();
    }
}

void ActionEditor::treatMenuAdd(QAction* action)
{
    shared_ptr<MachineActuatorComponent> actuator = this->actuator.lock();

    if (actuator != nullptr)
    {
        actuator->addActionByName(action->text());
    }
    else
    {
        this->updateContent();
    }
}

void ActionEditor::itemValueChangedEventHandler(QTableWidgetItem* item)
{
    shared_ptr<MachineActuatorComponent> actuator = this->actuator.lock();

    if (actuator != nullptr)
    {
        if (item->column() == 2)
        {
            actuator->setActionValue(tableItemsMapping[item].lock(), LogicValue::fromString(item->text()));
        }
    }
    else
    {
        this->updateContent();
    }

}
