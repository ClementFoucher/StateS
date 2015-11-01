/*
 * Copyright © 2014-2015 Clément Foucher
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

#ifndef MACHINEBUILDERTAB_H
#define MACHINEBUILDERTAB_H

// Parent
#include <QWidget>

// C++ classes
using namespace std;
#include "memory.h"

// StateS classes
class CollapsibleWidgetWithTitle;
class MachineComponentVisualizer;
class DynamicLineEdit;

// To access enums
#include "machinebuilder.h"
#include "machine.h"


class MachineBuilderTab : public QWidget
{
    Q_OBJECT

public:
    explicit MachineBuilderTab(shared_ptr<Machine> machine, shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent = nullptr);

protected:
    void showEvent      (QShowEvent* e)  override;
    void mousePressEvent(QMouseEvent* e) override;

private slots:
    void toolChangedEventHandler(MachineBuilder::tool newTool);
    void singleUsetoolChangedEventHandler(MachineBuilder::singleUseTool tempTool);
    void nameTextChangedEventHandler(const QString& name);
    void updateContent();

private:
    void updateHint(MachineBuilder::tool newTool);

private:
    CollapsibleWidgetWithTitle* hintDisplay    = nullptr;
    CollapsibleWidgetWithTitle* machineDisplay = nullptr;

    weak_ptr<MachineComponentVisualizer> machineComponentView;
    weak_ptr<Machine>                    machine;

    DynamicLineEdit* stateName = nullptr;
};

#endif // MACHINEBUILDERTAB_H

