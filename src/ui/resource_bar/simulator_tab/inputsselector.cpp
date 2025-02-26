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
#include "inputsselector.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QScrollArea>
#include <QVBoxLayout>
#include <QStyle>
#include <QLabel>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "inputvariableselector.h"


InputsSelector::InputsSelector(QWidget* parent) :
	QWidget(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	auto mainLayout = new QVBoxLayout();
	this->setLayout(mainLayout);
	mainLayout->setAlignment(Qt::AlignTop);

	auto inputList = machine->getInputs();
	if (inputList.count() != 0)
	{
		auto inputListHint = new QLabel(tr("Click on bits from the list below to switch input value:"));
		inputListHint->setAlignment(Qt::AlignCenter);
		inputListHint->setWordWrap(true);
		mainLayout->addWidget(inputListHint);

		this->scrollArea = new QScrollArea();
		mainLayout->addWidget(this->scrollArea);

		this->scrollAreaWidget = new QWidget();
		this->scrollAreaWidgetLayout = new QVBoxLayout(this->scrollAreaWidget);
		this->scrollArea->setWidget(this->scrollAreaWidget);

		for (auto& currentInput : inputList)
		{
			auto currentVariableSelector = new InputVariableSelector(currentInput, this);
			this->scrollAreaWidgetLayout->addWidget(currentVariableSelector);
		}
	}
	else
	{
		auto noInputHint = new QLabel("<i>" + tr("No input available on the machine") +"</i>");
		noInputHint->setAlignment(Qt::AlignCenter);
		noInputHint->setWordWrap(true);
		mainLayout->addWidget(noInputHint);
	}
}

void InputsSelector::resizeEvent(QResizeEvent*)
{
	if (this->scrollAreaWidget != nullptr)
	{
		int width  = this->scrollArea->width() - this->scrollArea->style()->pixelMetric(QStyle::PM_ScrollBarExtent) - this->scrollArea->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent);
		int height = this->scrollAreaWidgetLayout->itemAt(0)->sizeHint().height() * this->scrollAreaWidgetLayout->count();

		this->scrollAreaWidget->resize(width, height);
	}
}
