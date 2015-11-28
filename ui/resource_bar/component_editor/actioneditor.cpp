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
#include <QKeyEvent>
#include <QHeaderView>

// StateS classes
#include "machineactuatorcomponent.h"
#include "actionlisteditor.h"
#include "contextmenu.h"
#include "machine.h"
#include "signal.h"
#include "rangeeditordialog.h"


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
    connect(actionList, &QTableWidget::itemSelectionChanged, this, &ActionEditor::updateButtonsState);
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

void ActionEditor::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key::Key_Delete)
    {
        if (actionList->selectedItems().count() >= 1)
        {
            removeAction();
            return;
        }
    }

    QWidget::keyPressEvent(e);
}

void ActionEditor::contextMenuEvent(QContextMenuEvent* event)
{
    QPoint correctedPos = actionList->mapFromParent(event->pos());
    correctedPos.setX(correctedPos.x() - actionList->verticalHeader()->width());
    correctedPos.setY(correctedPos.y() - actionList->horizontalHeader()->height());
    QTableWidgetItem* cellUnderMouse = actionList->itemAt(correctedPos);

    if (cellUnderMouse != nullptr)
    {
        shared_ptr<Signal> currentSignal = tableItemsMapping[cellUnderMouse].lock();
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

        if ( (currentSignal != nullptr) && (l_actuator != nullptr) )
        {
            this->currentSignal = currentSignal;

            ContextMenu* menu = new ContextMenu();
            menu->addTitle(tr("Action on signal") + " " + currentSignal->getName());

            QVariant data;
            data.convert(QVariant::Int);
            QAction* actionToAdd = nullptr;

            if (currentSignal->getSize() > 1)
            {

                actionToAdd = menu->addAction(tr("Affect whole signal"));
                actionToAdd->setCheckable(true);
                if (l_actuator->getActionParam1(currentSignal) == -1)
                    actionToAdd->setChecked(true);
                data.setValue((int)ContextAction::AffectSwitchWhole);
                actionToAdd->setData(data);

                actionToAdd = menu->addAction(tr("Affect signal single bit"));
                actionToAdd->setCheckable(true);
                if ( (l_actuator->getActionParam1(currentSignal) != -1) && (l_actuator->getActionParam2(currentSignal) == -1) )
                        actionToAdd->setChecked(true);
                data.setValue((int)ContextAction::AffectSwitchSingle);
                actionToAdd->setData(data);

                actionToAdd = menu->addAction(tr("Affect signal range"));
                actionToAdd->setCheckable(true);
                if ( (l_actuator->getActionParam1(currentSignal) != -1) && (l_actuator->getActionParam2(currentSignal) != -1) )
                        actionToAdd->setChecked(true);
                data.setValue((int)ContextAction::AffectSwitchRange);
                actionToAdd->setData(data);

                menu->addSeparator();

                if ( (l_actuator->getActionParam1(currentSignal) != -1) || (l_actuator->getActionParam2(currentSignal) != -1) )
                {
                    if (l_actuator->getActionParam2(currentSignal) == -1)
                        actionToAdd = menu->addAction(tr("Edit affected bit"));
                    else
                        actionToAdd = menu->addAction(tr("Edit range"));

                    data.setValue((int)ContextAction::AffectEditRange);
                    actionToAdd->setData(data);

                    menu->addSeparator();
                }
            }

            actionToAdd = menu->addAction(tr("Delete action"));
            data.setValue((int)ContextAction::DeleteAction);
            actionToAdd->setData(data);

            actionToAdd = menu->addAction(tr("Cancel"));
            data.setValue((int)ContextAction::Cancel);
            actionToAdd->setData(data);

            menu->popup(this->mapToGlobal(event->pos()));

            connect(menu, &QMenu::triggered, this, &ActionEditor::treatMenuEventHandler);
        }
    }
}

