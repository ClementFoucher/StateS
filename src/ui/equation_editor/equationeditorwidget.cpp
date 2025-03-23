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
#include <QMouseEvent>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "equationpartmimedata.h"
#include "equation.h"
#include "contextmenu.h"
#include "inverterbarwidget.h"
#include "equationeditordialog.h"
#include "rangeextractorwidget.h"
#include "constanteditorwidget.h"
#include "logicvalue.h"
#include "variable.h"
#include "operand.h"


// A graphic equation can either represent
// a logic equation, a logic variable or a constant

EquationEditorWidget::EquationEditorWidget(shared_ptr<Equation> equation, int operandNumber, bool isTemplate, QWidget* parent) :
	QFrame(parent)
{
	this->equation      = equation;
	this->isTemplate    = isTemplate;
	this->operandNumber = operandNumber;

	this->configure();
	this->buildEquation();
}

EquationEditorWidget::EquationEditorWidget(componentId_t variableId, int operandNumber, bool isTemplate, QWidget* parent) :
	QFrame(parent)
{
	this->localEquation = shared_ptr<Equation>(new Equation(OperatorType_t::identity));
	this->localEquation->setOperand(0, variableId);

	this->equation      = this->localEquation;
	this->isTemplate    = isTemplate;
	this->operandNumber = operandNumber;

	this->configure();
	this->buildEquation();
}

EquationEditorWidget::EquationEditorWidget(LogicValue constant, int operandNumber, bool isTemplate, QWidget* parent) :
	QFrame(parent)
{
	this->localEquation = shared_ptr<Equation>(new Equation(OperatorType_t::identity));
	this->localEquation->setOperand(0, constant);

	this->equation      = this->localEquation;
	this->isTemplate    = isTemplate;
	this->operandNumber = operandNumber;

	this->configure();
	this->buildEquation();
}

// Replaces one operand with the given one in logic equation.
// This function is called by children EquationEditorWidget to replace themselves.
// Graphic equation is rebuilt after logic equation update.
void EquationEditorWidget::replaceOperand(uint operandNumber, shared_ptr<Equation> newOperand)
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;

	if (operandNumber >= l_equation->getOperandCount()) return;


	if (newOperand == nullptr)
	{
		l_equation->clearOperand(operandNumber);
	}
	else if (newOperand->getOperatorType() != OperatorType_t::identity)
	{
		l_equation->setOperand(operandNumber, newOperand);
	}
	else // (newOperand->getOperatorType() == OperatorType_t::identity)
	{
		auto newOperandOperand = newOperand->getOperand(0);
		if (newOperandOperand != nullptr)
		{
			if (newOperandOperand->getSource() == OperandSource_t::variable)
			{
				l_equation->setOperand(operandNumber, newOperandOperand->getVariableId());
			}
			else if (newOperandOperand->getSource() == OperandSource_t::constant)
			{
				l_equation->setOperand(operandNumber, newOperandOperand->getConstant());
			}
		}
	}

	this->buildEquation();
}

// Register the new constant value of a child without replacing it
void EquationEditorWidget::replaceOperandConstantValue(uint operandNumber, LogicValue newValue)
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;


	if (operandNumber >= l_equation->getOperandCount()) return;

	l_equation->setOperand(operandNumber, newValue);
}

void EquationEditorWidget::setVariableLock(bool lock)
{
	this->lockVariable = lock;
	// We have to rebuild the equation to pass the value to the children
	this->buildEquation();
}

// Returns the equation currently represented by the graphic object.
// This is typically a pointer to a part of a logic equation,
// unless we are top level, then the whole equation is returned.
shared_ptr<Equation> EquationEditorWidget::getLogicEquation() const
{
	if (rootEquation != nullptr)
	{
		return rootEquation;
	}
	else
	{
		return equation.lock();
	}
}

/**
 * @brief EquationEditorWidget::forceCompleteRendering is for the case
 * where template equation differs from grab display.
 * Setting this changes the display.
 */
void EquationEditorWidget::forceCompleteRendering()
{
	this->completeRendering = true;
	this->buildEquation();
}

