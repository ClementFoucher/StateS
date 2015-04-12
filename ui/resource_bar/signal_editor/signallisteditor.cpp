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
    }
    else if (editorType == Machine::signal_type::Output)
    {
        signalsList = new QTableWidget(0, 2);

        signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(0)->setText(tr("Output"));

        connect(machine.get(), &Machine::outputListChangedEvent, this, &SignalListEditor::updateList);
    }
    else if (editorType == Machine::signal_type::LocalVariable)
    {
        signalsList = new QTableWidget(0, 3);

        signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(0)->setText(tr("Variable"));

        signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(2)->setText(tr("Initial value"));

        connect(machine.get(), &Machine::localVariableListChangedEvent, this, &SignalListEditor::updateList);
    }
    else if (editorType == Machine::signal_type::Constant)
    {
        signalsList = new QTableWidget(0, 3);

        signalsList->setHorizontalHeaderItem(0, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(0)->setText(tr("Constant"));

        signalsList->setHorizontalHeaderItem(2, new QTableWidgetItem());
        signalsList->horizontalHeaderItem(2)->setText(tr("Value"));

        connect(machine.get(), &Machine::constantListChangedEvent, this, &SignalListEditor::updateList);
    }

    signalsList->setHorizontalHeaderItem(1, new QTableWidgetItem());
    signalsList->horizontalHeaderItem(1)->setText(tr("Size"));

    signalsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    listDelegate = new DynamicTableItemDelegate(&currentTableItem, signalsList);
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

    connect(signalsList,  &QTableWidget::itemSelectionChanged, this, &SignalListEditor::updateButtonsEnableState);

    updateList();
}

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
            currentItem->setWhatsThis(currentItem->text()); // What's this is used for line editor to identify errors
            signalsList->setItem(signalsList->rowCount()-1, 0, currentItem);
            associatedSignals[currentItem] = sig;

            // Signal size
            currentItem = new QTableWidgetItem(QString::number(sig->getSize()));
            currentItem->setWhatsThis(currentItem->text());

            signalsList->setItem(signalsList->rowCount()-1, 1, currentItem);
            associatedSignals[currentItem] = sig;

            // Signal (initial) value
            if (signalsList->columnCount() == 3)
            {
                currentItem = new QTableWidgetItem(sig->getInitialValue().toString());
                currentItem->setWhatsThis(currentItem->text());
                signalsList->setItem(signalsList->rowCount()-1, 2, currentItem);
                associatedSignals[currentItem] = sig;
            }

            // Select signal if it was selected before list clear
            if (selection.contains(sig->getName()))
                currentItem->setSelected(true);
        }
    }
}

void SignalListEditor::beginAddSignal()
{
    shared_ptr<Machine> machine = this->machine.lock();

    if (machine != nullptr)
    {
        QString baseName;
        QString currentName;

        if (editorType == Machine::signal_type::Input)
        {
            baseName = "Input #";
        }
        else if (editorType == Machine::signal_type::Output)
        {
            baseName = "Output #";
        }
        else if (editorType == Machine::signal_type::LocalVariable)
        {
            baseName = "Variable #";
        }
        else if (editorType == Machine::signal_type::Constant)
        {
            baseName = "Constant #";
        }

        uint i = 0;
        bool nameIsValid = false;


        while (!nameIsValid)
        {
            currentName = baseName + QString::number(i);

            nameIsValid = true;
            foreach(shared_ptr<Signal> colleage, machine->getAllSignals())
            {
                if (colleage->getName() == currentName)
                {
                    nameIsValid = false;
                    i++;
                    break;
                }
            }
        }

        signalsList->insertRow(signalsList->rowCount());

        QTableWidgetItem* currentItem = new QTableWidgetItem(currentName);
        currentItem->setWhatsThis(currentItem->text());
        signalsList->setItem(signalsList->rowCount()-1, 0, currentItem);
        currentTableItem = currentItem;

        currentItem = new QTableWidgetItem("1");
        currentItem->setWhatsThis(currentItem->text());
        signalsList->setItem(signalsList->rowCount()-1, 1, currentItem);

        if (signalsList->columnCount() == 3)
        {
            currentItem = new QTableWidgetItem("0");
            currentItem->setWhatsThis(currentItem->text());
            signalsList->setItem(signalsList->rowCount()-1, 2, currentItem);
        }

        switchMode(mode::addingSignal);
    }
}


