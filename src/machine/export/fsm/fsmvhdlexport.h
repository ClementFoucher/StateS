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

// Parent
#include <QObject>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QMap>
class QString;
class QTextStream;

// StateS classes
#include "statestypes.h"
class ActionOnVariable;
class Equation;
class Operand;


class FsmVhdlExport : public QObject
{
	Q_OBJECT

	/////
	// Type declarations
public:
	class ExportCompatibility
	{
	public:
		QList<componentId_t> bothMooreAndMealy;
		QList<componentId_t> bothTempAndKeepValue;
		QList<componentId_t> rangeAdressed;
		QList<componentId_t> mealyWithKeep;

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
	void generateVhdlCharacteristics();
	WrittableVariableCharacteristics_t determineWrittableVariableCharacteristics(componentId_t variableId, bool storeResults);
	QString generateVhdlSignalName(const QString& prefix, const QString& name) const;
	QString cleanNameForVhdl(const QString& name) const;

	void writeHeader(QTextStream& stream) const;
	void writeEntity(QTextStream& stream) const;
	void writeArchitecture(QTextStream& stream) const;
	void writeMooreOutputs(QTextStream& stream) const;
	void writeMealyOutputs(QTextStream& stream) const;

	void writeAsynchronousProcessSensitivityList(QTextStream& stream) const;
	void writeSignalAffectationValue(QTextStream& stream, shared_ptr<ActionOnVariable> action) const;

	QString generateEquationText(shared_ptr<Equation> equation) const;
	QString generateOperandText(shared_ptr<Operand> operand) const;

	/////
	// Object variables
private:
	bool resetLogicPositive;
	bool prefixSignals;

	QMap<componentId_t, QString> variableVhdlName;
	QMap<componentId_t, QString> stateVhdlName;
	QString machineVhdlName;

	// The following is used to determine how a writtable variable should be affected value.

	// For now, we only handle these cases:
	// - A variable is either Mealy or Moore, not both.
	// - A variable either keeps its value or has an active-on-state/pulse value, not both.

	QList<componentId_t> mooreVariables;
	QList<componentId_t> mealyVariables; // TODO: Mealy variables are currently ignored.

	QList<componentId_t> tempValueVariables;
	QList<componentId_t> keepValueVariables;

	// At first, the variables with range adressing should be treated
	// as independant bits, each bit acting like a whole variable.
	// Then maybe determine independant ranges, not alway go @ bit level.

	// TODO: Range adresssed variables are currently ignored.
	//QList<componentId_t> rangeAdressedVariables;

};

#endif // FSMVHDLEXPORT_H

