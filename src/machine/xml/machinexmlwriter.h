/*
 * Copyright © 2017-2026 Clément Foucher
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

#ifndef MACHINEXMLWRITER_H
#define MACHINEXMLWRITER_H

// Parent
#include <QObject>

// Stdlib
#include <memory>

// Qt
class QXmlStreamWriter;
class QFile;

// StateS
#include "machine.h"
class MachineActuatorComponent;
class ViewConfiguration;
class Equation;


class MachineXmlWriter : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class WriteMode_t { writeToFile, writeToUndo };

	/////
	// Constructors/destructors
protected:
	explicit MachineXmlWriter(WriteMode_t mode, std::shared_ptr<ViewConfiguration> viewConfiguration);

	/////
	// Object functions
public:
	void writeMachineToFile(); // Throws StatesException
	QString getMachineXml();

protected:
	void writeMachineToStream();
	virtual void writeSubmachineToStream() = 0;
	virtual void writeMachineType() = 0;

	void writeActuatorActions(std::shared_ptr<MachineActuatorComponent> component);
	void writeLogicEquation(std::shared_ptr<Equation> equation);

private:
	void createSaveFile(); // Throws StatesException
	void createSaveString();
	void finalizeSaveFile();

	void writeUiConfiguration();
	void writeMachineVariables();
	void writeMachineVariable(Machine::VariableNature_t nature, ComponentId variableId);

	/////
	// Object variables
protected:
	std::shared_ptr<ViewConfiguration> viewConfiguration;

	std::shared_ptr<QXmlStreamWriter> stream;
	QString xmlString;

	WriteMode_t mode;

private:
	std::shared_ptr<QFile> file;

};

#endif // MACHINEXMLWRITER_H
