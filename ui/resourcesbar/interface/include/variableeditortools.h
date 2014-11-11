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

#ifndef VARIABLEEDITORTOOLS_H
#define VARIABLEEDITORTOOLS_H

#include <QWidget>

#include <QGridLayout>
#include <QListWidget>
#include <QPushButton>

class Machine;
class IoListDelegate;

class VariableEditorTools : public QWidget
{
    Q_OBJECT

public:
    enum class type{inputs, outputs, variables};

private:
    enum class mode{initMode, standard, addingIO, renamingIO};

public:
    explicit VariableEditorTools(Machine* machine, type editorType);
    ~VariableEditorTools();

public slots:
    void updateList();

signals:
    bool addIOEvent(const QString& name);
    bool removeIOEvent(const QString& name);
    bool renameIOEvent(const QString& oldName, const QString& newName);

private slots:
    void removeSelectedIOs();
    void toggleButtonsEnable();
    void beginAddIO();
    void endAddIO(QWidget* io);
    void beginRenameIO(QListWidgetItem* io);
    void endRenameIO(QWidget* io);

private:
    void switchMode(mode newMode);

    mode currentMode = mode::initMode;

    QGridLayout* layout = nullptr;

    QListWidget    * ioList = nullptr;
    IoListDelegate * listDelegate = nullptr;

    QPushButton * buttonAdd = nullptr;
    QPushButton * buttonRemove = nullptr;
    QPushButton * buttonCancel = nullptr;

    Machine* machine = nullptr;
    type editorType;

    QListWidgetItem* currentIO = nullptr;


};

#endif // VARIABLEEDITORTOOLS_H
