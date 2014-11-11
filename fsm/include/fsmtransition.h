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

#ifndef FSMTRANSITION_H
#define FSMTRANSITION_H

#include <QObject>

class Fsm;
class FsmState;
class FsmGraphicalTransition;
class LogicVariable;

class FsmTransition : public QObject
{
    Q_OBJECT

public:
    explicit FsmTransition(Fsm* parent, FsmState* source, FsmState* target, FsmGraphicalTransition* graphicalRepresentation, LogicVariable* condition = nullptr);
    ~FsmTransition();

    FsmState* getTarget() const;
    void setTarget(FsmState* value);

    FsmState* getSource() const;
    void setSource(FsmState* value);

    FsmGraphicalTransition* getGraphicalRepresentation() const;
    void setGraphicalRepresentation(FsmGraphicalTransition* representation);
    void clearGraphicalRepresentation();

    Fsm* getOwningMachine() const;

    void setCrossed() const;

    LogicVariable* getCondition() const;
    void setCondition(LogicVariable* variable);

    QList<LogicVariable*> getActions() const;
    void addAction(const QString& variableName);
    void addAction(LogicVariable* variable);
    void removeAction(const QString& variableName);
    void clearActions();
    void setActions(const QList<LogicVariable*>* newActions);

public slots:
    void clearCondition();
    void removeAction(LogicVariable* variable);

signals:
    void transitionConfigurationChanged();

private:
    Fsm* owningMachine = nullptr;
    FsmGraphicalTransition* graphicalRepresentation = nullptr;

    FsmState* source = nullptr;
    FsmState* target = nullptr;

    LogicVariable* condition = nullptr;
    QList<LogicVariable*> actions;
};

#endif // FSMTRANSITION_H
