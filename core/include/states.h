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

#ifndef STATES_H
#define STATES_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QPixmap;

// StateS classes
class StatesUi;
class Machine;


class StateS : public QObject
{
    Q_OBJECT

public: // Static
    static QPixmap getPixmapFromSvg(const QString& path);
    static QString getVersion();

public:
    explicit StateS();
    ~StateS();

    void run();

private slots:
    void generateNewFsm();
    void clearMachine();
    void loadMachine(const QString& path);
    void saveCurrentMachine(const QString& path);
    void saveCurrentMachineInCurrentFile();

private:
    unique_ptr<StatesUi> statesUi;
    shared_ptr<Machine>  machine;

    QString currentFilePath = QString::null;
};

#endif // STATES_H
