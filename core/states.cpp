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


QString StateS::getVersion()
{
    return "0.3.K";
}

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
    connect(this->statesUi.get(), &StatesUi::addCheckpoint,                        this, &StateS::addCheckpoint);
    connect(this->statesUi.get(), &StatesUi::undo,                                 this, &StateS::undo);

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

StateS::~StateS()
{
    // Clear references to machine in UI before deletion
    this->statesUi->setMachine(nullptr);
}

void StateS::run()
{
    // Display interface
    this->statesUi->show();
}

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

void StateS::machineChanged()
{
    this->statesUi->setAddCheckpointButtonEnabled(true);
    this->statesUi->setUndoButtonEnabled(true);
    this->machineIsAtCheckpoint = false;
}

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

/*
 * Replace existing machine with a newly created FSM.
 * This is the 'New' action.
 */
void StateS::generateNewFsm()
{
    this->clearMachine();

    shared_ptr<Machine> newMachine = shared_ptr<Fsm>(new Fsm());
    this->loadNewMachine(newMachine, QString::null);
}

/*
 * Just delete current machine.
 * This is the 'Close' action.
 */
void StateS::clearMachine()
{
    this->loadNewMachine(nullptr, QString::null);
}

/*
 * Load a machine from a saved file.
 * This is the 'load' action.
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

/*
 * Save current machine to file.
 * This is the 'save as' action.
 */
void StateS::saveCurrentMachine(const QString& path, shared_ptr<MachineConfiguration> configuration)
{
    bool fileOk = false;

    QFileInfo file(path);
    if ( (file.exists()) && ( (file.permissions() & QFileDevice::WriteUser) != 0) )
        fileOk = true;
    else if ( (! file.exists()) && (file.absoluteDir().exists()) )
        fileOk = true;

    if (fileOk)
    {
        this->currentFilePath = path;

        this->saveCurrentMachineInCurrentFile(configuration);

        this->statesUi->setCurrentFilePath(this->currentFilePath);
    }
}

/*
 * Save current machine to currently registered save file.
 * This is the 'save' action.
 */
void StateS::saveCurrentMachineInCurrentFile(shared_ptr<MachineConfiguration> configuration)
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

            saveManager->writeMachineToFile(dynamic_pointer_cast<Fsm>(this->machine), configuration, this->currentFilePath); // Throws StatesException
            QList<QString> warnings = saveManager->getWarnings();
            if (!warnings.isEmpty())
            {
                this->statesUi->displayErrorMessage(tr("Issues occured writing the file. StateS still managed to save machine."), warnings);
            }
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
            this->latestXmlCode = QString();
            return;
        }

        this->latestXmlCode = saveManager->getMachineXml(this->machine);
    }
    else
    {
        this->latestXmlCode = QString();
    }
}

void StateS::refreshMachine(shared_ptr<Machine> newMachine)
{
    if (this->machine != nullptr)
    {
        disconnect(this->machine.get(), &Machine::machineEdited, this, &StateS::machineChanged);
    }
    this->statesUi->setMachine(newMachine);
    this->machine = newMachine;

    if (this->machine != nullptr)
    {
        connect(this->machine.get(), &Machine::machineEdited, this, &StateS::machineChanged);
    }
}

void StateS::loadNewMachine(shared_ptr<Machine> newMachine, const QString& title)
{
    this->refreshMachine(newMachine);

    this->statesUi->setAddCheckpointButtonEnabled(false);
    this->statesUi->setUndoButtonEnabled(false);

    this->undoQueue = QStack<QList<Patch>>();
    this->updateLatestXml();
    this->machineIsAtCheckpoint = true;
    this->currentFilePath = title;
    this->statesUi->setCurrentFilePath(this->currentFilePath);
}
