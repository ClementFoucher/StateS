/*
 * Copyright © 2021 Clément Foucher
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

#ifndef MACHINEMANAGER_H
#define MACHINEMANAGER_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Machine;
class MachineStatus;
class UndoRedoManager;
class ViewConfiguration;
class MachineBuilder;


class MachineManager : public QObject, public enable_shared_from_this<MachineManager>
{
	Q_OBJECT

public:
	explicit MachineManager();

	// Required to handle UndoRedoManaer as there
	// is a circular dependency between these two
	void build();
	void clear();

	// Mutators
	void setMachine(shared_ptr<Machine> newMachine);
	void setViewConfiguration(shared_ptr<ViewConfiguration> viewConfiguration);

	// Acessors
	shared_ptr<Machine>        getMachine()        const;
	shared_ptr<MachineStatus>  getMachineStatus()  const;
	shared_ptr<MachineBuilder> getMachineBuilder() const;
	// Actions
	void undo();
	void redo();

	// Other
	void addConnection(QMetaObject::Connection connection);

signals:
	void machineUpdatedEvent(bool isNewMachine);
	void machineViewUpdateRequestedEvent();

	void undoActionAvailabilityChangeEvent(bool undoAvailable);
	void redoActionAvailabilityChangeEvent(bool redoAvailable);

private slots:
	void freshMachineAvailableFromUndoRedo(shared_ptr<Machine> updatedMachine);
	void machineUnsavedFlagChangedEventHandler();

private:
	void setMachineInternal(shared_ptr<Machine> newMachine, bool isNewMachine);

private:
	shared_ptr<Machine>        machine;
	shared_ptr<MachineStatus>  machineStatus;
	shared_ptr<MachineBuilder> machineBuilder;

	// Temporary
	shared_ptr<ViewConfiguration>    viewConfiguration;

	shared_ptr<UndoRedoManager> undoRedoManager;

	QList<QMetaObject::Connection> connections;
};

#endif // MACHINEMANAGER_H
