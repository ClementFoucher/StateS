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

#ifndef FSMXMLPARSER_H
#define FSMXMLPARSER_H

// Parent class
#include "machinexmlparser.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class FsmState;


class FsmXmlParser : public MachineXmlParser
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class MainTag_t
	{
		none,
		states_group,
		transitions_group
	};
	enum class SubTag_t
	{
		none,
		// First level subtags
		state,
		transition,
		// Second level subtags
		actions_group,
		condition,
		// Sublevel subtags
		action,
		logicVariable,
		logicEquation,
		operand
	};

	/////
	// Constructors/destructors
public:
	explicit FsmXmlParser(const QString& xmlString);
	explicit FsmXmlParser(shared_ptr<QFile> file);

	/////
	// Object functions
protected:
	virtual void              parseSubmachineStartElement() override;
	virtual IsSubmachineEnd_t parseSubmachineEndElement()   override;

private:
	void parseStateNode();
	void parseTransitionNode();

	void processEndCondition();

	shared_ptr<FsmState> getStateByName(const QString& name) const;

	/////
	// Object variables
private:
	// Remember position in file
	MainTag_t currentMainTag = MainTag_t::none;
	SubTag_t  currentSubTag  = SubTag_t::none;
	int unexpectedTagLevel = 0;

};

#endif // FSMXMLPARSER_H
