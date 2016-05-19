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
#include "signallisteditor.h"

// Qt classes
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QHeaderView>

#include <QDebug>

// StateS classes
#include "dynamiclineedit.h"
#include "signal.h"
#include "dynamictableitemdelegate.h"
#include "tablewidgetwithresizeevent.h"
#include "contextmenu.h"
#include "statesexception.h"


SignalListEditor::SignalListEditor(shared_ptr<Machine> machine, Machine::signal_type editorType, QWidget* parent) :
    QWidget(parent)
{
    this->machine = machine;
    this->editorType = editorType;

    QVBoxLayout* layout = new QVBoxLayout(this);

    if (editorType == Machine::signal_type::Input)
    {
        this->signalsList = new TableWidgetWithResizeEvent(0, 3);

        this->signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        this->signalsList->horizontalHeaderItem(0)->setText(tr("Input"));

        this->signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        this->signalsList->horizontalHeaderItem(2)->setText(tr("Initial value"));

        connect(machine.get(), &Machine::inputListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Input");
    }
    else if (editorType == Machine::signal_type::Output)
    {
        this->signalsList = new TableWidgetWithResizeEvent(0, 2);

        this->signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        this->signalsList->horizontalHeaderItem(0)->setText(tr("Output"));

        connect(machine.get(), &Machine::outputListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Output");
    }
    else if (editorType == Machine::signal_type::LocalVariable)
    {
        this->signalsList = new TableWidgetWithResizeEvent(0, 3);

        this->signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        this->signalsList->horizontalHeaderItem(0)->setText(tr("Variable"));

        this->signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        this->signalsList->horizontalHeaderItem(2)->setText(tr("Initial value"));

        connect(machine.get(), &Machine::localVariableListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Variable");
    }
    else if (editorType == Machine::signal_type::Constant)
    {
        this->signalsList = new TableWidgetWithResizeEvent(0, 3);

        this->signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        this->signalsList->horizontalHeaderItem(0)->setText(tr("Constant"));

        this->signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        this->signalsList->horizontalHeaderItem(2)->setText(tr("Value"));

        connect(machine.get(), &Machine::constantListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Constant");
    }

    this->newSignalsPrefix += " #";

    this->signalsList->setHorizontalHeaderItem(1, new QTableWidgetItem());
    this->signalsList->horizontalHeaderItem(1)->setText(tr("Size"));

    this->signalsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->listDelegate = new DynamicTableItemDelegate(signalsList);
    this->signalsList->setItemDelegate(listDelegate);

    // Don't allow to adjust height
    this->signalsList->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    layout->addWidget(this->signalsList);

    // Buttons

    this->buttonLayout = new QGridLayout();

    this->buttonAdd    = new QPushButton(tr("Add"));
    this->buttonRemove = new QPushButton(tr("Remove"));
    this->buttonUp     = new QPushButton("↥");
    this->buttonDown   = new QPushButton("↧");

    // Row 0 reserved for OK/Cancel buttons
    // Row 1 for edition buttons
    // Size 1 for arrows, 20 for other buttons = 42
    // TODO: correct these sizes
    this->buttonLayout->addWidget(this->buttonUp,     1, 0,  1, 1);
    this->buttonLayout->addWidget(this->buttonDown,   1, 1,  1, 1);
    this->buttonLayout->addWidget(this->buttonAdd,    1, 2,  1, 20);
    this->buttonLayout->addWidget(this->buttonRemove, 1, 22, 1, 20);

    layout->addLayout(this->buttonLayout);

    connect(this->buttonUp,     &QAbstractButton::clicked, this, &SignalListEditor::raiseSignal);
    connect(this->buttonDown,   &QAbstractButton::clicked, this, &SignalListEditor::lowerSignal);
    connect(this->buttonAdd,    &QAbstractButton::clicked, this, &SignalListEditor::beginAddSignal);
    connect(this->buttonRemove, &QAbstractButton::clicked, this, &SignalListEditor::removeSelectedSignals);

    // To enable/disable buttons when a signal is selected
    connect(this->signalsList,  &QTableWidget::itemSelectionChanged, this, &SignalListEditor::updateButtonsEnableState);

    connect(this->signalsList, &TableWidgetWithResizeEvent::resized, this, &SignalListEditor::handleListResizedEvent);

    this->updateList();
}

/**
 * @brief SignalListEditor::updateList
 * UpdateList totally resets the widget.
 * The list is filled with signals existing
 * in machine at this time.
 * This can be called to take into account the
 * creation/edition of a signal, or to cancel
 * a creation/edition that has not been validated.
 */
void SignalListEditor::updateList()
{
    this->signalsList->clearContents();
    this->signalsList->setRowCount(0);
    this->associatedSignals.clear();

    this->switchMode(mode::standard);

    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {

        disconnect(this->signalsList,  &QTableWidget::itemSelectionChanged, this, &SignalListEditor::updateButtonsEnableState);

        // Get signals I have to deal with
        QList<shared_ptr<Signal>> signalsToAdd;

        if (this->editorType == Machine::signal_type::Input)
        {
            signalsToAdd = l_machine->getInputsAsSignals();
        }
        else if (this->editorType == Machine::signal_type::Output)
        {
            signalsToAdd = l_machine->getOutputsAsSignals();
        }
        else if (this->editorType == Machine::signal_type::LocalVariable)
        {
            signalsToAdd = l_machine->getLocalVariables();
        }
        else if (this->editorType == Machine::signal_type::Constant)
        {
            signalsToAdd = l_machine->getConstants();
        }

        foreach (shared_ptr<Signal> sig, signalsToAdd)
        {
            this->signalsList->insertRow(this->signalsList->rowCount());

            // Signal name
            QTableWidgetItem* currentItem = new QTableWidgetItem(sig->getName());
            Qt::ItemFlags currentFlags = currentItem->flags();
            currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
            this->signalsList->setItem(this->signalsList->rowCount()-1, 0, currentItem);
            this->associatedSignals[currentItem] = sig;

            // Signal size
            currentItem = new QTableWidgetItem(QString::number(sig->getSize()));
            currentFlags = currentItem->flags();
            currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
            this->signalsList->setItem(this->signalsList->rowCount()-1, 1, currentItem);
            this->associatedSignals[currentItem] = sig;

            // Signal (initial) value
            if (this->signalsList->columnCount() == 3)
            {
                currentItem = new QTableWidgetItem(sig->getInitialValue().toString());
                currentFlags = currentItem->flags();
                currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
                this->signalsList->setItem(this->signalsList->rowCount()-1, 2, currentItem);
                this->associatedSignals[currentItem] = sig;
            }

            // Select signal if it was selected before list clear

            if ( (this->signalSelectionToRestore != QString::null) && (this->signalSelectionToRestore == sig->getName()) )
            {
                this->signalsList->selectRow(this->signalsList->rowCount()-1);
                this->signalSelectionToRestore = QString::null;
            }
        }

        this->signalsList->setFocus();

        this->signalSelectionToRestore = QString::null;
        this->updateButtonsEnableState();
        connect(this->signalsList,  &QTableWidget::itemSelectionChanged, this, &SignalListEditor::updateButtonsEnableState);
    }
}

void SignalListEditor::updateButtonsEnableState()
{
    this->buttonRemove->setEnabled(false);
    this->buttonUp->setEnabled(false);
    this->buttonDown->setEnabled(false);

    if (this->currentMode == mode::standard)
    {
        this->buttonAdd->setEnabled(true);

        if (this->signalsList->selectedItems().count() != 0)
        {
            this->buttonRemove->setEnabled(true);

            // Exactly one line selected
            if (this->signalsList->selectedItems().count() == this->signalsList->columnCount())
            {
                if (this->signalsList->selectedItems()[0]->row() != 0)
                    this->buttonUp->setEnabled(true);

                if (this->signalsList->selectedItems()[0]->row() != this->signalsList->rowCount()-1)
                    this->buttonDown->setEnabled(true);
            }
        }
    }
    else
    {
        this->buttonAdd->setEnabled(false);
    }
}

void SignalListEditor::handleListResizedEvent()
{
    if (this->currentTableItem != nullptr)
    {
        this->signalsList->scrollToItem(this->currentTableItem);
    }
}


void SignalListEditor::keyPressEvent(QKeyEvent* event)
{
    bool transmitEvent = true;

    if (event->key() == Qt::Key::Key_Escape)
    {
        this->cancelCurrentEdit();
        transmitEvent = false;
    }
    else if (event->key() == Qt::Key::Key_Delete)
    {
        // TODO: hanlde multiple deletion when undo is implemented
        if (this->getSelectedSignals().count() == 1)
        {
            removeSelectedSignals();
        }
        transmitEvent = false;
    }
    else if (event->key() == Qt::Key::Key_Tab)
    {
        transmitEvent = false;
    }

    if (transmitEvent)
        QWidget::keyPressEvent(event);
}

void SignalListEditor::keyReleaseEvent(QKeyEvent *event)
{
    bool transmitEvent = true;

    if (event->key() == Qt::Key::Key_Escape)
    {
        transmitEvent = false;
    }
    else if (event->key() == Qt::Key::Key_Delete)
    {
        transmitEvent = false;
    }
    else if (event->key() == Qt::Key::Key_Tab)
    {
        transmitEvent = false;
    }

    if (transmitEvent)
        QWidget::keyPressEvent(event);
}

void SignalListEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QPoint correctedPos = signalsList->mapFromParent(event->pos());
    correctedPos.setX(correctedPos.x() - signalsList->verticalHeader()->width());
    correctedPos.setY(correctedPos.y() - signalsList->horizontalHeader()->height());
    QTableWidgetItem* cellUnderMouse = signalsList->itemAt(correctedPos);

    if (cellUnderMouse != nullptr)
    {
        QList<QString> list = this->getSelectedSignals();
        if (list.count() == 1)
        {
            shared_ptr<Signal> currentSignal = associatedSignals[cellUnderMouse].lock();

            if (currentSignal != nullptr)
            {
                this->currentSignal = currentSignal;

                ContextMenu* menu = new ContextMenu();
                menu->addTitle(tr("Action on signal") + " " + currentSignal->getName());

                QVariant data;
                data.convert(QVariant::Int);
                QAction* actionToAdd = nullptr;

                actionToAdd = menu->addAction(tr("Up"));
                data.setValue((int)ContextAction::Up);
                actionToAdd->setData(data);

                actionToAdd = menu->addAction(tr("Down"));
                data.setValue((int)ContextAction::Down);
                actionToAdd->setData(data);

                menu->addSeparator();

                actionToAdd = menu->addAction(tr("Delete signal"));
                data.setValue((int)ContextAction::DeleteSignal);
                actionToAdd->setData(data);

                actionToAdd = menu->addAction(tr("Cancel"));
                data.setValue((int)ContextAction::Cancel);
                actionToAdd->setData(data);

                menu->popup(this->mapToGlobal(event->pos()));

                connect(menu, &QMenu::triggered, this, &SignalListEditor::treatMenuEventHandler);
            }
        }
        else if (list.count() != 0)
        {
            ContextMenu* menu = new ContextMenu();
            menu->addTitle(tr("Action on all selected signals"));

            QVariant data;
            data.convert(QVariant::Int);
            QAction* actionToAdd = nullptr;

            actionToAdd = menu->addAction(tr("Delete signals"));
            data.setValue((int)ContextAction::DeleteSignal);
            actionToAdd->setData(data);

            actionToAdd = menu->addAction(tr("Cancel"));
            data.setValue((int)ContextAction::Cancel);
            actionToAdd->setData(data);

            menu->popup(this->mapToGlobal(event->pos()));

            connect(menu, &QMenu::triggered, this, &SignalListEditor::treatMenuEventHandler);
        }
    }
}


void SignalListEditor::beginAddSignal()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        QString initialName = l_machine->getUniqueSignalName(this->newSignalsPrefix);

        this->signalsList->insertRow(signalsList->rowCount());

        this->currentSignalName = new QTableWidgetItem(initialName);
        this->signalsList->setItem(this->signalsList->rowCount()-1, 0, this->currentSignalName);

        this->currentSignalSize = new QTableWidgetItem("1");
        Qt::ItemFlags currentFlags = this->currentSignalSize->flags();
        this->currentSignalSize->setFlags(currentFlags & ~Qt::ItemIsEditable);
        this->signalsList->setItem(this->signalsList->rowCount()-1, 1, this->currentSignalSize);

        if (this->signalsList->columnCount() == 3)
        {
            this->currentSignalValue = new QTableWidgetItem("0");
            currentFlags = this->currentSignalValue->flags();
            this->currentSignalValue->setFlags(currentFlags & ~Qt::ItemIsEditable);
            this->signalsList->setItem(this->signalsList->rowCount()-1, 2, this->currentSignalValue);
        }

        this->currentTableItem = this->currentSignalName;
        this->signalsList->selectRow(this->currentTableItem->row());
        switchMode(mode::addingSignal);

        connect(this->signalsList, &QTableWidget::itemClicked, this, &SignalListEditor::addingSignalSwitchField);
        connect(this->signalsList, &QTableWidget::currentItemChanged, this, &SignalListEditor::addingSignalCurrentItemChanged);
    }
}

void SignalListEditor::addingSignalSwitchField(QTableWidgetItem* newItem)
{
    if (newItem->flags() & Qt::ItemIsEnabled) // Ignore clicks on disabled cells
    {
        this->signalsList->closePersistentEditor(this->currentTableItem);

        // Resize value if needed
        if ( (this->currentTableItem == this->currentSignalSize) || (this->currentTableItem == this->currentSignalValue) )
        {
            this->fixSignalSize();
        }

        Qt::ItemFlags currentFlags = this->currentTableItem->flags();
        this->currentTableItem->setFlags(currentFlags & ~Qt::ItemIsEditable);

        this->currentTableItem = newItem;

        this->editCurrentCell();
    }
}

void SignalListEditor::addingSignalCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem*)
{
    if (current != nullptr)
    {
        this->addingSignalSwitchField(current);
    }
}

