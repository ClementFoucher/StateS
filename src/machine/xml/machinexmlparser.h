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
#include "statestypes.h"
class Machine;
class ViewConfiguration;
class GraphicAttributes;
class MachineComponent;
class Equation;
class Variable;


class MachineXmlParser : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
protected:
	enum class IsRoot_t : bool
	{
		yes = true,
		no  = false
	};

	enum class IsSubmachineEnd_t : bool
	{
		yes = true,
		no  = false
	};

private:
	enum class Tag_t
	{
		none,
		// Level 0
		machine, // Currently, tag is named FSM => TODO rename
		// Level 1
		configuration,
		variables, // Currently, tag is named Signals => TODO rename
		// Level 2
		configurationViewScale,
		configurationViewCentralPoint,
		variablesInput,
		variablesOutput,
		variablesInternal, // Currently, tag is named Variable => TODO rename
		variablesConstant,
		// Other tag in machine: passed to submachine parser
		submachineTag
	};

	/////
	// Constructors/destructors
private:
	explicit MachineXmlParser();

protected:
	explicit MachineXmlParser(const QString& xmlString);
	explicit MachineXmlParser(shared_ptr<QFile> file);

	/////
	// Object functions
public:
	void doParse();

	shared_ptr<Machine>           getMachine();
	shared_ptr<GraphicAttributes> getGraphicMachineConfiguration();
	shared_ptr<ViewConfiguration> getViewConfiguration();
	QList<QString>                getIssues();

protected:
	void parseActionNode();
	void parseLogicEquationNode();
	void parseOperandNode();
	void parseOperandVariableNode();
	void parseOperandConstantNode();

	IsRoot_t processEndLogicVariableNode();
	IsRoot_t processEndLogicEquationNode();

	virtual void              parseSubmachineStartElement() = 0;
	virtual IsSubmachineEnd_t parseSubmachineEndElement()   = 0;

	QString getCurrentNodeName() const;
	QString getCurrentNodeStringAttribute(const QString& name) const;
	uint getCurrentNodeUintAttribute(const QString& name, bool* ok) const;
	int getCurrentNodeIntAttribute(const QString& name, bool* ok) const;
	float getCurrentNodeFloatAttribute(const QString& name, bool* ok) const;
	bool getCurrentNodeBoolAttribute(const QString& name) const;
	componentId_t getCurrentNodeIdAttribute() const;

	shared_ptr<Equation> getCurrentEquation();

	void addGraphicAttribute(uint componentId, QString name, QString value);

	void addIssue(const QString& warning);

private:
	void parseStartElement();
	void parseEndElement();

	void parseMachineName();

	void parseConfigurationViewScale();
	void parseConfigurationViewCentralPoint();
	void parseVariableNode();

	shared_ptr<Variable> getVariableByName(const QString& variableName) const;

	/////
	// Object variables
protected:
	shared_ptr<Machine> machine;

	componentId_t currentComponentId;

private:
	shared_ptr<QFile> file; // Must keep it to maintain a reference to the file so that it is not destroyed
	shared_ptr<QXmlStreamReader> xmlReader;

	shared_ptr<ViewConfiguration> viewConfiguration;
	shared_ptr<GraphicAttributes> graphicAttributes;

	QStack<shared_ptr<Equation>> equationStack;
	QStack<uint> operandRankStack;

	QList<QString> issues;

	// Remember position in file
	Tag_t currentTag = Tag_t::none;
	int unexpectedTagLevel = 0;

	// Temporary workaround to identify constant parsing
	bool isParsingConstantOperand = false;

};

#endif // MACHINEXMLPARSER_H
