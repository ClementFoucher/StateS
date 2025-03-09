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

#ifndef MACHINEEDITORTAB_H
#define MACHINEEDITORTAB_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class MachineComponentVisualizer;
class CollapsibleWidgetWithTitle;
class DynamicLineEditor;


class MachineEditorTab : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineEditorTab(shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void showEvent      (QShowEvent*  e) override;
	virtual void mousePressEvent(QMouseEvent* e) override;

private slots:
	void nameTextChangedEventHandler(const QString& newName);
	void updateMachineName();

	/////
	// Object variables
private:
	weak_ptr<MachineComponentVisualizer> machineComponentView;

	CollapsibleWidgetWithTitle* hintDisplay    = nullptr;
	CollapsibleWidgetWithTitle* machineDisplay = nullptr;

	DynamicLineEditor* machineName = nullptr;

};

#endif // MACHINEEDITORTAB_H
