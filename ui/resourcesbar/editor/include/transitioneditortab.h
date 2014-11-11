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

#include <QGridLayout>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QTableWidget>

#include "editortab.h"

class FsmTransition;

class TransitionEditorTab : public EditorTab
{
    Q_OBJECT

public:
    explicit TransitionEditorTab(FsmTransition* transition, QWidget* parent = 0);
    ~TransitionEditorTab();

    void changeEditedTransition(FsmTransition* transition);

private slots:
    void setCondition();
    void clearCondition();
    void treatMenuSetCondition(QAction*);
    void addAction();
    void removeAction();
    void treatMenuAdd(QAction*);
    void updateContent();

private:
    FsmTransition* transition = nullptr;

    QGridLayout* layout = nullptr;

    QTableWidget* actionList = nullptr;
    QPushButton* buttonAddAction = nullptr;
    QPushButton* buttonRemoveAction = nullptr;

    QLabel* conditionText = nullptr;
    QPushButton* buttonSetCondition = nullptr;
    QPushButton* buttonClearCondition = nullptr;
};

#endif // TRANSITIONEDITORTAB_H
