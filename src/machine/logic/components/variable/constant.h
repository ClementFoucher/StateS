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

#ifndef CONSTANT_H
#define CONSTANT_H

// Parent
#include "variable.h"


class Constant : public Variable
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit Constant(const QString& name);
	explicit Constant(const QString& name, uint size); // Throws StatesException

	/////
	// Object functions
public:

	virtual void setInitialValue(const LogicValue& newInitialValue)   override; // Throws StatesException
	virtual void setCurrentValue(const LogicValue&)                   override; // Throws StatesException
	virtual void setCurrentValueSubRange(const LogicValue&, int, int) override; // Throws StatesException

};

#endif // CONSTANT_H
