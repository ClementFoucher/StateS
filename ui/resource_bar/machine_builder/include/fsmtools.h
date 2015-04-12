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

#ifndef FSMTOOLS_H
#define FSMTOOLS_H

// Parent
#include "machinetools.h"

// Qt classes
class QWidget;
class QPushButton;


class FsmTools : public MachineTools
{
    Q_OBJECT

public:
    explicit FsmTools(QWidget* parent = nullptr);

    tool getTool() const override;
    bool setTool(MachineTools::tool newTool) override;

private slots:
    void buttonAddStatePushed(bool activated);
    void buttonAddInitialStatePushed(bool activated);
    void buttonAddTransitionPushed(bool activated);
    void buttonNotToolPushed(bool);

private:
    QPushButton* buttonAddState        = nullptr;
    QPushButton* buttonAddInitialState = nullptr;
    QPushButton* buttonAddTransition   = nullptr;
    QPushButton* buttonNoTool          = nullptr;
};

#endif // FSMTOOLS_H
