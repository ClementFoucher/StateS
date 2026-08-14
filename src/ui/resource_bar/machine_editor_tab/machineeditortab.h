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

#ifndef MACHINEEDITORTAB_H
#define MACHINEEDITORTAB_H

// Parent
#include <QWidget>

// Stdlib
#include <memory>

// Qt
class QGroupBox;

// StateS
class MachineComponentVisualizer;
class CollapsibleWidgetWithTitle;
class SelfManagedDynamicLineEditor;


class MachineEditorTab : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit MachineEditorTab(std::shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void showEvent      (QShowEvent*   event) override;
	virtual void mousePressEvent(QMouseEvent*  event) override;
	virtual void resizeEvent    (QResizeEvent* event) override;

private slots:
	void nameTextChangedEventHandler(const QString& newName);
	void updateMachineName();

	/////
	// Object variables
private:
	std::weak_ptr<MachineComponentVisualizer> machineComponentView;

	SelfManagedDynamicLineEditor* machineName     = nullptr;
	QGroupBox*                    variablesEditor = nullptr;
	CollapsibleWidgetWithTitle*   machineDisplay  = nullptr;

};

#endif // MACHINEEDITORTAB_H
