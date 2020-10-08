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
#include "statesexception.h"
#include "machineconfiguration.h"
#include "fsm.h"
#include "machinexmlwriter.h"
#include "fsmxmlwriter.h"
#include "machinexmlparser.h"
#include "diffundocommand.h"
#include "fsmundocommand.h"


////
// Static members

shared_ptr<Machine> StateS::machine = nullptr;
QString StateS::machineXmlRepresentation = QString();


QString StateS::getVersion()
{
	return "0.4";
}

shared_ptr<Machine> StateS::getCurrentMachine()
{
	return machine;
}

QString StateS::getCurrentXmlCode()
{
	return machineXmlRepresentation;
}


////
// Object members

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

	connect(this->statesUi.get(), &StatesUi::newFsmRequestEvent,                   this, &StateS::generateNewFsm);
	connect(this->statesUi.get(), &StatesUi::clearMachineRequestEvent,             this, &StateS::clearMachine);
	connect(this->statesUi.get(), &StatesUi::loadMachineRequestEvent,              this, &StateS::loadMachine);
	connect(this->statesUi.get(), &StatesUi::saveMachineRequestEvent,              this, &StateS::saveCurrentMachine);
	connect(this->statesUi.get(), &StatesUi::saveMachineInCurrentFileRequestEvent, this, &StateS::saveCurrentMachineInCurrentFile);
	connect(this->statesUi.get(), &StatesUi::undoRequestEvent,                     this, &StateS::undo);
	connect(this->statesUi.get(), &StatesUi::redoRequestEvent,                     this, &StateS::redo);

	connect(&this->undoStack, &QUndoStack::cleanChanged,   this, &StateS::undoStackCleanStateChangeEventHandler);
	connect(&this->undoStack, &QUndoStack::canUndoChanged, this, &StateS::undoActionAvailabilityChangeEventHandler);
	connect(&this->undoStack, &QUndoStack::canRedoChanged, this, &StateS::redoActionAvailabilityChangeEventHandler);

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

void StateS::run()
{
	// Display interface
	this->statesUi->show();
}

/**
 * @brief StateS::machineChangedEventHandler notifies that
 * the machine have been changed using a diff.
 * This must be handled here because such an undo command
 * emits events that must be connected to a slot.
 * The updateXmlRepresentation has not to be called as we
 * update the XML representation as part of the process.
 */
void StateS::computeDiffUndoCommand(MachineUndoCommand::undo_command_id commandId)
{
	QString previousXmlCode = this->machineXmlRepresentation;
	shared_ptr<MachineXmlWriter> saveManager = MachineXmlWriter::buildMachineWriter(machine);
	this->machineXmlRepresentation = saveManager->getMachineXml();

	DiffUndoCommand* undoCommand = new DiffUndoCommand(previousXmlCode, commandId);
	connect(undoCommand, &DiffUndoCommand::applyUndoRedo, this, &StateS::refreshMachineFromDiffUndoRedo);

	this->undoStack.push(undoCommand);
	this->setMachineDirty();
}

/**
 * @brief StateS::machineUndoCommandHandler indicates that
 * the machine has changed and produced a specific undo command.
 * Specific undo commands are particularly monitored changes
 * which can have advanced behavior like undo merge capacity.
 * They don't rely on a diff but do specific changes to the machine.
 * @param undoCommand
 */
void StateS::addUndoCommand(MachineUndoCommand* undoCommand)
{
	this->undoStack.push(undoCommand);
	this->updateXmlRepresentation();
	this->setMachineDirty();
}

/**
 * @brief StateS::undo gets the machine back to the
 * latest registered checkpoint.
 * This is the 'Undo' action.
 */
void StateS::undo()
{
	this->undoStack.undo();
	this->updateXmlRepresentation();
}

/**
 * @brief StateS::redo advances by one checkpoint
 * in the undo stack, coming back to the state before
 * the latest 'Undo' action.
 */