bool EquationEditorWidget::validEdit()
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return false;


	bool result  = false;
	if (this->editorWidget != nullptr)
	{
		return this->editorWidget->validEdit();
	}
	else
	{
		for (QObject* child : this->children())
		{
			EquationEditorWidget* operand = dynamic_cast<EquationEditorWidget*>(child);
			if (operand != nullptr)
			{
				result = operand->validEdit();
				if (result == true) break;
			}
		}
	}

	return result;
}

bool EquationEditorWidget::cancelEdit()
{
	shared_ptr<Equation> l_equation = dynamic_pointer_cast<Equation>(this->equation.lock());
	if (l_equation == nullptr) return false;


	bool result  = false;
	if (this->editorWidget != nullptr)
	{
		return this->editorWidget->cancelEdit();
	}
	else
	{
		for (QObject* child : this->children())
		{
			EquationEditorWidget* operand = dynamic_cast<EquationEditorWidget*>(child);
			if (operand != nullptr)
			{
				result = operand->cancelEdit();
				if (result == true) break;
			}
		}
	}

	return result;
}

// Triggered when mouse enters widget
void EquationEditorWidget::enterEvent(QEnterEvent*)
{
	EquationEditorWidget* parentEquation = this->parentEquation();

	if (parentEquation != nullptr)
	{
		parentEquation->enterChildrenEvent();
	}

	this->setHilightedBorderColor();
}

// Triggered when mouse leaves widget
// (not trigerred when children is entered)
void EquationEditorWidget::leaveEvent(QEvent*)
{
	EquationEditorWidget* parentEquation = this->parentEquation();

	if (parentEquation != nullptr)
	{
		parentEquation->leaveChildrenEvent();
	}

	this->setDefaultBorderColor();
}

// Generate drag-n-drop image when mouse is pressed
void EquationEditorWidget::mousePressEvent(QMouseEvent* event)
{
	if (this->isTemplate == true)
	{
		if (event->button() == Qt::LeftButton)
		{
			QDrag* drag = new QDrag(this);
			auto equation = this->getLogicEquation();
			QMimeData* mimeData = new EquationPartMimeData(equation->clone());

			drag->setMimeData(mimeData);

			auto l_equation = this->equation.lock();

			// Drag image may not match template display: create a correct equation
			if ( (l_equation != nullptr) && (l_equation->getOperatorType() == OperatorType_t::extractOp) )
			{
				EquationEditorWidget displayGraphicEquation(l_equation->clone(), true);
				displayGraphicEquation.forceCompleteRendering();

				drag->setPixmap(displayGraphicEquation.grab());
			}
			else if (l_equation->getOperatorType() == OperatorType_t::identity)
			{
				auto operand = l_equation->getOperand(0);
				if (operand == nullptr) return;


				auto operandSource = operand->getSource();
				if (operandSource == OperandSource_t::variable) // This is a simple variable => do not use template
				{
					EquationEditorWidget displayGraphicEquation(l_equation->getOperand(0)->getVariableId(), -1);

					drag->setPixmap(displayGraphicEquation.grab());
				}
				else if (operandSource == OperandSource_t::constant)
				{
					drag->setPixmap(this->grab());
				}
			}
			else
			{
				drag->setPixmap(this->grab());
			}

			this->inMouseEvent = true;
			drag->exec();
		}
	}
	else // (this->isTemplate == false)
	{
		if (event->button() == Qt::LeftButton)
		{
			auto l_equation = this->equation.lock();

			if ( (l_equation != nullptr) && (l_equation->getOperatorType() == OperatorType_t::identity) )
			{
				auto operand = l_equation->getOperand(0);
				if (operand != nullptr)
				{
					auto operandSource = operand->getSource();
					if (operandSource == OperandSource_t::constant)
					{
						if (this->editorWidget != nullptr)
						{
							this->editorWidget->setEdited(true);
						}

						this->inMouseEvent = true;
					}
				}
			}
		}
	}

	if (this->inMouseEvent == false)
	{
		QFrame::mousePressEvent(event);
	}
}

void EquationEditorWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (this->inMouseEvent == false)
	{
		QFrame::mouseMoveEvent(event);
	}
}

void EquationEditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (this->inMouseEvent == false)
	{
		QFrame::mouseMoveEvent(event);
	}
	else
	{
		this->inMouseEvent = false;
	}
}

void EquationEditorWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (this->inMouseEvent == false)
	{
		QFrame::mouseMoveEvent(event);
	}
}

// Triggered when mouse enters widget while dragging an object
void EquationEditorWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (this->isTemplate == false)
	{
		this->setHilightedBorderColor();

		// Check if object is actually a graphic equation
		const EquationPartMimeData* mimeData = dynamic_cast<const EquationPartMimeData*>(event->mimeData());
		if (mimeData != nullptr)
		{
			event->acceptProposedAction();
		}
	}
}

// Trigerred when mouse leaves widget while dragging an object
// (also trigerred when child is entered)
void EquationEditorWidget::dragLeaveEvent(QDragLeaveEvent*)
{
	this->setDefaultBorderColor();
}

void EquationEditorWidget::dropEvent(QDropEvent* event)
{
	const EquationPartMimeData* mimeData = dynamic_cast<const EquationPartMimeData*>(event->mimeData());
	if (mimeData == nullptr) return;

	this->droppedEquation = mimeData->getEquation();
	if (this->droppedEquation == nullptr) return;


	auto l_equation = this->equation.lock();
	if (l_equation == nullptr)
	{
		// Automatically replace empty operands
		this->replaceEquation(this->droppedEquation);
	}
	else
	{
		// Ask what to do
		ContextMenu* menu = new ContextMenu();
		menu->addTitle(tr("What should I do?"));
		menu->addSubTitle(tr("Existing equation:") +  " <i>"  + l_equation->getText() + "</i>");
		menu->addSubTitle(tr("Dropped equation:") + " <i> " + this->droppedEquation->getText() + "</i>");

		QVariant data;
		QAction* a = new QAction(menu);
		a->setText(tr("Replace existing equation by dropped equation"));
		a->setToolTip(tr("New equation would be:") + " <i> " + this->droppedEquation->getText() + "</i>");
		data.setValue((int)DropAction_t::ReplaceExisting);
		a->setData(data);
		menu->addAction(a);

		if ( (this->droppedEquation->getOperatorType() != OperatorType_t::identity) &&
		     (this->droppedEquation->getOperatorType() != OperatorType_t::extractOp) )
		{
			QString actionText = tr("Set existing equation as operand of dropped equation");

			a = new QAction(menu);
			a->setText(actionText);
			data.setValue((int)DropAction_t::ExistingAsOperand);
			a->setData(data);

			// Build tooltip
			auto newEquation = this->droppedEquation->clone();
			newEquation->setOperand(0, l_equation);

			a->setToolTip(tr("New equation would be: ") + "<br /><i>" + newEquation->getText() + "</i>");
			newEquation.reset();

			menu->addAction(a);
		}

		a = menu->addAction(tr("Cancel"));
		data.setValue((int)CommonAction_t::Cancel);
		a->setData(data);

		menu->popup(this->mapToGlobal(event->position().toPoint()));

		connect(menu, &QMenu::triggered, this, &EquationEditorWidget::treatMenuEventHandler);
	}

	event->acceptProposedAction();
}

