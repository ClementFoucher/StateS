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

#ifndef SIGNALLISTEDITOR_H
#define SIGNALLISTEDITOR_H

// Parent
#include <QWidget>

// Qt classes
#include <QGridLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QMap>

// To access enums
#include "machine.h"

// StateS classes
class DynamicTableItemDelegate;

class SignalListEditor : public QWidget
{
    Q_OBJECT

private:
    enum class mode{initMode, standard, addingSignal, renamingSignal, resizingSignal, changingSignalInitialValue};

public:
    explicit SignalListEditor(Machine* machine, Machine::signal_types editorType);

public slots:
    void updateList();

signals:
    bool addSignalEvent(Machine::signal_types, const QString& name);
    bool removeSignalEvent(const QString& name);
    bool renameSignalEvent(const QString& oldName, const QString& newName);
    bool resizeSignalEvent(const QString& name, uint newSize);
    bool changeSignalInitialValueEvent(const QString& name, LogicValue newValue);

private slots:
    void removeSelectedSignals();
    void updateButtonsEnableState();
    void beginAddSignal();
    void endAddSignal(QWidget* signalNameWidget);
    void beginEditSignal(QTableWidgetItem* characteristicToEdit);
    void endRenameSignal(QWidget* signalNameWidget);
    void endResizeSignal(QWidget* signalSizeWidget);
    void endChangeSignalInitialValue(QWidget* signalInitialValueWidget);

private:
    void switchMode(mode newMode);

    mode currentMode = mode::initMode;

    Machine* machine = nullptr;
    Machine::signal_types editorType;

    QGridLayout* layout = nullptr;

    QTableWidget* signalsList = nullptr;
    DynamicTableItemDelegate* listDelegate = nullptr;

    QPushButton * buttonAdd = nullptr;
    QPushButton * buttonRemove = nullptr;
//    QPushButton * buttonCancel = nullptr;

    QTableWidgetItem* currentTableItem = nullptr;
    QMap<QTableWidgetItem*, Signal*> associatedSignals;
};

#endif // SIGNALLISTEDITOR_H