void SignalListEditor::endAddSignal(QWidget* signalNameWidget)
{
    QLineEdit* editor = reinterpret_cast<QLineEdit*>(signalNameWidget);
    QString finalName = editor->text();

    // If success, reloads list through events.
    // This resets mode.
    shared_ptr<Signal> newSignal = addSignalEvent(this->editorType, finalName);

    if (newSignal != nullptr)
    {
        // If case call fails, list has not been reloaded:
        // currentItem is preserved, put it back in edit mode
        signalsList->editItem(currentTableItem);

        // Button cancel should be displayed before,
        // but see note in switchMode()
        //        buttonCancel = new QPushButton("Cancel");
        //        this->layout->addWidget(buttonCancel,1,0,1,2);
        //        connect(buttonCancel, SIGNAL(clicked()), this, SLOT(listChanged()));
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

void SignalListEditor::endRenameSignal(QWidget *signalNameWidget)
{
    QLineEdit* editor = reinterpret_cast<QLineEdit*>(signalNameWidget);
    QString finalName = editor->text();

    shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

    if ( (currentSignal != nullptr) && (finalName != currentSignal->getName()) )
    {
        bool success = renameSignalEvent(currentSignal->getName(), finalName);

        if (!success)
        {
            signalsList->editItem(currentTableItem);

            // Button cancel should be displayed before,
            // but see note in switchMode()
            //            buttonCancel = new QPushButton("Cancel");
            //            this->layout->addWidget(buttonCancel,1,0,1,2);
            //            connect(buttonCancel, SIGNAL(clicked()), this, SLOT(listChanged()));
        }

    }
    else
        updateList();

}

void SignalListEditor::endResizeSignal(QWidget *signalSizeWidget)
{
    QLineEdit* editor = reinterpret_cast<QLineEdit*>(signalSizeWidget);
    uint finalSize = (uint)editor->text().toInt();

    shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

    if ( (currentSignal != nullptr) && (finalSize != currentSignal->getSize()) )
    {
        bool success = resizeSignalEvent(currentSignal->getName(), finalSize);

        if (!success)
        {
            signalsList->editItem(currentTableItem);

            // Button cancel should be displayed before,
            // but see note in switchMode()
            //            buttonCancel = new QPushButton("Cancel");
            //            this->layout->addWidget(buttonCancel,1,0,1,2);
            //            connect(buttonCancel, SIGNAL(clicked()), this, SLOT(listChanged()));
        }

    }
    else
        updateList();

}

void SignalListEditor::endChangeSignalInitialValue(QWidget* signalInitialValueWidget)
{
    QLineEdit* editor = reinterpret_cast<QLineEdit*>(signalInitialValueWidget);
    LogicValue newInitialValue = LogicValue::fromString(editor->text());

    shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

    if ( (currentSignal != nullptr) && (newInitialValue != currentSignal->getInitialValue()) )
    {
        bool success = changeSignalInitialValueEvent(currentSignal->getName(), newInitialValue);

        if (!success)
        {
            signalsList->editItem(currentTableItem);

            // Button cancel should be displayed before,
            // but see note in switchMode()
            //            buttonCancel = new QPushButton("Cancel");
            //            this->layout->addWidget(buttonCancel,1,0,1,2);
            //            connect(buttonCancel, SIGNAL(clicked()), this, SLOT(listChanged()));
        }

    }
    else
        updateList();
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
    mode previousMode = currentMode;
    currentMode = newMode;

    if (newMode == mode::standard)
    {
        if (previousMode == mode::renamingSignal)
        {
            disconnect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endRenameSignal);
        }
        else if (previousMode == mode::addingSignal)
        {
            disconnect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endAddSignal);
        }
        else if (previousMode == mode::resizingSignal)
        {
            disconnect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endResizeSignal);
        }
        else if (previousMode == mode::changingSignalInitialValue)
        {
            disconnect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endChangeSignalInitialValue);
        }

        currentTableItem = nullptr;
        listDelegate->setValidator(nullptr);

        //        delete buttonCancel;
        //        buttonCancel = nullptr;

        connect(signalsList, &QTableWidget::itemDoubleClicked, this, &SignalListEditor::beginEditSignal);
    }
    else if ( (newMode == mode::addingSignal)  || (newMode == mode::renamingSignal) || (newMode == mode::resizingSignal) || (newMode == mode::changingSignalInitialValue) )
    {
        disconnect(signalsList, &QTableWidget::itemDoubleClicked, this, &SignalListEditor::beginEditSignal);

        // Problem is before click is taken in consideration,
        // field is validated by losing focus.
        // Which, if name is correct, will trigger IO creation/rename
        // and button delete witll be deleted as a consequence...

        //"endrenameio" is called BEFORE click on "cancel" is taken in consideration... So button is deleted?
        //        buttonCancel = new QPushButton("Cancel");
        //        this->layout->addWidget(buttonCancel,1,0,1,2);
        //        connect(buttonCancel, SIGNAL(clicked()), this, SLOT(listChanged()));

        // Disable all other items in list
        // (Should also disable other lists to avoid begin another
        // edit while current could be faulty)



        if (newMode == mode::addingSignal)
        {
            // Track end of name edition (can't track itemChanged as user may want to keep initial name => no name change
            connect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endAddSignal);
        }
        else if (newMode == mode::renamingSignal)
        {
            connect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endRenameSignal);
        }
        else if (newMode == mode::resizingSignal)
        {
            listDelegate->setValidator(new QIntValidator(1, 64));
            connect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endResizeSignal);
        }
        else if (newMode == mode::changingSignalInitialValue)
        {
            shared_ptr<Signal> currentSignal = associatedSignals[currentTableItem].lock();

            if (currentSignal != nullptr)
            {
                QRegularExpression re("[01]{" + QString::number(currentSignal->getSize()) + "}");
                listDelegate->setValidator(new QRegularExpressionValidator(re, 0));

                connect(listDelegate, &QAbstractItemDelegate::closeEditor, this, &SignalListEditor::endChangeSignalInitialValue);
            }
        }

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

        signalsList->editItem(currentTableItem);
    }

    updateButtonsEnableState();
}
