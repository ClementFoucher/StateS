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

#ifndef STATES_H
#define STATES_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QUndoStack>

// StateS classes
#include "machineundocommand.h"
class StatesUi;
class Machine;


/**
 * @brief The StateS class is the root object of this application:
 * it owns the UI and the current machine under edition.
 *
 * Only one StateS instance is allowed in the application, or
 * it could lead to unpredictible behavior when dealing with
 * static members.
 *
 * It is the only one able to change the current machine.
 *
 * This class is also used statically for generic functions.
 */
class StateS : public QObject
{
	Q_OBJECT

public:
	static QString getVersion();
	static QString getCopyrightYears();
	static shared_ptr<Machine> getCurrentMachine();
	static QString getCurrentXmlCode();

public:
	explicit StateS(const QString& initialFilePath = QString());

	void run();

private slots:
	// New / load / save
	void clearMachine();
	void generateNewFsm();

	void loadMachine(const QString& path);

	void saveCurrentMachine(const QString& path);
	void saveCurrentMachineInCurrentFile();

	// Undo management
	void computeDiffUndoCommand(MachineUndoCommand::undo_command_id commandId);
	void addUndoCommand(MachineUndoCommand* undoCommand);

	void undo();
	void redo();

	void refreshMachineFromDiffUndoRedo(shared_ptr<Machine> machine);

	// UI updating
	void undoStackCleanStateChangeEventHandler(bool clean);
	void undoActionAvailabilityChangeEventHandler(bool undoAvailable);
	void redoActionAvailabilityChangeEventHandler(bool redoAvailable);

private:
	void loadNewMachine(shared_ptr<Machine> newMachine, const QString& path = QString());
	void refreshMachine(shared_ptr<Machine> newMachine, bool maintainView);

	void updateXmlRepresentation();
	void updateFilePath(const QString& newPath);

	// UI
	void setMachineDirty();

private:
	static QString machineXmlRepresentation;
	static shared_ptr<Machine> machine;

private:
	shared_ptr<StatesUi> statesUi;

	QString currentFilePath = QString();
	QUndoStack undoStack;
};

#endif // STATES_H
