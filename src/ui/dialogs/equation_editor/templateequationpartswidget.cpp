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
#include "templateequationpartswidget.h"

// Qt classes
#include <QLabel>
#include <QBoxLayout>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "equation.h"
#include "variable.h"
#include "constanteditorwidget.h"
#include "equationeditorwidget.h"
#include "variableeditorwidget.h"


TemplateEquationPartsWidget::TemplateEquationPartsWidget(QWidget* parent) :
	QWidget(parent)
{
	// Build layout
	auto templatesLayout = new QHBoxLayout(this);

	// Add vertical spacer on the left
	templatesLayout->addStretch(1);

	// Add inputs
	auto inputs = this->getInputs();
	if (inputs != nullptr)
	{
		templatesLayout->addWidget(inputs);
	}

	// Add internal variables
	auto internalVariables = this->getInternalVariables();
	if (internalVariables != nullptr)
	{
		templatesLayout->addWidget(internalVariables);
	}

	// Add constants
	auto constants = this->getConstants();
	if (constants != nullptr)
	{
		templatesLayout->addWidget(constants);
	}

	// Add operators
	auto operators = this->getOperators();
	if (operators != nullptr)
	{
		templatesLayout->addWidget(operators);
	}

	// Add vertical spacer on the right
	templatesLayout->addStretch(1);
}

QWidget* TemplateEquationPartsWidget::getInputs() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto inputIds = machine->getInputVariablesIds();
	if (inputIds.isEmpty() == true) return nullptr;


	// Build widget
	auto inputsWidget = new QWidget();
	auto inputsLayout = new QVBoxLayout(inputsWidget);
	inputsLayout->setAlignment(Qt::AlignTop);

	// Title
	auto inputsTitle = new QLabel("<b>" + tr("Inputs") + "</b>");
	inputsLayout->addWidget(inputsTitle, 0, Qt::AlignHCenter);

	// Machine inputs
	this->buildVariableList(inputsLayout, inputIds);

	return inputsWidget;
}

QWidget* TemplateEquationPartsWidget::getInternalVariables() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto internalVariableIds = machine->getInternalVariablesIds();
	if (internalVariableIds.isEmpty() == true) return nullptr;


	// Build widget
	auto internalVariablesWidget = new QWidget();
	auto internalVariablesLayout = new QVBoxLayout(internalVariablesWidget);
	internalVariablesLayout->setAlignment(Qt::AlignTop);

	// Title
	auto internalVariablesTitle = new QLabel("<b>" + tr("Variables")+ "</b>");
	internalVariablesLayout->addWidget(internalVariablesTitle, 0, Qt::AlignHCenter);

	// Machine internal variables
	this->buildVariableList(internalVariablesLayout, internalVariableIds);

	return internalVariablesWidget;
}

QWidget* TemplateEquationPartsWidget::getConstants() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;


	// Build widget
	auto constantsWidget = new QWidget();
	auto constantsLayout = new QVBoxLayout(constantsWidget);
	constantsLayout->setAlignment(Qt::AlignTop);

	// Title
	auto constantsTitle = new QLabel("<b>" + tr("Constants")+ "</b>");
	constantsLayout->addWidget(constantsTitle, 0, Qt::AlignHCenter);

	// Custom constant for bit vectors
	auto bitVectorConstant = new ConstantEditorWidget(LogicValue::getValue0(1), 0, true, constantsWidget);
	constantsLayout->addWidget(bitVectorConstant);

	// Machine constants (which are actually variables)
	this->buildVariableList(constantsLayout, machine->getConstantsIds());

	return constantsWidget;
}

