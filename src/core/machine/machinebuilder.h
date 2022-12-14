/*
 * Copyright © 2014-2021 Clément Foucher
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

#ifndef MACHINEBUILDER_H
#define MACHINEBUILDER_H

// Parent
#include <QObject>


class MachineBuilder : public QObject
{
	Q_OBJECT

public:
	enum class tool {none,
		             initial_state,
		             state,
		             transition
	                };

	enum class singleUseTool { none,
		                       drawTransitionFromScene,
		                       editTransitionSource,
		                       editTransitionTarget
	                         };
public:
	explicit MachineBuilder();

	tool getTool() const;
	void setTool(tool newTool);
	void setSingleUseTool(singleUseTool t);

signals:
	void changedToolEvent(tool t);
	void singleUseToolSelected(singleUseTool t);

private:
	tool currentTool = tool::none;

};

#endif // MACHINEBUILDER_H
