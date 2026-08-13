/*
 * Copyright © 2014-2026 Clément Foucher
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

// Stdlib
#include <memory>
#include <variant>
using namespace std;

// StateS
#include "statestypes.h"
#include "machinevalue.h"
class Equation;


class EquationPartMimeData : public QMimeData
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class ContentType_t
	{
		variable,
		equation,
		constant
	};

	/////
	// Constructors/destructors
public:
	explicit EquationPartMimeData(const QString& text, uint availableActions, shared_ptr<Equation> equation) :
		text{text},
		availableActions{availableActions},
		contentType{ContentType_t::equation},
		content{equation}
	{}
	explicit EquationPartMimeData(const QString& text, uint availableActions, componentId_t variableId) :
		text{text},
		availableActions{availableActions},
		contentType{ContentType_t::variable},
		content{variableId}
	{}
	explicit EquationPartMimeData(const QString& text, uint availableActions, MachineValue constant) :
		text{text},
		availableActions{availableActions},
		contentType{ContentType_t::constant},
		content{constant}
	{}

	/////
	// Object functions
public:
	QString getText() const;
	uint getAvailableActions() const;

	ContentType_t getContentType() const;

	shared_ptr<Equation> getEquation()   const;
	componentId_t        getVariableId() const;
	MachineValue         getConstant()   const;

	/////
	// Object variables
private:
	QString text;
	uint availableActions;

	ContentType_t contentType;
	std::variant<componentId_t, shared_ptr<Equation>, MachineValue> content;

};

#endif // EQUATIONPARTMIMEDATA_H
