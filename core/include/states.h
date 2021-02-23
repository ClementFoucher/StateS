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
class MachineManager;
class StatesUi;


/**
 * @brief The StateS class is the root object of this application:
 * it builds the machine manager and the UI.
 *
 * Its main purpose is to deal with the machine changes:
 * building new, loading, saving, etc.
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
	~StateS();

	void run();

private slots:
	// Handle signals from UI
	void generateNewFsm();
	void clearMachine();

	void loadMachine(const QString& path);

	void saveCurrentMachine(const QString& path);
	void saveCurrentMachineInCurrentFile();

private:
	// These pointers can shared by objects that are persistent throughout the application life
	shared_ptr<MachineManager> machineManager;
	shared_ptr<StatesUi>       statesUi;
};

#endif // STATES_H
