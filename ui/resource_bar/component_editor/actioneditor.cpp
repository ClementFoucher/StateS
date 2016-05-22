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

#include <QDebug>

// StateS classes
#include "machineactuatorcomponent.h"
#include "actionlisteditor.h"
#include "contextmenu.h"
#include "machine.h"
#include "signal.h"
#include "rangeeditordialog.h"
#include "collapsiblewidgetwithtitle.h"
#include "statesexception.h"
#include "dynamictableitemdelegate.h"
#include "dynamiclineedit.h"

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

    this->actionList = new QTableWidget(0, 3);
    this->actionList->setHorizontalHeaderItem(0, new QTableWidgetItem());
    this->actionList->setHorizontalHeaderItem(1, new QTableWidgetItem());
    this->actionList->setHorizontalHeaderItem(2, new QTableWidgetItem());
    this->actionList->horizontalHeaderItem(0)->setText(tr("Type"));
    this->actionList->horizontalHeaderItem(1)->setText(tr("Signal"));
    this->actionList->horizontalHeaderItem(2)->setText(tr("Value"));
    this->actionList->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(this->actionList, &QTableWidget::itemSelectionChanged, this, &ActionEditor::updateButtonsState);
    connect(this->actionList, &QTableWidget::itemDoubleClicked,    this, &ActionEditor::editValue);
    this->listDelegate = new DynamicTableItemDelegate(this->actionList);
    this->actionList->setItemDelegate(this->listDelegate);
    layout->addWidget(this->actionList, 1, 0, 1, 2);

    this->buttonAddAction = new QPushButton(tr("Add action"));
    connect(this->buttonAddAction, &QAbstractButton::clicked, this, &ActionEditor::addAction);
    layout->addWidget(this->buttonAddAction, 2, 0, 1, 1);

    this->buttonRemoveAction = new QPushButton(tr("Remove action"));
    connect(this->buttonRemoveAction, &QAbstractButton::clicked, this, &ActionEditor::removeAction);
    layout->addWidget(this->buttonRemoveAction, 2, 1, 1, 1);

    this->hintDisplay = new CollapsibleWidgetWithTitle();
    QString hintTitle = tr("Hint:") + " " + tr("Editing actions");

    QString hint;
    hint += "<br />";
    hint += tr("Double-click") + " " + tr("on an affected value") + " " + tr("to edit it.");
    hint += "<br />";
    hint += tr("Right-click") + " " + tr("on a vector signal") + " " + tr("to display range options.");
    hint += "<br />";

    QLabel* hintText = new QLabel(hint);
    hintText->setAlignment(Qt::AlignCenter);
    hintText->setWordWrap(true);

    this->hintDisplay->setContent(hintTitle, hintText, true);

    layout->addWidget(this->hintDisplay, 3, 0, 1, 2);

    this->updateContent();
}

void ActionEditor::keyPressEvent(QKeyEvent* e)
{
    bool transmitEvent = true;

    if (e->key() == Qt::Key::Key_Delete)
    {
        if (actionList->selectedItems().count() >= 1)
        {
            removeAction();
        }
        transmitEvent = false;
    }
    else if (e->key() == Qt::Key::Key_Escape)
    {
        this->cancelEdit();
        transmitEvent = false;
    }

    if (transmitEvent == true)
        QWidget::keyPressEvent(e);
}

void ActionEditor::keyReleaseEvent(QKeyEvent* e)
{
    bool transmitEvent = true;

    if (e->key() == Qt::Key::Key_Delete)
    {
        transmitEvent = false;
    }
    else if (e->key() == Qt::Key::Key_Escape)
    {
        transmitEvent = false;
    }

    if (transmitEvent == true)
        QWidget::keyReleaseEvent(e);
}