void ActionEditor::updateContent()
{
    disconnect(actionList, &QTableWidget::itemChanged, this, &ActionEditor::itemValueChangedEventHandler);

    actionList->clearContents();
    actionList->setRowCount(0);
    tableItemsMapping.clear();

    shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

    if (l_actuator != nullptr)
    {
        foreach(shared_ptr<Signal> sig, l_actuator->getActions())
        {
            actionList->insertRow(actionList->rowCount());

            // Add action list cell
            actionList->setCellWidget(actionList->rowCount()-1, 0, new ActionListEditor(l_actuator, sig));

            // Build name
            QString nameText = sig->getName();
            if (sig->getSize() > 1)
            {
                int param1 = l_actuator->getActionParam1(sig);
                int param2 = l_actuator->getActionParam2(sig);

                if (param1 != -1)
                {
                    nameText += "[" + QString::number(param1);

                    if (param2 != -1)
                    {
                        nameText += ".." + QString::number(param2);
                    }

                    nameText += "]";
                }
            }

            // Add signal name cell
            QTableWidgetItem* currentTableWidget = new QTableWidgetItem(nameText);
            currentTableWidget->setFlags(currentTableWidget->flags() ^ Qt::ItemIsEditable);
            actionList->setItem(actionList->rowCount()-1, 1, currentTableWidget);
            tableItemsMapping[currentTableWidget] = sig;

            // Add action value cell

            int actionValueSize;

            if (sig->getSize() == 1)
                actionValueSize = 1;
            else
            {
                int param1 = l_actuator->getActionParam1(sig);
                int param2 = l_actuator->getActionParam2(sig);

                if ( (param1 != -1) && (param2 == -1) )
                    actionValueSize = 1;
                else if ( (param1 == -1) && (param2 == -1) )
                    actionValueSize = sig->getSize();
                else
                    actionValueSize = param1 - param2 + 1;
            }

            if (actionValueSize == 1)
            {
                LogicValue actionValue;

                // Values are always implicit
                switch (l_actuator->getActionType(sig))
                {
                case MachineActuatorComponent::action_types::activeOnState:
                case MachineActuatorComponent::action_types::assign:
                case MachineActuatorComponent::action_types::pulse:
                case MachineActuatorComponent::action_types::set:
                    actionValue = LogicValue::getValue1(1);
                    break;
                case MachineActuatorComponent::action_types::reset:
                    actionValue = LogicValue::getValue0(1);
                    break;
                }

                currentTableWidget = new QTableWidgetItem(actionValue.toString());
                currentTableWidget->setFlags(0);
            }
            else
            {
                LogicValue actionValue;

                // Values are always implicit
                switch (l_actuator->getActionType(sig))
                {
                case MachineActuatorComponent::action_types::set:
                    actionValue = LogicValue::getValue1(sig->getSize());
                    currentTableWidget = new QTableWidgetItem(actionValue.toString());
                    currentTableWidget->setFlags(0);
                    break;
                case MachineActuatorComponent::action_types::reset:
                    actionValue = LogicValue::getValue0(sig->getSize());
                    currentTableWidget = new QTableWidgetItem(actionValue.toString());
                    currentTableWidget->setFlags(0);
                    break;
                case MachineActuatorComponent::action_types::activeOnState:
                case MachineActuatorComponent::action_types::assign:
                case MachineActuatorComponent::action_types::pulse:
                    actionValue = l_actuator->getActionValue(sig);
                    currentTableWidget = new QTableWidgetItem(actionValue.toString());
                    currentTableWidget->setFlags(currentTableWidget->flags() | Qt::ItemIsEditable);
                    break;
                }
            }

            actionList->setItem(actionList->rowCount()-1, 2, currentTableWidget);
            tableItemsMapping[currentTableWidget] = sig;
        }
    }

    connect(actionList, &QTableWidget::itemChanged, this, &ActionEditor::itemValueChangedEventHandler);
    this->updateButtonsState();
}

