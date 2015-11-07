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

#ifndef FSMSTATE_H
#define FSMSTATE_H

// Parent
#include "fsmcomponent.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QList>
#include <QPointF>

// StateS classes
class Fsm;
class FsmTransition;
class FsmGraphicState;


class FsmState : public FsmComponent
{
    Q_OBJECT

public:
    explicit FsmState(shared_ptr<Fsm> parent, const QString& name);
    ~FsmState();

    // An FsmState owns its outgoing transitions
    void addOutgoingTransition(shared_ptr<FsmTransition> transition);
    void removeOutgoingTransition(shared_ptr<FsmTransition> transition);
    const QList<shared_ptr<FsmTransition>> getOutgoingTransitions() const;

    // An FsmState only weak-references incoming transitions
    void addIncomingTransition(shared_ptr<FsmTransition> transition);
    void removeIncomingTransition(shared_ptr<FsmTransition> transition);
    const QList<weak_ptr<FsmTransition>> getIncomingTransitions();

    bool getIsActive() const;
    void setActive(bool value);

    bool isInitial() const;
    void setInitial();

    QString getName() const;
    void setName(const QString& value);

    FsmGraphicState* getGraphicRepresentation() const;
    void setGraphicRepresentation(FsmGraphicState* representation);
    void clearGraphicRepresentation();

    QPointF position; // Public because we don't care, just used by loader

signals:
    void stateRenamedEvent();
    void stateLogicStateChangedEvent();
    void stateGraphicRepresentationMoved(); // Separate this event to not trigger everything on state move

private:
    void cleanIncomingTransitionsList();

    QList<weak_ptr<FsmTransition>> inputTransitions;
    QList<shared_ptr<FsmTransition>> outputTransitions;

    FsmGraphicState* graphicRepresentation = nullptr;
    QString name;

    bool isActive = false;
};

#endif // FSMSTATE_H
