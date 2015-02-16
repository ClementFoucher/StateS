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

#ifndef TRANSITIONEDITORTAB_H
#define TRANSITIONEDITORTAB_H

// Parent
#include "componenteditortab.h"

// Qt classes
#include <QGridLayout>
#include <QPushButton>
#include <QWidget>
#include <QLabel>

// StateS classes
class FsmTransition;
class ActionEditor;

class TransitionEditorTab : public ComponentEditorTab
{
    Q_OBJECT

public:
    explicit TransitionEditorTab(FsmTransition* transition, QWidget* parent = nullptr);

    void changeEditedTransition(FsmTransition* transition);

private slots:
    void setCondition();
    void clearCondition();
    void treatMenuSetCondition(QAction*);
    void updateContent();

private:
    void updateLocalContent();

    FsmTransition* transition = nullptr;

    QGridLayout* layout = nullptr;

    ActionEditor* actionEditor = nullptr;

    QLabel* conditionText        = nullptr;
    QLabel* conditionWarningText = nullptr;
    QPushButton* buttonSetCondition   = nullptr;
    QPushButton* buttonClearCondition = nullptr;
};

#endif // TRANSITIONEDITORTAB_H
