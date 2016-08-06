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

public: // Static

    enum EquationErrorEnum{
        resized_requested       = 0,
        out_of_range_access     = 1,
        set_value_requested     = 2,
        reduced_operand_while_0 = 3,
        change_operand_illegal  = 4
    };

private:
    static bool signalHasSize(shared_ptr<Signal> sig);

public:
    // This enum is always treated using a switch in order to obtain a warning
    // when adding a new member in all places it is used.
    enum class nature{notOp,  // Not equations always have exactly one operand
                      andOp,
                      orOp,
                      xorOp,
                      nandOp,
                      norOp,
                      xnorOp,
                      equalOp, // Equal equations always have exactly two operand and are size one
                      diffOp,  // Diff  equations always have exactly two operand and are size one
                      extractOp, // Extract equations always have exacly one operand
                      concatOp,
                      constant, // To allow dynamically creating constants (which are not machine signals)
                      identity // For internal use only, exactly one operand
                     };

    enum class computationFailureCause{uncomputed,
                                       nofail,
                                       nullOperand,
                                       incompleteOperand,
                                       sizeMismatch,
                                       missingParameter,
                                       incorrectParameter,
                                       notImplemented
                                      };

public:
    explicit Equation(nature function, int allowedOperandCount = -1, int param1 = -1, int param2 = -1);
    explicit Equation(nature function, const QVector<shared_ptr<Signal>>& operandList, int param1 = -1, int param2 = -1);

    shared_ptr<Equation> clone() const;

    uint getSize() const override;
    void resize(uint) override; // Throws StatesException

    QString getText() const override;
    QString getColoredText(bool activeColored, bool errorColored) const;

    void setCurrentValue(const LogicValue& value) override; // Throws StatesException
    computationFailureCause getComputationFailureCause() const;

    nature getFunction() const;
    void setFunction(const nature& newFunction, int param1 = -1, int param2 = -1);
    void setParameters(int param1, int param2 = -1); // TODO: throw exception when function is not extract? Or simply qDebug...
    bool isInverted() const;
    int getParam1() const; // TODO: throw exception when function is not extract?
    int getParam2() const; // TODO: throw exception when function is not extract?

    shared_ptr<Signal> getOperand(uint i) const; // Throws StatesException
    bool setOperand(uint i, shared_ptr<Signal> newOperand, bool quiet = false); // Throws StatesException
    void clearOperand(uint i, bool quiet = false); // Throws StatesException

    QVector<shared_ptr<Signal>> getOperands() const;

    uint getOperandCount() const;
    void increaseOperandCount(); // Throws StatesException
    void decreaseOperandCount(); // Throws StatesException

private slots:
    void computeCurrentValue();

private:
    void increaseOperandCountInternal();
    void decreaseOperandCountInternal(); // Throws StatesException

private:
    computationFailureCause failureCause = computationFailureCause::uncomputed;

    nature function;
    // Different storage for different ownership (weak/shared)
    QVector<weak_ptr<Signal>>     signalOperands;
    QVector<shared_ptr<Equation>> equationOperands;

    // This size hold the maximum operands count
    // It can be increased or decreased (min 2 operands)
    // except for constant size operators (ident, not, eq, diff, constant)
    uint allowedOperandCount = 0;

    // Parameters
    int param1;
    int param2;
};

#endif // EQUATION_H