QWidget* TemplateEquationPartsWidget::getOperators() const
{
	// Build widget
	auto operatorsWidget = new QWidget();
	auto operatorsLayout = new QGridLayout(operatorsWidget);
	operatorsLayout->setAlignment(Qt::AlignTop);

	// Title
	auto operatorsTitle = new QLabel("<b>" + tr("Logic functions") + "</b>");
	operatorsLayout->addWidget(operatorsTitle, 0, 0, 1, 2, Qt::AlignHCenter);

	// Operators
	auto notOperator    = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::notOp),    0, true, operatorsWidget);
	auto concatOperator = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::concatOp), 0, true, operatorsWidget);
	auto equalOperator  = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::equalOp),  0, true, operatorsWidget);
	auto diffOperator   = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::diffOp),   0, true, operatorsWidget);

	auto and2Operator  = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::andOp,  2), 0, true, operatorsWidget);
	auto or2Operator   = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::orOp,   2), 0, true, operatorsWidget);
	auto xor2Operator  = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::xorOp,  2), 0, true, operatorsWidget);
	auto nand2Operator = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::nandOp, 2), 0, true, operatorsWidget);
	auto nor2Operator  = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::norOp,  2), 0, true, operatorsWidget);
	auto xnor2Operator = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::xnorOp, 2), 0, true, operatorsWidget);

	auto and3Operator  = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::andOp,  3), 0, true, operatorsWidget);
	auto or3Operator   = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::orOp,   3), 0, true, operatorsWidget);
	auto xor3Operator  = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::xorOp,  3), 0, true, operatorsWidget);
	auto nand3Operator = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::nandOp, 3), 0, true, operatorsWidget);
	auto nor3Operator  = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::norOp,  3), 0, true, operatorsWidget);
	auto xnor3Operator = new EquationEditorWidget(make_shared<Equation>(OperatorType_t::xnorOp, 3), 0, true, operatorsWidget);

	// 1 per row
	operatorsLayout->addWidget(notOperator,    1, 0, 1, 2);
	operatorsLayout->addWidget(concatOperator, 2, 0, 1, 2);

	// Equal/diff
	operatorsLayout->addWidget(equalOperator,  3, 0, 1, 1);
	operatorsLayout->addWidget(diffOperator,   3, 1, 1, 1);

	// Other logic operators
	operatorsLayout->addWidget(and2Operator,   4, 0, 1, 1);
	operatorsLayout->addWidget(and3Operator,   4, 1, 1, 1);
	operatorsLayout->addWidget(or2Operator,    5, 0, 1, 1);
	operatorsLayout->addWidget(or3Operator,    5, 1, 1, 1);
	operatorsLayout->addWidget(xor2Operator,   6, 0, 1, 1);
	operatorsLayout->addWidget(xor3Operator,   6, 1, 1, 1);
	operatorsLayout->addWidget(nand2Operator,  7, 0, 1, 1);
	operatorsLayout->addWidget(nand3Operator,  7, 1, 1, 1);
	operatorsLayout->addWidget(nor2Operator,   8, 0, 1, 1);
	operatorsLayout->addWidget(nor3Operator,   8, 1, 1, 1);
	operatorsLayout->addWidget(xnor2Operator,  9, 0, 1, 1);
	operatorsLayout->addWidget(xnor3Operator,  9, 1, 1, 1);

	return operatorsWidget;
}

void TemplateEquationPartsWidget::buildVariableList(QVBoxLayout* layout, QList<componentId_t> variableList) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	for (auto varId : variableList)
	{
		auto variableEditorWidget = new VariableEditorWidget(varId, 0, true);

		auto variable = machine->getVariable(varId);
		if (variable->getSize() > 1)
		{
			auto hLayout = new QHBoxLayout();
			hLayout->addWidget(variableEditorWidget);

			auto rangeEquation = make_shared<Equation>(OperatorType_t::extractOp);
			rangeEquation->setOperand(0, varId);
			rangeEquation->setRange(0);
			auto rangeEditorWidget = new EquationEditorWidget(rangeEquation, 0, true);
			hLayout->addWidget(rangeEditorWidget);

			layout->addLayout(hLayout);
		}
		else
		{
			layout->addWidget(variableEditorWidget);
		}
	}
}
