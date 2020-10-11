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

#ifndef FSMDRAWINGTOOLBAR_H
#define FSMDRAWINGTOOLBAR_H

// Parent class
#include "drawingtoolbar.h"

// C++ classes
#include <memory>
using namespace std;


class FsmDrawingToolBar : public DrawingToolBar
{
	Q_OBJECT

public:
	explicit FsmDrawingToolBar(shared_ptr<MachineBuilder> machineBuilder, QWidget* parent = nullptr);

protected slots:
	bool toolChangedEventHandler(MachineBuilder::tool newTool) override;

private slots:
	void mouseToolRequestedEvent(bool activated);
	void initialStateToolRequestedEvent(bool activated);
	void stateToolRequestedEvent(bool activated);
	void transitionToolRequestedEvent(bool activated);

private:
	QAction* actionMouse           = nullptr;
	QAction* actionAddInitialState = nullptr;
	QAction* actionAddState        = nullptr;
	QAction* actionAddTransition   = nullptr;
};

#endif // FSMDRAWINGTOOLBAR_H
