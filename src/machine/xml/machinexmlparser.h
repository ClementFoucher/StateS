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

#ifndef MACHINEXMLPARSER_H
#define MACHINEXMLPARSER_H

// Parent class
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QStack>
class QFile;
class QXmlStreamReader;

// StateS classes
class Machine;
class ViewConfiguration;
class GraphicAttributes;
class MachineActuatorComponent;
class Equation;


class MachineXmlParser : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
protected:
	explicit MachineXmlParser();

	/////
	// Object functions
public:
	void doParse();

	shared_ptr<Machine>           getMachine();
	shared_ptr<GraphicAttributes> getGraphicMachineConfiguration();
	shared_ptr<ViewConfiguration> getViewConfiguration();
	QList<QString>                getWarnings();

protected:

	void parseMachineName(const QString& fileName);
	void parseConfiguration();
	void parseVariable();
	void parseAction();
	void parseLogicEquation();

	void treatBeginOperand(uint operandRank);
	void treatEndOperand();

private:
	virtual void buildMachineFromXml() = 0;

	/////
	// Object variables
protected:
	shared_ptr<GraphicAttributes> graphicAttributes;

	QList<QString> warnings;
	shared_ptr<Machine> machine;

	shared_ptr<MachineActuatorComponent> currentActuator;
	shared_ptr<Equation> rootLogicEquation;
	shared_ptr<Equation> currentLogicEquation;

	shared_ptr<QXmlStreamReader> xmlReader;
	shared_ptr<QFile> file; // Keep it just to maintain a reference to the file so that it is not destroyed

private:
	shared_ptr<ViewConfiguration> viewConfiguration;
	QStack<shared_ptr<Equation>> equationStack;
	QStack<uint> operandRankStack;

};

#endif // MACHINEXMLPARSER_H
