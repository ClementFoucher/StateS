/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "nulleditorwidget.h"

// Qt classes
#include <QLabel>
#include <QHBoxLayout>


NullEditorWidget::NullEditorWidget(uint rankInParentOperands, QWidget* parent) :
	EquationPartEditorWidget(rankInParentOperands, false, parent)
{
	auto layout = new QHBoxLayout(this);
	auto text = new QLabel("...");
	text->setAlignment(Qt::AlignCenter);
	layout->addWidget(text);
}

uint NullEditorWidget::getAllowedMenuActions() const
{
	return 0;
}

uint NullEditorWidget::getAllowedDropActions() const
{
	return (uint)DropAction_t::ReplaceExisting;
}

QString NullEditorWidget::getText() const
{
	return QString("(" + tr("null operand") + ")");
}

QString NullEditorWidget::getToolTipText() const
{
	return QString("(" + tr("null operand") + ")");
}

bool NullEditorWidget::getReplaceWithoutAsking() const
{
	return true;
}

bool NullEditorWidget::getIsErroneous() const
{
	return true;
}

QDrag* NullEditorWidget::buildDrag()
{
	// Sould never be called
	return nullptr;
}

void NullEditorWidget::processSpecificMenuAction(ContextAction_t)
{
	// Nothing to do here
}

void NullEditorWidget::processSpecificDropAction(DropAction_t)
{
	// Nothing to do here
}