void SignalListEditor::endAddSignal()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        disconnect(this->signalsList, &QTableWidget::currentItemChanged, this, &SignalListEditor::addingSignalCurrentItemChanged);
        disconnect(this->signalsList, &QTableWidget::itemClicked, this, &SignalListEditor::addingSignalSwitchField);
        this->signalsList->closePersistentEditor(this->currentTableItem);

        QString finalName;

        if (this->currentTableItem == this->currentSignalName)
        {
            DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
            finalName = editor->text();
        }
        else
        {
           finalName = this->currentSignalName->text();
        }

        this->signalSelectionToRestore = finalName;
        LogicValue initialValue = LogicValue::getNullValue();

        if (this->currentSignalValue != nullptr)
        {
            try
            {
                initialValue = LogicValue::fromString(this->currentSignalValue->text()); // Throws StatesException
                uint size = (uint)this->currentSignalSize->text().toInt();

                if (initialValue.getSize() < size)
                    initialValue.resize(size); // Throws StatesException
            }
            catch (const StatesException& e)
            {
                if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
                {
                    qDebug() << "(SignalListEditor:) Info: Wrong input for initial value, change ignored.";
                }
                else
                    throw;
            }
        }
        else
        {
            // To force correct size (actual value is ignored)
            initialValue = LogicValue(this->currentSignalSize->text().toInt());
        }

        // If success, list is reloaded through events,
        // which resets mode.
        shared_ptr<Signal> newSignal = l_machine->addSignal(this->editorType, finalName, initialValue);

        // If adding signal failed, continue editing signal name
        if (newSignal == nullptr)
        {
            this->fixSignalSize();

            this->signalSelectionToRestore = QString::null;
            this->currentTableItem = this->currentSignalName;
            this->currentSignalName->setText(finalName);

            connect(this->signalsList, &QTableWidget::currentItemChanged, this, &SignalListEditor::addingSignalCurrentItemChanged);
            connect(this->signalsList, &QTableWidget::itemClicked, this, &SignalListEditor::addingSignalSwitchField);
            this->editCurrentCell(true);
        }
    }
}


