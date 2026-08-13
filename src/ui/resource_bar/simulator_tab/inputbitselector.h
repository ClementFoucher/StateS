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

#ifndef INPUTBITSELECTOR_H
#define INPUTBITSELECTOR_H

// Parent
#include "inputtoggleselector.h"


class InputBitSelector : public InputToggleSelector
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit InputBitSelector(ComponentId variableId, uint bitNumber, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void mousePressEvent(QMouseEvent* event) override;

private slots:
	virtual void variableValueChangedEventHandler() override;

	/////
	// Object variables
private:
	uint bitNumber = 0;

};

#endif // INPUTBITSELECTOR_H
