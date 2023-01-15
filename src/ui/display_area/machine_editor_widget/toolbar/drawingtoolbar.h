/*
 * Copyright © 2020-2023 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DRAWINGTOOLBAR_H
#define DRAWINGTOOLBAR_H

// Parent class
#include <QToolBar>

// StateS classes
#include "statestypes.h"


class DrawingToolBar : public QToolBar
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit DrawingToolBar(QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	void beginAddActions();
	void endAddActions();

	virtual void resetTool() = 0;

protected slots:
	virtual bool toolChangedEventHandler(MachineBuilderTool_t newTool) = 0;

};

#endif // DRAWINGTOOLBAR_H
