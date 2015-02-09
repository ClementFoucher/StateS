/*
 * Copyright © 2014 Clément Foucher
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

#include <QPushButton>
#include <QGridLayout>
#include <QTableWidget>

#include "componenteditortab.h"

class FsmState;
class DynamicLineEdit;
class ActionEditor;

class StateEditorTab : public ComponentEditorTab
{
    Q_OBJECT

public:
    explicit StateEditorTab(FsmState* state, QWidget* parent = nullptr);

    void changeEditedState(FsmState* state);
    void setEditName();

private slots:
    void nameChanged(const QString& name);
    void updateContent();

private:
    void updateLocalContent();

    FsmState* state = nullptr;

    DynamicLineEdit* textStateName = nullptr;
    ActionEditor* actionEditor = nullptr;
};

#endif // STATEEDITORTAB_H
