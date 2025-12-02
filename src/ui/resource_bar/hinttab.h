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

#ifndef HINTTAB_H
#define HINTTAB_H

// Parent
#include <QWidget>

// C++ classes
using namespace std;
#include <memory.h>

// StateS classes
#include "statestypes.h"
class CollapsibleWidgetWithTitle;
class HintWidget;
class MachineComponentVisualizer;


class HintTab : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit HintTab(shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void showEvent(QShowEvent* event) override;

private slots:
	void toolChangedEventHandler(MachineBuilderTool_t newTool);
	void singleUsetoolChangedEventHandler(MachineBuilderSingleUseTool_t tempTool);

private:
	void updateHint(MachineBuilderTool_t newTool);

	/////
	// Object variables
private:
	HintWidget*                 hintDisplay    = nullptr;
	CollapsibleWidgetWithTitle* machineDisplay = nullptr;

	weak_ptr<MachineComponentVisualizer> machineComponentView;

};

#endif // HINTTAB_H
