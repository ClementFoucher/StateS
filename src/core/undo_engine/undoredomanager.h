/*
 * Copyright © 2020-2021 Clément Foucher
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
class MachineManager;
class Machine;


class UndoRedoManager : public QObject
{
	Q_OBJECT

public:
	explicit UndoRedoManager(shared_ptr<MachineManager> machineManager);

	void undo();
	void redo();

	void clearStack();
	void setClean();

signals:
	void freshMachineAvailableEvent(shared_ptr<Machine> machine);

	void undoActionAvailabilityChangeEvent(bool undoAvailable);
	void redoActionAvailabilityChangeEvent(bool redoAvailable);

private slots:
	void machineUpdatedEventHandler(bool isNewMachine);

	void undoStackCleanStateChangeEventHandler(bool clean);

private:
	void addUndoCommand(MachineUndoCommand* undoCommand);
	void buildDiffUndoCommand(MachineUndoCommand::undo_command_id commandId);
	void setMachineUnsavedFlag(bool unsaved);

private:
	shared_ptr<MachineManager> machineManager;

	QUndoStack undoStack;
};

#endif // UNDOREDOMANAGER_H
