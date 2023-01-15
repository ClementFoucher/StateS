/*
 * Copyright © 2017-2023 Clément Foucher
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
class Signal;
class MachineActuatorComponent;
class ViewConfiguration;


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
	virtual void writeMachineToFile(); // Throws StatesException
	virtual QString getMachineXml();

protected:
	virtual void writeMachineToStream() = 0;

	void createSaveFile(); // Throws StatesException
	void createSaveString();
	void finalizeSaveFile();

	void writeMachineCommonElements();
	void writeMachineConfiguration();
	void writeMachineSignals();
	void writeActuatorActions(shared_ptr<MachineActuatorComponent> component);
	void writeLogicEquation(shared_ptr<Signal> equation);

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
