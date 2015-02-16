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

#ifndef FSMSTATE_H
#define FSMSTATE_H

// Parent
#include "fsmcomponent.h"

// Qt classes
#include <QList>
#include <QPointF>

// StateS classes
class Fsm;
class FsmTransition;
class FsmGraphicalState;

class FsmState : public FsmComponent
{
    Q_OBJECT

public:
    explicit FsmState(Fsm* parent);
    explicit FsmState(Fsm* parent, const QString& name);
    ~FsmState();

    void addIncomingTransition(FsmTransition* transition);
    void addOutgoingTransition(FsmTransition* transition);
    void removeIncomingTransition(FsmTransition* transition);
    void removeOutgoingTransition(FsmTransition* transition);

    const QList<FsmTransition*>& getOutgoingTransitions() const;
    const QList<FsmTransition*>& getIncomingTransitions() const;

    bool getIsActive() const;
    void setActive(bool value);

    bool isInitial() const;
    void setInitial();

    QString getName() const;
    bool setName(const QString& value);
    void setUniqueName();

    FsmGraphicalState* getGraphicalRepresentation() const;
    void setGraphicalRepresentation(FsmGraphicalState* representation);
    void clearGraphicalRepresentation();

    QPointF position; // Public because we don't care, just used by loader

private:
    QList<FsmTransition*> inputTransitions;
    QList<FsmTransition*> outputTransitions;

    FsmGraphicalState* graphicalRepresentation = nullptr;
    QString name;

    bool isActive = false;
};

#endif // FSMSTATE_H
