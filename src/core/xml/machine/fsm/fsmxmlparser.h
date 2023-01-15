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

#ifndef FSMXMLPARSER_H
#define FSMXMLPARSER_H

// Parent class
#include "machinexmlparser.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Fsm;
class FsmState;


class FsmXmlParser : public MachineXmlParser
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class Group_t
	{
		none,
		configuration_group,
		signals_group,
		states_group,
		transitions_group
	};
	enum class Subgroup_t
	{
		none,
		state, transition,
		actions_group, action,
		condition, logicVariable, logicEquation, operand
	};

	/////
	// Constructors/destructors
public:
	explicit FsmXmlParser(const QString& xmlString);
	explicit FsmXmlParser(shared_ptr<QFile> file);

	/////
	// Object functions
public:
	void treatStartElement();
	void treatEndElement();

private:
	explicit FsmXmlParser();

	virtual void buildMachineFromXml() override;

	void parseState();
	void parseTransition();

	shared_ptr<FsmState> getStateByName(const QString& name) const;

	/////
	// Object variables
private:
	QString fileName;

	// Remember position in file
	Group_t    currentGroup;
	Subgroup_t currentSubGroup;

	int currentLevel;

};

#endif // FSMXMLPARSER_H
