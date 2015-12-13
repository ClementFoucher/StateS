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

#ifndef FSMVHDLEXPORT_H
#define FSMVHDLEXPORT_H

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>
class QString;
class QTextStream;

// StateS classes
#include "machineactuatorcomponent.h"
class Fsm;
class Signal;
class FsmState;


class FsmVhdlExport
{
public:
    class ExportCompatibility
    {
    public:
        QList<shared_ptr<Signal>> bothMooreAndMealy;
        QList<shared_ptr<Signal>> bothTempAndKeepValue;
        QList<shared_ptr<Signal>> rangeAdressed;
        QList<shared_ptr<Signal>> mealyWithKeep;

        bool isCompatible()
        {
            if (bothMooreAndMealy.isEmpty()    &&
                bothTempAndKeepValue.isEmpty() &&
                rangeAdressed.isEmpty()        &&
                mealyWithKeep.isEmpty()
               )
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

private:
    struct WrittableSignalCharacteristics
    {
        bool isMoore = false;
        bool isMealy = false;
        bool isTempValue = false;
        bool isKeepValue = false;
        bool isRangeAdressed = false;
    };

public:
    FsmVhdlExport(shared_ptr<Fsm> machine);
    void setOptions(bool resetLogicPositive, bool prefixSignals);

    bool writeToFile(const QString& path);
    shared_ptr<ExportCompatibility> checkCompatibility();


private:
    void generateVhdlCharacteristics(shared_ptr<Fsm> l_machine);
    WrittableSignalCharacteristics determineWrittableSignalCharacteristics(shared_ptr<Fsm> l_machine, shared_ptr<Signal> signal, bool storeResults);
    QString generateSignalVhdlName(const QString& prefix, const QString& name) const;
    QString cleanNameForVhdl(const QString& name) const;

    void writeHeader(QTextStream& stream) const;
    void writeEntity(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
    void writeArchitecture(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
    void writeMooreOutputs(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
    void writeMealyOutputs(QTextStream& stream, shared_ptr<Fsm> l_machine) const;

    void writeAsynchronousProcessSensitivityList(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
    void writeSignalAffectationValue(QTextStream& stream, shared_ptr<FsmState> state, shared_ptr<Signal> signal, MachineActuatorComponent::action_types type) const;

    QString generateEquationText(shared_ptr<Signal> equation, shared_ptr<Fsm> l_machine) const;

private:
    weak_ptr<Fsm> machine;
    bool resetLogicPositive;
    bool prefixSignals;

    QMap<shared_ptr<Signal>, QString> signalVhdlName;
    QMap<shared_ptr<FsmState>, QString> stateVhdlName;
    QString machineVhdlName;

    // The following is used to determine how a writtable signal should be affected value.

    // For now, we only handle these cases:
    // - A signal is either Mealy or Moore, not both.
    // - A signal either keeps its value or has an active-on-state/pulse value, not both.

    QList<shared_ptr<Signal>> mooreSignals;
    QList<shared_ptr<Signal>> mealySignals; // TODO: Mealy signals are currently ignored.

    QList<shared_ptr<Signal>> tempValueSignals;
    QList<shared_ptr<Signal>> keepValueSignals;

    // At first, the signals with range adressing should be treated
    // as independant bits, each bit acting like a whole signal.
    // Then maybe determine independant ranges, not alway go @ bit level.

    // TODO: Range adresssed signals are currently ignored.
    //QList<shared_ptr<Signal>> rangeAdressedSignals;
};

#endif // FSMVHDLEXPORT_H

