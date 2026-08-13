/*
 * Copyright © 2014-2026 Clément Foucher
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


/**
 * @brief The MachineBuilder class stores information
 * about tools currently in use by the editor.
 */
class MachineBuilder : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class Tool_t          { none, initialState, state, transition };
	enum class SingleUseTool_t { none, drawTransitionFromScene, editTransitionSource, editTransitionTarget };

	/////
	// Constructors/destructors
public:
	explicit MachineBuilder() = default;

	/////
	// Object functions
public:
	void setTool(Tool_t newTool);
	void setSingleUseTool(SingleUseTool_t t);
	void resetTool();

	Tool_t getTool() const;

	/////
	// Signals
signals:
	void changedToolEvent(Tool_t t);
	void singleUseToolSelected(SingleUseTool_t t);

	/////
	// Object variables
private:
	Tool_t currentTool = Tool_t::none;

};

#endif // MACHINEBUILDER_H
