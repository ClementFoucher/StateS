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

#ifndef CONDITIONEDITOR_H
#define CONDITIONEDITOR_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QLabel;
class QPushButton;
class QGridLayout;

// StateS classes
class Signal;
class TruthTable;
class TruthTableDisplay;
class FsmTransition;


class ConditionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ConditionEditor(shared_ptr<FsmTransition> transition, QWidget* parent = nullptr);
    ~ConditionEditor();

    void updateTransition(shared_ptr<FsmTransition> newTransition);

private slots:
    void editCondition();
    void clearCondition();

    void treatMenuSetCondition(QAction*);

    void updateContent();

private:
    void expandTruthTable();
    void collapseTruthTable();
    bool isTruthTableDisplayed();

    weak_ptr<FsmTransition> transition;
    TruthTable* truthTable = nullptr;

    QGridLayout* layout = nullptr;

    QLabel* conditionText        = nullptr;
    QLabel* conditionWarningText = nullptr;

    QPushButton* buttonSetCondition   = nullptr;

    QPushButton*       buttonExpandTruthTable = nullptr;
    TruthTableDisplay* truthTableDisplay      = nullptr;
};

#endif // CONDITIONEDITOR_H
