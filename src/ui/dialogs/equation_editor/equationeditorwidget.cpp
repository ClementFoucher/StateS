/*
 * Copyright © 2014-2026 Clément Foucher
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
#include "equationeditorwidget.h"

// Qt
#include <QDrag>
#include <QLabel>
#include <QMessageBox>
#include <QBoxLayout>

// StateS
#include "equationpartmimedata.h"
#include "equation.h"
#include "equationcomputer.h"
#include "operand.h"
#include "inverterbarwidget.h"
#include "equationeditordialog.h"
#include "constanteditorwidget.h"
#include "variableeditorwidget.h"
#include "nulleditorwidget.h"


EquationEditorWidget::EquationEditorWidget(std::shared_ptr<Equation> equation, uint rankInParentOperands, bool isTemplate, QWidget* parent) :
	EquationPartEditorWidget(rankInParentOperands, isTemplate, parent)
{
	this->equation = equation;

	if (this->isTemplate == true)
	{
		this->buildTemplateEquation();
	}
	else
	{
		this->buildCompleteEquation();
	}

	connect(this->equation.get(), &Equation::equationTextChangedEvent, this, &EquationEditorWidget::equationChangedEventHandler);
}

void EquationEditorWidget::replaceOperand(uint operandRank, std::shared_ptr<Equation> newOperand)
{
	if (this->equation == nullptr) return;


	this->equation->setOperand(operandRank, newOperand);
	this->updateOperandWidget(operandRank);
}

void EquationEditorWidget::replaceOperand(uint operandRank, MachineValue newConstant, bool isProcessingDrop)
{
	if (this->equation == nullptr) return;


	this->equation->setOperand(operandRank, newConstant);
	this->updateOperandWidget(operandRank);

	if (isProcessingDrop == true)
	{
		auto newOperand = dynamic_cast<ConstantEditorWidget*>(this->operands.at(operandRank));
		if (newOperand != nullptr)
		{
			newOperand->beginEdit();
		}
	}
}

void EquationEditorWidget::replaceOperand(uint operandRank, ComponentId newVariable)
{
	if (this->equation == nullptr) return;


	this->equation->setOperand(operandRank, newVariable);
	this->updateOperandWidget(operandRank);
}

void EquationEditorWidget::clearOperand(uint operandRank)
{
	if (this->equation == nullptr) return;


	this->equation->clearOperand(operandRank);
	this->updateOperandWidget(operandRank);
}

std::shared_ptr<Equation> EquationEditorWidget::getLogicEquation() const
{
	return this->equation;
}

void EquationEditorWidget::closeOpenEditors()
{
	if (this->parentEquationEditor != nullptr)
	{
		// Pass the instruction to the parent
		this->parentEquationEditor->closeOpenEditors();
	}
	else
	{
		// We are root
		this->validEdit();
	}
}

bool EquationEditorWidget::validEdit()
{
	if (this->equation == nullptr) return false;


	const auto operandChildren = this->findChildren<EquationPartEditorWidget*>("", Qt::FindDirectChildrenOnly);
	for (auto& operand : operandChildren)
	{
		bool result = operand->validEdit();
		if (result == true)
		{
			// Only one editor can be active at a time
			return true;
		}
	}

	return false;
}

bool EquationEditorWidget::cancelEdit()
{
	if (this->equation == nullptr) return false;


	const auto operandChildren = this->findChildren<EquationPartEditorWidget*>("", Qt::FindDirectChildrenOnly);
	for (auto& operand : operandChildren)
	{
		bool result = operand->cancelEdit();
		if (result == true)
		{
			// Only one editor can be active at a time
			return true;
		}
	}

	return false;
}

uint EquationEditorWidget::getAllowedMenuActions() const
{
	if (this->equation == nullptr) return 0;


	uint allowedActions = static_cast<uint>(ContextAction_t::Remove);

	switch(this->equation->getOperator())
	{
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xnorOp:
	case Equation::Operator_t::concatOp:
		allowedActions |= static_cast<uint>(ContextAction_t::IncreaseOperandCount);
		if (this->equation->getOperandCount() > 2)
		{
			allowedActions |= static_cast<uint>(ContextAction_t::DecreaseOperandCount);
		}
		break;
	case Equation::Operator_t::extractOp:
		if (this->equation->getOperandCount() == 3)
		{
			allowedActions |= static_cast<uint>(ContextAction_t::ExtractSwitchSingle);
		}
		else // (this->equation->getOperandCount() == 2)
		{
			allowedActions |= static_cast<uint>(ContextAction_t::ExtractSwitchRange);
		}
		break;
	case Equation::Operator_t::notOp:
	case Equation::Operator_t::plusOp:
	case Equation::Operator_t::minusOp:
	case Equation::Operator_t::identity:
	case Equation::Operator_t::equalOp:
	case Equation::Operator_t::diffOp:
	case Equation::Operator_t::lessThanOp:
	case Equation::Operator_t::lessThanOrEqualToOp:
	case Equation::Operator_t::greaterThanOp:
	case Equation::Operator_t::greaterThanOrEqualToOp:
		break;
	}

	switch(this->equation->getOperator())
	{
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::concatOp:
	case Equation::Operator_t::extractOp:
	case Equation::Operator_t::equalOp:
	case Equation::Operator_t::diffOp:
	case Equation::Operator_t::lessThanOp:
	case Equation::Operator_t::lessThanOrEqualToOp:
	case Equation::Operator_t::greaterThanOp:
	case Equation::Operator_t::greaterThanOrEqualToOp:
		allowedActions |= static_cast<uint>(ContextAction_t::Invert);
		break;
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xnorOp:
	case Equation::Operator_t::notOp:
		allowedActions |= static_cast<uint>(ContextAction_t::RemoveInverter);
		break;
	case Equation::Operator_t::plusOp:
	case Equation::Operator_t::minusOp:
	case Equation::Operator_t::identity:
		break;
	}

	return allowedActions;
}

uint EquationEditorWidget::getAllowedDropActions() const
{
	if (this->equation == nullptr) return 0;


	uint allowedActions = static_cast<uint>(DropAction_t::ReplaceExisting) |
	                      static_cast<uint>(DropAction_t::ExistingAsOperand);

	switch(this->equation->getOperator())
	{
	case Equation::Operator_t::notOp:
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xnorOp:
		allowedActions |= static_cast<uint>(DropAction_t::RemoveInverter);
		break;
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::concatOp:
	case Equation::Operator_t::extractOp:
	case Equation::Operator_t::identity:
	case Equation::Operator_t::equalOp:
	case Equation::Operator_t::diffOp:
	case Equation::Operator_t::lessThanOp:
	case Equation::Operator_t::lessThanOrEqualToOp:
	case Equation::Operator_t::greaterThanOp:
	case Equation::Operator_t::greaterThanOrEqualToOp:
	case Equation::Operator_t::plusOp:
	case Equation::Operator_t::minusOp:
		// Nothing more
		break;
	}

	return allowedActions;
}

QString EquationEditorWidget::getText() const
{
	if (this->equation == nullptr) return QString();


	return this->equation->getText();
}

QString EquationEditorWidget::getToolTipText() const
{
	if (this->isTemplate == true) return QString();

	if (this->equation == nullptr) return QString();


	QString toolTipText;
	switch (this->equation->getComputationFailureCause())
	{
	case Equation::ComputationFailureCause_t::nofail:
		switch (this->equation->getComputationWarning())
		{
		case Equation::ComputationWarning_t::differentTypeComparison:
			toolTipText += "<b>" + tr("Warning:") + "</b> ";
			toolTipText += "<br>";
			toolTipText += tr("Comparison is done between variables that have different types.");
			switch (this->equation->getOperator())
			{
			case Equation::Operator_t::equalOp:
				toolTipText += "<br>";
				toolTipText += tr("As a result, they will never be equal and this equation will always be false.");
				break;
			case Equation::Operator_t::diffOp:
				toolTipText += "<br>";
				toolTipText += tr("As a result, they will always be different and this equation will always be true.");
				break;
			default:
				break;
			}
			break;
		case Equation::ComputationWarning_t::differentSizeComparison:
			toolTipText += "<b>" + tr("Warning:") + "</b> ";
			toolTipText += "<br>";
			toolTipText += tr("Comparison is done between bit vector variables that have different sizes.");
			switch (this->equation->getOperator())
			{
			case Equation::Operator_t::equalOp:
				toolTipText += "<br>";
				toolTipText += tr("As a result, they will never be equal and this equation will always be false.");
				break;
			case Equation::Operator_t::diffOp:
				toolTipText += "<br>";
				toolTipText += tr("As a result, they will always be different and this equation will always be true.");
				break;
			default:
				break;
			}
			break;
		case Equation::ComputationWarning_t::noWarning:
			toolTipText = tr("Equation:") + " " + this->equation->getText();
			break;
		}
		break;
	case Equation::ComputationFailureCause_t::nullOperand:
		toolTipText  = "<b>" + tr("Error:") + "</b> ";
		toolTipText += "<br>";
		toolTipText += tr("At least one of the operands is empty.");
		break;
	case Equation::ComputationFailureCause_t::invalidOperandValue:
		toolTipText  = "<b>" + tr("Error:") + "</b> ";
		toolTipText += "<br>";
		toolTipText += tr("At least one of the operands has an invalid value.");
		break;
	case Equation::ComputationFailureCause_t::incorrectOperandType:
		toolTipText  = "<b>" + tr("Error:") + "</b> ";
		toolTipText += "<br>";
		toolTipText += tr("At least one of the operands has an invalid type.");
		switch (this->equation->getOperator())
		{
		case Equation::Operator_t::extractOp:
			toolTipText += "<br>";
			toolTipText += tr("The current operator can only have a bit vector as the first operand,");
			toolTipText += " ";
			if (this->equation->getOperandCount() == 2)
			{
				toolTipText += tr("and a natural integer as second operand.");
			}
			else // (this->equation->getOperandCount() == 3)
			{
				toolTipText += tr("and natural integers as other operands.");
			}
			break;
		case Equation::Operator_t::concatOp:
		case Equation::Operator_t::andOp:
		case Equation::Operator_t::nandOp:
		case Equation::Operator_t::orOp:
		case Equation::Operator_t::norOp:
		case Equation::Operator_t::xorOp:
		case Equation::Operator_t::xnorOp:
		case Equation::Operator_t::notOp:
			toolTipText += "<br>";
			toolTipText += tr("The current operator can only have boolean or bit vector operands.");
			break;
		case Equation::Operator_t::lessThanOp:
		case Equation::Operator_t::lessThanOrEqualToOp:
		case Equation::Operator_t::greaterThanOp:
		case Equation::Operator_t::greaterThanOrEqualToOp:
		case Equation::Operator_t::plusOp:
		case Equation::Operator_t::minusOp:
			toolTipText += "<br>";
			toolTipText += tr("The current operator can only have natural or relative operands.");
			break;
		case Equation::Operator_t::equalOp:
		case Equation::Operator_t::diffOp:
		case Equation::Operator_t::identity:
			break;
		}
		break;
	case Equation::ComputationFailureCause_t::operandsSizesMismatch:
		toolTipText  = "<b>" + tr("Error:") + "</b> ";
		toolTipText += "<br>";
		toolTipText += tr("The sizes of the operands do not match between each other.");
		toolTipText += "<br>";
		toolTipText += tr("All bit vector operands must have the same size.");
		break;
	case Equation::ComputationFailureCause_t::operandsTypesMismatch:
		toolTipText  = "<b>" + tr("Error:") + "</b> ";
		toolTipText += "<br>";
		toolTipText += tr("The types of the operands do not match between each other.");
		toolTipText += "<br>";
		toolTipText += tr("All operands must have the same type.");
		break;
	case Equation::ComputationFailureCause_t::incorrectOperandValue:
		toolTipText  = "<b>" + tr("Error:") + "</b> ";
		toolTipText += "<br>";
		toolTipText += tr("The value of an operand is incorrect.");
		toolTipText += "<br>";
		if (this->equation->getOperandCount() == 2)
		{
			toolTipText += tr("The index of the bit to extract must be within the vector size.");
		}
		else // (this->equation->getOperandCount() == 3)
		{
			toolTipText += tr("The indexes of the subvector to extract must be within the vector size,");
			toolTipText += " ";
			toolTipText += tr("and the first index must be greater than the second index.");
		}
		break;
	}

	return toolTipText;
}

bool EquationEditorWidget::getReplaceWithoutAsking() const
{
	if (this->equation == nullptr) return true;


	for (uint i = 0 ; i < this->equation->getOperandCount() ; i++)
	{
		auto operand = this->equation->getOperand(i);
		if (operand != nullptr) return false;
	}

	return true;
}

bool EquationEditorWidget::getIsErroneous() const
{
	if (this->equation == nullptr) return true;


	if (this->equation->getComputationFailureCause() == Equation::ComputationFailureCause_t::nofail)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool EquationEditorWidget::getHasWarning() const
{
	if (this->equation == nullptr) return false;


	if (this->equation->getComputationWarning() == Equation::ComputationWarning_t::noWarning)
	{
		return false;
	}
	else
	{
		return true;
	}
}

QDrag* EquationEditorWidget::buildDrag()
{
	if (this->equation == nullptr) return new QDrag(this);


	auto drag = new QDrag(this);

	uint availableActions = static_cast<uint>(DropAction_t::ReplaceExisting);
	if (this->equation->getOperator() == Equation::Operator_t::notOp)
	{
		availableActions |= static_cast<uint>(DropAction_t::RemoveInverter);
	}
	if (this->equation->getOperator() != Equation::Operator_t::extractOp)
	{
		availableActions |= static_cast<uint>(DropAction_t::ExistingAsOperand);
	}

	QMimeData* mimeData = new EquationPartMimeData(this->getTemplateText(), availableActions, this->equation->clone());

	drag->setMimeData(mimeData);

	// If this is an extract operator, drag image will not match template display: create a correct equation
	if (this->equation->getOperator() == Equation::Operator_t::extractOp)
	{
		// Build a dummy display with no parent so that it is rendered as independent window
		EquationEditorWidget displayGraphicEquation(this->equation->clone(), 0, false);
		displayGraphicEquation.setStyleSheet(EquationPartEditorWidget::activeStyleSheet);
		drag->setPixmap(displayGraphicEquation.grab());
	}
	else
	{
		drag->setPixmap(this->grab());
	}

	return drag;
}

void EquationEditorWidget::processSpecificMenuAction(ContextAction_t action)
{
	if (this->equation == nullptr) return;


	bool rebuild = false;
	switch (action)
	{
	case ContextAction_t::IncreaseOperandCount:
		this->equation->increaseOperandCount();
		rebuild = true;
		break;
	case ContextAction_t::DecreaseOperandCount:
	{
		bool valid = false;

		auto operandToRemove = this->equation->getOperand(this->equation->getOperandCount()-1);
		if (operandToRemove != nullptr)
		{
			QMessageBox::StandardButton reply;
			auto text = tr("Rightmost operand deletion requested, but it is not empty.") +
			               "<br>" +
			               tr("Its content is:") + " " + operandToRemove->getText() +
			               "<br>" +
			               tr("Delete rightmost operand?") ;
			reply = QMessageBox::question(this, tr("User confirmation needed"), text, QMessageBox::Ok | QMessageBox::Cancel);

			if (reply == QMessageBox::StandardButton::Ok)
			{
				valid = true;
			}
		}
		else
		{
			valid = true;
		}

		if (valid == true)
		{
			this->equation->decreaseOperandCount();
			rebuild = true;
		}
		break;
	}
	case ContextAction_t::ExtractSwitchSingle:
		this->equation->decreaseOperandCount();
		rebuild = true;
		break;
	case ContextAction_t::ExtractSwitchRange:
	{
		this->equation->increaseOperandCount();

		unsigned int rangeL = 0;
		auto operand1 = this->equation->getOperand(1);
		if (operand1 != nullptr)
		{
			if (operand1->getType() == MachineValue::Type_t::natural)
			{
				rangeL = operand1->getInitialValue().getNaturalValue();
			}
		}

		if (rangeL > 0)
		{
			this->equation->setOperand(2, NaturalValue{rangeL - 1});
		}
		else
		{
			this->equation->setOperand(1, NaturalValue{1});
			this->equation->setOperand(2, NaturalValue{0});
		}
		rebuild = true;
		break;
	}
	case ContextAction_t::Invert:
		this->setInverted(true);
		// Do NOT do anything after, as this may cease to exist
		break;
	case ContextAction_t::RemoveInverter:
		this->setInverted(false);
		// Do NOT do anything after, as this may cease to exist
		break;
	case ContextAction_t::AddExtractor:
		// This action is not supported by this class
		break;
	case ContextAction_t::Remove:
	case ContextAction_t::Cancel:
	case ContextAction_t::Edit:
		// Common actions: processed by parent class
		break;
	}

	if (rebuild == true)
	{
		this->clear();
		this->buildCompleteEquation();
	}
}

void EquationEditorWidget::processSpecificDropAction(DropAction_t action)
{
	switch (action)
	{
	case DropAction_t::ExistingAsOperand:
	{
		if (this->tempValueNature != TempValueNature_t::equation) return;

		auto tempEquation = std::get<std::shared_ptr<Equation>>(this->tempValue);
		if (tempEquation == nullptr) return;

		if (this->equation == nullptr) return;


		tempEquation->setOperand(0, this->equation);
		this->tempValue = tempEquation;
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
	}
	case DropAction_t::RemoveInverter:
		this->setInverted(false);
		// Do NOT do anything after, as this may cease to exist
		break;
	case DropAction_t::ReplaceExisting:
	case DropAction_t::Cancel:
		// Common actions: processed by parent class
		break;
	}
}

void EquationEditorWidget::equationChangedEventHandler()
{
	this->setToolTip(this->getToolTipText());
	this->updateBorderColor();
}

void EquationEditorWidget::clear()
{
	// Clear previous content
	for (auto child : this->children())
	{
		// We must not delete the widgets immediately,
		// as we may be in the process of an edition
		auto childWidget = dynamic_cast<QWidget*>(child);
		if (childWidget != nullptr)
		{
			child->deleteLater();
		}
	}
	// But we must delete the layout to allow for its replacement
	delete this->layout();

	this->operands.clear();
}

void EquationEditorWidget::buildTemplateEquation()
{
	if (this->equation == nullptr) return;


	auto equationText = new QLabel(this->getTemplateText());
	equationText->setAlignment(Qt::AlignCenter);

	auto equationLayout = new QHBoxLayout(this);
	equationLayout->addWidget(equationText);
}

void EquationEditorWidget::buildCompleteEquation()
{
	if (this->equation == nullptr) return;


	auto equationOperator = this->equation->getOperator();

	auto equationLayout = new QHBoxLayout();
	equationLayout->setAlignment(Qt::AlignVCenter);

	// Prefix text
	if (equationOperator == Equation::Operator_t::concatOp)
	{
		equationLayout->addWidget(new QLabel("{"));
	}

	// Operands and separators texts
	uint operandCount = this->equation->getOperandCount();
	for (uint operandRank = 0 ; operandRank < operandCount ; operandRank++)
	{
		// Add operand
		auto operandWidget = this->buildOperandEditorWidget(operandRank);
		this->operands.append(operandWidget);
		equationLayout->addWidget(operandWidget);

		// Add operator, except for last operand
		if (operandRank < operandCount - 1)
		{
			QString operatorText;
			switch (equationOperator)
			{
			case Equation::Operator_t::andOp:
			case Equation::Operator_t::nandOp:
				operatorText = "•";
				break;
			case Equation::Operator_t::orOp:
			case Equation::Operator_t::norOp:
				operatorText = "+";
				break;
			case Equation::Operator_t::xorOp:
			case Equation::Operator_t::xnorOp:
				operatorText = "⊕";
				break;
			case Equation::Operator_t::equalOp:
				operatorText = "=";
				break;
			case Equation::Operator_t::diffOp:
				operatorText = "≠";
				break;
			case Equation::Operator_t::lessThanOp:
				operatorText = "<";
				break;
			case Equation::Operator_t::lessThanOrEqualToOp:
				operatorText = "≤";
				break;
			case Equation::Operator_t::greaterThanOp:
				operatorText = ">";
				break;
			case Equation::Operator_t::greaterThanOrEqualToOp:
				operatorText = "≥";
				break;
			case Equation::Operator_t::concatOp:
				operatorText = ":";
				break;
			case Equation::Operator_t::plusOp:
				operatorText = "+";
				break;
			case Equation::Operator_t::minusOp:
				operatorText = "−";
				break;
			case Equation::Operator_t::extractOp:
				if (operandRank == 0)
				{
					operatorText = "[";
				}
				else // (operandRank == 1)
				{
					operatorText = "‥";
				}
			case Equation::Operator_t::notOp:
			case Equation::Operator_t::identity:
				// Single operand operators
				break;
			}

			auto operatorLabel = new QLabel(operatorText);
			operatorLabel->setAlignment(Qt::AlignCenter);
			equationLayout->addWidget(operatorLabel);
		}
	}

	// Suffix text
	if (equationOperator == Equation::Operator_t::concatOp)
	{
		equationLayout->addWidget(new QLabel("}"));
	}
	else if (equationOperator == Equation::Operator_t::extractOp)
	{
		equationLayout->addWidget(new QLabel("]"));
	}

	if (EquationComputer::isInverted(equationOperator) == false)
	{
		this->setLayout(equationLayout);
	}
	else // (EquationComputer::isInverted(equationOperator) == true)
	{
		auto verticalLayout = new QVBoxLayout();

		auto inverterBar = new InverterBarWidget();

		verticalLayout->addWidget(inverterBar);
		verticalLayout->addLayout(equationLayout);

		this->setLayout(verticalLayout);
	}
}

void EquationEditorWidget::updateOperandWidget(uint operandRank)
{
	auto oldOperand = this->operands.at(operandRank);
	if (oldOperand == nullptr) return;

	auto newOperand = this->buildOperandEditorWidget(operandRank);
	if (newOperand == nullptr) return;


	this->layout()->replaceWidget(oldOperand, newOperand);
	this->operands[operandRank] = newOperand;

	// Do not delete the old operand now as we may be in a call from this object
	oldOperand->deleteLater();
	// The event loop will only return after mouse button is released:
	// this may take a while if the user started a grab action.
	oldOperand->hide();
}

void EquationEditorWidget::setInverted(bool invert)
{
	if (this->equation == nullptr) return;


	bool reverseOperand = false;
	bool addInverter    = false;
	bool removeInverter = false;

	if (invert == true)
	{
		switch(this->equation->getOperator())
		{
		case Equation::Operator_t::andOp:
		case Equation::Operator_t::orOp:
		case Equation::Operator_t::xorOp:
		case Equation::Operator_t::equalOp:
		case Equation::Operator_t::diffOp:
		case Equation::Operator_t::lessThanOp:
		case Equation::Operator_t::lessThanOrEqualToOp:
		case Equation::Operator_t::greaterThanOp:
		case Equation::Operator_t::greaterThanOrEqualToOp:
			reverseOperand = true;
			break;
		case Equation::Operator_t::concatOp:
		case Equation::Operator_t::extractOp:
		case Equation::Operator_t::identity:
			addInverter = true;
			break;
		case Equation::Operator_t::notOp:
		case Equation::Operator_t::nandOp:
		case Equation::Operator_t::norOp:
		case Equation::Operator_t::xnorOp:
		case Equation::Operator_t::plusOp:
		case Equation::Operator_t::minusOp:
			// Should not happen
			break;
		}
	}
	else // (invert == false)
	{
		switch(this->equation->getOperator())
		{
		case Equation::Operator_t::notOp:
			removeInverter = true;
			break;
		case Equation::Operator_t::nandOp:
		case Equation::Operator_t::norOp:
		case Equation::Operator_t::xnorOp:
			reverseOperand = true;
			break;
		case Equation::Operator_t::andOp:
		case Equation::Operator_t::orOp:
		case Equation::Operator_t::xorOp:
		case Equation::Operator_t::concatOp:
		case Equation::Operator_t::extractOp:
		case Equation::Operator_t::equalOp:
		case Equation::Operator_t::diffOp:
		case Equation::Operator_t::lessThanOp:
		case Equation::Operator_t::lessThanOrEqualToOp:
		case Equation::Operator_t::greaterThanOp:
		case Equation::Operator_t::greaterThanOrEqualToOp:
		case Equation::Operator_t::plusOp:
		case Equation::Operator_t::minusOp:
		case Equation::Operator_t::identity:
			// Should not happen
			break;
		}
	}

	if (addInverter == true)
	{
		auto tempEquation = std::make_shared<Equation>(Equation::Operator_t::notOp);
		tempEquation->setOperand(0, this->equation);

		this->tempValueNature = TempValueNature_t::equation;
		this->tempValue = tempEquation;
		this->replaceByTempValue();
	}
	else if (removeInverter == true)
	{
		auto operand = this->equation->getOperand(0);
		if (operand != nullptr)
		{
			switch (operand->getSource())
			{
			case Operand::Source_t::variable:
				this->tempValueNature = TempValueNature_t::variable;
				this->tempValue = operand->getVariableId();
				break;
			case Operand::Source_t::equation:
				this->tempValueNature = TempValueNature_t::equation;
				this->tempValue = operand->getEquation();
				break;
			case Operand::Source_t::constant:
				this->tempValueNature = TempValueNature_t::constant;
				this->tempValue = operand->getConstant();
				break;
			}
		}
		else // (operand == nullptr)
		{
			this->tempValueNature = TempValueNature_t::equation;
			this->tempValue = nullptr;
		}

		this->replaceByTempValue();
	}
	else if (reverseOperand == true)
	{
		Equation::Operator_t newOperator;

		switch(this->equation->getOperator())
		{
		case Equation::Operator_t::andOp:
			newOperator = Equation::Operator_t::nandOp;
			break;
		case Equation::Operator_t::orOp:
			newOperator = Equation::Operator_t::norOp;
			break;
		case Equation::Operator_t::xorOp:
			newOperator = Equation::Operator_t::xnorOp;
			break;
		case Equation::Operator_t::nandOp:
			newOperator = Equation::Operator_t::andOp;
			break;
		case Equation::Operator_t::norOp:
			newOperator = Equation::Operator_t::orOp;
			break;
		case Equation::Operator_t::xnorOp:
			newOperator = Equation::Operator_t::xorOp;
			break;
		case Equation::Operator_t::equalOp:
			newOperator = Equation::Operator_t::diffOp;
			break;
		case Equation::Operator_t::diffOp:
			newOperator = Equation::Operator_t::equalOp;
			break;
		case Equation::Operator_t::lessThanOp:
			newOperator = Equation::Operator_t::greaterThanOp;
			break;
		case Equation::Operator_t::lessThanOrEqualToOp:
			newOperator = Equation::Operator_t::greaterThanOrEqualToOp;
			break;
		case Equation::Operator_t::greaterThanOp:
			newOperator = Equation::Operator_t::lessThanOp;
			break;
		case Equation::Operator_t::greaterThanOrEqualToOp:
			newOperator = Equation::Operator_t::lessThanOrEqualToOp;
			break;
		case Equation::Operator_t::notOp:
		case Equation::Operator_t::concatOp:
		case Equation::Operator_t::extractOp:
		case Equation::Operator_t::plusOp:
		case Equation::Operator_t::minusOp:
		case Equation::Operator_t::identity:
			// Should not happen
			newOperator = this->equation->getOperator();
			break;
		}

		this->equation->setOperator(newOperator);
		this->clear();
		this->buildCompleteEquation();
	}
}

EquationPartEditorWidget* EquationEditorWidget::buildOperandEditorWidget(uint operandRank)
{
	if (this->equation == nullptr) return nullptr;

	if (operandRank >= this->equation->getOperandCount()) return nullptr;


	auto operand = this->equation->getOperand(operandRank);
	if (operand != nullptr)
	{
		switch (operand->getSource())
		{
		case Operand::Source_t::equation:
			return new EquationEditorWidget(operand->getEquation(), operandRank, false, this);
			break;
		case Operand::Source_t::variable:
			return new VariableEditorWidget(operand->getVariableId(), operandRank, false, this);
			break;
		case Operand::Source_t::constant:
			return new ConstantEditorWidget(operand->getInitialValue(), operandRank, false, this);
			break;
		}
	}
	else
	{
		return new NullEditorWidget(operandRank, this);
	}
}

QString EquationEditorWidget::getTemplateText() const
{
	if (this->equation == nullptr) return QString();


	QString text;

	switch(this->equation->getOperator())
	{
	case Equation::Operator_t::notOp:
		text += tr("not");
		break;
	case Equation::Operator_t::andOp:
		text += tr("and");
		break;
	case Equation::Operator_t::orOp:
		text += tr("or");
		break;
	case Equation::Operator_t::xorOp:
		text += tr("xor");
		break;
	case Equation::Operator_t::nandOp:
		text += tr("nand");
		break;
	case Equation::Operator_t::norOp:
		text += tr("nor");
		break;
	case Equation::Operator_t::xnorOp:
		text += tr("xnor");
		break;
	case Equation::Operator_t::equalOp:
		text += tr("Equality");
		break;
	case Equation::Operator_t::diffOp:
		text += tr("Difference");
		break;
	case Equation::Operator_t::lessThanOp:
		text += tr("Less than");
		break;
	case Equation::Operator_t::lessThanOrEqualToOp:
		text += tr("Less than")  + "<br>" + tr("or equal");
		break;
	case Equation::Operator_t::greaterThanOp:
		text += tr("Greater than");
		break;
	case Equation::Operator_t::greaterThanOrEqualToOp:
		text += tr("Greater than")  + "<br>" + tr("or equal");
		break;
	case Equation::Operator_t::plusOp:
		text += tr("Plus");
		break;
	case Equation::Operator_t::minusOp:
		text += tr("Minus");
		break;
	case Equation::Operator_t::concatOp:
		text += tr("Concatenate");
		break;
	case Equation::Operator_t::extractOp:
		text += "[…]";
		break;
	case Equation::Operator_t::identity:
		// Nothing to do: special case.
		break;
	}

	switch(this->equation->getOperator())
	{
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xnorOp:
		text += " " + QString::number(this->equation->getOperandCount());
		break;
	case Equation::Operator_t::notOp:
	case Equation::Operator_t::equalOp:
	case Equation::Operator_t::diffOp:
	case Equation::Operator_t::lessThanOp:
	case Equation::Operator_t::lessThanOrEqualToOp:
	case Equation::Operator_t::greaterThanOp:
	case Equation::Operator_t::greaterThanOrEqualToOp:
	case Equation::Operator_t::plusOp:
	case Equation::Operator_t::minusOp:
	case Equation::Operator_t::concatOp:
	case Equation::Operator_t::extractOp:
	case Equation::Operator_t::identity:
		break;
	}

	return text;
}
