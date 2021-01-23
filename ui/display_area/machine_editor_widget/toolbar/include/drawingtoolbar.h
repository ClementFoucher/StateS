/*
 * Copyright © 2020 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// For enums
#include "machinebuilder.h"


class DrawingToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit DrawingToolBar(shared_ptr<MachineBuilder> machineBuilder, QWidget* parent = nullptr);

	void setMachineBuilder(shared_ptr<MachineBuilder> newMachineBuilder);

protected:
	void beginAddActions();
	void endAddActions();

	virtual void resetTool() = 0;

protected slots:
	virtual bool toolChangedEventHandler(MachineBuilder::tool newTool) = 0;

protected:
	weak_ptr<MachineBuilder> machineBuilder;

};

#endif // DRAWINGTOOLBAR_H
