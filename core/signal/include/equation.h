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

#ifndef EQUATION_H
#define EQUATION_H

// Parent
#include "signal.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QVector>

// StateS classes
class Operator;


/**
 * @brief
 * An equation is a gathering of signals and other equations, linked by an operator.
 * Equations are thus "compouned signals", which value is dynamic and depends
 * on the values of operands at each moment.
 *
 * Equation size in bits is also dynamic and depends on operands size,
 * except for equality and difference operators, which size is always 1.
 *
 * An equation can store as operand:
 * - nullptr => operand is not set
 * - machine signals
 * - equations with a size
 * - equations with no size
 *
 * An equation with any of its operands set to nullptr or to unsized equations
 * will have no visible size, and always returns a null logic value.
 * An equation with operands of different sizes will behave the same.
 *
 * An equation store its own copy (cloned at initialization) of Equation operands => shared_ptr.
 * Machine Signals are primary held by the machine => weak_ptr.
 *
 */

class Equation : public Signal
{
    Q_OBJECT

public:
    enum class nature{notOp,  // Not equations always have exactly one operand
                      andOp,
                      orOp,
                      xorOp,
                      nandOp,
                      norOp,
                      xnorOp,
                      equalOp, // Equal equations always have exactly two operand and are size one
                      diffOp,  // Diff  equations always have exactly two operand and are size one
                      identity // For internal use only, exactly one operand
                     };

    enum class computationFailureCause{uncomputed,
                                       nofail,
                                       nullOperand,
                                       incompleteOperand,
                                       sizeMismatch
                                      };

public:
    explicit Equation(nature function, uint allowedOperandCount);
    explicit Equation(nature function, const QVector<shared_ptr<Signal>>& operandList);

    shared_ptr<Equation> clone() const;

    uint getSize() const override;
    bool resize(uint) override;

    QString getText(bool colored = false) const override;

    LogicValue getCurrentValue() const override;
    computationFailureCause getComputationFailureCause() const;

    nature getFunction() const;
    void setFunction(const nature& newFunction);
    bool isInverted() const;

    shared_ptr<Signal> getOperand(uint i) const;
    bool setOperand(uint i, shared_ptr<Signal> newOperand, bool quiet = false);
    void clearOperand(uint i, bool quiet = false);

    QVector<shared_ptr<Signal>> getOperands() const;

    uint getOperandCount() const;
    bool increaseOperandCount(bool quiet = false);
    bool decreaseOperandCount(bool quiet = false);

signals:
    void equationOperandChangedEvent();
    void equationOperandCountChangedEvent();
    void equationFunctionChangedEvent();

private slots:
    void computeCurrentValue();

private:
    bool signalHasSize(shared_ptr<Signal> sig) const;

    // Current value is stored instead of dynamically computed
    // to avoid emit change events if value acually didn't changed
    LogicValue currentValue = LogicValue::getNullValue();
    computationFailureCause failureCause = computationFailureCause::uncomputed;

    nature function;
    // Different storage for different ownership
    QVector<weak_ptr<Signal>> signalOperands;
    QVector<shared_ptr<Equation>> equationOperands;

    // This size hold the maximum operands count
    // It can be increased or decreased (min 2 operands)
    // except for constant size operators (ident, not, eq, diff)
    uint allowedOperandCount = 0;
};

#endif // EQUATION_H
