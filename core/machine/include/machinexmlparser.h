/*
 * Copyright © 2017 Clément Foucher
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
class MachineConfiguration;
class MachineActuatorComponent;
class Signal;
class Equation;


class MachineXmlParser : public QObject
{
	Q_OBJECT

public:
	// Pseudo-constructors returning the correct inheriting class depending on the machine type.
	static shared_ptr<MachineXmlParser> buildStringParser(const QString& xmlString);
	static shared_ptr<MachineXmlParser> buildFileParser  (shared_ptr<QFile> file);

	shared_ptr<Machine>              getMachine();
	shared_ptr<MachineConfiguration> getConfiguration();
	QList<QString>                   getWarnings();

protected:
	explicit MachineXmlParser(QObject* parent = nullptr);

	void parseMachineName(const QString& fileName);
	void parseConfiguration();
	void parseSignal();
	void parseAction();
	void parseLogicEquation();

	void treatBeginOperand(uint operandRank);
	void treatEndOperand();

private:
	virtual void buildMachineFromXml() = 0;

protected:
	QList<QString> warnings;
	shared_ptr<Machine> machine;

	shared_ptr<MachineActuatorComponent> currentActuator;
	shared_ptr<Signal> rootLogicEquation;
	shared_ptr<Signal> currentLogicEquation;

	shared_ptr<QXmlStreamReader> xmlReader;

private:
	shared_ptr<MachineConfiguration> configuration;
	QStack<shared_ptr<Equation>> equationStack;
	QStack<uint> operandRankStack;
};

#endif // MACHINEXMLPARSER_H


