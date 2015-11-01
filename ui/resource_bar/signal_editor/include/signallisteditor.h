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

#ifndef SIGNALLISTEDITOR_H
#define SIGNALLISTEDITOR_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>
class QGridLayout;
class QPushButton;
class QTableWidgetItem;

// To access enums
#include "machine.h"

// StateS classes
class DynamicTableItemDelegate;
class TableWidgetWithResizeEvent;


class SignalListEditor : public QWidget
{
    Q_OBJECT

private:
    enum class mode{initMode, standard, addingSignal, renamingSignal, resizingSignal, changingSignalInitialValue};

public:
    explicit SignalListEditor(shared_ptr<Machine> machine, Machine::signal_type editorType, QWidget* parent = nullptr);

protected:
    void keyPressEvent  (QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void updateList();
    void removeSelectedSignals();
    void updateButtonsEnableState();
    void beginAddSignal();
    void beginEditSignal(QTableWidgetItem* characteristicToEdit);
    void validateCurrentEdit();
    void cancelCurrentEdit();
    void raiseSignal();
    void lowerSignal();

private:
    void switchMode(mode newMode);

    void endAddSignal();
    void endRenameSignal();
    void endResizeSignal();
    void endChangeSignalInitialValue();

    void handleListResizedEvent();

private:
    // Static
    weak_ptr<Machine> machine;
    Machine::signal_type editorType;
    QString newSignalsPrefix;

    // Dynamic
    mode currentMode = mode::initMode;

    // Widgets
    QGridLayout* buttonLayout = nullptr;

    TableWidgetWithResizeEvent* signalsList = nullptr;
    DynamicTableItemDelegate* listDelegate = nullptr;

    QPushButton* buttonAdd    = nullptr;
    QPushButton* buttonRemove = nullptr;
    QPushButton* buttonCancel = nullptr;
    QPushButton* buttonOK     = nullptr;
    QPushButton* buttonUp     = nullptr;
    QPushButton* buttonDown   = nullptr;

    // Cell under edition
    QTableWidgetItem* currentTableItem = nullptr;
    QString signalUnderEdition = QString::null;

    // Used to know which signal is associated to each cell in table
    QMap<QTableWidgetItem*, weak_ptr<Signal>> associatedSignals;
};

#endif // SIGNALLISTEDITOR_H
