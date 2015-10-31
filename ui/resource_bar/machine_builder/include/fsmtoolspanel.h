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

#ifndef FSMTOOLSPANEL_H
#define FSMTOOLSPANEL_H

// Parent
#include "machinetoolspanel.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QWidget;
class QPushButton;

// StateS classes
class MachineBuilder;


class FsmToolsPanel : public MachineToolsPanel
{
    Q_OBJECT

public:
    explicit FsmToolsPanel(shared_ptr<MachineBuilder> machineBuilder, QWidget* parent = nullptr);

protected slots:
    bool toolChangedEventHandler(MachineBuilder::tool newTool) override;

private slots:
    void buttonAddStatePushed(bool activated);
    void buttonAddInitialStatePushed(bool activated);
    void buttonAddTransitionPushed(bool activated);
    void buttonNotToolPushed(bool);

private:
    void lockOnError();

    // QWidgets with parent
    QPushButton* buttonAddState        = nullptr;
    QPushButton* buttonAddInitialState = nullptr;
    QPushButton* buttonAddTransition   = nullptr;
    QPushButton* buttonNoTool          = nullptr;
};

#endif // FSMTOOLSPANEL_H
