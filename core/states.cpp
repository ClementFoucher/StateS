/*
 * Copyright © 2014-2020 Clément Foucher
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
#include "undoredomanager.h"
#include "statesexception.h"
#include "viewconfiguration.h"
#include "fsm.h"
#include "machinexmlwriter.h"
#include "machinexmlparser.h"
#include "machinestatus.h"


////
// Static functions for convenience

QString StateS::getVersion()
{
	return STATES_VERSION;
}

QString StateS::getCopyrightYears()
{
	return STATES_YEARS;
}

////
// Object members

/**
 * @brief StateS::StateS Constructor to the main StateS object.
 * Buiilds the interface and undo/redo manager.
 * @param initialFilePath Parameter indicating that we have to
 * load an initial machine from file.
 */
StateS::StateS(const QString& initialFilePath)
{
	// Build interface
	this->statesUi = unique_ptr<StatesUi>(new StatesUi());
	connect(this->statesUi.get(), &StatesUi::newFsmRequestEvent,                   this, &StateS::generateNewFsm);
	connect(this->statesUi.get(), &StatesUi::clearMachineRequestEvent,             this, &StateS::clearMachine);
	connect(this->statesUi.get(), &StatesUi::loadMachineRequestEvent,              this, &StateS::loadMachine);
	connect(this->statesUi.get(), &StatesUi::saveMachineRequestEvent,              this, &StateS::saveCurrentMachine);
	connect(this->statesUi.get(), &StatesUi::saveMachineInCurrentFileRequestEvent, this, &StateS::saveCurrentMachineInCurrentFile);

	// Build undo/redo manager
	this->undoRedoManager = shared_ptr<UndoRedoManager>(new UndoRedoManager(this->statesUi));
	connect(this->undoRedoManager.get(), &UndoRedoManager::freshMachineAvailableEvent, this, &StateS::freshMachineAvailableFromUndoRedo);

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

/**
 * @brief StateS::run is the function called in the event loop
 * to display the UI.
 */
void StateS::run()
{
	// Display interface
	this->statesUi->show();
}

/**
 * @brief StateS::generateNewFsm replaces the existing machine with a newly created FSM.
 * This is the 'New' action.
 */
void StateS::generateNewFsm()
{
	shared_ptr<Machine> newMachine = nullptr;

	if (this->machine != nullptr)
	{
		// If a machine is already existing, preserve paths
		shared_ptr<MachineStatus> machineStatus = MachineStatus::clonePaths(machine->getMachineStatus());
		newMachine = shared_ptr<Fsm>(new Fsm(machineStatus));
	}
	else
	{
		newMachine = shared_ptr<Fsm>(new Fsm());
	}

	this->refreshMachine(newMachine, true);
}

/**
 * @brief StateS::clearMachine cleans the currently existing machine.
 * This is the 'Close' action.
 */
void StateS::clearMachine()
{
	this->refreshMachine(nullptr, true);
}

/**
 * @brief StateS::loadMachine loads a machine from a saved file.
 * This is the 'Load' action.
 * @param path Path of file to load from.
 */
void StateS::loadMachine(const QString& path)
{
	QFileInfo file(path);

	if ( (file.exists()) && ( (file.permissions() & QFileDevice::ReadUser) != 0) )
	{
		try
		{
			// Build file parser
			shared_ptr<MachineXmlParser> parser =  MachineXmlParser::buildFileParser(shared_ptr<QFile>(new QFile(path)));

			if (this->machine != nullptr)
			{
				// If a machine is already existing, preserve paths
				shared_ptr<MachineStatus> machineStatus = MachineStatus::clonePaths(this->machine->getMachineStatus());
				parser->setMachineStatus(machineStatus);
			}

			// Parse and check for warnings
			parser->doParse();
			QList<QString> warnings = parser->getWarnings();
			if (!warnings.isEmpty())
			{
				this->statesUi->displayErrorMessage(tr("Issues occured reading the file. StateS still managed to load machine."), warnings);
			}

			// If we reached this point, there should have been no exception
			this->refreshMachine(parser->getMachine(), true);
			this->statesUi->setViewConfiguration(parser->getViewConfiguration());
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
 * This is the 'Save as' action.
 * @param path Path of file to save to.
 */
void StateS::saveCurrentMachine(const QString& path)
{
	if (this->machine != nullptr)
	{
		bool fileOk = false;

		QFileInfo file(path);
		if ( (file.exists()) && ( (file.permissions() & QFileDevice::WriteUser) != 0) )
		{
			fileOk = true;
		}
		else if ( (! file.exists()) && (file.absoluteDir().exists()) )
		{
			fileOk = true;
		}

		if (fileOk)
		{
			shared_ptr<MachineStatus> machineStatus = this->machine->getMachineStatus();
			machineStatus->setSaveFilePath(path);
			machineStatus->setHasSaveFile(true);
			this->saveCurrentMachineInCurrentFile();
		}
	}
}

/**
 * @brief StateS::saveCurrentMachineInCurrentFile saves the current machine to
 * currently registered save file.
 * This is the 'Save' action.
 */
void StateS::saveCurrentMachineInCurrentFile()
{
	if (this->machine != nullptr)
	{
		bool fileOk = false;
		shared_ptr<MachineStatus> machineStatus = machine->getMachineStatus();
		QFileInfo file = machineStatus->getSaveFileFullPath();
		if ( (file.exists()) && ( (file.permissions() & QFileDevice::WriteUser) != 0) )
		{
			fileOk = true;
		}
		else if ( (! file.exists()) && (file.absoluteDir().exists()) )
		{
			fileOk = true;
		}

		if (fileOk)
		{
			try
			{
				shared_ptr<MachineXmlWriter> saveManager = MachineXmlWriter::buildMachineWriter(this->machine);

				saveManager->writeMachineToFile(this->statesUi->getViewConfiguration(), file.filePath()); // Throws StatesException
				machineStatus->setUnsavedFlag(false);
				this->undoRedoManager->setClean();
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
}

/**
 * @brief StateS::freshMachineAvailableFromUndoRedo
 * This function handles new machine emitted by
 * undo/redo manager.
 * @param machine New machine built by manager.
 */
void StateS::freshMachineAvailableFromUndoRedo(shared_ptr<Machine> machine)
{
	this->refreshMachine(machine, false);
}

/**
 * @brief StateS::refreshMachine changes the currently referenced machine.
 * Can be used either to load a new machine or to load a checkpoint of
 * the current machine.
 * @param newMachine New machine to use.
 * @param machineChanged Indicated wether the machine changed or if it
 * is a simple refresh from an undo/redo action.
 */
void StateS::refreshMachine(shared_ptr<Machine> newMachine, bool machineChanged)
{
	// Update references to machin in sub-classes
	this->statesUi->setMachine(newMachine, !machineChanged);
	this->undoRedoManager->setMachine(newMachine, machineChanged);
	MachineUndoCommand::setMachine(newMachine, machineChanged);

	// Update the local machine AFTER setting it in sub-classes as they still
	// need the old machine to clear it, and the following line deletes the
	// old object.
	this->machine = newMachine;
}