// Triggerd by right-click
void EquationEditorWidget::contextMenuEvent(QContextMenuEvent* event)
{
	if (this->isTemplate == true) return;

	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;

	auto operatorType = l_equation->getOperatorType();
	if ( (this->lockVariable == true) && (operatorType != OperatorType_t::extractOp) ) return;


	if (operatorType != OperatorType_t::identity)
	{
		ContextMenu* menu = new ContextMenu();
		menu->addTitle(tr("Equation:") +  " <i>" + l_equation->getText() + "</i>");

		QVariant data;
		QAction* addedAction = nullptr;

		switch(operatorType)
		{
		case OperatorType_t::andOp:
		case OperatorType_t::orOp:
		case OperatorType_t::xorOp:
		case OperatorType_t::nandOp:
		case OperatorType_t::norOp:
		case OperatorType_t::xnorOp:
		case OperatorType_t::concatOp:
			addedAction = menu->addAction(tr("Add one operand to that operator"));
			data.setValue((int)ContextAction_t::IncrementOperandCount);
			addedAction->setData(data);

			if (l_equation->getOperandCount() > 2)
			{
				addedAction = menu->addAction(tr("Remove one operand from that operator"));
				data.setValue((int)ContextAction_t::DecrementOperandCount);
				addedAction->setData(data);
			}
			break;
		case OperatorType_t::extractOp:

			if (l_equation->getRangeR() == -1)
				addedAction = menu->addAction(tr("Edit index"));
			else
				addedAction = menu->addAction(tr("Edit range"));
			data.setValue((int)ContextAction_t::EditRange);
			addedAction->setData(data);

			menu->addSeparator();

			addedAction = menu->addAction(tr("Extract single bit"));
			addedAction->setCheckable(true);
			if (l_equation->getRangeR() == -1)
				addedAction->setChecked(true);
			data.setValue((int)ContextAction_t::ExtractSwitchSingle);
			addedAction->setData(data);

			addedAction = menu->addAction(tr("Extract range"));
			addedAction->setCheckable(true);
			if (l_equation->getRangeR() != -1)
				addedAction->setChecked(true);
			data.setValue((int)ContextAction_t::ExtractSwitchRange);
			addedAction->setData(data);

			break;
		case OperatorType_t::notOp:
		case OperatorType_t::equalOp:
		case OperatorType_t::diffOp:
		case OperatorType_t::identity:
			// Nothing
			break;
		}

		menu->addSeparator();

		if (this->lockVariable == false)
		{
			addedAction = menu->addAction(tr("Delete"));
			data.setValue((int)ContextAction_t::DeleteEquation);
			addedAction->setData(data);
		}

		addedAction = menu->addAction(tr("Cancel"));
		data.setValue((int)CommonAction_t::Cancel);
		addedAction->setData(data);

		menu->popup(this->mapToGlobal(event->pos()));

		connect(menu, &QMenu::triggered, this, &EquationEditorWidget::treatMenuEventHandler);
	}
	else // (operatorType == OperatorType_t::identity)
	{
		auto operand = l_equation->getOperand(0);
		if (operand == nullptr) return;


		ContextMenu* menu = new ContextMenu();
		QVariant data;
		QAction* addedAction = nullptr;

		auto operandSource = operand->getSource();
		if (operandSource == OperandSource_t::constant)
		{
			menu->addTitle(tr("Constant:") +  " <i>" + l_equation->getText() + "</i>");

			addedAction = menu->addAction(tr("Edit constant value"));
			data.setValue((int)ContextAction_t::EditValue);
			addedAction->setData(data);

			menu->addSeparator();
		}
		else
		{
			menu->addTitle(tr("Variable:") +  " <i>" + l_equation->getText() + "</i>");
		}

		addedAction = menu->addAction(tr("Delete"));
		data.setValue((int)ContextAction_t::DeleteEquation);
		addedAction->setData(data);

		addedAction = menu->addAction(tr("Cancel"));
		data.setValue((int)CommonAction_t::Cancel);
		addedAction->setData(data);

		menu->popup(this->mapToGlobal(event->pos()));

		connect(menu, &QMenu::triggered, this, &EquationEditorWidget::treatMenuEventHandler);
	}
}

void EquationEditorWidget::enterChildrenEvent()
{
	this->setDefaultBorderColor();
}

void EquationEditorWidget::leaveChildrenEvent()
{
	this->setHilightedBorderColor();
}

