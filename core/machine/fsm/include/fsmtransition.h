/*
 * Copyright © 2014-2016 Clément Foucher
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


class FsmTransition : public FsmComponent
{
    Q_OBJECT

public:
    explicit FsmTransition(shared_ptr<Fsm> parent, shared_ptr<FsmState> source, shared_ptr<FsmState> target, shared_ptr<Signal> condition, FsmGraphicTransition* graphicRepresentation = nullptr);
    ~FsmTransition();

    shared_ptr<FsmState> getTarget() const;
    void setTarget(shared_ptr<FsmState> value);

    shared_ptr<FsmState> getSource() const;
    void setSource(shared_ptr<FsmState> value);

    FsmGraphicTransition* getGraphicRepresentation() const;
    void setGraphicRepresentation(FsmGraphicTransition* representation); // TODO: throw exception
    void clearGraphicRepresentation();

    void setCrossed() const;

    shared_ptr<Signal> getCondition() const;
    void setCondition(shared_ptr<Signal> signalNewCondition);

    void clearCondition();

    virtual uint getAllowedActionTypes() const override;

    qreal sliderPos; // Public because we don't care, just used by loader

signals:
    void conditionChangedEvent();

private:
    FsmGraphicTransition* graphicRepresentation = nullptr;

    weak_ptr<FsmState> source;
    weak_ptr<FsmState> target;

    shared_ptr<Equation> condition;
};

#endif // FSMTRANSITION_H
