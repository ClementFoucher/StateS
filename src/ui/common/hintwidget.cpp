/*
 * Copyright © 2025 Clément Foucher
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
#include "hintwidget.h"

// Qt classes
#include <QLabel>


HintWidget::HintWidget(const QString& title, const QString& textContent, QWidget* parent) :
	CollapsibleWidgetWithTitle(parent)
{
	this->setContent(title, textContent);
}

HintWidget::HintWidget(QWidget* parent) :
	CollapsibleWidgetWithTitle(parent)
{
}

void HintWidget::setContent(const QString& title, const QString& textContent)
{
	QLabel* newContent = new QLabel(textContent, this);
	newContent->setAlignment(Qt::AlignCenter);
	newContent->setWordWrap(true);

	CollapsibleWidgetWithTitle::setContent(title, newContent, true);
}