void ActionEditor::contextMenuEvent(QContextMenuEvent* event)
{
    QPoint correctedPos = this->actionList->mapFromParent(event->pos());
    correctedPos.setX(correctedPos.x() - this->actionList->verticalHeader()->width());
    correctedPos.setY(correctedPos.y() - this->actionList->horizontalHeader()->height());
    QTableWidgetItem* cellUnderMouse = this->actionList->itemAt(correctedPos);

    if (cellUnderMouse != nullptr)
    {
        shared_ptr<Signal> currentSignal = this->tableItemsMapping[cellUnderMouse].lock();
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

        if ( (currentSignal != nullptr) && (l_actuator != nullptr) )
        {
            this->currentSignal = currentSignal;

            ContextMenu* menu = new ContextMenu();
            menu->addTitle(tr("Action on signal") + " <i>" + currentSignal->getName() + "</i>");

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
    this->actionList->clearContents();
    this->actionList->setRowCount(0);
    this->tableItemsMapping.clear();
    this->listDelegate->setValidator(nullptr);

    shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

    if (l_actuator != nullptr)
    {
        foreach(shared_ptr<Signal> sig, l_actuator->getActions())
        {
            this->actionList->insertRow(this->actionList->rowCount());

            // Add action list cell
            this->actionList->setCellWidget(this->actionList->rowCount()-1, 0, new ActionListEditor(l_actuator, sig));

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
            Qt::ItemFlags currentFlags = currentTableWidget->flags();
            currentTableWidget->setFlags(currentFlags & ~Qt::ItemIsEditable);
            this->actionList->setItem(this->actionList->rowCount()-1, 1, currentTableWidget);
            this->tableItemsMapping[currentTableWidget] = sig;

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

                currentFlags = currentTableWidget->flags();
                currentFlags = currentFlags & ~Qt::ItemIsEnabled;
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

                    currentFlags = currentTableWidget->flags();
                    currentFlags = currentFlags & ~Qt::ItemIsEnabled;

                    break;
                case MachineActuatorComponent::action_types::reset:
                    actionValue = LogicValue::getValue0(sig->getSize());
                    currentTableWidget = new QTableWidgetItem(actionValue.toString());

                    currentFlags = currentTableWidget->flags();
                    currentFlags = currentFlags & ~Qt::ItemIsEnabled;

                    break;
                case MachineActuatorComponent::action_types::activeOnState:
                case MachineActuatorComponent::action_types::assign:
                case MachineActuatorComponent::action_types::pulse:
                    actionValue = l_actuator->getActionValue(sig);
                    currentTableWidget = new QTableWidgetItem(actionValue.toString());

                    currentFlags = currentTableWidget->flags();

                    break;
                }

            }

            currentFlags = currentFlags & ~Qt::ItemIsEditable;
            currentTableWidget->setFlags(currentFlags);

            this->actionList->setItem(this->actionList->rowCount()-1, 2, currentTableWidget);
            this->tableItemsMapping[currentTableWidget] = sig;
        }
    }

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

void ActionEditor::editValue(QTableWidgetItem* item)
{
    if ( ( (item->flags() & Qt::ItemIsEnabled) != 0 ) && ( item->column() == 2 ) )
    {
        if (this->itemUnderEdition == nullptr) // Do not allow multiple simultaneous editions
        {
            shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
            shared_ptr<Signal> sig = this->tableItemsMapping[item].lock();

            if ( (l_actuator != nullptr) && (sig != nullptr) )
            {
                uint currentValueSize = l_actuator->getActionValue(sig).getSize();
                QRegularExpression re("[01]{0," + QString::number(currentValueSize) + "}");
                this->listDelegate->setValidator(shared_ptr<QValidator>(new QRegularExpressionValidator(re, 0)));

                Qt::ItemFlags currentFlags = item->flags();
                item->setFlags(currentFlags | Qt::ItemIsEditable);

                this->actionList->openPersistentEditor(item);

                this->itemUnderEdition = item;
                DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
                connect(editor, &DynamicLineEdit::returnPressed,   this, &ActionEditor::validateEdit);
            }
        }
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

            menu->popup(this->buttonAddAction->mapToGlobal(QPoint(this->buttonAddAction->width(), -menu->sizeHint().height())));
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

void ActionEditor::validateEdit()
{
    if (this->itemUnderEdition != nullptr)
    {
        DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
        disconnect(editor, &DynamicLineEdit::returnPressed,   this, &ActionEditor::validateEdit);

        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
        shared_ptr<Signal> l_signal = this->tableItemsMapping[this->itemUnderEdition].lock();

        if ( (l_actuator != nullptr) && (l_signal != nullptr) )
        {
            if (this->itemUnderEdition->column() == 2)
            {
                int param1 = l_actuator->getActionParam1(l_signal);
                int param2 = l_actuator->getActionParam2(l_signal);

                try
                {
                    DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

                    LogicValue value = LogicValue::fromString(editor->text()); // Throws StatesException
                    bool result = l_actuator->setActionValue(l_signal, value, param1, param2);

                    if (result == false)
                    {
                        this->editValue(this->itemUnderEdition);
                        this->listDelegate->getCurentEditor()->markAsErroneous();
                    }

                    this->itemUnderEdition = nullptr;
                }
                catch (const StatesException& e)
                {
                    if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
                    {
                        qDebug() << "(ActionEditor:) Info: Wrong input for action value, change ignored.";
                    }
                    else
                        throw;
                }
            }
        }
        else
        {
            this->itemUnderEdition = nullptr;
            this->updateContent();
        }
    }
}

void ActionEditor::cancelEdit()
{
    if (this->itemUnderEdition != nullptr)
    {
        DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
        disconnect(editor, &DynamicLineEdit::returnPressed,   this, &ActionEditor::validateEdit);

        this->itemUnderEdition = nullptr;
    }

    this->updateContent();
}

void ActionEditor::treatMenuEventHandler(QAction* action)
{
    QVariant data = action->data();
    int dataValue = data.toInt();

    shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
    shared_ptr<Signal> l_currentSignal = this->currentSignal.lock();

    try
    {
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
                    newInitialValue.resize(l_currentSignal->getSize()); // Throws StatesException
                    l_actuator->setActionValue(l_currentSignal, newInitialValue, -1, -1);
                }
                break;
            case ContextAction::AffectSwitchSingle:
                if (param1 == -1)
                {
                    LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                    newInitialValue.resize(1); // Throws StatesException: ignored, value is not 0
                    l_actuator->setActionValue(l_currentSignal, newInitialValue, 0, -1);
                }
                else if  (param2 != -1)
                {
                    LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                    newInitialValue.resize(1); // Throws StatesException: ignored, value is not 0
                    l_actuator->setActionValue(l_currentSignal, newInitialValue, param1, -1);
                }
                break;
            case ContextAction::AffectSwitchRange:
                if (param1 == -1)
                {
                    LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                    newInitialValue.resize(2); // Throws StatesException: ignored, value is not 0
                    l_actuator->setActionValue(l_currentSignal, newInitialValue, 1, 0);
                }
                else if  (param2 == -1)
                {
                    if (param1 != 0)
                    {
                        LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                        newInitialValue.resize(param1 + 1); // Throws StatesException
                        l_actuator->setActionValue(l_currentSignal, newInitialValue, param1, 0);
                    }
                    else
                    {
                        LogicValue newInitialValue = l_actuator->getActionValue(l_currentSignal);
                        newInitialValue.resize(2); // Throws StatesException: ignored, value is not 0
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
                        newAffectValue.resize(newParam1 - newParam2 + 1); // Throws StatesException
                    else
                        newAffectValue.resize(1); // Throws StatesException: ignored, value is not 0
                    l_actuator->setActionValue(l_currentSignal, newAffectValue, newParam1, newParam2);
                }
                break;
            }
        }
    }
    catch (const StatesException& e)
    {
        if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::resized_to_0) )
        {
            qDebug() << "(ActionEditor:) Warning: Error trying to resize a signal. A param value is probably broken.";
        }
        else
            throw;
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
            selectionString.append(this->tableItemsMapping[currentItem].lock());
        }
    }

    return selectionString;
}
