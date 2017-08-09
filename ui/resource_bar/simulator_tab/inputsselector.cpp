/*
 * Copyright © 2014-2015 Clément Foucher
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
#include "inputsselector.h"

// Qt classes
#include <QVBoxLayout>
#include <QStyle>

// StateS classes
#include "input.h"
#include "inputsignalselector.h"


InputsSelector::InputsSelector(QList<shared_ptr<Input> > inputList, QWidget *parent) :
    QScrollArea(parent)
{
	this->setWidget(new QWidget());

	QVBoxLayout* layout = new QVBoxLayout(this->widget());

	foreach (shared_ptr<Input> currentInput, inputList)
	{
		InputSignalSelector* currentSignalSelector = new InputSignalSelector(currentInput);
		layout->addWidget(currentSignalSelector);
	}
}

void InputsSelector::resizeEvent(QResizeEvent*)
{
	int width = this->width() - this->style()->pixelMetric(QStyle::PM_ScrollBarExtent) - this->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
	int height = (this->widget()->layout()->itemAt(0)->sizeHint().height() * this->widget()->layout()->count());

	this->widget()->resize(width, height);
}


