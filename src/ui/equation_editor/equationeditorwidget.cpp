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
#include "equationeditorwidget.h"

// Qt classes
#include <QDrag>
#include <QLabel>
#include <QMessageBox>
#include <QBoxLayout>

// StateS classes
#include "equationpartmimedata.h"
#include "equation.h"
#include "operand.h"
#include "inverterbarwidget.h"
#include "equationeditordialog.h"
#include "rangeeditor.h"
#include "constanteditorwidget.h"
#include "variableeditorwidget.h"
#include "nulleditorwidget.h"


EquationEditorWidget::EquationEditorWidget(shared_ptr<Equation> equation, uint rankInParentOperands, bool isTemplate, QWidget* parent) :
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

void EquationEditorWidget::replaceOperand(uint operandRank, shared_ptr<Equation> newOperand)
{
	if (this->equation == nullptr) return;


	this->equation->setOperand(operandRank, newOperand);
	this->updateOperandWidget(operandRank);

	if (this->equation->getOperatorType() == OperatorType_t::extractOp)
	{
		this->fixExtractorRange();
	}
}

void EquationEditorWidget::replaceOperand(uint operandRank, LogicValue newConstant, bool isProcessingDrop)
{
	if (this->equation == nullptr) return;


	this->equation->setOperand(operandRank, newConstant);
	this->updateOperandWidget(operandRank);

	if (this->equation->getOperatorType() == OperatorType_t::extractOp)
	{
		this->fixExtractorRange();
	}

	if (isProcessingDrop == true)
	{
		auto newOperand = dynamic_cast<ConstantEditorWidget*>(this->operands.at(operandRank));
		if (newOperand != nullptr)
		{
			newOperand->beginEdit();
		}
	}
}

void EquationEditorWidget::replaceOperand(uint operandRank, componentId_t newVariable)
{
	if (this->equation == nullptr) return;


	this->equation->setOperand(operandRank, newVariable);
	this->updateOperandWidget(operandRank);

	if (this->equation->getOperatorType() == OperatorType_t::extractOp)
	{
		this->fixExtractorRange();
	}
}

void EquationEditorWidget::clearOperand(uint operandRank)
{
	if (this->equation == nullptr) return;


	this->equation->clearOperand(operandRank);
	this->updateOperandWidget(operandRank);

	if (this->equation->getOperatorType() == OperatorType_t::extractOp)
	{
		this->fixExtractorRange();
	}
}

shared_ptr<Equation> EquationEditorWidget::getLogicEquation() const
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

void EquationEditorWidget::beginEdit()
{
	if (this->rangeEditor != nullptr)
	{
		this->rangeEditor->setMode(RangeEditor::Mode_t::editor_mode);
	}
}

bool EquationEditorWidget::validEdit()
{
	if (this->equation == nullptr) return false;


	if (this->rangeEditor != nullptr)
	{
		bool editValidated = this->rangeEditor->setMode(RangeEditor::Mode_t::compact_mode, true);
		if (editValidated == true)
		{
			return true;
		}
	}

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


	if (this->rangeEditor != nullptr)
	{
		bool editCanceled = this->rangeEditor->setMode(RangeEditor::Mode_t::compact_mode, false);
		if (editCanceled == true)
		{
			return true;
		}
	}

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


	uint allowedActions = (uint)ContextAction_t::Remove;

	switch(this->equation->getOperatorType())
	{
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		allowedActions |= (uint)ContextAction_t::IncreaseOperandCount;
		if (this->equation->getOperandCount() > 2)
		{
			allowedActions |= (uint)ContextAction_t::DecreaseOperandCount;
		}
		break;
	case OperatorType_t::extractOp:
		allowedActions |= (uint)ContextAction_t::Edit;
		if (this->equation->getRangeR() != -1)
		{
			allowedActions |= (uint)ContextAction_t::ExtractSwitchSingle;
		}
		else
		{
			auto operand = this->equation->getOperand(0);
			if (operand->getInitialValue().getSize() > 1)
			{
				allowedActions |= (uint)ContextAction_t::ExtractSwitchRange;
			}
		}
		break;
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		break;
	}

	switch(this->equation->getOperatorType())
	{
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::concatOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		allowedActions |= (uint)ContextAction_t::Invert;
		break;
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::notOp:
		allowedActions |= (uint)ContextAction_t::RemoveInverter;
		break;
	case OperatorType_t::identity:
		break;
	}

	return allowedActions;
}