// Begin edit signal is trigerred by double-click on table item
void SignalListEditor::beginEditSignal(QTableWidgetItem* characteristicToEdit)
{
    this->currentTableItem = characteristicToEdit;

    if (characteristicToEdit->column() == 0)
    {
        switchMode(mode::renamingSignal);
    }
    else if (characteristicToEdit->column() == 1)
    {
        switchMode(mode::resizingSignal);
    }
    else if (characteristicToEdit->column() == 2)
    {
        switchMode(mode::changingSignalInitialValue);
    }
}

void SignalListEditor::endRenameSignal()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

        QString finalName = editor->text();

        shared_ptr<Signal> currentSignal = this->associatedSignals[currentTableItem].lock();

        if ( (currentSignal != nullptr) && (finalName != currentSignal->getName()) )
        {
            this->signalSelectionToRestore = finalName;
            bool success = l_machine->renameSignal(currentSignal->getName(), finalName);

            if (!success)
            {
                this->signalSelectionToRestore = QString::null;
                this->editCurrentCell(true);
            }
        }
        else
        {
            // Reset list
            updateList();
        }
    }
}

void SignalListEditor::endResizeSignal()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        try
        {
            DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

            uint finalSize = (uint)editor->text().toInt();

            shared_ptr<Signal> currentSignal = this->associatedSignals[currentTableItem].lock();

            if ( (currentSignal != nullptr) && (finalSize != currentSignal->getSize()) )
            {
                this->signalSelectionToRestore = currentSignal->getName();

                l_machine->resizeSignal(currentSignal->getName(), finalSize); // Throws StatesException
            }
            else
            {
                // Reset list
                updateList();
            }
        }
        catch (const StatesException& e)
        {
            if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::resized_to_0) )
            {
                qDebug() << "(SignalListEditor:) Info: Wrong input for signal size, change ignored.";
                this->signalSelectionToRestore = QString::null;
                this->editCurrentCell(true);
            }
            else
                throw;
        }
    }
}

