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

#ifndef SIGNAL_H
#define SIGNAL_H

// Parent
#include <QObject>

// Qt classes
#include <QString>

// StateS basic types
#include "logicvalue.h"

class Signal : public QObject
{
    Q_OBJECT

public:
    explicit Signal(const QString& name, const LogicValue& initialValue, bool isConstant = false);
    explicit Signal(const QString& name, uint bitCount);

    // Convenience for size 1 signals
    explicit Signal(const QString& name, bool initialValue = false, bool isConstant = false);

    ~Signal();

    QString getName() const;
    void setName(const QString& value);

    virtual uint getSize() const;
    virtual bool resize(uint newSize);

    LogicValue getInitialValue() const;
    bool setInitialValue(const LogicValue& newInitialValue);
    void reinitialize();

    virtual QString getText(bool colored = false) const;

    bool setCurrentValue(const LogicValue& value);
    virtual LogicValue getCurrentValue() const;

    bool resetValue();

    // Convenience function for one bit signals
    bool set();

    bool isAllZeros() const;
    bool isAllOnes() const;

    // Concepts of true and false are only applicable to size 1 signals
    // A signal with size > 1 will be neither true nor false
    bool isTrue() const;
    bool isFalse() const;

    bool getIsConstant() const;

signals:
    void signalConfigurationChangedEvent();
    void signalResizedEvent();
    void signalStateChangedEvent();
    void signalDeletedEvent(Signal* emitter);

private:
    // Static
    QString name;

    bool isConstant = false;
    LogicValue initialValue;

    // Dynamic
    LogicValue currentValue;
};

#endif // SIGNAL_H
