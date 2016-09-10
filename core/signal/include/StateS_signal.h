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

#ifndef SIGNAL_H
#define SIGNAL_H

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// StateS basic types
#include "logicvalue.h"


class Signal : public QObject, public enable_shared_from_this<Signal>
{
    Q_OBJECT

public: // Static

    enum SignalErrorEnum{
        building_zero_sized = 0,
        resized_to_0        = 1,
        size_mismatch       = 2,
        signal_is_not_bool  = 3,
        value_is_read_only  = 4
    };

public:
    explicit Signal(const QString& name, uint size); // Throws StatesException
    explicit Signal(const QString& name);

    ~Signal();

    QString getName() const;
    void setName(const QString& value); // TODO: check signal name here and throw StatesException

    virtual uint getSize() const;
    virtual void resize(uint newSize); // Throws StatesException

    LogicValue getInitialValue() const;
    virtual void setInitialValue(const LogicValue& newInitialValue); // Throws StatesException
    void reinitialize();

    virtual QString getText() const;
    QString getColoredText(bool activeColored) const;

    virtual void setCurrentValue(const LogicValue& value); // Throws StatesException
    virtual void setCurrentValueSubRange(const LogicValue& value, int rangeL, int rangeR); // Throws StatesException
    LogicValue getCurrentValue() const;

    void resetValue();
    void set();

    // Concepts of true and false are only applicable to size 1 signals
    // A signal with size > 1 will neither be true nor false
    bool isTrue()  const; // Throws StatesException
    bool isFalse() const; // Throws StatesException

signals:
    // General events
    void signalStaticConfigurationChangedEvent(); // Triggered when object "savable" values are modified
    void signalDynamicStateChangedEvent();        // Triggered when object "discardable" values are modified

    // Specific events detail
    void signalRenamedEvent();
    void signalResizedEvent();
    void SignalInitialValueChangedEvent();

    // Deletion event
    void signalDeletedEvent();

private:
    // Static
    QString name;
    LogicValue initialValue;

protected:
    // Dynamic
    LogicValue currentValue;
};

#endif // SIGNAL_H
