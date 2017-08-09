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
#ifndef FSMXMLPARSER_H
#define FSMXMLPARSER_H

// Parent class
#include "machinexmlparser.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Fsm;


class FsmXmlParser : public MachineXmlParser
{
	Q_OBJECT

private:
	enum class group_e { none, configuration_group, signals_group, states_group, transitions_group };
	enum class subgroup_e { none,
		                    state, transition,
		                    actions_group, action,
		                    condition, logicVariable, logicEquation, operand
	                      };

public:
	explicit FsmXmlParser(const QString& xmlString);
	explicit FsmXmlParser(shared_ptr<QFile> file);

	void treatStartElement();
	void treatEndElement();

private:
	explicit FsmXmlParser(QObject* parent = nullptr);

	virtual void buildMachineFromXml() override;

	void parseState();
	void parseTransition();

	shared_ptr<Fsm> getFsm() const;

private:
	QString fileName;

	// Remember position in file
	group_e    currentGroup;
	subgroup_e currentSubGroup;

	int currentLevel;
};

#endif // FSMXMLPARSER_H