void ActionEditor::updateButtonsState()
{
    // TODO: handle multiple items when undo is implemented
    if (this->getSelectedSignals().count() == 1)
    {
        this->buttonRemoveAction->setEnabled(true);
    }
    else
    {
        this->buttonRemoveAction->setEnabled(false);
    }
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
        QList<shared_ptr<Signal>> list = this->getSelectedSignals();

        // TODO: handle multiple items when undo is implemented
        if (list.count() == 1)
        {
            actuator->removeActionByName(list[0]->getName());
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
    shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
    shared_ptr<Signal> l_signal = tableItemsMapping[item].lock();

    if ( (l_actuator != nullptr) && (l_signal != nullptr) )
    {
        if (item->column() == 2)
        {
            int param1 = l_actuator->getActionParam1(l_signal);
            int param2 = l_actuator->getActionParam2(l_signal);

            l_actuator->setActionValue(l_signal, LogicValue::fromString(item->text()), param1, param2);
        }
    }
    else
    {
        this->updateContent();
    }

}

void ActionEditor::treatMenuEventHandler(QAction* action)
{
    QVariant data = action->data();
    int dataValue = data.toInt();

    shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
    shared_ptr<Signal> l_currentSignal = this->currentSignal.lock();

    int param1 = l_actuator->getActionParam1(l_currentSignal);
    int param2 = l_actuator->getActionParam2(l_currentSignal);

    if ( (l_actuator != nullptr) && (l_currentSignal != nullptr) )
    {
        switch (dataValue)
        {
        case ContextAction::Cancel:
            break;
        case ContextAction::DeleteAction:
            l_actuator->removeActionByName(l_currentSignal->getName());
            break;
        case ContextAction::AffectSwitchWhole:
            if ( (param1 != -1) || (param2 != -1) )
            {
                LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                newInitialValue.resize(l_currentSignal->getSize());
                l_actuator->setActionValue(l_currentSignal, newInitialValue, -1, -1);
            }
            break;
        case ContextAction::AffectSwitchSingle:
            if (param1 == -1)
            {
                LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                newInitialValue.resize(1);
                l_actuator->setActionValue(l_currentSignal, newInitialValue, 0, -1);
            }
            else if  (param2 != -1)
            {
                LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                newInitialValue.resize(1);
                l_actuator->setActionValue(l_currentSignal, newInitialValue, param1, -1);
            }
            break;
        case ContextAction::AffectSwitchRange:
            if (param1 == -1)
            {
                LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                newInitialValue.resize(2);
                l_actuator->setActionValue(l_currentSignal, newInitialValue, 1, 0);
            }
            else if  (param2 == -1)
            {
                if (param1 != 0)
                {
                    LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                    newInitialValue.resize(param1 + 1);
                    l_actuator->setActionValue(l_currentSignal, newInitialValue, param1, 0);
                }
                else
                {
                    LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                    newInitialValue.resize(2);
                    l_actuator->setActionValue(l_currentSignal, newInitialValue, 1, 0);
                }
            }
            break;
        case ContextAction::AffectEditRange:
            unique_ptr<RangeEditorDialog>rangeEditor = unique_ptr<RangeEditorDialog>(new RangeEditorDialog(l_actuator, l_currentSignal));
            rangeEditor->exec();

            if (rangeEditor->result() == QDialog::Accepted)
            {
                LogicValue newAffectValue = l_actuator->getActionValue(l_currentSignal);
                int newParam1 = rangeEditor->getParam1();
                int newParam2 = rangeEditor->getParam2();

                if (newParam2 != -1)
                    newAffectValue.resize(newParam1 - newParam2 + 1);
                else
                    newAffectValue.resize(1);
                l_actuator->setActionValue(l_currentSignal, newAffectValue, newParam1, newParam2);
            }
            break;
        }
    }

    this->currentSignal.reset();
}

QList<shared_ptr<Signal>> ActionEditor::getSelectedSignals()
{
    QList<shared_ptr<Signal>> selectionString;

    foreach (QModelIndex index, this->actionList->selectionModel()->selectedRows(1))
    {
        QTableWidgetItem* currentItem = this->actionList->item(index.row(), 1);
        if (currentItem != nullptr)
        {
            selectionString.append(tableItemsMapping[currentItem].lock());
        }
    }

    return selectionString;
}
