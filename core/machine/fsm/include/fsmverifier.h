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

#ifndef FSMVERIFIER_H
#define FSMVERIFIER_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QList>

// StateS classes
class TruthTable;
class Fsm;


// For now, use pointers to TruthTables for storage reasons
// (QHash vs shared_ptr).
// Should be reviewd a some point.
class FsmVerifier : public QObject
{
    Q_OBJECT

public:
    enum severity { blocking, structure, tool, hint };

    class Issue
    {
    public:
        QString text = QString::null;
        severity type = severity::hint;
        TruthTable* proof = nullptr;
        QList<int> proofsHighlight;
    };

public:
    explicit FsmVerifier(shared_ptr<Fsm> machine);
    ~FsmVerifier();

    const QList<shared_ptr<Issue>>& getIssues();
    const QList<shared_ptr<Issue>>& verifyFsm(bool checkVhdl);

private:
    void clearProofs();

    weak_ptr<Fsm> machine;
    QList<TruthTable> internalProofs;

    QList<shared_ptr<Issue>> issues;
};

#endif // FSMVERIFIER_H
