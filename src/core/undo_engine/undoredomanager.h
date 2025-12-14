/*
 * Copyright © 2020-2025 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QUndoStack>

// Sates classes
class Machine;
class GraphicAttributes;
class StatesUndoCommand;


class UndoRedoManager : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit UndoRedoManager();

	/////
	// Object functions
public:
	void undo();
	void redo();

	void beginMacro(const QString& text);
	void endMacro();

	void setClean();

	void addUndoCommand(StatesUndoCommand* undoCommand);
	void buildAndAddDiffUndoCommand(const QString& undoDescription = QString());

	void notifyMachineReplaced();

signals:
	void freshMachineAvailableEvent(shared_ptr<Machine> machine, shared_ptr<GraphicAttributes> machineConfiguration);

	void undoActionAvailabilityChangeEvent(bool undoAvailable);
	void redoActionAvailabilityChangeEvent(bool redoAvailable);

private slots:
	void undoStackCleanStateChangeEventHandler(bool clean);

	/////
	// Object variables
private:
	QUndoStack undoStack;
	bool recordingMacro = false;

};

#endif // UNDOREDOMANAGER_H
