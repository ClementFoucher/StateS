/*
 * Copyright © 2014-2017 Clément Foucher
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
#include "states.h"

// Qt classes
#include <QFileInfo>
#include <QDir>

// Diff Match Patch classes
#include <diff_match_patch.h>

// StateS classes
#include "statesui.h"
#include "fsm.h"
#include "fsmsavefilemanager.h"
#include "statesexception.h"
#include "machineconfiguration.h"

#include <QDebug>


QString StateS::getVersion()
{
    return "0.3.K";
}

/**
 * @brief StateS::StateS is the main object, handling both the UI
 * and the master Machine object representing the currenly edited
 * machine. It is also responsible for the time machine mechanism.
 * @param initialFilePath
 */
StateS::StateS(const QString& initialFilePath)
{
    // Create interface
    this->statesUi = unique_ptr<StatesUi>(new StatesUi());
    this->undoQueue = QStack<QList<Patch>>();

    connect(this->statesUi.get(), &StatesUi::newFsmRequestEvent,                   this, &StateS::generateNewFsm);
    connect(this->statesUi.get(), &StatesUi::clearMachineRequestEvent,             this, &StateS::clearMachine);
    connect(this->statesUi.get(), &StatesUi::loadMachineRequestEvent,              this, &StateS::loadFsm);
    connect(this->statesUi.get(), &StatesUi::saveMachineRequestEvent,              this, &StateS::saveCurrentMachine);
    connect(this->statesUi.get(), &StatesUi::saveMachineInCurrentFileRequestEvent, this, &StateS::saveCurrentMachineInCurrentFile);
    connect(this->statesUi.get(), &StatesUi::addCheckpointRequestEvent,            this, &StateS::addCheckpoint);
    connect(this->statesUi.get(), &StatesUi::undoRequestEvent,                     this, &StateS::undo);

    // Initialize machine
    if (! initialFilePath.isEmpty())
    {
        this->loadFsm(initialFilePath);
    }
    else
    {
        this->generateNewFsm();
    }
}

void StateS::run()
{
    // Display interface
    this->statesUi->show();
}

/**
 * @brief StateS::addCheckpoint computes the differences since
 * the latest checkpoint, store them in the undo stack then
 * store the current state as the latest checkpoint.
 */
void StateS::addCheckpoint()
{
    QString newXml;

    shared_ptr<MachineSaveFileManager> saveManager;
    if (dynamic_pointer_cast<Fsm>(this->machine) != nullptr)
    {
        saveManager = shared_ptr<MachineSaveFileManager>(new FsmSaveFileManager());
    }
    else
    {
        this->latestXmlCode = QString();
        return;
    }

    newXml = saveManager->getMachineXml(this->machine);

    // Compute diff
    diff_match_patch diffComputer = diff_match_patch();
    QList<Patch> diffList = diffComputer.patch_make(newXml, this->latestXmlCode);
    this->undoQueue.push(diffList);

    this->latestXmlCode = newXml;

    this->statesUi->setAddCheckpointButtonEnabled(false);
    this->statesUi->setUndoButtonEnabled(true);
    this->machineIsAtCheckpoint = true;
}

/**
 * @brief StateS::machineChanged notifies that the machine
 * have been changed since the latest checkpoint. This allow
 * to register a new checkpoint.
 */
void StateS::machineChanged()
{
    this->statesUi->setAddCheckpointButtonEnabled(true);
    this->statesUi->setUndoButtonEnabled(true);
    this->statesUi->setUnsavedFlag(true);
    if (this->currentFilePath.length() != 0)
        this->statesUi->setSaveActionEnabled(true);

    this->machineIsAtCheckpoint = false;
    this->machineSaved = false;
}

/**
 * @brief StateS::undo gets the machine back to the
 * latest registered checkpoint.
 * This is the 'Undo' action.
 */