void EquationEditorWidget::treatMenuEventHandler(QAction* action)
{
	auto l_equation  = this->equation.lock();
	if (l_equation == nullptr) return;


	QVariant data = action->data();
	int dataValue = data.toInt();

	switch (dataValue)
	{
	case CommonAction_t::Cancel:
		break;

	case DropAction_t::ReplaceExisting:

		// Nothing should be done after that line because it
		// will cause parent to rebuild, deleting this
		this->replaceEquation(this->droppedEquation);

		break;

	case DropAction_t::ExistingAsOperand:
	{
		auto newEquation = this->droppedEquation->clone();
		newEquation->setOperand(0, l_equation);

		// Nothing should be done after that line because it
		// will cause parent to rebuild, deleting this
		this->replaceEquation(newEquation);

		break;
	}
	case ContextAction_t::DeleteEquation:
	{
		bool valid = false;

		if (l_equation != nullptr)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete equation?") + "<br />" + tr("Content is:") + " " + l_equation->getText(), QMessageBox::Ok | QMessageBox::Cancel);
			if (reply == QMessageBox::StandardButton::Ok)
			{
				valid = true;
			}
		}
		else
		{
			valid = true;
		}

		if (valid)
		{
			this->replaceEquation(nullptr);
		}

		break;
	}
	case ContextAction_t::IncrementOperandCount:

		if (l_equation != nullptr)
		{
			l_equation->increaseOperandCount();

			this->buildEquation();
		}

		break;

	case ContextAction_t::DecrementOperandCount:

		if (l_equation != nullptr)
		{
			bool valid = false;

			auto operand = l_equation->getOperand(l_equation->getOperandCount() - 1);

			if (operand->getSource() == OperandSource_t::equation)
			{
				auto equationOperand = operand->getEquation();
				QMessageBox::StandardButton reply;
				reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete last oprand?") + "<br />" + tr("Content is:") + " " + equationOperand->getText(), QMessageBox::Ok | QMessageBox::Cancel);

				if (reply == QMessageBox::StandardButton::Ok)
				{
					valid = true;
				}
			}
			else
			{
				valid = true;
			}

			if (valid)
			{
				l_equation->decreaseOperandCount();

				this->buildEquation();
			}
		}

		break;

	case ContextAction_t::EditRange:
		if (this->editorWidget != nullptr)
		{
			this->editorWidget->setEdited(true);
		}
		break;
	case ContextAction_t::ExtractSwitchSingle:
		if (l_equation->getOperatorType() == OperatorType_t::extractOp)
		{
			l_equation->setRange(l_equation->getRangeL());
		}
		break;
	case ContextAction_t::ExtractSwitchRange:
		if (l_equation->getOperatorType() == OperatorType_t::extractOp)
		{
			l_equation->setRange(l_equation->getRangeL(), 0);
		}
		break;
	case ContextAction_t::EditValue:
		if (this->editorWidget != nullptr)
		{
			this->editorWidget->setEdited(true);
		}
		break;
	}
}

void EquationEditorWidget::treatRangeLeftBoundChanged(int newIndex)
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;


	if (l_equation->getOperatorType() == OperatorType_t::extractOp)
	{
		l_equation->setRange(newIndex, l_equation->getRangeR());
	}
}

void EquationEditorWidget::treatRangeRightBoundChanged(int newIndex)
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;


	if (l_equation->getOperatorType() == OperatorType_t::extractOp)
	{
		l_equation->setRange(l_equation->getRangeL(), newIndex);
	}
}

void EquationEditorWidget::treatConstantValueChanged(LogicValue newValue)
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;

	if (l_equation->getOperatorType() != OperatorType_t::identity) return;

	auto operand = l_equation->getOperand(0);
	if (operand == nullptr) return;

	auto operandSource = operand->getSource();
	if (operandSource != OperandSource_t::constant) return;


	l_equation->setOperand(0, newValue);

	EquationEditorWidget* parentEquation = this->parentEquation();
	if (parentEquation != nullptr)
	{
		// Register constant value change in parent equation
		parentEquation->replaceOperandConstantValue((uint)this->operandNumber, newValue);
	}
}

void EquationEditorWidget::updateBorder()
{
	if (this->mouseIn == true)
	{
		this->setHilightedBorderColor();
	}
	else
	{
		this->setDefaultBorderColor();
	}
}

void EquationEditorWidget::configure()
{
	if (this->isTemplate == false)
	{
		this->setAcceptDrops(true);
	}

	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;


	if (this->parentEquation() == nullptr)
	{
		// We are top level
		this->rootEquation = l_equation;
	}

	if (this->isTemplate == false)
	{
		connect(l_equation.get(), &Equation::equationInitialValueChangedEvent, this, &EquationEditorWidget::updateBorder);
	}
}

