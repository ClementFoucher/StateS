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

#ifndef EQUATIONEDITOR_H
#define EQUATIONEDITOR_H

#include <QDialog>

#include <QListWidget>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>

class Machine;
class GraphicEquation;
class LogicEquation;
class LogicVariable;

class EquationEditor : public QDialog
{
    Q_OBJECT

public:
    explicit EquationEditor(Machine* machine, LogicVariable* initialEquation, QWidget* parent = 0);

    LogicVariable* getResultEquation() const;

private:
    Machine* machine = nullptr;

    QVBoxLayout* mainLayout         = nullptr;

    QHBoxLayout* resourcesLayout    = nullptr;
    QVBoxLayout* inputListLayout    = nullptr;
    QVBoxLayout* variableListLayout = nullptr;
    QGridLayout* operatorListLayout = nullptr;
    QHBoxLayout* buttonsLayout      = nullptr;

    GraphicEquation* equation = nullptr;

    QPushButton* buttonOK = nullptr;
    QPushButton* buttonCancel = nullptr;
};

#endif // EQUATIONEDITOR_H

