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

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

// Parent
#include <QWidget>

// Qt classes
#include <QString>
#include <QPushButton>
#include <QTableWidget>
#include <QMap>

// StateS Classes
class MachineActuatorComponent;
class Signal;

class ActionEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ActionEditor(MachineActuatorComponent* actuator, QString title = QString(), QWidget* parent = nullptr);

    void changeActuator(MachineActuatorComponent* actuator);

private slots:
    void updateContent();

    void addAction();
    void removeAction();
    void treatMenuAdd(QAction*);
    void itemValueChanged(QTableWidgetItem* item);

private:
    MachineActuatorComponent* actuator;

    QMap<QTableWidgetItem*, Signal*> tableItemsMapping;

    QTableWidget* actionList = nullptr;

    QPushButton* buttonAddAction = nullptr;
    QPushButton* buttonRemoveAction = nullptr;
};

#endif // ACTIONEDITOR_H
