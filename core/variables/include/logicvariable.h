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

#ifndef LOGICVARIABLE_H
#define LOGICVARIABLE_H

#include <QObject>

#include <QString>

class LogicVariable : public QObject
{
    Q_OBJECT

public:
    static LogicVariable* constant0;
    static LogicVariable* constant1;

public:
    explicit LogicVariable(const QString &name);
    explicit LogicVariable();
    ~LogicVariable();

    QString getName() const;
    void setName(const QString& value);

    virtual QString getText(bool colored = false) const;

    void setCurrentState(bool value, bool pulse = false);

    virtual bool getCurrentState() const;
    bool isActive() const;
    bool isInactive() const;

    bool getIsConstant() const;
    void setIsConstant(bool value);

    void updateState();

signals:
    void renamedEvent();
    void stateChangedEvent();
    void deletedEvent(LogicVariable* emitter);

private:
    explicit LogicVariable(bool active);

    bool isConstant = false;
    bool isPulse = false;
    QString name;

    bool currentState = false;
};

#endif // LOGICVARIABLE_H