// Set passive border color:
// neutral unless current equation is incorrect
void EquationEditorWidget::setDefaultBorderColor()
{
	this->mouseIn = false;

	if (this->isTemplate == true)
	{
		this->setStyleSheet("EquationEditorWidget {border: 1px solid lightgrey; border-radius: 10px}");
	}
	else
	{
		auto l_equation = this->equation.lock();

		if ( (l_equation == nullptr) || (l_equation->getInitialValue().isNull()) )
		{
			this->setStyleSheet("EquationEditorWidget {border: 1px solid red; border-radius: 10px}");

			if (l_equation == nullptr)
			{
				this->setToolTip(tr("Empty operand"));
			}
			else if (l_equation->getOperatorType() != OperatorType_t::identity)
			{
				if (l_equation->getComputationFailureCause() == EquationComputationFailureCause_t::nullOperand)
					this->setToolTip(tr("Empty operand"));
				else if (l_equation->getComputationFailureCause() == EquationComputationFailureCause_t::incompleteOperand)
					this->setToolTip(tr("One operand is not correct"));
				else if (l_equation->getComputationFailureCause() == EquationComputationFailureCause_t::sizeMismatch)
					this->setToolTip(tr("The sizes of operands do not match between each other"));
				else if (l_equation->getComputationFailureCause() == EquationComputationFailureCause_t::missingParameter)
					this->setToolTip(tr("A parameter of the equation is missing value"));
				else if (l_equation->getComputationFailureCause() == EquationComputationFailureCause_t::incorrectParameter)
					this->setToolTip(tr("A parameter of the equation has an invalid value"));
			}
		}
		else
		{
			this->setStyleSheet("EquationEditorWidget {border: 1px solid lightgrey; border-radius: 10px}");
		}
	}
}

// Set active border color
void EquationEditorWidget::setHilightedBorderColor()
{
	this->mouseIn = true;
	this->setStyleSheet("EquationEditorWidget {border: 1px solid blue; border-radius: 10px}");
}

// Handle user input to replace equation.
// This is done by telling parent to replace ourself by
// new equation, unless we are top level.
// Graphic equation is rebuilt by parent, which destoys
// current object.
void EquationEditorWidget::replaceEquation(shared_ptr<Equation> newEquation)
{
	// Find equation parent
	auto parentEquation = this->parentEquation();

	if (parentEquation == nullptr)
	{
		auto l_equation = this->equation.lock();

		if (l_equation != nullptr)
		{
			disconnect(l_equation.get(), &Equation::equationInitialValueChangedEvent, this, &EquationEditorWidget::updateBorder);
		}

		// We are top level
		this->equation = newEquation;
		this->rootEquation = newEquation;
		// For variable and constant equation, no need to store localEquation
		// as we already have a reference through rootEquation.

		if (newEquation != nullptr)
		{
			connect(newEquation.get(), &Equation::equationInitialValueChangedEvent, this, &EquationEditorWidget::updateBorder);
		}

		this->buildEquation();
	}
	else if (this->operandNumber >= 0)
	{
		// Parent will handle replacement
		parentEquation->replaceOperand((uint)this->operandNumber, newEquation);
	}
}

// Builds graphic representation of equation
// based on current logic equation pointer
void EquationEditorWidget::buildEquation()
{
	// Clear previous content
	QObjectList toDelete = this->children();
	qDeleteAll(toDelete);

	if (this->equation.expired() == false)
	{
		if ( (this->isTemplate == true) && (this->completeRendering == false) )
		{
			this->buildTemplateEquation();
		}
		else
		{
			this->buildCompleteEquation();
		}
	}
	else
	{
		// Empty equation
		QHBoxLayout* emptyLayout = new QHBoxLayout();
		QLabel* emptyText = new QLabel("…");

		emptyText->setAlignment(Qt::AlignCenter);
		emptyLayout->addWidget(emptyText);
		this->setLayout(emptyLayout);
	}

	this->setDefaultBorderColor();
}

