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
#include <QHash>
#include <QList>
#include <QVector>
class QString;

// StateS classes
#include "truthtable.h"
class Fsm;

// For now, use pointers to TruthTables for storage reasons
// (QHash vs shared_ptr).
// Should be reviewd a some point.

class FsmVerifier : public QObject
{
    Q_OBJECT

public:
    explicit FsmVerifier(shared_ptr<Fsm> machine);
    ~FsmVerifier();

    QList<QString> verifyFsm();
    QVector<TruthTable*> getProofs();
    QHash<TruthTable*, QList<int>> getProofsHighlight();

private:
    void clearProofs();

    weak_ptr<Fsm> machine;
    QList<TruthTable> internalProofs;

    QVector<TruthTable*> proofs;
    QHash<TruthTable*, QList<int>> proofsHighlight;
};

#endif // FSMVERIFIER_H
