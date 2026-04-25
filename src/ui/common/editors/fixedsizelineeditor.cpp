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

// Current class header
#include "fixedsizelineeditor.h"


FixedSizeLineEditor::FixedSizeLineEditor(uint characters, QWidget* parent) :
	QLineEdit(parent)
{
	auto fm = this->fontMetrics();
	auto m  = this->textMargins();
	auto c  = this->contentsMargins();

	this->fixedWidth  = characters*fm.size(Qt::TextSingleLine, "0").width();
	this->fixedWidth += (m.left() + m.right());
	this->fixedWidth += (c.left() + c.right());
}

QSize FixedSizeLineEditor::sizeHint() const
{
	auto baseSize = QLineEdit::sizeHint();
	baseSize.setWidth(this->fixedWidth);

	return baseSize;
}
