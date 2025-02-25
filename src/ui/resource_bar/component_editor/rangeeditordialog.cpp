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
#include <QVBoxLayout>
#include <QPushButton>

// StateS classes
#include "equation.h"
#include "graphicequation.h"
#include "actiononvariable.h"


RangeEditorDialog::RangeEditorDialog(shared_ptr<ActionOnVariable> action, QWidget* parent) :
	QDialog(parent)
{
	int rangeL = action->getActionRangeL();
	int rangeR = action->getActionRangeR();

	this->setWindowTitle(tr("Edit range"));

	QVBoxLayout* layout = new QVBoxLayout(this);

	QLabel* title;

	if (rangeR == -1)
	{
		title = new QLabel("<b>" + tr("Choose extracted bit") + "</b>", this);
	}
	else
	{
		title = new QLabel("<b>" + tr("Choose range") + "</b>", this);
	}
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);

	this->equation = shared_ptr<Equation>(new Equation(OperatorType_t::extractOp, 1));
	this->equation->setRange(rangeL, rangeR);
	this->equation->setOperand(0, action->getVariableActedOn());

	GraphicEquation* graphicEquation = new GraphicEquation(this->equation, -1, false, this);
	graphicEquation->setVariableLock(true);
	layout->addWidget(graphicEquation);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	layout->addLayout(buttonsLayout);

	QPushButton* buttonOK = new QPushButton(tr("OK"), this);
	connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
	buttonsLayout->addWidget(buttonOK);

	QPushButton* buttonCancel = new QPushButton(tr("Cancel"), this);
	connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
	buttonsLayout->addWidget(buttonCancel);
}

int RangeEditorDialog::getRangeL() const
{
	return this->equation->getRangeL();
}

int RangeEditorDialog::getRangeR() const
{
	return this->equation->getRangeR();
}
