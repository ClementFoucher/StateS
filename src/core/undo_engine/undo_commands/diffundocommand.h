/*
 * Copyright © 2017-2025 Clément Foucher
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

#ifndef DIFFUNDOCOMMAND_H
#define DIFFUNDOCOMMAND_H

// Parent class
#include "statesundocommand.h"

// C++ classes
#include <memory>
using namespace std;

// Third-party classes
#include "dtl.hpp"

// StateS classes
class Machine;
class GraphicAttributes;


class DiffUndoCommand : public StatesUndoCommand
{
	Q_OBJECT

	/////
	// Type declarations
private:
	using DtlDiff = dtl::Diff<QString, QStringList>;

	/////
	// Static functions
public:
	static void updateXmlRepresentation();

	/////
	// Static variables
private:
	static QString machineXmlRepresentation;

	/////
	// Constructors/destructors
public:
	explicit DiffUndoCommand(const QString& description);

	/////
	// Object functions
public:
	virtual void undo() override;
	virtual void redo() override;

	virtual bool mergeWith(const QUndoCommand* command) override;

private:
	void replaceMachine(const QStringList& newXmlCode);

	DtlDiff computeDiff(const QStringList& oldString, const QStringList& newString) const;
	QStringList patchString(const DtlDiff& patch, const QStringList& string) const;

signals:
	// As applying a diff undo command reloads a complete machine,
	// this signal is used to communicate with the main Machine Manager
	// object which monitors it, applying a machine refresh.
	void applyUndoRedo(shared_ptr<Machine> machine, shared_ptr<GraphicAttributes> machineConfiguration);

	/////
	// Object variables
private:
	DtlDiff undoDiff;
	DtlDiff redoDiff;

};

#endif // DIFFUNDOCOMMAND_H
