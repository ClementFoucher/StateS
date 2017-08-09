/*
 * Copyright © 2014-2017 Clément Foucher
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

#ifndef SIGNALSEDITORTAB_H
#define SIGNALSEDITORTAB_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class Machine;
class MachineComponentVisualizer;
class CollapsibleWidgetWithTitle;


class SignalEditorTab : public QWidget
{
	Q_OBJECT

public:
	explicit SignalEditorTab(shared_ptr<Machine> machine, shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent = nullptr);

	void setHintCollapsed(bool collapse);
	void setVisuCollapsed(bool collapse);
	bool getHintCollapsed();
	bool getVisuCollapsed();

protected:
	void showEvent(QShowEvent* e) override;

private:
	weak_ptr<MachineComponentVisualizer> machineComponentView;

	CollapsibleWidgetWithTitle* hintDisplay;
	CollapsibleWidgetWithTitle* machineDisplay;
};

#endif // SIGNALSEDITORTAB_H

