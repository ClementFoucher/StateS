/*
 * Copyright © 2020 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UNDOREDOMANAGER_H
#define UNDOREDOMANAGER_H

// Parent
#include <QObject>

// Qt classes
#include <QUndoStack>

// Sates classes
#include "machineundocommand.h"
class Machine;
class StatesUi;


class UndoRedoManager : public QObject
{
	Q_OBJECT

public:
	explicit UndoRedoManager(shared_ptr<StatesUi> ui);

	void setMachine(shared_ptr<Machine> newMachine, bool clearStack);
	void setClean();

signals:
	void freshMachineAvailableEvent(shared_ptr<Machine> machine);

private slots:
	void undo();
	void redo();

	void undoStackCleanStateChangeEventHandler(bool clean);
	void undoActionAvailabilityChangeEventHandler(bool undoAvailable);
	void redoActionAvailabilityChangeEventHandler(bool redoAvailable);

private:
	void addUndoCommand(MachineUndoCommand* undoCommand);
	void buildDiffUndoCommand(MachineUndoCommand::undo_command_id commandId);

private:
	QUndoStack undoStack;
	weak_ptr<Machine>  machine;
	weak_ptr<StatesUi> statesUi;
};

#endif // UNDOREDOMANAGER_H
