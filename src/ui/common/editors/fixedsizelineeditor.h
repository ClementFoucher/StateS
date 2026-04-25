/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FIXEDSIZELINEEDITOR_H
#define FIXEDSIZELINEEDITOR_H

// Parent
#include <QLineEdit>


class FixedSizeLineEditor : public QLineEdit
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit FixedSizeLineEditor(uint characters, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	virtual QSize sizeHint() const override;

	/////
	// Object variables
private:
	uint fixedWidth = 0;

};

#endif // FIXEDSIZELINEEDITOR_H