void SignalListEditor::endChangeSignalInitialValue()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        try
        {
            DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

            LogicValue newInitialValue = LogicValue::fromString(editor->text()); // Throws StatesException

            shared_ptr<Signal> currentSignal = this->associatedSignals[currentTableItem].lock();

            if ( (currentSignal != nullptr) && (newInitialValue != currentSignal->getInitialValue()) )
            {
                if (newInitialValue.getSize() < currentSignal->getSize())
                    newInitialValue.resize(currentSignal->getSize()); // Throws StatesException

                this->signalSelectionToRestore = currentSignal->getName();

                l_machine->changeSignalInitialValue(currentSignal->getName(), newInitialValue); // Throws StatesException
            }
            else
            {
                updateList();
            }
        }
        catch (const StatesException& e)
        {
            if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == Signal::SignalErrorEnum::size_mismatch) )
            {
                qDebug() << "(SignalListEditor:) Info: Wrong input for signal initial value, change ignored.";
                this->signalSelectionToRestore = QString::null;
                this->editCurrentCell(true);
            }
            else
                throw;
        }
    }
}

void SignalListEditor::validateCurrentEdit()
{
    if (this->currentMode == mode::addingSignal)
    {
        this->endAddSignal();
    }
    else if (this->currentMode == mode::renamingSignal)
    {
        this->endRenameSignal();
    }
    else if (this->currentMode == mode::resizingSignal)
    {
        this->endResizeSignal();
    }
    else if (this->currentMode == mode::changingSignalInitialValue)
    {
        this->endChangeSignalInitialValue();
    }
}




