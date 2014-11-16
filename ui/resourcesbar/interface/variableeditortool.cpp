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

#include <QListWidgetItem>
#include <QList>
#include <QLineEdit>
#include <iolistdelegate.h>

#include <QDebug>

#include "variableeditortools.h"

#include "machine.h"
#include "input.h"
#include "output.h"
#include "inout.h"
#include "dynamiclineedit.h"
#include "io.h"

VariableEditorTools::VariableEditorTools(Machine* machine, type editorType)
{
    this->machine = machine;
    this->editorType = editorType;

    layout = new QGridLayout(this);

    ioList = new QListWidget();
    ioList->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    listDelegate = new IoListDelegate(&currentIO, ioList);
    ioList->setItemDelegate(listDelegate);

    buttonAdd = new QPushButton(tr("Add"));
    buttonRemove = new QPushButton(tr("Remove"));

    this->layout->addWidget(ioList,0,0,1,2);
    this->layout->addWidget(buttonAdd,2,0,1,1);
    this->layout->addWidget(buttonRemove,2,1,1,1);

    if (editorType == type::inputs)
    {
        connect(machine, SIGNAL(InputListChangedEvent()), this, SLOT(updateList()));
        connect(this, SIGNAL(addIOEvent(QString)), machine, SLOT(addInput(QString)));
        connect(this, SIGNAL(removeIOEvent(QString)), machine, SLOT(deleteInput(QString)));
        connect(this, SIGNAL(renameIOEvent(QString,QString)), machine, SLOT(renameInput(QString,QString)));
    }
    else if (editorType == type::outputs)
    {
        connect(machine, SIGNAL(OutputListChangedEvent()), this, SLOT(updateList()));
        connect(this, SIGNAL(addIOEvent(QString)), machine, SLOT(addOutput(QString)));
        connect(this, SIGNAL(removeIOEvent(QString)), machine, SLOT(deleteOutput(QString)));
        connect(this, SIGNAL(renameIOEvent(QString,QString)), machine, SLOT(renameOutput(QString,QString)));
    }
    else if (editorType == type::variables)
    {
        connect(machine, SIGNAL(LocalVariableListChangedEvent()), this, SLOT(updateList()));
        connect(this, SIGNAL(addIOEvent(QString)), machine, SLOT(addLocalVariable(QString)));
        connect(this, SIGNAL(removeIOEvent(QString)), machine, SLOT(deleteLocalVariable(QString)));
        connect(this, SIGNAL(renameIOEvent(QString,QString)), machine, SLOT(renameLocalVariable(QString,QString)));
    }


    connect(buttonAdd, SIGNAL(clicked()), this, SLOT(beginAddIO()));
    connect(buttonRemove, SIGNAL(clicked()), this, SLOT(removeSelectedIOs()));
    connect(ioList, SIGNAL(itemSelectionChanged()), this, SLOT(toggleButtonsEnable()));

    updateList();
}

VariableEditorTools::~VariableEditorTools()
{
    if (editorType == type::inputs)
    {
        disconnect(machine, SIGNAL(InputListChangedEvent()), this, SLOT(updateList()));
    }
    else if (editorType == type::outputs)
    {
        disconnect(machine, SIGNAL(OutputListChangedEvent()), this, SLOT(updateList()));
    }
    else if (editorType == type::variables)
    {
        disconnect(machine, SIGNAL(LocalVariableListChangedEvent()), this, SLOT(updateList()));
    }
}

void VariableEditorTools::updateList()
{
    QList<QString> selection;

    foreach (QListWidgetItem* i, ioList->selectedItems())
    {
        selection.append(i->text());
    }

    ioList->clear();

    switchMode(mode::standard);

    QList<LogicVariable*> list;

    if (editorType == type::inputs)
    {
        QList<Input*> inputs = machine->getInputs();
        list = *reinterpret_cast<QList<LogicVariable*>*> (&inputs);
    }
    else if (editorType == type::outputs)
    {
        QList<Output*> outputs = machine->getOutputs();
        list = *reinterpret_cast<QList<LogicVariable*>*> (&outputs);
    }
    else if (editorType == type::variables)
        list = machine->getLocalVariables();

    foreach (LogicVariable *var, list)
    {
        QListWidgetItem* currentItem = new QListWidgetItem(var->getName(), ioList);
        currentItem->setWhatsThis(var->getName());

        if (selection.count() != 0)
        {
            foreach (QString text, selection)
            {
                if (text == currentItem->text())
                {
                    currentItem->setSelected(true);
                    break;
                }
            }
        }
    }
}

