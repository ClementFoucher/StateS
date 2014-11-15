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

#ifndef LOGICEQUATION_H
#define LOGICEQUATION_H

#include <QMap>

#include "logicvariable.h"

class Operator;

class LogicEquation : public LogicVariable
{
    Q_OBJECT

public:
    enum class nature{notOp, andOp, orOp, xorOp, nandOp, norOp, xnorOp };

public:
    explicit LogicEquation(uint size, nature function, LogicVariable* operand1 = nullptr, LogicVariable* operand2 = nullptr);
    explicit LogicEquation(nature function, const QMap<int, LogicVariable *> &operandList);
    ~LogicEquation();

    nature getFunction() const;
    void setFunction(const nature& value);

    LogicVariable* getOperand(uint i) const;
    void setOperand(uint i, LogicVariable* newOperand);

    void increaseOperandCount();
    void decreaseOperandCount();

    QString getText() const override;

    bool getCurrentState() const override;
    bool isInverted() const;

    LogicEquation* clone() const;

    uint getSize() const;

    const QMap<int, LogicVariable *>& getOperands() const;

private slots:
    void childDeletedEvent(LogicVariable* var);

private:
    nature function;
    uint size = 0;

    QMap<int, LogicVariable*> operands;
};

#endif // LOGICEQUATION_H