void SignalListEditor::cancelCurrentEdit()
{
    if (this->currentMode != mode::standard)
    {
        if (this->currentMode == mode::addingSignal)
        {
            disconnect(this->signalsList, &QTableWidget::currentItemChanged, this, &SignalListEditor::addingSignalCurrentItemChanged);
            disconnect(this->signalsList, &QTableWidget::itemClicked, this, &SignalListEditor::addingSignalSwitchField);
        }
        this->switchMode(mode::standard);
        // Reset list
        this->updateList();
    }
}


void SignalListEditor::raiseSignal()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        // Only one even if multiple selected for now

        this->signalSelectionToRestore = this->signalsList->item(this->signalsList->selectionModel()->selectedRows()[0].row(), 0)->text();

        l_machine->changeSignalRank(this->signalsList->item(this->signalsList->selectionModel()->selectedRows()[0].row(), 0)->text(),
                                    this->signalsList->selectionModel()->selectedRows()[0].row()-1);
    }
}

void SignalListEditor::lowerSignal()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        // Only one even if multiple selected for now

        this->signalSelectionToRestore = this->signalsList->item(this->signalsList->selectionModel()->selectedRows()[0].row(), 0)->text();

        l_machine->changeSignalRank(this->signalsList->item(this->signalsList->selectionModel()->selectedRows()[0].row(), 0)->text(),
                                    this->signalsList->selectionModel()->selectedRows()[0].row()+1);
    }
}

