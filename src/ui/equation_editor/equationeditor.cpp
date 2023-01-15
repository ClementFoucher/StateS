/*
 * Copyright © 2014-2023 Clément Foucher
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
#include "equationeditor.h"

// Qt classes
#include <QLabel>
#include <QIcon>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QKeyEvent>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "equation.h"
#include "graphicequation.h"
#include "input.h"
#include "pixmapgenerator.h"


EquationEditor::EquationEditor(shared_ptr<Signal> initialEquation, QWidget* parent) :
    QDialog(parent)
{
	this->setWindowIcon(QIcon(PixmapGenerator::getStatesWindowIcon()));
	this->setWindowTitle(tr("StateS equation editor"));

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	//
	// Title
	QLabel* title = new QLabel("<b>" + tr("Equation editor") + "</b>");
	title->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(title);

	//
	// Resources
	QLabel* resourcesTitle = new QLabel("<i>" + tr("Drag and drop equation components from here") + "…</i>");
	resourcesTitle->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(resourcesTitle);

	// Signals
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	QWidget* resourcesWidget = new QWidget();
	QHBoxLayout* resourcesLayout = new QHBoxLayout(resourcesWidget);

	if (machine->getInputs().count() != 0)
	{
		QVBoxLayout* inputListLayout = new QVBoxLayout();
		inputListLayout->setAlignment(Qt::AlignTop);
		resourcesLayout->addLayout(inputListLayout);

		QLabel* inputsTitle = new QLabel("<b>" + tr("Inputs") + "</b>");
		inputsTitle->setAlignment(Qt::AlignCenter);
		inputListLayout->addWidget(inputsTitle);

		foreach (shared_ptr<Signal> input, machine->getInputs())
		{
			inputListLayout->addWidget(new GraphicEquation(input, true));
		}
	}

	// Variables
	if (machine->getLocalVariables().count() != 0)
	{
		QVBoxLayout* variableListLayout = new QVBoxLayout();
		variableListLayout->setAlignment(Qt::AlignTop);
		resourcesLayout->addLayout(variableListLayout);

		QLabel* variablesTitle = new QLabel("<b>" + tr("Variables")+ "</b>");
		variablesTitle->setAlignment(Qt::AlignCenter);
		variableListLayout->addWidget(variablesTitle);

		foreach (shared_ptr<Signal> variable, machine->getLocalVariables())
		{
			variableListLayout->addWidget(new GraphicEquation(variable, true));
		}
	}

	// Constants
	QVBoxLayout* constantListLayout = new QVBoxLayout();
	constantListLayout->setAlignment(Qt::AlignTop);
	resourcesLayout->addLayout(constantListLayout);

	QLabel* constantsTitle = new QLabel("<b>" + tr("Constants")+ "</b>");
	constantsTitle->setAlignment(Qt::AlignCenter);
	constantListLayout->addWidget(constantsTitle);

	constantListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::constant)), true));

	foreach (shared_ptr<Signal> constant, machine->getConstants())
	{
		constantListLayout->addWidget(new GraphicEquation(constant, true));
	}

	// Operators
	QGridLayout* operatorListLayout = new QGridLayout();
	operatorListLayout->setAlignment(Qt::AlignTop);
	resourcesLayout->addLayout(operatorListLayout);

	QLabel* operatorsTitle = new QLabel("<b>" + tr("Logic functions") + "</b>");
	operatorsTitle->setAlignment(Qt::AlignCenter);
	operatorListLayout->addWidget(operatorsTitle, 0, 0, 1, 2);

	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::notOp     )), true), 1, 0, 1, 2);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::concatOp  )), true), 2, 0, 1, 2);

	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::equalOp   )), true), 3, 0, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::diffOp    )), true), 3, 1, 1, 1);

	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::andOp,   2)), true), 4, 0, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::orOp,    2)), true), 5, 0, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::xorOp,   2)), true), 6, 0, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::nandOp,  2)), true), 7, 0, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::norOp,   2)), true), 8, 0, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::xnorOp,  2)), true), 9, 0, 1, 1);

	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::andOp,   3)), true), 4, 1, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::orOp,    3)), true), 5, 1, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::xorOp,   3)), true), 6, 1, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::nandOp,  3)), true), 7, 1, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::norOp,   3)), true), 8, 1, 1, 1);
	operatorListLayout->addWidget(new GraphicEquation(shared_ptr<Equation>(new Equation(EquationNature_t::xnorOp,  3)), true), 9, 1, 1, 1);

	// Add resources in a scroll area
	QScrollArea* scrollArea = new QScrollArea();
	scrollArea->setAlignment(Qt::AlignHCenter);
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setWidget(resourcesWidget);
	mainLayout->addWidget(scrollArea);


	//
	// Equation
	QLabel* equationTitle = new QLabel("<i>… " + tr("to here.") + "</i>");
	equationTitle->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(equationTitle);

	if (initialEquation != nullptr)
	{
		shared_ptr<Equation> newEquation = dynamic_pointer_cast<Equation>(initialEquation);
		if (newEquation != nullptr)
			this->equationDisplay = new GraphicEquation(newEquation->clone(), false);
		else
			this->equationDisplay = new GraphicEquation(initialEquation, false);
	}
	else
	{
		this->equationDisplay = new GraphicEquation(nullptr, false);
	}

	mainLayout->addWidget(this->equationDisplay, 0, Qt::AlignHCenter);

	QLabel* equationInfo= new QLabel(tr("You can also use right-click on equation members to edit"));
	equationInfo->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(equationInfo);

	//
	// Buttons
	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	mainLayout->addLayout(buttonsLayout);

	QPushButton* buttonOK = new QPushButton(tr("OK"));
	buttonsLayout->addWidget(buttonOK);
	connect(buttonOK, &QAbstractButton::clicked, this, &EquationEditor::accept);

	QPushButton* buttonCancel = new QPushButton(tr("Cancel"));
	buttonsLayout->addWidget(buttonCancel);
	connect(buttonCancel, &QAbstractButton::clicked, this, &EquationEditor::reject);
}

shared_ptr<Signal> EquationEditor::getResultEquation() const
{
	return this->equationDisplay->getLogicEquation();
}

void EquationEditor::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key_Return)
	{
		if (this->equationDisplay->validEdit() == true)
		{
			transmitEvent = false;
		}
	}
	else if (event->key() == Qt::Key_Escape)
	{
		if (this->equationDisplay->cancelEdit() == true)
		{
			transmitEvent = false;
		}
	}

	if (transmitEvent)
	{
		QDialog::keyPressEvent(event);
	}
}

void EquationEditor::mousePressEvent(QMouseEvent* event)
{
	this->equationDisplay->validEdit() ;
	QDialog::mousePressEvent(event);
}
