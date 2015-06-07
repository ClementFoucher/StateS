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
#include <QGridLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QKeyEvent>

// StateS classes
#include "dynamiclineedit.h"
#include "signal.h"
#include "dynamictableitemdelegate.h"


SignalListEditor::SignalListEditor(shared_ptr<Machine> machine, Machine::signal_type editorType, QWidget *parent) :
    QWidget(parent)
{
    this->machine = machine;
    this->editorType = editorType;

    layout = new QGridLayout(this);

    if (editorType == Machine::signal_type::Input)
    {
        signalsList = new QTableWidget(0, 3);

        signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(0)->setText(tr("Input"));

        signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(2)->setText(tr("Initial value"));

        connect(machine.get(), &Machine::inputListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Input");
    }
    else if (editorType == Machine::signal_type::Output)
    {
        signalsList = new QTableWidget(0, 2);

        signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(0)->setText(tr("Output"));

        connect(machine.get(), &Machine::outputListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Output");
    }
    else if (editorType == Machine::signal_type::LocalVariable)
    {
        signalsList = new QTableWidget(0, 3);

        signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(0)->setText(tr("Variable"));

        signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(2)->setText(tr("Initial value"));

        connect(machine.get(), &Machine::localVariableListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Variable");
    }
    else if (editorType == Machine::signal_type::Constant)
    {
        signalsList = new QTableWidget(0, 3);

        signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(0)->setText(tr("Constant"));

        signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(2)->setText(tr("Value"));

        connect(machine.get(), &Machine::constantListChangedEvent, this, &SignalListEditor::updateList);

        this->newSignalsPrefix = tr("Constant");
    }

    this->newSignalsPrefix += " #";

    signalsList->setHorizontalHeaderItem(1, new QTableWidgetItem());
    signalsList->horizontalHeaderItem(1)->setText(tr("Size"));

    signalsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    listDelegate = new DynamicTableItemDelegate(/*&currentTableItem, */signalsList);
    signalsList->setItemDelegate(listDelegate);

    buttonAdd    = new QPushButton(tr("Add"));
    buttonRemove = new QPushButton(tr("Remove"));

    this->layout->addWidget(signalsList,  0, 0, 1, 2);
    this->layout->addWidget(buttonAdd,    2, 0, 1, 1);
    this->layout->addWidget(buttonRemove, 2, 1, 1, 1);

    connect(this, &SignalListEditor::addSignalEvent,                machine.get(), &Machine::addSignal);
    connect(this, &SignalListEditor::removeSignalEvent,             machine.get(), &Machine::deleteSignal);
    connect(this, &SignalListEditor::renameSignalEvent,             machine.get(), &Machine::renameSignal);
    connect(this, &SignalListEditor::resizeSignalEvent,             machine.get(), &Machine::resizeSignal);
    connect(this, &SignalListEditor::changeSignalInitialValueEvent, machine.get(), &Machine::changeSignalInitialValue);

    connect(buttonAdd,    &QAbstractButton::clicked, this, &SignalListEditor::beginAddSignal);
    connect(buttonRemove, &QAbstractButton::clicked, this, &SignalListEditor::removeSelectedSignals);

    // To enable "Remove" button when a signal is selected
    connect(signalsList,  &QTableWidget::itemSelectionChanged, this, &SignalListEditor::updateButtonsEnableState);

    updateList();
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
    // Save selection list
    QList<QString> selection;

    foreach (QModelIndex index, signalsList->selectionModel()->selectedRows())
    {
        selection.append(signalsList->item(index.row(), 1)->text());
    }

    signalsList->clearContents();
    signalsList->setRowCount(0);
    associatedSignals.clear();

    switchMode(mode::standard);

    shared_ptr<Machine> machine = this->machine.lock();

    if (machine != nullptr)
    {

        // Get signals I have to deal with
        QList<shared_ptr<Signal>> signalsToAdd;

        if (editorType == Machine::signal_type::Input)
        {
            signalsToAdd = machine->getInputsAsSignals();
        }
        else if (editorType == Machine::signal_type::Output)
        {
            signalsToAdd = machine->getOutputsAsSignals();
        }
        else if (editorType == Machine::signal_type::LocalVariable)
        {
            signalsToAdd = machine->getLocalVariables();
        }
        else if (editorType == Machine::signal_type::Constant)
        {
            signalsToAdd = machine->getConstants();
        }

        foreach (shared_ptr<Signal> sig, signalsToAdd)
        {
            signalsList->insertRow(signalsList->rowCount());

            // Signal name
            QTableWidgetItem* currentItem = new QTableWidgetItem(sig->getName());
            signalsList->setItem(signalsList->rowCount()-1, 0, currentItem);
            associatedSignals[currentItem] = sig;

            // Signal size
            currentItem = new QTableWidgetItem(QString::number(sig->getSize()));

            signalsList->setItem(signalsList->rowCount()-1, 1, currentItem);
            associatedSignals[currentItem] = sig;

            // Signal (initial) value
            if (signalsList->columnCount() == 3)
            {
                currentItem = new QTableWidgetItem(sig->getInitialValue().toString());
                signalsList->setItem(signalsList->rowCount()-1, 2, currentItem);
                associatedSignals[currentItem] = sig;
            }

            // Select signal if it was selected before list clear
            if (selection.contains(sig->getName()))
                currentItem->setSelected(true);
        }
    }
}

void SignalListEditor::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key::Key_Escape)
    {
        this->cancelCurrentEdit();
    }

    emit QWidget::keyPressEvent(event);
}

void SignalListEditor::beginAddSignal()
{
    shared_ptr<Machine> machine = this->machine.lock();

    if (machine != nullptr)
    {
        QString initialName;

        initialName = machine->getUniqueSignalName(this->newSignalsPrefix);

        signalsList->insertRow(signalsList->rowCount());

        QTableWidgetItem* currentItem = new QTableWidgetItem(initialName);
        signalsList->setItem(signalsList->rowCount()-1, 0, currentItem);
        currentTableItem = currentItem;

        currentItem = new QTableWidgetItem("1");
        signalsList->setItem(signalsList->rowCount()-1, 1, currentItem);

        if (signalsList->columnCount() == 3)
        {
            currentItem = new QTableWidgetItem("0");
            signalsList->setItem(signalsList->rowCount()-1, 2, currentItem);
        }

        switchMode(mode::addingSignal);

        this->listDelegate->getCurentEditor()->setFocus();
        this->signalsList->scrollToBottom();
    }
}

// Begin edit signal is trigerred by double-click on table item
void SignalListEditor::beginEditSignal(QTableWidgetItem* characteristicToEdit)
{
    currentTableItem = characteristicToEdit;

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
        this->updateList();
}

void SignalListEditor::endAddSignal()
{
    DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

    if (editor != nullptr)
    {
        QString finalName = editor->text();

        // If success, list is reloaded through events,
        // which resets mode.
        shared_ptr<Signal> newSignal = addSignalEvent(this->editorType, finalName);

        // If adding signal failed, continue editing signal name
        if (newSignal == nullptr)
        {
            editor->markAsErroneous();
        }
    }
}


void SignalListEditor::endRenameSignal()
{
    DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

    if (editor != nullptr)
    {
        QString finalName = editor->text();

        shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

        if ( (currentSignal != nullptr) && (finalName != currentSignal->getName()) )
        {
            bool success = renameSignalEvent(currentSignal->getName(), finalName);

            if (!success)
            {
                editor->markAsErroneous();
            }

        }
        else
            updateList();
    }
}

void SignalListEditor::endResizeSignal()
{
    DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

    if (editor != nullptr)
    {
        uint finalSize = (uint)editor->text().toInt();

        shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

        if ( (currentSignal != nullptr) && (finalSize != currentSignal->getSize()) )
        {
            bool success = resizeSignalEvent(currentSignal->getName(), finalSize);

            if (!success)
            {
                editor->markAsErroneous();
            }
        }
        else
            updateList();
    }
}

void SignalListEditor::endChangeSignalInitialValue()
{
    DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

    if (editor != nullptr)
    {
        LogicValue newInitialValue = LogicValue::fromString(editor->text());

        shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

        if ( (currentSignal != nullptr) && (newInitialValue != currentSignal->getInitialValue()) )
        {
            bool success = changeSignalInitialValueEvent(currentSignal->getName(), newInitialValue);

            if (!success)
            {
                editor->markAsErroneous();
            }
        }
        else
            updateList();
    }
}

void SignalListEditor::removeSelectedSignals()
{
    QList<QString> selection;

    foreach (QModelIndex index, signalsList->selectionModel()->selectedRows())
    {
        selection.append(signalsList->item(index.row(), 0)->text());
    }

    foreach (QString signalName, selection)
    {
        emit removeSignalEvent(signalName);
    }
}


void SignalListEditor::updateButtonsEnableState()
{
    if (currentMode == mode::standard)
    {
        buttonAdd->setEnabled(true);

        if (signalsList->selectedItems().count() != 0)
            buttonRemove->setEnabled(true);
        else
            buttonRemove->setEnabled(false);
    }
    else
    {
        buttonAdd->setEnabled(false);
        buttonRemove->setEnabled(false);
    }
}

void SignalListEditor::switchMode(mode newMode)
{
    this->currentMode = newMode;

    this->updateButtonsEnableState();

    if (newMode == mode::standard)
    {
        currentTableItem = nullptr;
        listDelegate->setValidator(nullptr);

        delete buttonCancel;
        buttonCancel = nullptr;

        delete buttonOK;
        buttonOK = nullptr;

        connect(signalsList, &QTableWidget::itemDoubleClicked, this, &SignalListEditor::beginEditSignal);
    }
    else if ( (newMode == mode::addingSignal)  || (newMode == mode::renamingSignal) || (newMode == mode::resizingSignal) || (newMode == mode::changingSignalInitialValue) )
    {
        disconnect(signalsList, &QTableWidget::itemDoubleClicked, this, &SignalListEditor::beginEditSignal);

        buttonOK = new QPushButton(tr("OK"));
        this->layout->addWidget(buttonOK,1,0,1,1);
        connect(buttonOK, &QPushButton::clicked, this, &SignalListEditor::validateCurrentEdit);

        buttonCancel = new QPushButton(tr("Cancel"));
        this->layout->addWidget(buttonCancel,1,1,1,1);
        connect(buttonCancel, &QPushButton::clicked, this, &SignalListEditor::cancelCurrentEdit);

        if (newMode == mode::resizingSignal)
        {
            listDelegate->setValidator(shared_ptr<QValidator>(new QIntValidator(1, 64)));
        }
        else if (newMode == mode::changingSignalInitialValue)
        {
            shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

            if (currentSignal != nullptr)
            {
                QRegularExpression re("[01]{" + QString::number(currentSignal->getSize()) + "}");
                listDelegate->setValidator(shared_ptr<QValidator>(new QRegularExpressionValidator(re, 0)));
            }
        }

        // Disable all other items in list
        // (Should also disable other lists to avoid begin another
        // edit while current could be faulty)

        Qt::ItemFlags currentFlags = currentTableItem->flags();

        for (int i = 0 ; i < signalsList->rowCount() ; i++)
        {
            signalsList->item(i, 0)->setFlags(0);
            signalsList->item(i, 1)->setFlags(0);

            if (signalsList->columnCount() == 3)
            {
                signalsList->item(i, 2)->setFlags(0);
            }
        }

        currentTableItem->setFlags(currentFlags | Qt::ItemIsEditable);

        signalsList->openPersistentEditor(currentTableItem);

        DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
        if (editor != nullptr)
        {
            connect(editor, &DynamicLineEdit::returnPressed, this, &SignalListEditor::validateCurrentEdit);
        }
    }
}
