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

#ifndef EQUATIONPARTMIMEDATA_H
#define EQUATIONPARTMIMEDATA_H

// Parent
#include <QMimeData>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class Equation;


class EquationPartMimeData : public QMimeData
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit EquationPartMimeData(const QString& text, uint availableActions, shared_ptr<Equation> equation);
	explicit EquationPartMimeData(const QString& text, uint availableActions, componentId_t variableId);
	explicit EquationPartMimeData(const QString& text, uint availableActions, LogicValue constant);

	/////
	// Object functions
public:
	OperandSource_t getSource() const;
	uint getAvailableActions() const;

	shared_ptr<Equation> getEquation()   const;
	componentId_t        getVariableId() const;
	LogicValue           getConstant()   const;

	QString getText() const;

	/////
	// Object variables
private:
	OperandSource_t source;
	uint availableActions;

	shared_ptr<Equation> equation;
	componentId_t        variableId = nullId;
	LogicValue           constant   = LogicValue();

	QString text;

};

#endif // EQUATIONPARTMIMEDATA_H
