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

#ifndef EQUATION_H
#define EQUATION_H

// Parent
#include "signal.h"

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
 * An equation can store as operand:
 * - nullptr => operand is not set
 * (- system constants)
 * - machine signals
 * - equations with a size
 * - equations with no size
 *
 * Equation size in bits is also dynamic and depends on oprands size,
 * except for equality and difference operators, which size is always 1.
 *
 * An equation with all its operands set to nullptr will have no size,
 * and always returns a null logic value.
 *
 * An equation with only (system constants,) usized equations and nullptr operands
 * will have no size neither(, but its value will be computed and return a size 1
 * result. Its null oprands will be ignored in value computation.)
 *
 * An equation containing at least one signal defined in a machine, or one equation
 * with a size, have a fixed size, and does not accept operands with a different size.
 * (Operands with no size and system constants can still be used as operands.)
 *
 * (Equations with no size value should be interpreted as a system constant, and resized
 * before use if necessary.)
 *
 * Equation store copies of all dynamic elements (equations (and system constants)).
 * References to machine signals are not copied.
 * At operand assignment, we make copy of needed elements.
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
                      diffOp   // Diff  equations always have exactly two operand and are size one
                     };

public:
    explicit Equation(nature function, const QVector<Signal*>& operandList);
    explicit Equation(nature function, uint allowedOperandCount);
    ~Equation();

    Equation* clone() const; //override;

    uint getSize() const override;
    bool resize(uint) override;

    QString getText(bool colored = false) const override;

    LogicValue getCurrentValue() const override;

    nature getFunction() const;
    void setFunction(const nature& newFunction);
    bool isInverted() const;

    Signal* getOperand(uint i) const;
    bool setOperand(uint i, Signal* newOperand);
    void clearOperand(uint i);

    const QVector<Signal*>& getOperands() const;

    uint getOperandCount() const;
    bool increaseOperandCount(bool force = false);
    bool decreaseOperandCount(bool force = false);

private slots:
    void operandDeletedEvent(Signal* var);
    void operandResizedEvent();

private:
    bool signalHasSize(Signal* sig);

    nature function;
    QVector<Signal*> operands;

    // This size hold the maximum operands count
    // It can be increased or decreased (min 2 operands)
    // except for constant size operators (not, eq, diff)
    uint allowedOperandCount = 0;

    // This holds the number of operands that are not nullptr
    uint actualOperandCount = 0;

    // This holds the number of operands with a size
    uint sizedOperandCount = 0;

    // Current size is dynamically computed size based on operands size
    // If operands contain at least 1 sized signal, then equation size is set
    uint currentSignalSize = 0;
};

#endif // EQUATION_H