void SignalListEditor::removeSelectedSignals()
{
    shared_ptr<Machine> l_machine = this->machine.lock();

    if (l_machine != nullptr)
    {
        QList<QString> selection;
        int lastSelectionIndex = -1;

        foreach (QModelIndex index, this->signalsList->selectionModel()->selectedRows())
        {
            selection.append(this->signalsList->item(index.row(), 0)->text());
            if (lastSelectionIndex < index.row())
                lastSelectionIndex = index.row();
        }

        foreach (QString signalName, selection)
        {
            // Select following signal (if not last)
            if (lastSelectionIndex < this->signalsList->rowCount()-1)
                this->signalSelectionToRestore = signalsList->item(lastSelectionIndex+1,0)->text();
            else
                this->signalSelectionToRestore = QString::null;

            l_machine->deleteSignal(signalName);
        }
    }
}

void SignalListEditor::treatMenuEventHandler(QAction* action)
{
    QVariant data = action->data();
    int dataValue = data.toInt();

    switch (dataValue)
    {
    case ContextAction::Cancel:
        break;
    case ContextAction::DeleteSignal:
        this->removeSelectedSignals();
        break;
    case ContextAction::Up:
        this->raiseSignal();
        break;
    case ContextAction::Down:
        this->lowerSignal();
        break;
    }
}

void SignalListEditor::switchMode(mode newMode)
{
    if (newMode != this->currentMode)
    {
        this->currentMode = newMode;

        this->updateButtonsEnableState();

        if (newMode == mode::standard)
        {
            this->currentTableItem   = nullptr;
            this->currentSignalName  = nullptr;
            this->currentSignalSize  = nullptr;
            this->currentSignalValue = nullptr;

            this->listDelegate->setValidator(nullptr);

            delete this->buttonCancel;
            delete this->buttonOK;

            this->buttonCancel = nullptr;
            this->buttonOK     = nullptr;

            connect(this->signalsList, &QTableWidget::itemDoubleClicked, this, &SignalListEditor::beginEditSignal);
        }
        else if ( (newMode == mode::addingSignal)  || (newMode == mode::renamingSignal) || (newMode == mode::resizingSignal) || (newMode == mode::changingSignalInitialValue) )
        {
            disconnect(signalsList, &QTableWidget::itemDoubleClicked, this, &SignalListEditor::beginEditSignal);

            this->buttonOK = new QPushButton(tr("OK"));
            this->buttonLayout->addWidget(this->buttonOK, 0, 0, 1, 21);
            connect(this->buttonOK, &QPushButton::clicked, this, &SignalListEditor::validateCurrentEdit);

            this->buttonCancel = new QPushButton(tr("Cancel"));
            this->buttonLayout->addWidget(this->buttonCancel, 0, 21, 1, 21);
            connect(this->buttonCancel, &QPushButton::clicked, this, &SignalListEditor::cancelCurrentEdit);

            // Disable all items in list except edited item row
            // (TODO: Should also disable other lists to avoid begin another
            // edit while current could be faulty)
            for (int i = 0 ; i < this->signalsList->rowCount() ; i++)
            {
                if (i != this->currentTableItem->row())
                {
                    for (int j = 0 ; j < this->signalsList->columnCount() ; j++)
                    {
                        Qt::ItemFlags currentFlags = this->signalsList->item(i, j)->flags();
                        this->signalsList->item(i, j)->setFlags(currentFlags & ~Qt::ItemIsEnabled);
                    }
                }
            }

            this->editCurrentCell();
        }
    }
}