uint EquationEditorWidget::getAllowedDropActions() const
{
	if (this->equation == nullptr) return 0;


	uint allowedActions = (uint)DropAction_t::ReplaceExisting |
	                      (uint)DropAction_t::ExistingAsOperand;

	switch(this->equation->getOperatorType())
	{
	case OperatorType_t::notOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
		allowedActions |= (uint)DropAction_t::RemoveInverter;
		break;
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::concatOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::identity:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
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


	switch (this->equation->getComputationFailureCause())
	{
	case EquationComputationFailureCause_t::nofail:
		return QString(tr("Equation:") + " " + this->equation->getColoredText());
		break;
	case EquationComputationFailureCause_t::nullOperand:
		return "<b>" + tr("Error:") + "</b> " + tr("One of the operands is empty");
		break;
	case EquationComputationFailureCause_t::invalidOperand:
		return "<b>" + tr("Error:") + "</b> " + tr("One of the operands is invalid");
		break;
	case EquationComputationFailureCause_t::sizeMismatch:
		return "<b>" + tr("Error:") + "</b> " + tr("The sizes of the operands do not match between each other");
		break;
	case EquationComputationFailureCause_t::missingParameter:
		return "<b>" + tr("Error:") + "</b> " + tr("A parameter is missing its value");
		break;
	case EquationComputationFailureCause_t::incorrectParameter:
		return "<b>" + tr("Error:") + "</b> " + tr("The value of a parameter is incorrect");
		break;
	}
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


	if (this->equation->getComputationFailureCause() == EquationComputationFailureCause_t::nofail)
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

	uint availableActions = (uint)DropAction_t::ReplaceExisting;
	if (this->equation->getOperatorType() == OperatorType_t::notOp)
	{
		availableActions |= (uint)DropAction_t::RemoveInverter;
	}
	if (this->equation->getOperatorType() != OperatorType_t::extractOp)
	{
		availableActions |= (uint)DropAction_t::ExistingAsOperand;
	}

	QMimeData* mimeData = new EquationPartMimeData(this->getTemplateText(), availableActions, this->equation->clone());

	drag->setMimeData(mimeData);

	// If this is an extract operator, drag image will not match template display: create a correct equation
	if (this->equation->getOperatorType() == OperatorType_t::extractOp)
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


	switch (action)
	{
	case ContextAction_t::IncreaseOperandCount:
		this->equation->increaseOperandCount();
		this->clear();
		this->buildCompleteEquation();
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
			this->clear();
			this->buildCompleteEquation();
		}
		break;
	}
	case ContextAction_t::ExtractSwitchSingle:
		if (this->rangeEditor == nullptr) return;


		this->rangeEditor->setExtractSingleBit();
		break;
	case ContextAction_t::ExtractSwitchRange:
		if (this->rangeEditor == nullptr) return;


		this->rangeEditor->setExtractRange();
		break;
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
}