void VariableEditorTools::beginAddIO()
{
    QString baseName;
    QString currentName;

    if (editorType == type::inputs)
    {
        baseName = "Input #";
    }
    else if (editorType == type::outputs)
    {
        baseName = "Output #";
    }
    else if (editorType == type::variables)
    {
        baseName = "Variable #";
    }

    uint i = 0;
    bool nameIsValid = false;

    while (!nameIsValid)
    {
        currentName = baseName + QString::number(i);

        nameIsValid = true;
        foreach(LogicVariable* colleage, machine->getAllVariables())
        {
            if (colleage->getName() == currentName)
            {
                nameIsValid = false;
                i++;
                break;
            }
        }
    }

    currentIO = new QListWidgetItem(currentName, ioList);
    currentIO->setWhatsThis(currentName);

    switchMode(mode::addingIO);
}


void VariableEditorTools::endAddIO(QWidget* io)
{
    QLineEdit* editor = reinterpret_cast<QLineEdit*>(io);
    QString finalName = editor->text();

    // If success, reloads list through events.
    // This resets mode.
    bool success = addIOEvent(finalName);

    if (!success)
    {
        // If case call fails, list has not been reloaded:
        // currentIO is preserved
        ioList->editItem(currentIO);

        // Button cancel should be displayed before,
        // but see note in switchMode()
        //        buttonCancel = new QPushButton("Cancel");
        //        this->layout->addWidget(buttonCancel,1,0,1,2);
        //        connect(buttonCancel, SIGNAL(clicked()), this, SLOT(listChanged()));
    }
}

void VariableEditorTools::beginRenameIO(QListWidgetItem* io)
{
    currentIO = io;

    switchMode(mode::renamingIO);
}

void VariableEditorTools::endRenameIO(QWidget *io)
{
    QLineEdit* editor = reinterpret_cast<QLineEdit*>(io);
    QString finalName = editor->text();

    if (finalName != currentIO->whatsThis())
    {
        bool success = renameIOEvent(currentIO->whatsThis(), finalName);

        if (!success)
        {
            ioList->editItem(currentIO);;

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

void VariableEditorTools::removeSelectedIOs()
{
    QList<QString> selection;

    foreach (QListWidgetItem* i, ioList->selectedItems())
    {
        selection.append(i->text());
    }

    foreach (QString ioName, selection)
    {
        emit removeIOEvent(ioName);
    }
}


void VariableEditorTools::toggleButtonsEnable()
{
    if (currentMode == mode::standard)
    {
        buttonAdd->setEnabled(true);

        if (ioList->selectedItems().count() != 0)
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

void VariableEditorTools::switchMode(mode newMode)
{
    mode previousMode = currentMode;
    currentMode = newMode;

    if (newMode == mode::standard)
    {
        if (previousMode == mode::renamingIO)
        {
            disconnect(listDelegate, SIGNAL(closeEditor(QWidget*)), this, SLOT(endRenameIO(QWidget*)));
        }
        else if (previousMode == mode::addingIO)
        {
            disconnect(listDelegate, SIGNAL(closeEditor(QWidget*)), this, SLOT(endAddIO(QWidget*)));
        }

        currentIO = nullptr;

        //        delete buttonCancel;
        //        buttonCancel = nullptr;

        connect(ioList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(beginRenameIO(QListWidgetItem*)));
    }
    else if ((newMode == mode::addingIO)  || (newMode == mode::renamingIO))
    {
        disconnect(ioList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(beginRenameIO(QListWidgetItem*)));

        // Problem is before click is taken in consideration,
        // field is validated by losing focus.
        // Which, if name is correct, will trigger IO creation/rename
        // and button delete witll be deleted as a consequence...

        //"endrenameio" is called BEFORE click on "cancel" is taken in consideration... So button is deleted?
        //        buttonCancel = new QPushButton("Cancel");
        //        this->layout->addWidget(buttonCancel,1,0,1,2);
        //        connect(buttonCancel, SIGNAL(clicked()), this, SLOT(listChanged()));

        // Disable everything except editability of other items until name is set
        foreach(QListWidgetItem* item, ioList->findItems("*", Qt::MatchWildcard))
        {
            if (item != currentIO)
                item->setFlags(0);
        }

        currentIO->setFlags(currentIO->flags () | Qt::ItemIsEditable);
        ioList->editItem(currentIO);

        if (newMode == mode::addingIO)
        {
            // Track end of name edition (can't track itemChanged as user may want to keep initial name => no name change
            connect(listDelegate, SIGNAL(closeEditor(QWidget*)), this, SLOT(endAddIO(QWidget*)));
        }
        else if (newMode == mode::renamingIO)
        {
            // Track changes on name
            connect(listDelegate, SIGNAL(closeEditor(QWidget*)), this, SLOT(endRenameIO(QWidget*)));
        }
    }

    toggleButtonsEnable();
}
