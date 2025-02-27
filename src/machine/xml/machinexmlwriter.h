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

#ifndef MACHINEXMLWRITER_H
#define MACHINEXMLWRITER_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QXmlStreamWriter;
class QFile;

// StateS classes
#include "statestypes.h"
class Variable;
class MachineActuatorComponent;
class ViewConfiguration;
class Equation;


class MachineXmlWriter : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
protected:
	explicit MachineXmlWriter(MachineXmlWriterMode_t mode, shared_ptr<ViewConfiguration> viewConfiguration);

	/////
	// Object functions
public:
	void writeMachineToFile(); // Throws StatesException
	QString getMachineXml();

protected:
	void writeMachineToStream();
	virtual void writeSubmachineToStream() = 0;
	virtual void writeMachineType() = 0;

	void writeActuatorActions(shared_ptr<MachineActuatorComponent> component);
	void writeLogicEquation(shared_ptr<Equation> equation);

private:
	void createSaveFile(); // Throws StatesException
	void createSaveString();
	void finalizeSaveFile();

	void writeUiConfiguration();
	void writeMachineVariables();
	void writeMachineVariable(VariableNature_t nature, componentId_t variableId);

	/////
	// Object variables
protected:
	shared_ptr<ViewConfiguration> viewConfiguration;

	shared_ptr<QXmlStreamWriter> stream;
	QString xmlString;

	MachineXmlWriterMode_t mode;

private:
	shared_ptr<QFile> file;

};

#endif // MACHINEXMLWRITER_H
