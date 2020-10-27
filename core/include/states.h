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
class StatesUi;
class Machine;
class UndoRedoManager;


/**
 * @brief The StateS class is the root object of this application:
 * it owns the current machine under edition, the UI and the
 * undo/redo manager.
 *
 * Its main purpose is to deal with the machine object:
 * Changing references, building new, loading, saving, etc.
 *
 * This class also gives access to static generic functions.
 */
class StateS : public QObject
{
	Q_OBJECT

public:
	static QString getVersion();
	static QString getCopyrightYears();

public:
	explicit StateS(const QString& initialFilePath = QString());

	void run();

private slots:
	// Handle signals from UI
	void generateNewFsm();
	void clearMachine();

	void loadMachine(const QString& path);

	void saveCurrentMachine(const QString& path);
	void saveCurrentMachineInCurrentFile();

	// Handle signals from UndoRedoManager
	void freshMachineAvailableFromUndoRedo(shared_ptr<Machine> machine);

private:
	void refreshMachine(shared_ptr<Machine> newMachine, bool machineChanged);

private:
	shared_ptr<Machine>         machine         = nullptr;
	shared_ptr<StatesUi>        statesUi        = nullptr;
	shared_ptr<UndoRedoManager> undoRedoManager = nullptr;
};

#endif // STATES_H