void SignalListEditor::editCurrentCell(bool erroneous)
{
    // Set validator

    if (this->currentMode == mode::resizingSignal)
    {
        this->listDelegate->setValidator(shared_ptr<QValidator>(new QIntValidator(1, 64)));
    }
    else if (this->currentMode == mode::changingSignalInitialValue)
    {
        shared_ptr<Signal> currentSignal = this->associatedSignals[this->currentTableItem].lock();

        if (currentSignal != nullptr)
        {
            // A string made of only '0' and '1' chars, witch length is between 0 and size
            QRegularExpression re("[01]{0," + QString::number(currentSignal->getSize()) + "}");
            this->listDelegate->setValidator(shared_ptr<QValidator>(new QRegularExpressionValidator(re, 0)));
        }
    }
    else if (this->currentMode == mode::addingSignal)
    {
        if (this->currentTableItem == this->currentSignalSize)
        {
            this->listDelegate->setValidator(shared_ptr<QValidator>(new QIntValidator(1, 64)));
        }
        else if (this->currentTableItem == this->currentSignalValue)
        {
            uint currentSize = (uint)this->currentSignalSize->text().toInt();

            // A string made of only '0' and '1' chars, witch length is between 0 and size
            QRegularExpression re("[01]{0," + QString::number(currentSize) + "}");
            this->listDelegate->setValidator(shared_ptr<QValidator>(new QRegularExpressionValidator(re, 0)));
        }
        else
        {
            this->listDelegate->setValidator(nullptr);
        }
    }
    else // Renaming
    {
        this->listDelegate->setValidator(nullptr);
    }

    // Begin edition
    Qt::ItemFlags currentFlags = this->currentTableItem->flags();
    this->currentTableItem->setFlags(currentFlags | Qt::ItemIsEditable);

    this->signalsList->openPersistentEditor(this->currentTableItem);

    DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
    connect(editor, &DynamicLineEdit::returnPressed, this, &SignalListEditor::validateCurrentEdit);

    // Done

    if (erroneous)
    {
        editor->markAsErroneous();
    }

    editor->setFocus();
}

void SignalListEditor::fixSignalSize()
{
    try
    {
        LogicValue currentInitialValue = LogicValue::fromString(this->currentSignalValue->text()); // Throws StatesException

        uint newSize = (uint)this->currentSignalSize->text().toInt();
        currentInitialValue.resize(newSize); // Throws StatesException

        this->currentSignalValue->setText(currentInitialValue.toString());
    }
    catch (const StatesException& e)
    {
        if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
        {
            qDebug() << "(SignalListEditor:) Info: Wrong input for initial value, change ignored.";
        }
        else
            throw;
    }
}

QList<QString> SignalListEditor::getSelectedSignals()
{
    QList<QString> selectionString;

    foreach (QModelIndex index, this->signalsList->selectionModel()->selectedRows())
    {
        QTableWidgetItem* currentItem = this->signalsList->item(index.row(), 1);
        if (currentItem != nullptr)
            selectionString.append(currentItem->text());
    }

    return selectionString;
}
