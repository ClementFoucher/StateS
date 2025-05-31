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
#include "rangeeditordialog.h"

// Qt classes
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QRadioButton>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "equation.h"
#include "variable.h"
#include "rangeeditor.h"


RangeEditorDialog::RangeEditorDialog(componentId_t variableId, int rangeL, int rangeR, QWidget* parent) :
	StatesDialog(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	//
	// Build object
	this->setWindowTitle(tr("Edit range"));

	this->equation = make_shared<Equation>(OperatorType_t::extractOp, 1);
	this->equation->setOperand(0, variableId);
	this->equation->setRange(rangeL, rangeR);

	//
	// Title
	if (rangeR == -1)
	{
		this->title = new QLabel("<b>" + tr("Choose extracted bit") + "</b>");
	}
	else
	{
		this->title = new QLabel("<b>" + tr("Choose extracted range") + "</b>");
	}
	this->title->setAlignment(Qt::AlignCenter);

	//
	// Equation
	auto variableName = new QLabel(variable->getName());
	variableName->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

	this->rangeExtractor = new RangeEditor(this->equation);

	auto extractorLayout = new QHBoxLayout();
	extractorLayout->addStretch();
	extractorLayout->addWidget(variableName);
	extractorLayout->addWidget(this->rangeExtractor);
	extractorLayout->addStretch();

	//
	// Extractor type
	auto extractSingleRadioButton = new QRadioButton(tr("Extract single bit"));
	auto extractRangeRadioButton  = new QRadioButton(tr("Extract range"));

	if (rangeR == -1)
	{
		extractSingleRadioButton->setChecked(true);
	}
	else
	{
		extractRangeRadioButton->setChecked(true);
	}

	connect(extractSingleRadioButton, &QRadioButton::toggled, this, &RangeEditorDialog::extractSingleBitSelectedEventHandler);
	connect(extractRangeRadioButton,  &QRadioButton::toggled, this, &RangeEditorDialog::extractRangeSelectedEventHandler);

	auto extractorTypeLayout = new QHBoxLayout();
	extractorTypeLayout->addWidget(extractSingleRadioButton);
	extractorTypeLayout->addWidget(extractRangeRadioButton);

	//
	// Buttons
	auto okButton     = new QPushButton(tr("OK"));
	auto cancelButton = new QPushButton(tr("Cancel"));

	connect(okButton,     &QPushButton::clicked, this, &QDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	//
	// Build complete rendering
	auto layout = new QVBoxLayout(this);

	layout->addWidget(title);
	layout->addLayout(extractorLayout);
	layout->addLayout(extractorTypeLayout);
	layout->addLayout(buttonsLayout);
}

int RangeEditorDialog::getRangeL() const
{
	return this->equation->getRangeL();
}

int RangeEditorDialog::getRangeR() const
{
	return this->equation->getRangeR();
}

void RangeEditorDialog::extractSingleBitSelectedEventHandler(bool checked)
{
	if (checked == true)
	{
		this->rangeExtractor->setExtractSingleBit();

		this->title->setText("<b>" + tr("Choose extracted bit") + "</b>");
	}
}

void RangeEditorDialog::extractRangeSelectedEventHandler(bool checked)
{
	if (checked == true)
	{
		this->rangeExtractor->setExtractRange();

		this->title->setText("<b>" + tr("Choose extracted range") + "</b>");
	}
}
