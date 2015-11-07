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
#include "states.h"

// Qt classes
#include <QFileInfo>
#include <QDir>

// StateS classes
#include "statesui.h"
#include "fsm.h"


QString StateS::getVersion()
{
    return "0.3.9";
}

StateS::StateS(const QString& initialFilePath)
{
    // Create interface
    this->statesUi = unique_ptr<StatesUi>(new StatesUi());

    connect(this->statesUi.get(), &StatesUi::newFsmRequestEvent,                  this, &StateS::generateNewFsm);
    connect(this->statesUi.get(), &StatesUi::clearMachineRequestEvent,            this, &StateS::clearMachine);
    connect(this->statesUi.get(), &StatesUi::loadMachineRequestEvent,             this, &StateS::loadMachine);
    connect(this->statesUi.get(), &StatesUi::saveMachineRequestEvent,             this, &StateS::saveCurrentMachine);
    connect(this->statesUi.get(), &StatesUi::saveMachineInCurrentFileRequestEvent,this, &StateS::saveCurrentMachineInCurrentFile);

    // Initialize machine
    if (! initialFilePath.isEmpty())
    {
        this->loadMachine(initialFilePath);
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

/*
 * Replace existing machine with a newly created FSM.
 * This is the 'New' action.
 */
void StateS::generateNewFsm()
{
    this->clearMachine();

    this->machine = shared_ptr<Fsm>(new Fsm());
    this->statesUi->setMachine(this->machine);
}

/*
 * Just delete current machine.
 * This is the 'Close' action.
 */
void StateS::clearMachine()
{
    this->statesUi->setMachine(nullptr);
    this->machine.reset();
    this->currentFilePath = QString::null;
}

/*
 * Load a machine from a saved file.
 * This is the 'load' action.
 */
void StateS::loadMachine(const QString& path)
{
    QFileInfo file(path);

    if ( (file.exists()) && ( (file.permissions() & QFileDevice::ReadUser) != 0) )
    {
        this->clearMachine();

        this->currentFilePath = path;

        // TODO: in one step
        // shared_ptr<Fsm> fsm = shared_ptr<Fsm>(new Fsm(this->currentFilePath));
        shared_ptr<Fsm> fsm = shared_ptr<Fsm>(new Fsm());
        if (this->currentFilePath != QString::null)
            fsm->loadFromFile(this->currentFilePath);

        this->machine = fsm;

        this->statesUi->setMachine(this->machine, this->currentFilePath);
    }
}

/*
 * Save current machine to file.
 * This is the 'save as' action.
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
        this->currentFilePath = path;

        this->saveCurrentMachineInCurrentFile();

        this->statesUi->setCurrentFilePath(this->currentFilePath);
    }
}

/*
 * Save current machine to currently registered save file.
 * This is the 'save' action.
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
        this->machine->saveMachine(this->currentFilePath);
    }
}