void StateS::undo()
{
    shared_ptr<MachineSaveFileManager> saveManager;
    if (dynamic_pointer_cast<Fsm>(this->machine) != nullptr)
    {
        saveManager = shared_ptr<MachineSaveFileManager>(new FsmSaveFileManager());
    }
    else
    {
        this->latestXmlCode = QString();
        return;
    }

    if (this->machineIsAtCheckpoint == true)
    {
        if (this->undoQueue.isEmpty() == false)
        {
            QList<Patch> latestAction = this->undoQueue.pop();

            diff_match_patch diffUnroller = diff_match_patch();

            QPair<QString, QVector<bool> > result = diffUnroller.patch_apply(latestAction, this->latestXmlCode);

            shared_ptr<Machine> newMachine = saveManager->loadMachineFromXml(result.first);

            this->refreshMachine(newMachine);

            this->latestXmlCode = result.first;
        }
    }
    else
    {
        this->refreshMachine(saveManager->loadMachineFromXml(this->latestXmlCode));

        this->machineIsAtCheckpoint = true;
    }

    if (this->undoQueue.isEmpty() == true)
    {
        this->statesUi->setUndoButtonEnabled(false);
    }

    this->statesUi->setAddCheckpointButtonEnabled(false);
}

/**
 * @brief StateS::generateNewFsm replaces the existing machine with a newly created FSM.
 * This is the 'New' action.
 */
void StateS::generateNewFsm()
{
    this->clearMachine();

    shared_ptr<Machine> newMachine = shared_ptr<Fsm>(new Fsm());
    this->loadNewMachine(newMachine);
}

/**
 * @brief StateS::clearMachine cleans the currently existing
 * machine.
 * This is the 'Close' action.
 */
void StateS::clearMachine()
{
    this->loadNewMachine(nullptr);
}

/**
 * @brief StateS::loadFsm loads a machine from a saved file.
 * This is the 'load' action.
 * @param path
 */
void StateS::loadFsm(const QString& path)
{
    QFileInfo file(path);

    if ( (file.exists()) && ( (file.permissions() & QFileDevice::ReadUser) != 0) )
    {
        this->clearMachine();

        try
        {
            shared_ptr<MachineSaveFileManager> saveManager(new FsmSaveFileManager());

            shared_ptr<Machine> newMachine = saveManager->loadMachineFromFile(path); // Throws StatesException

            QList<QString> warnings = saveManager->getWarnings();
            if (!warnings.isEmpty())
            {
                this->statesUi->displayErrorMessage(tr("Issues occured reading the file. StateS still managed to load machine."), warnings);
            }

            this->loadNewMachine(newMachine, path);

            this->statesUi->setConfiguration(saveManager->getConfiguration());
        }
        catch (const StatesException& e)
        {
            if (e.getSourceClass() == "FsmSaveFileManager")
            {
                this->statesUi->displayErrorMessage(tr("Unable to load file."), QString(e.what()));
            }
            else if (e.getSourceClass() == "MachineSaveFileManager")
            {
                this->statesUi->displayErrorMessage(tr("Unable to load file."), QString(e.what()));
            }
            else
                throw;
        }
    }
}

/**
 * @brief StateS::saveCurrentMachine saves the current machine to
 * a specified file.
 * This is the 'save as' action.
 * @param path
 * @param configuration
 */
void StateS::saveCurrentMachine(const QString& path)
{
    bool fileOk = false;

    QFileInfo file(path);
    if ( (file.exists()) && ( (file.permissions() & QFileDevice::WriteUser) != 0) )
        fileOk = true;
    else if ( (! file.exists()) && (file.absoluteDir().exists()) )
        fileOk = true;

    if (fileOk)
    {
        this->updateFilePath(path);

        this->saveCurrentMachineInCurrentFile();

        this->statesUi->setTitle(this->currentFilePath);
    }
}

/**
 * @brief StateS::saveCurrentMachineInCurrentFile saves the current machine to
 * currently registered save file.
 * This is the 'save' action.
 * @param configuration
 */
