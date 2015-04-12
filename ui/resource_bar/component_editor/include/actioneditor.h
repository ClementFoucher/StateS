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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>
class QString;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;

// StateS Classes
class MachineActuatorComponent;
class Signal;


class ActionEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ActionEditor(shared_ptr<MachineActuatorComponent> actuator, QString title = QString(), QWidget* parent = nullptr);

    void changeActuator(shared_ptr<MachineActuatorComponent> actuator);

private slots:
    void updateContent();

    void addAction();
    void removeAction();
    void treatMenuAdd(QAction*);
    void itemValueChangedEventHandler(QTableWidgetItem* item);

private:
    weak_ptr<MachineActuatorComponent> actuator;

    QMap<QTableWidgetItem*, weak_ptr<Signal>> tableItemsMapping;

    QTableWidget* actionList = nullptr;

    QPushButton* buttonAddAction = nullptr;
    QPushButton* buttonRemoveAction = nullptr;
};

#endif // ACTIONEDITOR_H
