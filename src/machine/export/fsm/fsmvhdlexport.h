/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "statestypes.h"
class Fsm;
class Variable;
class ActionOnVariable;


class FsmVhdlExport
{

	/////
	// Type declarations
public:
	class ExportCompatibility
	{
	public:
		QList<shared_ptr<Variable>> bothMooreAndMealy;
		QList<shared_ptr<Variable>> bothTempAndKeepValue;
		QList<shared_ptr<Variable>> rangeAdressed;
		QList<shared_ptr<Variable>> mealyWithKeep;

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
	struct WrittableVariableCharacteristics_t
	{
		bool isMoore         = false;
		bool isMealy         = false;
		bool isTempValue     = false;
		bool isKeepValue     = false;
		bool isRangeAdressed = false;
	};

	/////
	// Constructors/destructors
public:
	explicit FsmVhdlExport();

	/////
	// Object functions
public:
	void setOptions(bool resetLogicPositive, bool prefixSignals);

	bool writeToFile(const QString& path);
	shared_ptr<ExportCompatibility> checkCompatibility();

private:
	void generateVhdlCharacteristics(shared_ptr<Fsm> l_machine);
	WrittableVariableCharacteristics_t determineWrittableVariableCharacteristics(shared_ptr<Fsm> l_machine, shared_ptr<Variable> variable, bool storeResults);
	QString generateVhdlSignalName(const QString& prefix, const QString& name) const;
	QString cleanNameForVhdl(const QString& name) const;

	void writeHeader(QTextStream& stream) const;
	void writeEntity(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
	void writeArchitecture(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
	void writeMooreOutputs(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
	void writeMealyOutputs(QTextStream& stream, shared_ptr<Fsm> l_machine) const;

	void writeAsynchronousProcessSensitivityList(QTextStream& stream, shared_ptr<Fsm> l_machine) const;
	void writeSignalAffectationValue(QTextStream& stream, shared_ptr<ActionOnVariable> action) const;

	QString generateEquationText(shared_ptr<Variable> equation, shared_ptr<Fsm> l_machine) const;

	/////
	// Object variables
private:
	bool resetLogicPositive;
	bool prefixSignals;

	QMap<shared_ptr<Variable>, QString> variableVhdlName;
	QMap<componentId_t, QString> stateVhdlName;
	QString machineVhdlName;

	// The following is used to determine how a writtable variable should be affected value.

	// For now, we only handle these cases:
	// - A variable is either Mealy or Moore, not both.
	// - A variable either keeps its value or has an active-on-state/pulse value, not both.

	QList<shared_ptr<Variable>> mooreVariables;
	QList<shared_ptr<Variable>> mealyVariables; // TODO: Mealy variables are currently ignored.

	QList<shared_ptr<Variable>> tempValueVariables;
	QList<shared_ptr<Variable>> keepValueVariables;

	// At first, the variables with range adressing should be treated
	// as independant bits, each bit acting like a whole variable.
	// Then maybe determine independant ranges, not alway go @ bit level.

	// TODO: Range adresssed variables are currently ignored.
	//QList<shared_ptr<Variable>> rangeAdressedVariables;

};

#endif // FSMVHDLEXPORT_H

