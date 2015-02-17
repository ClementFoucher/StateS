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

#ifndef FSMTRANSITION_H
#define FSMTRANSITION_H

// Parent
#include "fsmcomponent.h"

// StateS classes
class Fsm;
class FsmState;
class FsmGraphicalTransition;
class Signal;

class FsmTransition : public FsmComponent
{
    Q_OBJECT

public:
    explicit FsmTransition(Fsm* parent, FsmState* source, FsmState* target, FsmGraphicalTransition* graphicalRepresentation, Signal* condition = nullptr);
    ~FsmTransition();

    FsmState* getTarget() const;
    void setTarget(FsmState* value);

    FsmState* getSource() const;
    void setSource(FsmState* value);

    FsmGraphicalTransition* getGraphicalRepresentation() const;
    void setGraphicalRepresentation(FsmGraphicalTransition* representation);
    void clearGraphicalRepresentation();

    void setCrossed() const;

    Signal* getCondition() const;
    void setCondition(Signal* signal);

public slots:
    void clearCondition();

private:
    FsmGraphicalTransition* graphicalRepresentation = nullptr;

    FsmState* source = nullptr;
    FsmState* target = nullptr;

    Signal* condition = nullptr;
};

#endif // FSMTRANSITION_H