void StateS::saveCurrentMachineInCurrentFile()
{
    bool fileOk = false;

    QFileInfo file(this->currentFilePath);
    if ( (file.exists()) && ( (file.permissions() & QFileDevice::WriteUser) != 0) )
        fileOk = true;
    else if ( (! file.exists()) && (file.absoluteDir().exists()) )
        fileOk = true;

    if (fileOk)
    {
        try
        {
            shared_ptr<MachineSaveFileManager> saveManager;
            if (dynamic_pointer_cast<Fsm>(this->machine) != nullptr)
            {
                saveManager = shared_ptr<MachineSaveFileManager>(new FsmSaveFileManager());
            }
            else
            {
                return;
            }

            saveManager->writeMachineToFile(dynamic_pointer_cast<Fsm>(this->machine), this->statesUi->getConfiguration(), this->currentFilePath); // Throws StatesException
            QList<QString> warnings = saveManager->getWarnings();
            if (!warnings.isEmpty())
            {
                this->statesUi->displayErrorMessage(tr("Issues occured writing the file. StateS still managed to save machine."), warnings);
            }
            this->statesUi->setSaveActionEnabled(false);
        }
        catch (const StatesException& e)
        {
            if (e.getSourceClass() == "FsmSaveFileManager")
            {
                this->statesUi->displayErrorMessage(tr("Unable to save file."), QString(e.what()));
            }
            if (e.getSourceClass() == "MachineSaveFileManager")
            {
                this->statesUi->displayErrorMessage(tr("Unable to save file."), QString(e.what()));
            }
            else
                throw;
        }
    }
}

/**
 * @brief StateS::updateLatestXml computes the current machine XML
 * to make the current state the latest checkpoint.
 */
void StateS::updateLatestXml()
{
    if (this->machine != nullptr)
    {
        shared_ptr<MachineSaveFileManager> saveManager;
        if (dynamic_pointer_cast<Fsm>(this->machine) != nullptr)
        {
            saveManager = shared_ptr<MachineSaveFileManager>(new FsmSaveFileManager());
        }
        else
        {
            qDebug() << "Unable to compute XML from a unkown machine type";
        }

        if (saveManager != nullptr)
        {
            this->latestXmlCode = saveManager->getMachineXml(this->machine);
        }
        else
        {
            this->latestXmlCode = QString();
        }
    }
    else
    {
        this->latestXmlCode = QString();
    }
}

void StateS::updateFilePath(const QString& newPath)
{
    this->currentFilePath = newPath;

    if ( (this->currentFilePath.size() != 0) && (this->machineSaved == false))
    {
        this->statesUi->setSaveActionEnabled(true);
    }
    else
    {
        this->statesUi->setSaveActionEnabled(false);
    }
}

/**
 * @brief StateS::refreshMachine changes the currently referenced machine.
 * Can be used either to load a new machine or to load a checkpoint of
 * the current machine.
 * @param newMachine
 */
void StateS::refreshMachine(shared_ptr<Machine> newMachine)
{
    // Cut links with older machine
    if (this->machine != nullptr)
    {
        disconnect(this->machine.get(), &Machine::machineEdited, this, &StateS::machineChanged);
    }

    // Renew machine
    this->statesUi->setMachine(newMachine);
    this->machine = newMachine;

    // Establish links with new machine
    if (this->machine != nullptr)
    {
        connect(this->machine.get(), &Machine::machineEdited, this, &StateS::machineChanged);
        this->statesUi->setSaveAsActionEnabled(true);
        this->statesUi->setExportActionsEnabled(true);
    }
    else
    {
        this->statesUi->setSaveAsActionEnabled(false);
        this->statesUi->setExportActionsEnabled(false);
    }
}

/**
 * @brief StateS::loadNewMachine loads a new machine: refreshes the
 * machine, sets the UI in the initial state for a new/loaded machine,
 * and sets a initial checkpoint on current state. The file path can be
 * provided if machine is loaded from a file.
 * @param newMachine
 * @param title
 */
void StateS::loadNewMachine(shared_ptr<Machine> newMachine, const QString& path)
{
    this->machineSaved = true;

    // Refresh current machine
    this->refreshMachine(newMachine);

    // Update UI
    this->statesUi->setTitle(path);
    this->statesUi->setUnsavedFlag(false);
    this->statesUi->setAddCheckpointButtonEnabled(false);
    this->statesUi->setUndoButtonEnabled(false);
    this->statesUi->setSaveActionEnabled(false);

    // Initialize time machine
    this->undoQueue = QStack<QList<Patch>>();
    this->updateLatestXml();
    this->machineIsAtCheckpoint = true;

    // Update file path
    this->updateFilePath(path);
}
