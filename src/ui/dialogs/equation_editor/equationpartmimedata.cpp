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

// Current class header
#include "equationpartmimedata.h"


QString EquationPartMimeData::getText() const
{
	return this->text;
}

uint EquationPartMimeData::getAvailableActions() const
{
	return this->availableActions;
}

EquationPartMimeData::ContentType_t EquationPartMimeData::getContentType() const
{
	return this->contentType;
}

std::shared_ptr<Equation> EquationPartMimeData::getEquation() const
{
	if (this->contentType != ContentType_t::equation) return nullptr;


	return std::get<std::shared_ptr<Equation>>(this->content);
}

ComponentId EquationPartMimeData::getVariableId() const
{
	if (this->contentType != ContentType_t::variable) return nullId;


	return std::get<ComponentId>(this->content);
}

MachineValue EquationPartMimeData::getConstant() const
{
	if (this->contentType != ContentType_t::constant) return MachineValue{};


	return std::get<MachineValue>(this->content);
}
