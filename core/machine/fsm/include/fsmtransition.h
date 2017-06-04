/*
 * Copyright © 2014-2017 Clément Foucher
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

#ifndef FSMTRANSITION_H
#define FSMTRANSITION_H

// Parent
#include "fsmcomponent.h"

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class FsmState;
class FsmGraphicTransition;
class Equation;


class FsmTransition : public FsmComponent, public enable_shared_from_this<FsmTransition>
{
    Q_OBJECT

public:
    explicit FsmTransition(shared_ptr<Fsm> parent, shared_ptr<FsmState> source, shared_ptr<FsmState> target, FsmGraphicTransition* representation = nullptr);
    ~FsmTransition();

    void setTarget(shared_ptr<FsmState> value);
    shared_ptr<FsmState> getTarget() const;

    void setSource(shared_ptr<FsmState> value);
    shared_ptr<FsmState> getSource() const;

    void setCondition(shared_ptr<Signal> signalNewCondition);
    shared_ptr<Signal> getCondition() const;
    void clearCondition();

    FsmGraphicTransition* getGraphicRepresentation();

    virtual uint getAllowedActionTypes() const override;

signals:
    void conditionChangedEvent();
    void transitionSliderPositionChangedEvent();

private slots:
    void graphicRepresentationDeletedEventHandler();

private:
    FsmGraphicTransition* graphicRepresentation = nullptr;

    weak_ptr<FsmState> source;
    weak_ptr<FsmState> target;

    shared_ptr<Equation> condition;
};

#endif // FSMTRANSITION_H
