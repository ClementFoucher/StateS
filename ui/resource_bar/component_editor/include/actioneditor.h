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
class QPushButton;
class QTableWidget;
class QTableWidgetItem;

// StateS Classes
class MachineActuatorComponent;
class Signal;
class CollapsibleWidgetWithTitle;
class DynamicTableItemDelegate;


class ActionEditor : public QWidget
{
    Q_OBJECT

private:
    enum ContextAction { Cancel = 0, DeleteAction = 1, AffectSwitchWhole = 2, AffectSwitchSingle = 3, AffectSwitchRange = 4, AffectEditRange = 5};

public:
    explicit ActionEditor(shared_ptr<MachineActuatorComponent> actuator, QString title = QString(), QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void updateContent();
    void updateButtonsState();
    void editValue(QTableWidgetItem* item);

    void addAction();
    void removeAction();
    void treatMenuAdd(QAction*);

    void validateEdit();
    void cancelEdit();

    void treatMenuEventHandler(QAction* action);

private:
    QList<shared_ptr<Signal> > getSelectedSignals();

private:
    weak_ptr<MachineActuatorComponent> actuator;

    CollapsibleWidgetWithTitle* hintDisplay    = nullptr;

    QMap<QTableWidgetItem*, weak_ptr<Signal>> tableItemsMapping;

    // Qwidgets
    DynamicTableItemDelegate* listDelegate;
    QTableWidget* actionList = nullptr;
    QTableWidgetItem* itemUnderEdition = nullptr;

    QPushButton* buttonAddAction = nullptr;
    QPushButton* buttonRemoveAction = nullptr;

    weak_ptr<Signal> currentSignal;
};

#endif // ACTIONEDITOR_H