void EquationEditorWidget::buildTemplateEquation()
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;


	auto operatorType = l_equation->getOperatorType();
	if (operatorType != OperatorType_t::identity)
	{
		QHBoxLayout* equationLayout = new QHBoxLayout();
		QString text;

		switch(operatorType)
		{
		case OperatorType_t::notOp:
			text += "not";
			break;
		case OperatorType_t::andOp:
			text += "and";
			break;
		case OperatorType_t::orOp:
			text += "or";
			break;
		case OperatorType_t::xorOp:
			text += "xor";
			break;
		case OperatorType_t::nandOp:
			text += "nand";
			break;
		case OperatorType_t::norOp:
			text += "nor";
			break;
		case OperatorType_t::xnorOp:
			text += "xnor";
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
			// Handled in the other branch of the if
			break;
		}

		switch(operatorType)
		{
		case OperatorType_t::andOp:
		case OperatorType_t::orOp:
		case OperatorType_t::xorOp:
		case OperatorType_t::nandOp:
		case OperatorType_t::norOp:
		case OperatorType_t::xnorOp:
			text += " " + QString::number(l_equation->getOperandCount());
			break;
		case OperatorType_t::notOp:
		case OperatorType_t::equalOp:
		case OperatorType_t::diffOp:
		case OperatorType_t::concatOp:
		case OperatorType_t::extractOp:
		case OperatorType_t::identity:
			break;
		}

		QLabel* variableText = new QLabel(text);
		variableText->setAlignment(Qt::AlignCenter);
		equationLayout->addWidget(variableText);

		this->setLayout(equationLayout);
	}
	else // (operatorType == OperatorType_t::identity)
	{
		auto operand = l_equation->getOperand(0);
		if (operand == nullptr) return;


		auto operandSource = operand->getSource();
		if (operandSource == OperandSource_t::variable)
		{
			this->buildVariableEquation();
		}
		else if (operandSource == OperandSource_t::constant)
		{
			QHBoxLayout* equationLayout = new QHBoxLayout();
			QString text = tr("Custom value");

			QLabel* variableText = new QLabel(text);
			variableText->setAlignment(Qt::AlignCenter);
			equationLayout->addWidget(variableText);

			this->setLayout(equationLayout);
		}
	}
}

void EquationEditorWidget::buildVariableEquation()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;

	auto operatorType = l_equation->getOperatorType();
	if (operatorType != OperatorType_t::identity) return;

	auto operand = l_equation->getOperand(0);
	if (operand == nullptr) return;

	auto variableId = operand->getVariableId();

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	QHBoxLayout* equationLayout = new QHBoxLayout();
	QLabel* variableText = nullptr;

	variableText = new QLabel(variable->getName());
	this->setToolTip(tr("The variable") + " " + variable->getName() + " " + tr("is size") + " " + QString::number(variable->getSize()));

	variableText->setAlignment(Qt::AlignCenter);
	equationLayout->addWidget(variableText);

	if ( (this->isTemplate == true) && (variable->getSize() > 1) )
	{
		// Add a sub-widget to allow directly dragging sub-range from variable
		shared_ptr<Equation> extractor = shared_ptr<Equation>(new Equation(OperatorType_t::extractOp));
		extractor->setOperand(0, variableId);

		EquationEditorWidget* extractorWidget = new EquationEditorWidget(extractor, -1, true);
		equationLayout->addWidget(extractorWidget);
	}

	this->setLayout(equationLayout);
}

