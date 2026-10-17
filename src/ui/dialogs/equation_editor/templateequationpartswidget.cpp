/*
 * Copyright © 2025-2026 Clément Foucher
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

// Qt
#include <QLabel>
#include <QBoxLayout>
#include <QGroupBox>

// StateS
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
	//
	// Build subwidgets

	auto operators = this->getOperators();
	auto variables = this->getVariables();

	//
	// Build complete rendering

	auto mainLayout = new QHBoxLayout(this);

	mainLayout->addWidget(operators);
	mainLayout->addWidget(variables);
}

QWidget* TemplateEquationPartsWidget::getOperators() const
{
	//
	// Build widgets

	// Main widget
	auto operatorsWidget = new QGroupBox(tr("Operators"));

	// Sections titles
	auto comparisonOperatorsTitle = new QLabel("<b>" + tr("Comparision") + "</b>");
	auto logicFunctionsTitle      = new QLabel("<b>" + tr("Logic functions") + "</b>");
	auto arithmeticFunctionsTitle = new QLabel("<b>" + tr("Arithmetic functions") + "</b>");
	auto otherTitle               = new QLabel("<b>" + tr("Other") + "</b>");

	// Operators
	auto equalOperator          = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::equalOp),                0, true);
	auto diffOperator           = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::diffOp),                 0, true);
	auto lessOperator           = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::lessThanOp),             0, true);
	auto greaterOperator        = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::greaterThanOp),          0, true);
	auto lessOrEqualOperator    = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::lessThanOrEqualToOp),    0, true);
	auto greaterOrEqualOperator = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::greaterThanOrEqualToOp), 0, true);
	auto plusOperator           = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::plusOp),                 0, true);
	auto minusOperator          = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::minusOp),                0, true);

	auto notOperator    = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::notOp),     0, true);

	auto and2Operator   = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::andOp,  2), 0, true);
	auto or2Operator    = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::orOp,   2), 0, true);
	auto xor2Operator   = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::xorOp,  2), 0, true);
	auto nand2Operator  = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::nandOp, 2), 0, true);
	auto nor2Operator   = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::norOp,  2), 0, true);
	auto xnor2Operator  = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::xnorOp, 2), 0, true);

	auto and3Operator   = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::andOp,  3), 0, true);
	auto or3Operator    = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::orOp,   3), 0, true);
	auto xor3Operator   = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::xorOp,  3), 0, true);
	auto nand3Operator  = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::nandOp, 3), 0, true);
	auto nor3Operator   = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::norOp,  3), 0, true);
	auto xnor3Operator  = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::xnorOp, 3), 0, true);

	auto concatOperator = new EquationEditorWidget(std::make_shared<Equation>(Equation::Operator_t::concatOp),  0, true);

	//
	// Build complete rendering

	auto leftOperatorsLayout  = new QGridLayout();
	auto rightOperatorsLayout = new QGridLayout();
	leftOperatorsLayout ->setAlignment(Qt::AlignTop);
	rightOperatorsLayout->setAlignment(Qt::AlignTop);


	// Logic operators
	leftOperatorsLayout->addWidget(logicFunctionsTitle, 0, 0, 1, 2, Qt::AlignHCenter);

	leftOperatorsLayout->addWidget(notOperator,    1, 0, 1, 2);

	leftOperatorsLayout->addWidget(and2Operator,   2, 0, 1, 1);
	leftOperatorsLayout->addWidget(and3Operator,   2, 1, 1, 1);
	leftOperatorsLayout->addWidget(or2Operator,    3, 0, 1, 1);
	leftOperatorsLayout->addWidget(or3Operator,    3, 1, 1, 1);
	leftOperatorsLayout->addWidget(xor2Operator,   4, 0, 1, 1);
	leftOperatorsLayout->addWidget(xor3Operator,   4, 1, 1, 1);
	leftOperatorsLayout->addWidget(nand2Operator,  5, 0, 1, 1);
	leftOperatorsLayout->addWidget(nand3Operator,  5, 1, 1, 1);
	leftOperatorsLayout->addWidget(nor2Operator,   6, 0, 1, 1);
	leftOperatorsLayout->addWidget(nor3Operator,   6, 1, 1, 1);
	leftOperatorsLayout->addWidget(xnor2Operator,  7, 0, 1, 1);
	leftOperatorsLayout->addWidget(xnor3Operator,  7, 1, 1, 1);

	// Comparison
	rightOperatorsLayout->addWidget(comparisonOperatorsTitle, 0, 0, 1, 2, Qt::AlignHCenter);

	rightOperatorsLayout->addWidget(equalOperator,          1, 0, 1, 1);
	rightOperatorsLayout->addWidget(diffOperator,           1, 1, 1, 1);
	rightOperatorsLayout->addWidget(lessOperator,           2, 0, 1, 1);
	rightOperatorsLayout->addWidget(greaterOperator,        2, 1, 1, 1);
	rightOperatorsLayout->addWidget(lessOrEqualOperator,    3, 0, 1, 1);
	rightOperatorsLayout->addWidget(greaterOrEqualOperator, 3, 1, 1, 1);

	// Arithmetic
	rightOperatorsLayout->addWidget(arithmeticFunctionsTitle, 4, 0, 1, 2, Qt::AlignHCenter);

	rightOperatorsLayout->addWidget(plusOperator,  5, 0, 1, 1);
	rightOperatorsLayout->addWidget(minusOperator, 5, 1, 1, 1);

	// Other operators
	rightOperatorsLayout->addWidget(otherTitle, 6, 0, 1, 2, Qt::AlignHCenter);

	rightOperatorsLayout->addWidget(concatOperator, 7, 0, 1, 2);

	// Global widget
	auto operatorsLayout = new QHBoxLayout(operatorsWidget);
	operatorsLayout->addLayout(leftOperatorsLayout);
	operatorsLayout->addLayout(rightOperatorsLayout);

	return operatorsWidget;
}

QWidget* TemplateEquationPartsWidget::getVariables() const
{
	//
	// Build widgets

	auto variablesWidget = new QGroupBox(tr("Variables"));

	auto inputs            = this->getInputs();
	auto internalVariables = this->getInternalVariables();
	auto outputVariables   = this->getOutputVariables();
	auto constants         = this->getConstants();

	//
	// Build complete rendering

	auto variablesLayout = new QHBoxLayout(variablesWidget);

	if (inputs != nullptr)
	{
		variablesLayout->addWidget(inputs);
	}
	if (internalVariables != nullptr)
	{
		variablesLayout->addWidget(internalVariables);
	}
	if (outputVariables != nullptr)
	{
		variablesLayout->addWidget(outputVariables);
	}
	if (constants != nullptr)
	{
		variablesLayout->addWidget(constants);
	}

	return variablesWidget;
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
	auto internalVariablesTitle = new QLabel("<b>" + tr("Internal variables")+ "</b>");
	internalVariablesLayout->addWidget(internalVariablesTitle, 0, Qt::AlignHCenter);

	// Machine internal variables
	this->buildVariableList(internalVariablesLayout, internalVariableIds);

	return internalVariablesWidget;
}

QWidget* TemplateEquationPartsWidget::getOutputVariables() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto outputIds = machine->getOutputVariablesIds();
	if (outputIds.isEmpty() == true) return nullptr;


	// Build widget
	auto outputsWidget = new QWidget();
	auto outputsLayout = new QVBoxLayout(outputsWidget);
	outputsLayout->setAlignment(Qt::AlignTop);

	// Title
	auto outputsTitle = new QLabel("<b>" + tr("Outputs") + "</b>");
	outputsLayout->addWidget(outputsTitle, 0, Qt::AlignHCenter);

	// Machine inputs
	this->buildVariableList(outputsLayout, outputIds);

	return outputsWidget;
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

	// Custom constant for booleans
	auto booleanConstant = new ConstantEditorWidget(BooleanValue::falseValue(), 0, true, constantsWidget);
	constantsLayout->addWidget(booleanConstant);

	// Custom constant for bit vectors
	auto bitVectorConstant = new ConstantEditorWidget(BitVectorValue::allZeros(1), 0, true, constantsWidget);
	constantsLayout->addWidget(bitVectorConstant);

	// Custom constant for natural
	auto naturalConstant = new ConstantEditorWidget(NaturalValue{0}, 0, true, constantsWidget);
	constantsLayout->addWidget(naturalConstant);

	// Custom constant for relative
	auto relativeConstant = new ConstantEditorWidget(RelativeValue{0}, 0, true, constantsWidget);
	constantsLayout->addWidget(relativeConstant);

	// Machine constants (which are actually variables)
	this->buildVariableList(constantsLayout, machine->getConstantsIds());

	return constantsWidget;
}

void TemplateEquationPartsWidget::buildVariableList(QVBoxLayout* layout, QList<ComponentId> variableList) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;


	for (auto varId : variableList)
	{
		auto variableEditorWidget = new VariableEditorWidget(varId, 0, true);

		auto variable = machine->getVariable(varId);

		if ( (variable->getType() == MachineValue::Type_t::bitVector) &&
		     (variable->getInitialValue().getBitVectorValue().getSize() > 1)
		   )
		{
			auto hLayout = new QHBoxLayout();
			hLayout->addWidget(variableEditorWidget);

			auto rangeEquation = std::make_shared<Equation>(Equation::Operator_t::extractOp, 2);
			rangeEquation->setOperand(0, varId);
			rangeEquation->setOperand(1, NaturalValue{0});
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
