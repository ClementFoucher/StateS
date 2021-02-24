/*
 * Copyright © 2014-2021 Clément Foucher
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
#include "machinemanager.h"
#include "statesui.h"
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
	// Build machine manager
	this->machineManager = shared_ptr<MachineManager>(new MachineManager());
	this->machineManager->build();

	// Build interface
	this->statesUi = unique_ptr<StatesUi>(new StatesUi(this->machineManager));
	connect(this->statesUi.get(), &StatesUi::newFsmRequestEvent,                   this, &StateS::generateNewFsm);
	connect(this->statesUi.get(), &StatesUi::clearMachineRequestEvent,             this, &StateS::clearMachine);
	connect(this->statesUi.get(), &StatesUi::loadMachineRequestEvent,              this, &StateS::loadMachine);
	connect(this->statesUi.get(), &StatesUi::saveMachineRequestEvent,              this, &StateS::saveCurrentMachine);
	connect(this->statesUi.get(), &StatesUi::saveMachineInCurrentFileRequestEvent, this, &StateS::saveCurrentMachineInCurrentFile);

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
 * @brief StateS::~StateS destructor
 */
StateS::~StateS()
{
	// Force cleaning order to handle dependencies between members
	this->statesUi.reset();

	this->machineManager->clear();
	this->machineManager.reset();
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
 * @brief StateS::generateNewFsm replaces the existing machine
 * with a newly created FSM.
 * This is the 'Clean' action.
 */
void StateS::generateNewFsm()
{
	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	machineStatus->setHasSaveFile(false);
	machineStatus->setUnsavedFlag(false);

	shared_ptr<Machine> newMachine = shared_ptr<Fsm>(new Fsm());
	this->machineManager->setMachine(newMachine);
}

/**
 * @brief StateS::clearMachine cleans the currently existing machine.
 * This is the 'Close' action.
 */
void StateS::clearMachine()
{
	shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
	machineStatus->setHasSaveFile(false);
	machineStatus->setUnsavedFlag(false);

	this->machineManager->setMachine(nullptr);
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

			// Parse and check for warnings
			parser->doParse();
			QList<QString> warnings = parser->getWarnings();
			if (!warnings.isEmpty())
			{
				this->statesUi->displayErrorMessage(tr("Issues occured reading the file. StateS still managed to load machine."), warnings);
			}

			// If we reached this point, there should have been no exception
			this->machineManager->setMachine(parser->getMachine());

			shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
			machineStatus->setHasSaveFile(true);
			machineStatus->setUnsavedFlag(false);
			machineStatus->setSaveFilePath(path);

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
 * @brief StateS::saveCurrentMachine saves the current
 * machine to a specified file.
 * This is the 'Save as' action.
 * @param path Path of file to save to.
 */
void StateS::saveCurrentMachine(const QString& path)
{
	if (this->machineManager->getMachine() != nullptr)
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
			shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
			machineStatus->setHasSaveFile(true);
			machineStatus->setSaveFilePath(path);
			this->saveCurrentMachineInCurrentFile();
		}
	}
}

/**
 * @brief StateS::saveCurrentMachineInCurrentFile saves the
 * current machine to currently registered save file.
 * This is the 'Save' action.
 */
void StateS::saveCurrentMachineInCurrentFile()
{
	if (this->machineManager->getMachine() != nullptr)
	{
		bool fileOk = false;
		shared_ptr<MachineStatus> machineStatus = this->machineManager->getMachineStatus();
		QFileInfo file = QFileInfo(machineStatus->getSaveFileFullPath());
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
				shared_ptr<MachineXmlWriter> saveManager = MachineXmlWriter::buildMachineWriter(this->machineManager->getMachine());

				saveManager->writeMachineToFile(this->statesUi->getViewConfiguration(), file.filePath()); // Throws StatesException
				machineStatus->setUnsavedFlag(false);
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