void EquationEditorWidget::buildCompleteEquation()
{
	auto l_equation = this->equation.lock();
	if (l_equation == nullptr) return;


	auto operatorType = l_equation->getOperatorType();
	if (operatorType != OperatorType_t::identity)
	{
		QHBoxLayout* equationLayout = new QHBoxLayout();

		if (l_equation->getOperatorType() == OperatorType_t::concatOp)
		{
			equationLayout->addWidget(new QLabel("{"));
		}

		for (uint i = 0 ; i < l_equation->getOperandCount() ; i++)
		{
			auto operand = l_equation->getOperand(i);

			// Add operand
			EquationEditorWidget* graphicOperand = nullptr;
			if (operand == nullptr)
			{
				graphicOperand = new EquationEditorWidget(shared_ptr<Equation>(nullptr), i, false, this);
			}
			else // (operand != nullptr)
			{
				switch (operand->getSource())
				{
				case OperandSource_t::equation:
					graphicOperand = new EquationEditorWidget(operand->getEquation(), i, false, this);
					break;
				case OperandSource_t::variable:
					graphicOperand = new EquationEditorWidget(operand->getVariableId(), i, false, this);
					break;
				case OperandSource_t::constant:
					graphicOperand = new EquationEditorWidget(operand->getConstant(), i, false, this);
					break;
				}
			}

			if (this->lockVariable == true)
			{
				graphicOperand->setVariableLock(true);
			}
			equationLayout->addWidget(graphicOperand);

			// Add operator, except for last operand
			if (i < l_equation->getOperandCount() - 1)
			{
				QString operatorText;

				switch(operatorType)
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
					// No intermediate sign
					break;
				}

				if (operatorText.isEmpty() == false)
				{
					QLabel* operatorLabel = new QLabel(operatorText);
					operatorLabel->setAlignment(Qt::AlignCenter);
					equationLayout->addWidget(operatorLabel);
				}
			}
		}

		if (operatorType == OperatorType_t::concatOp)
		{
			equationLayout->addWidget(new QLabel("}"));
		}

		if (operatorType == OperatorType_t::extractOp)
		{
			auto rangeExtractor = new RangeExtractorWidget(l_equation, this);

			if (this->isTemplate == false)
			{
				connect(rangeExtractor, &RangeExtractorWidget::rangeLChanged, this, &EquationEditorWidget::treatRangeLeftBoundChanged);
				connect(rangeExtractor, &RangeExtractorWidget::rangeRChanged, this, &EquationEditorWidget::treatRangeRightBoundChanged);
			}

			equationLayout->addWidget(rangeExtractor);

			this->editorWidget = rangeExtractor;
		}

		if (l_equation->isInverted() == false)
		{
			this->setLayout(equationLayout);
		}
		else // (l_equation->isInverted() == true)
		{
			QVBoxLayout* verticalLayout = new QVBoxLayout();

			InverterBarWidget* inverterBar = new InverterBarWidget();

			verticalLayout->addWidget(inverterBar);
			verticalLayout->addLayout(equationLayout);

			this->setLayout(verticalLayout);
		}

		this->setToolTip(tr("This equation") + " " + tr("is size") + " " + QString::number(l_equation->getSize()));
	}
	else // (operatorType == OperatorType_t::identity)
	{
		auto operand = l_equation->getOperand(0);
		if (operand == nullptr) return;


		auto operandSource = operand->getSource();
		if (operandSource == OperandSource_t::variable)
		{
			this->buildVariableEquation();
		}
		else if (operandSource == OperandSource_t::constant)
		{
			QHBoxLayout* equationLayout = new QHBoxLayout();
			ConstantEditorWidget* constantSetter = new ConstantEditorWidget(l_equation->getInitialValue(), this);
			equationLayout->addWidget(constantSetter);

			if (!this->isTemplate)
			{
				connect(constantSetter, &ConstantEditorWidget::valueChanged, this, &EquationEditorWidget::treatConstantValueChanged);
			}

			this->editorWidget = constantSetter;
			this->setLayout(equationLayout);

			this->setToolTip(tr("This constant") + " " + tr("is size") + " " + QString::number(l_equation->getSize()));
		}
	}
}

// Scans QWidget hierarchy to find closest EquationEditorWidget parent.
// Returns nullptr if we are top level in this hierarchy.
EquationEditorWidget* EquationEditorWidget::parentEquation() const
{
	EquationEditorWidget* parentEquation = nullptr;

	QWidget* parentWidget = this->parentWidget();
	while (parentWidget != nullptr)
	{
		parentEquation = dynamic_cast<EquationEditorWidget*>(parentWidget);
		if (parentEquation != nullptr) break; // Found

		// No need to go beyond equation editor
		auto editor = dynamic_cast<EquationEditorDialog*>(parentWidget);
		if (editor != nullptr) break; // Not found

		parentWidget = parentWidget->parentWidget();
	}

	return parentEquation;
}