void EquationEditorWidget::processSpecificDropAction(DropAction_t action)
{
	switch (action)
	{
	case DropAction_t::ExistingAsOperand:
		if (this->tempEquation == nullptr) return;

		if (this->equation == nullptr) return;


		this->tempEquation->setOperand(0, this->equation);

		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
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

void EquationEditorWidget::rangeEditorBeginEditEventHandler()
{
	this->closeOpenEditors();
}

void EquationEditorWidget::equationChangedEventHandler()
{
	this->setToolTip(this->getToolTipText());
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
	this->rangeEditor = nullptr;
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


	auto equationLayout = new QHBoxLayout();
	equationLayout->setAlignment(Qt::AlignVCenter);

	if (this->equation->getOperatorType() == OperatorType_t::concatOp)
	{
		equationLayout->addWidget(new QLabel("{"));
	}

	// First operand is always there
	auto operandWidget = this->buildOperandEditorWidget(0);
	this->operands.append(operandWidget);
	equationLayout->addWidget(operandWidget);

	// Handle multi-operands operators
	for (uint operandRank = 1 ; operandRank < this->equation->getOperandCount() ; operandRank++)
	{
		// Add operator
		QString operatorText;
		switch(this->equation->getOperatorType())
		{
		case OperatorType_t::andOp:
		case OperatorType_t::nandOp:
			operatorText = "•";
			break;
		case OperatorType_t::orOp:
		case OperatorType_t::norOp:
			operatorText = "+";
			break;
		case OperatorType_t::xorOp:
		case OperatorType_t::xnorOp:
			operatorText = "⊕";
			break;
		case OperatorType_t::equalOp:
			operatorText = "=";
			break;
		case OperatorType_t::diffOp:
			operatorText = "≠";
			break;
		case OperatorType_t::concatOp:
			operatorText = ":";
			break;
		case OperatorType_t::notOp:
		case OperatorType_t::identity:
		case OperatorType_t::extractOp:
			// Single operand operators
			break;
		}

		auto operatorLabel = new QLabel(operatorText);
		operatorLabel->setAlignment(Qt::AlignCenter);
		equationLayout->addWidget(operatorLabel);

		// Add operand
		auto operandWidget = this->buildOperandEditorWidget(operandRank);
		this->operands.append(operandWidget);
		equationLayout->addWidget(operandWidget);
	}

	if (this->equation->getOperatorType() == OperatorType_t::concatOp)
	{
		equationLayout->addWidget(new QLabel("}"));
	}
	else if (this->equation->getOperatorType() == OperatorType_t::extractOp)
	{
		this->rangeEditor = new RangeEditor(this->equation, this);

		equationLayout->addWidget(this->rangeEditor);

		connect(this->rangeEditor, &RangeEditor::beginEditEvent, this, &EquationEditorWidget::rangeEditorBeginEditEventHandler);
	}

	if (this->equation->isInverted() == false)
	{
		this->setLayout(equationLayout);
	}
	else
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
		switch(this->equation->getOperatorType())
		{
		case OperatorType_t::andOp:
		case OperatorType_t::orOp:
		case OperatorType_t::xorOp:
		case OperatorType_t::equalOp:
		case OperatorType_t::diffOp:
			reverseOperand = true;
			break;
		case OperatorType_t::concatOp:
		case OperatorType_t::extractOp:
		case OperatorType_t::identity:
			addInverter = true;
			break;
		case OperatorType_t::notOp:
		case OperatorType_t::nandOp:
		case OperatorType_t::norOp:
		case OperatorType_t::xnorOp:
			// Should not happen
			break;
		}
	}
	else // (invert == false)
	{
		switch(this->equation->getOperatorType())
		{
		case OperatorType_t::notOp:
			removeInverter = true;
			break;
		case OperatorType_t::nandOp:
		case OperatorType_t::norOp:
		case OperatorType_t::xnorOp:
			reverseOperand = true;
			break;
		case OperatorType_t::andOp:
		case OperatorType_t::orOp:
		case OperatorType_t::xorOp:
		case OperatorType_t::concatOp:
		case OperatorType_t::extractOp:
		case OperatorType_t::equalOp:
		case OperatorType_t::diffOp:
		case OperatorType_t::identity:
			// Should not happen
			break;
		}
	}

	if (addInverter == true)
	{
		this->tempValueNature = OperandSource_t::equation;
		this->tempEquation = make_shared<Equation>(OperatorType_t::notOp);
		this->tempEquation->setOperand(0, this->equation);

		this->replaceByTempValue();
	}
	else if (removeInverter == true)
	{
		auto operand = this->equation->getOperand(0);
		if (operand != nullptr)
		{
			switch (operand->getSource())
			{
			case OperandSource_t::variable:
				this->tempValueNature = OperandSource_t::variable;
				this->tempVariableId = operand->getVariableId();
				break;
			case OperandSource_t::equation:
				this->tempValueNature = OperandSource_t::equation;
				this->tempEquation = operand->getEquation();
				break;
			case OperandSource_t::constant:
				this->tempValueNature = OperandSource_t::constant;
				this->tempConstant = operand->getConstant();
				break;
			}
		}
		else // (operand == nullptr)
		{
			this->tempValueNature = OperandSource_t::equation;
			this->tempEquation = nullptr;
		}

		this->replaceByTempValue();
	}
	else if (reverseOperand == true)
	{
		OperatorType_t newOperator;

		switch(this->equation->getOperatorType())
		{
		case OperatorType_t::andOp:
			newOperator = OperatorType_t::nandOp;
			break;
		case OperatorType_t::orOp:
			newOperator = OperatorType_t::norOp;
			break;
		case OperatorType_t::xorOp:
			newOperator = OperatorType_t::xnorOp;
			break;
		case OperatorType_t::nandOp:
			newOperator = OperatorType_t::andOp;
			break;
		case OperatorType_t::norOp:
			newOperator = OperatorType_t::orOp;
			break;
		case OperatorType_t::xnorOp:
			newOperator = OperatorType_t::xorOp;
			break;
		case OperatorType_t::equalOp:
			newOperator = OperatorType_t::diffOp;
			break;
		case OperatorType_t::diffOp:
			newOperator = OperatorType_t::equalOp;
			break;
		case OperatorType_t::notOp:
		case OperatorType_t::concatOp:
		case OperatorType_t::extractOp:
		case OperatorType_t::identity:
			// Should not happen
			newOperator = this->equation->getOperatorType();
			break;
		}

		this->equation->setOperatorType(newOperator);
		this->clear();
		this->buildCompleteEquation();
	}
}

void EquationEditorWidget::fixExtractorRange()
{
	if (this->equation == nullptr) return;

	if (this->equation->getOperatorType() != OperatorType_t::extractOp) return;

	if (this->rangeEditor == nullptr) return;


	auto operand = this->equation->getOperand(0);

	if (operand == nullptr)
	{
		this->equation->setRange(0, -1);
	}
	else // (operand != nullptr)
	{
		auto operandValue = operand->getInitialValue();
		int operandSize = operandValue.getSize();

		if (operandSize == 1)
		{
			this->equation->setRange(0, -1);
		}
		else // (operandSize != 1)
		{
			int currentRangeL = this->equation->getRangeL();
			int currentRangeR = this->equation->getRangeR();

			if (currentRangeL > operandSize-1)
			{
				currentRangeL = operandSize-1;
			}

			if (currentRangeR == -1)
			{
				if (currentRangeL < 0)
				{
					currentRangeL = 0;
				}
			}
			else // (currentRangeR != -1)
			{
				if (currentRangeL == 0)
				{
					currentRangeL = 1;
					currentRangeR = 0;
				}
				else if (currentRangeL <= currentRangeR)
				{
					currentRangeR = currentRangeL-1;
				}
			}

			this->equation->setRange(currentRangeL, currentRangeR);
		}
	}

	this->rangeEditor->refresh();
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
		case OperandSource_t::equation:
			return new EquationEditorWidget(operand->getEquation(), operandRank, false, this);
			break;
		case OperandSource_t::variable:
			return new VariableEditorWidget(operand->getVariableId(), operandRank, false, this);
			break;
		case OperandSource_t::constant:
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

	switch(this->equation->getOperatorType())
	{
	case OperatorType_t::notOp:
		text += tr("not");
		break;
	case OperatorType_t::andOp:
		text += tr("and");
		break;
	case OperatorType_t::orOp:
		text += tr("or");
		break;
	case OperatorType_t::xorOp:
		text += tr("xor");
		break;
	case OperatorType_t::nandOp:
		text += tr("nand");
		break;
	case OperatorType_t::norOp:
		text += tr("nor");
		break;
	case OperatorType_t::xnorOp:
		text += tr("xnor");
		break;
	case OperatorType_t::equalOp:
		text += tr("Equality");
		break;
	case OperatorType_t::diffOp:
		text += tr("Difference");
		break;
	case OperatorType_t::concatOp:
		text += tr("Concatenate");
		break;
	case OperatorType_t::extractOp:
		text += "[…]";
		break;
	case OperatorType_t::identity:
		// Nothing to do: special case.
		break;
	}

	switch(this->equation->getOperatorType())
	{
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
		text += " " + QString::number(this->equation->getOperandCount());
		break;
	case OperatorType_t::notOp:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
	case OperatorType_t::concatOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::identity:
		break;
	}

	return text;
}
