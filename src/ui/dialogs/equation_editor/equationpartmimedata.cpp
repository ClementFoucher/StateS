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

// Current class header
#include "equationpartmimedata.h"


EquationPartMimeData::EquationPartMimeData(const QString& text, uint availableActions, shared_ptr<Equation> equation)
{
	this->text = text;
	this->availableActions = availableActions;

	this->source = OperandSource_t::equation;
	this->equation = equation;
}

EquationPartMimeData::EquationPartMimeData(const QString& text, uint availableActions, componentId_t variableId)
{
	this->text = text;
	this->availableActions = availableActions;

	this->source = OperandSource_t::variable;
	this->variableId = variableId;
}

EquationPartMimeData::EquationPartMimeData(const QString& text, uint availableActions, LogicValue constant)
{
	this->text = text;
	this->availableActions = availableActions;

	this->source = OperandSource_t::constant;
	this->constant = constant;
}

OperandSource_t EquationPartMimeData::getSource() const
{
	return this->source;
}

uint EquationPartMimeData::getAvailableActions() const
{
	return this->availableActions;
}

shared_ptr<Equation> EquationPartMimeData::getEquation() const
{
	return this->equation;
}

componentId_t EquationPartMimeData::getVariableId() const
{
	return this->variableId;
}

LogicValue EquationPartMimeData::getConstant() const
{
	return this->constant;
}

QString EquationPartMimeData::getText() const
{
	return this->text;
}
