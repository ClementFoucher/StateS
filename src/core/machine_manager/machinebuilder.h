/*
 * Copyright © 2014-2025 Clément Foucher
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

// StateS classes
#include "statestypes.h"


/**
 * @brief The MachineBuilder class stores information
 * about tools currently in use by the editor.
 */
class MachineBuilder : public QObject
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineBuilder() = default;

	/////
	// Object functions
public:
	void setTool(MachineBuilderTool_t newTool);
	void setSingleUseTool(MachineBuilderSingleUseTool_t t);
	void resetTool();

	MachineBuilderTool_t getTool() const;

	/////
	// Signals
signals:
	void changedToolEvent(MachineBuilderTool_t t);
	void singleUseToolSelected(MachineBuilderSingleUseTool_t t);

	/////
	// Object variables
private:
	MachineBuilderTool_t currentTool = MachineBuilderTool_t::none;

};

#endif // MACHINEBUILDER_H
