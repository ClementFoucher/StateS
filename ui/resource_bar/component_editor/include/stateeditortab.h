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

#ifndef STATEEDITORTAB_H
#define STATEEDITORTAB_H

// Parent
#include "componenteditortab.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class FsmState;
class DynamicLineEdit;
class ActionEditor;


class StateEditorTab : public ComponentEditorTab
{
    Q_OBJECT

public:
    explicit StateEditorTab(shared_ptr<FsmState> state, QWidget* parent = nullptr);

    void changeEditedState(shared_ptr<FsmState> state);
    void setEditName();

protected:
    void mousePressEvent(QMouseEvent*) override;

private slots:
    void nameChangedEventHandler(const QString& name);
    void updateContent();

private:
    weak_ptr<FsmState> state;

    DynamicLineEdit* textStateName = nullptr;
    ActionEditor* actionEditor = nullptr;
};

#endif // STATEEDITORTAB_H