void StateS::redo()
{
	this->undoStack.redo();
	this->updateXmlRepresentation();
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
 * @brief StateS::loadMachine loads a machine from a saved file.
 * This is the 'Load' action.
 * @param path
 */
void StateS::loadMachine(const QString& path)
{
	QFileInfo file(path);

	if ( (file.exists()) && ( (file.permissions() & QFileDevice::ReadUser) != 0) )
	{
		this->clearMachine();

		try
		{
			shared_ptr<MachineXmlParser> parser =  MachineXmlParser::buildFileParser(shared_ptr<QFile>(new QFile(path)));

			QList<QString> warnings = parser->getWarnings();
			if (!warnings.isEmpty())
			{
				this->statesUi->displayErrorMessage(tr("Issues occured reading the file. StateS still managed to load machine."), warnings);
			}

			this->loadNewMachine(parser->getMachine(), path);
			this->statesUi->setConfiguration(parser->getConfiguration());
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

void StateS::refreshMachineFromDiffUndoRedo(shared_ptr<Machine> machine)
{
	this->refreshMachine(machine, true);
}

void StateS::undoStackCleanStateChangeEventHandler(bool clean)
{
	this->statesUi->setUnsavedFlag(!clean);

	if (this->currentFilePath.length() != 0)
	{
		this->statesUi->setSaveActionEnabled(!clean);
	}
}

void StateS::undoActionAvailabilityChangeEventHandler(bool undoAvailable)
{
	this->statesUi->setUndoButtonEnabled(undoAvailable);
}

void StateS::redoActionAvailabilityChangeEventHandler(bool redoAvailable)
{
	this->statesUi->setRedoButtonEnabled(redoAvailable);
}

/**
 * @brief StateS::saveCurrentMachine saves the current machine to
 * a specified file.
 * This is the 'Save as' action.
 * @param path
 * @param configuration
 */
void StateS::saveCurrentMachine(const QString& path)
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
		this->updateFilePath(path);
		this->saveCurrentMachineInCurrentFile();
	}
}

/**
 * @brief StateS::saveCurrentMachineInCurrentFile saves the current machine to
 * currently registered save file.
 * This is the 'Save' action.
 * @param configuration
 */
void StateS::saveCurrentMachineInCurrentFile()
{
	bool fileOk = false;

	QFileInfo file(this->currentFilePath);
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
			shared_ptr<MachineXmlWriter> saveManager = MachineXmlWriter::buildMachineWriter(machine);

			saveManager->writeMachineToFile(this->statesUi->getConfiguration(), this->currentFilePath); // Throws StatesException
			this->statesUi->setSaveActionEnabled(false);
			this->undoStack.setClean();
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
void StateS::updateXmlRepresentation()
{
	this->machineXmlRepresentation = QString();
	if (machine != nullptr)
	{
		shared_ptr<MachineXmlWriter> saveManager = MachineXmlWriter::buildMachineWriter(machine);
		if (saveManager != nullptr)
		{
			this->machineXmlRepresentation = saveManager->getMachineXml();
		}
	}
}

void StateS::updateFilePath(const QString& newPath)
{
	this->currentFilePath = newPath;

	// Update  UI

	if ( (this->currentFilePath.size() != 0) && (this->undoStack.isClean() == false))
	{
		this->statesUi->setSaveActionEnabled(true);
	}
	else
	{
		this->statesUi->setSaveActionEnabled(false);
	}

	this->statesUi->setTitle(this->currentFilePath);
}

void StateS::setMachineDirty()
{
	this->statesUi->setUnsavedFlag(true);
	if (this->currentFilePath.length() != 0)
	{
		this->statesUi->setSaveActionEnabled(true);
	}
}

/**
 * @brief StateS::refreshMachine changes the currently referenced machine.
 * Can be used either to load a new machine or to load a checkpoint of
 * the current machine.
 * @param newMachine
 */
void StateS::refreshMachine(shared_ptr<Machine> newMachine, bool maintainView)
{
	// Cut links with older machine
	if (machine != nullptr)
	{
		disconnect(machine.get(), &Machine::machineEditedWithoutUndoCommandGeneratedEvent, this, &StateS::computeDiffUndoCommand);
		disconnect(machine.get(), &Machine::machineEditedWithUndoCommandGeneratedEvent,    this, &StateS::addUndoCommand);
	}

	// Renew machine
	this->statesUi->setMachine(newMachine, maintainView);
	machine = newMachine;

	// Establish links with new machine
	if (machine != nullptr)
	{
		connect(machine.get(), &Machine::machineEditedWithoutUndoCommandGeneratedEvent, this, &StateS::computeDiffUndoCommand);
		connect(machine.get(), &Machine::machineEditedWithUndoCommandGeneratedEvent,    this, &StateS::addUndoCommand);

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
	// Refresh current machine
	this->refreshMachine(newMachine, false);

	// Update UI
	this->statesUi->setUnsavedFlag(false);
	this->statesUi->setUndoButtonEnabled(false);
	this->statesUi->setRedoButtonEnabled(false);
	this->statesUi->setSaveActionEnabled(false);

	// Initialize time machine
	this->undoStack.clear();
	this->updateXmlRepresentation();

	// Update file path
	this->updateFilePath(path);
}
