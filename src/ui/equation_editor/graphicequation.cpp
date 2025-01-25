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
#include "graphicequation.h"

// Qt classes
#include <QDrag>
#include <QMouseEvent>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>

// Debug
#include <QDebug>

// StateS classes
#include "equationmimedata.h"
#include "equation.h"
#include "contextmenu.h"
#include "inverterbar.h"
#include "equationeditor.h"
#include "rangeextractorwidget.h"
#include "constantvaluesetter.h"
#include "logicvalue.h"


// A graphic equation can either represent
// a logic equation or a logic signal

GraphicEquation::GraphicEquation(shared_ptr<Variable> equation, bool isTemplate, bool lockSignal, QWidget* parent) :
    QFrame(parent)
{
	this->equation   = equation;
	this->isTemplate = isTemplate;
	this->lockSignal = lockSignal;

	if (!isTemplate)
		this->setAcceptDrops(true);

	if (this->parentEquation() == nullptr)
		// We are top level
		this->rootEquation = equation;

	if (equation != nullptr)
		connect(equation.get(), &Variable::variableStaticConfigurationChangedEvent, this, &GraphicEquation::updateBorder);

	this->buildEquation();
}

// Builds graphic representation of equation
// based on current logic equation pointer
void GraphicEquation::buildEquation()
{
	// Clear previous content
	QObjectList toDelete = this->children();
	qDeleteAll(toDelete);

	if (!this->equation.expired())
	{
		if ( (this->isTemplate) && (! this->completeRendering) )
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

void GraphicEquation::buildTemplateEquation()
{
	shared_ptr<Variable> equationAsSignal = this->equation.lock();
	shared_ptr<Equation> equationAsEquation = dynamic_pointer_cast<Equation> (equationAsSignal);

	if (equationAsEquation != nullptr)
	{
		QHBoxLayout* equationLayout = new QHBoxLayout();
		QString text;

		switch(equationAsEquation->getFunction())
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
		case OperatorType_t::constant:
			text += tr("Custom value");
		case OperatorType_t::identity:
			// Nothing
			break;
		}

		switch(equationAsEquation->getFunction())
		{
		case OperatorType_t::andOp:
		case OperatorType_t::orOp:
		case OperatorType_t::xorOp:
		case OperatorType_t::nandOp:
		case OperatorType_t::norOp:
		case OperatorType_t::xnorOp:
			text += " " + QString::number(equationAsEquation->getOperandCount());
			break;
		case OperatorType_t::notOp:
		case OperatorType_t::equalOp:
		case OperatorType_t::diffOp:
		case OperatorType_t::concatOp:
		case OperatorType_t::extractOp:
		case OperatorType_t::identity:
		case OperatorType_t::constant:
			break;
		}

		QLabel* signalText = new QLabel(text);
		signalText->setAlignment(Qt::AlignCenter);
		equationLayout->addWidget(signalText);

		this->setLayout(equationLayout);
	}
	else if (equationAsSignal != nullptr)
	{
		this->buildSignalEquation();
	}
}

void GraphicEquation::buildCompleteEquation()
{
	shared_ptr<Variable> equationAsSignal = this->equation.lock();
	shared_ptr<Equation> equationAsEquation = dynamic_pointer_cast<Equation> (equationAsSignal);

	if (equationAsEquation != nullptr)
	{
		QHBoxLayout* equationLayout = new QHBoxLayout();

		if (equationAsEquation->getFunction() == OperatorType_t::concatOp)
		{
			equationLayout->addWidget(new QLabel("{"));
		}

		for (uint i = 0 ; i < equationAsEquation->getOperandCount() ; i++)
		{
			// Add operand
			equationLayout->addWidget(new GraphicEquation(equationAsEquation->getOperand(i), false, this->lockSignal, this)); // Throws StatesException - Constrained by operand count - ignored

			// Add operator, except for last operand
			if (i < equationAsEquation->getOperandCount() - 1)
			{
				QString operatorText;

				switch(equationAsEquation->getFunction())
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
				case OperatorType_t::constant:
					// No intermediate sign
					break;
				}

				if (! operatorText.isEmpty())
				{
					QLabel* operatorLabel = new QLabel(operatorText);
					operatorLabel->setAlignment(Qt::AlignCenter);
					equationLayout->addWidget(operatorLabel);
				}
			}
		}

		if (equationAsEquation->getFunction() == OperatorType_t::concatOp)
		{
			equationLayout->addWidget(new QLabel("}"));
		}

		if (equationAsEquation->getFunction() == OperatorType_t::extractOp)
		{
			RangeExtractorWidget* rangeExtractor = new RangeExtractorWidget(equationAsEquation);

			if (! this->isTemplate)
			{
				connect(rangeExtractor, &RangeExtractorWidget::rangeLChanged, this, &GraphicEquation::treatRangeLeftBoundChanged);
				connect(rangeExtractor, &RangeExtractorWidget::rangeRChanged, this, &GraphicEquation::treatRangeRightBoundChanged);
			}

			equationLayout->addWidget(rangeExtractor);

			connect(rangeExtractor, &RangeExtractorWidget::destroyed, this, &GraphicEquation::clearEditorWidget);
			this->editorWidget = rangeExtractor;
		}
		else if (equationAsEquation->getFunction() == OperatorType_t::constant)
		{
			ConstantValueSetter* constantSetter = new ConstantValueSetter(equationAsEquation->getCurrentValue());
			equationLayout->addWidget(constantSetter);

			if (!this->isTemplate)
			{
				connect(constantSetter, &ConstantValueSetter::valueChanged, this, &GraphicEquation::treatConstantValueChanged);
			}

			connect(constantSetter, &RangeExtractorWidget::destroyed, this, &GraphicEquation::clearEditorWidget);
			this->editorWidget = constantSetter;
		}


		if (!equationAsEquation->isInverted())
			this->setLayout(equationLayout);
		else
		{
			QVBoxLayout* verticalLayout = new QVBoxLayout();

			InverterBar* inverterBar = new InverterBar();

			verticalLayout->addWidget(inverterBar);
			verticalLayout->addLayout(equationLayout);

			this->setLayout(verticalLayout);
		}

		this->setToolTip(tr("This equation") + " " + tr("is size") + " " + QString::number(equationAsEquation->getSize()));
	}
	else if (equationAsSignal != nullptr)
	{
		this->buildSignalEquation();
	}
}

void GraphicEquation::clearEditorWidget()
{
	this->editorWidget = nullptr;
}

void GraphicEquation::buildSignalEquation()
{
	shared_ptr<Variable> equationAsSignal = this->equation.lock();
	if (equationAsSignal != nullptr)
	{
		QHBoxLayout* equationLayout = new QHBoxLayout();
		QLabel* signalText = nullptr;

		signalText = new QLabel(equationAsSignal->getName());
		this->setToolTip(tr("Signal") + " " + equationAsSignal->getName() + " " + tr("is size") + " " + QString::number(equationAsSignal->getSize()));

		signalText->setAlignment(Qt::AlignCenter);
		equationLayout->addWidget(signalText);

		if ( (isTemplate) && (equationAsSignal->getSize() > 1) )
		{
			// Add a sub-widget to allow directly dragging sub-range from signal
			shared_ptr<Equation> extractor = shared_ptr<Equation>(new Equation(OperatorType_t::extractOp));
			extractor->setOperand(0, equationAsSignal); // Throws StatesException - Extract op aways has operand 0 - ignored

			GraphicEquation* extractorWidget = new GraphicEquation(extractor, true);
			equationLayout->addWidget(extractorWidget);
		}

		this->setLayout(equationLayout);
	}
}


// Set passive border color:
// neutral unless current equation is incorrect
void GraphicEquation::setDefaultBorderColor()
{
	this->mouseIn = false;

	if (this->isTemplate)
		this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
	else
	{
		shared_ptr<Variable> equationAsSignal = this->equation.lock();

		if ( ( equationAsSignal == nullptr ) || (equationAsSignal->getCurrentValue().isNull()) )
		{
			this->setStyleSheet("GraphicEquation {border: 1px solid red; border-radius: 10px}");

			if ( equationAsSignal == nullptr )
				this->setToolTip(tr("Empty operand"));
			else
			{
				shared_ptr<Equation> equationAsEquation = dynamic_pointer_cast<Equation>(equationAsSignal);

				if (equationAsEquation != nullptr)
				{
					if (equationAsEquation->getComputationFailureCause() == EquationComputationFailureCause_t::nullOperand)
						this->setToolTip(tr("Empty operand"));
					else if (equationAsEquation->getComputationFailureCause() == EquationComputationFailureCause_t::incompleteOperand)
						this->setToolTip(tr("One operand is not correct"));
					else if (equationAsEquation->getComputationFailureCause() == EquationComputationFailureCause_t::sizeMismatch)
						this->setToolTip(tr("The sizes of operands do not match between each other"));
					else if (equationAsEquation->getComputationFailureCause() == EquationComputationFailureCause_t::missingParameter)
						this->setToolTip(tr("A parameter of the equation is missing value"));
					else if (equationAsEquation->getComputationFailureCause() == EquationComputationFailureCause_t::incorrectParameter)
						this->setToolTip(tr("A parameter of the equation has an invalid value"));
				}
			}
		}
		else
		{
			this->setStyleSheet("GraphicEquation {border: 1px solid lightgrey; border-radius: 10px}");
		}
	}
}

// Set active border color
void GraphicEquation::setHilightedBorderColor()
{
	this->mouseIn = true;
	this->setStyleSheet("GraphicEquation {border: 1px solid blue; border-radius: 10px}");
}

// Scans QWidget hierarchy to find closest GraphicEquation
// parent. Returns nullptr if we are top level in this hierarchy.
GraphicEquation* GraphicEquation::parentEquation() const
{
	GraphicEquation* parentEquation = nullptr;

	QWidget* parentWidget = this->parentWidget();
	while (parentWidget != nullptr)
	{
		parentEquation = dynamic_cast<GraphicEquation*>(parentWidget);
		if (parentEquation != nullptr)
			break; // Found

		// No need to go beyond equation editor
		EquationEditor* editor = dynamic_cast<EquationEditor*>(parentWidget);
		if (editor != nullptr)
			break; // Not found

		parentWidget = parentWidget->parentWidget();
	}

	return parentEquation;
}

// Handle user input to replace equation.
// This is done by telling parent to replace ourself by
// new equation, unless we are top level.
// Graphic equation is rebuilt by parent, which destoys
// current object.
void GraphicEquation::replaceEquation(shared_ptr<Variable> newEquation)
{
	// Find equation parent
	GraphicEquation* parentEquation = this->parentEquation();

	if (parentEquation == nullptr)
	{
		// We are top level
		this->equation = newEquation;
		this->rootEquation = newEquation;

		this->buildEquation();
	}
	else
	{
		// Parent will handle replacement
		parentEquation->updateEquation(this->equation.lock(), newEquation);
	}
}

// Replaces one operand with the given one in logic equation.
// This function is typically called by children operand
// to replace themselves.
// Graphic equation is rebuild after logic equation update.
void GraphicEquation::updateEquation(shared_ptr<Variable> oldOperand, shared_ptr<Variable> newOperand)
{
	shared_ptr<Equation> l_equation = dynamic_pointer_cast<Equation> (this->equation.lock());

	if (l_equation != nullptr) // This IS true, because we are called by an operand of ours. Anyway...
	{
		for (uint i = 0 ; i < l_equation->getOperandCount() ; i++)
		{
			if (l_equation->getOperand(i) == oldOperand) // Throws StatesException - Constrained by operand count - ignored
			{
				l_equation->setOperand(i, newOperand); // Throws StatesException - Constrained by operand count - ignored
				break;
			}
		}

		this->buildEquation();
	}
	else
		qDebug() << "(GraphicEquation) Error! Update equation called on something not an equation.";
}

// Returns the equation currently represented by the graphic object.
// This is typically a pointer to a part of a logic equation,
// unless we are top level, then the whole equation is returned.
shared_ptr<Variable> GraphicEquation::getLogicEquation() const
{
	if (rootEquation != nullptr)
		return rootEquation;
	else
		return equation.lock();
}

/**
 * @brief GraphicEquation::forceCompleteRendering is for the case
 * where template equation differs from grab display.
 * Setting this changes the display.
 */
void GraphicEquation::forceCompleteRendering()
{
	this->completeRendering = true;
	this->buildEquation();
}

bool GraphicEquation::validEdit()
{
	shared_ptr<Equation> l_equation = dynamic_pointer_cast<Equation>(this->equation.lock());
	bool result  = false;

	if (l_equation != nullptr)
	{
		if (this->editorWidget != nullptr)
		{
			return this->editorWidget->validEdit();
		}
		else
		{
			for (QObject* child : this->children())
			{
				GraphicEquation* operand = dynamic_cast<GraphicEquation*>(child);
				if (operand != nullptr)
				{
					result = operand->validEdit();
					if (result == true)
						break;
				}
			}
		}
	}

	return result;
}

bool GraphicEquation::cancelEdit()
{
	shared_ptr<Equation> l_equation = dynamic_pointer_cast<Equation>(this->equation.lock());
	bool result  = false;

	if (l_equation != nullptr)
	{
		if (l_equation->getFunction() == OperatorType_t::extractOp)
		{
			return this->editorWidget->cancelEdit();
		}
		else
		{
			for (QObject* child : this->children())
			{
				GraphicEquation* operand = dynamic_cast<GraphicEquation*>(child);
				if (operand != nullptr)
				{
					result = operand->cancelEdit();
					if (result == true)
						break;
				}
			}
		}
	}

	return result;
}

// Triggered when mouse enters widget
void GraphicEquation::enterEvent(QEnterEvent*)
{
	GraphicEquation* parentEquation = this->parentEquation();

	if (parentEquation != nullptr)
		parentEquation->enterChildrenEvent();

	this->setHilightedBorderColor();
}

// Triggered when mouse leaves widget
// (not trigerred when children is entered)
void GraphicEquation::leaveEvent(QEvent*)
{
	GraphicEquation* parentEquation = this->parentEquation();

	if (parentEquation != nullptr)
		parentEquation->leaveChildrenEvent();

	this->setDefaultBorderColor();
}

// Generate drag-n-drop image when mouse is pressed
void GraphicEquation::mousePressEvent(QMouseEvent* event)
{
	if (isTemplate)
	{
		if (event->button() == Qt::LeftButton)
		{
			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new EquationMimeData(this);

			drag->setMimeData(mimeData);

			shared_ptr<Variable> l_signal = this->equation.lock();
			shared_ptr<Equation> l_equation = dynamic_pointer_cast<Equation> (l_signal);

			// Drag image may not match template display: create a correct equation
			if ( (l_equation != nullptr) && (l_equation->getFunction() == OperatorType_t::extractOp) )
			{
				GraphicEquation displayGraphicEquation(l_equation->clone(), true);
				displayGraphicEquation.forceCompleteRendering();

				drag->setPixmap(displayGraphicEquation.grab());
			}
			else if ( (l_equation == nullptr) && (l_signal != nullptr) ) // This is a simple signal => do not use template
			{
				GraphicEquation displayGraphicEquation(this->equation.lock());

				drag->setPixmap(displayGraphicEquation.grab());
			}
			else
				drag->setPixmap(this->grab());

			this->inMouseEvent = true;
			drag->exec();
		}
	}
	else
	{
		if (event->button() == Qt::LeftButton)
		{
			shared_ptr<Equation> l_equation = dynamic_pointer_cast<Equation> (this->equation.lock());

			if ( (l_equation != nullptr) && (l_equation->getFunction() == OperatorType_t::constant) )
			{
				if (editorWidget != nullptr)
					editorWidget->setEdited(true);

				this->inMouseEvent = true;
			}
		}
	}

	if (!this->inMouseEvent)
		QFrame::mousePressEvent(event);
}

void GraphicEquation::mouseMoveEvent(QMouseEvent* event)
{
	if (! this->inMouseEvent)
		QFrame::mouseMoveEvent(event);
}

void GraphicEquation::mouseReleaseEvent(QMouseEvent* event)
{
	if (! this->inMouseEvent)
		QFrame::mouseMoveEvent(event);
	else
		this->inMouseEvent = false;
}

void GraphicEquation::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (! this->inMouseEvent)
		QFrame::mouseMoveEvent(event);
}

// Triggered when mouse enters widget while dragging an object
void GraphicEquation::dragEnterEvent(QDragEnterEvent* event)
{
	if (!this->isTemplate)
	{
		this->setHilightedBorderColor();

		// Check if object is actually a graphic equation
		const EquationMimeData* mimeData = dynamic_cast<const EquationMimeData *>(event->mimeData());
		if (mimeData != nullptr)
		{
			event->acceptProposedAction();
		}
	}
}

// Trigerred when mouse leaves widget while dragging an object
// (also trigerred when child is entered)
void GraphicEquation::dragLeaveEvent(QDragLeaveEvent*)
{
	this->setDefaultBorderColor();
}

// Triggerd by right-click
void GraphicEquation::contextMenuEvent(QContextMenuEvent* event)
{
	if (!isTemplate)
	{
		shared_ptr<Variable> l_equation = this->equation.lock();

		if (l_equation != nullptr)
		{
			shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (l_equation);

			if ( (complexEquation != nullptr) || (this->lockSignal == false) )
			{
				ContextMenu* menu = new ContextMenu();
				menu->addTitle(tr("Equation:") +  " <i>" + l_equation->getText() + "</i>");

				QVariant data;
				QAction* addedAction = nullptr;

				if (complexEquation != nullptr)
				{

					switch(complexEquation->getFunction())
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

						if (complexEquation->getOperandCount() > 2)
						{
							addedAction = menu->addAction(tr("Remove one operand from that operator"));
							data.setValue((int)ContextAction_t::DecrementOperandCount);
							addedAction->setData(data);
						}
						break;
					case OperatorType_t::extractOp:

						if (complexEquation->getRangeR() == -1)
							addedAction = menu->addAction(tr("Edit index"));
						else
							addedAction = menu->addAction(tr("Edit range"));
						//actionToAdd->setCheckable(true);
						data.setValue((int)ContextAction_t::EditRange);
						addedAction->setData(data);

						menu->addSeparator();

						addedAction = menu->addAction(tr("Extract single bit"));
						addedAction->setCheckable(true);
						if (complexEquation->getRangeR() == -1)
							addedAction->setChecked(true);
						data.setValue((int)ContextAction_t::ExtractSwitchSingle);
						addedAction->setData(data);

						addedAction = menu->addAction(tr("Extract range"));
						addedAction->setCheckable(true);
						if (complexEquation->getRangeR() != -1)
							addedAction->setChecked(true);
						data.setValue((int)ContextAction_t::ExtractSwitchRange);
						addedAction->setData(data);



						break;
					case OperatorType_t::constant:

						addedAction = menu->addAction(tr("Edit constant value"));
						data.setValue((int)ContextAction_t::EditValue);
						addedAction->setData(data);

						break;
					case OperatorType_t::notOp:
					case OperatorType_t::equalOp:
					case OperatorType_t::diffOp:
					case OperatorType_t::identity:
						// Nothing
						break;
					}
				}

				menu->addSeparator();

				addedAction = menu->addAction(tr("Delete"));
				data.setValue((int)ContextAction_t::DeleteEquation);
				addedAction->setData(data);

				addedAction = menu->addAction(tr("Cancel"));
				data.setValue((int)CommonAction_t::Cancel);
				addedAction->setData(data);

				menu->popup(this->mapToGlobal(event->pos()));

				connect(menu, &QMenu::triggered, this, &GraphicEquation::treatMenuEventHandler);
			}
		}
	}
}

void GraphicEquation::enterChildrenEvent()
{
	this->setDefaultBorderColor();
}


void GraphicEquation::leaveChildrenEvent()
{
	this->setHilightedBorderColor();
}


void GraphicEquation::dropEvent(QDropEvent* event)
{
	const EquationMimeData* mimeData = dynamic_cast<const EquationMimeData*>(event->mimeData());

	if (mimeData != nullptr)
	{
		// Obtain new equation
		this->droppedEquation = mimeData->getEquation()->getLogicEquation();

		shared_ptr<Variable> signalEquation = this->equation.lock();

		// Automatically replace empty operands
		if (signalEquation == nullptr)
		{
			replaceEquation(droppedEquation);
		}
		else
		{
			// Ask what to do
			ContextMenu* menu = new ContextMenu();
			menu->addTitle(tr("What should I do?"));
			menu->addSubTitle(tr("Existing equation:") +  " <i>"  + signalEquation->getText() + "</i>");
			menu->addSubTitle(tr("Dropped equation:") + " <i> " + droppedEquation->getText() + "</i>");

			QVariant data;
			QAction* a = new QAction(menu);
			a->setText(tr("Replace existing equation by dropped equation"));
			a->setToolTip(tr("New equation would be:") + " <i> " + droppedEquation->getText() + "</i>");
			data.setValue((int)DropAction_t::ReplaceExisting);
			a->setData(data);
			menu->addAction(a);

			shared_ptr<Equation> droppedComplexEquation = dynamic_pointer_cast<Equation> (droppedEquation);
			if ( (droppedComplexEquation != nullptr) &&
				 (droppedComplexEquation->getFunction() != OperatorType_t::extractOp) &&
				 (droppedComplexEquation->getFunction() != OperatorType_t::constant) )
			{
				QString actionText = tr("Set existing equation as operand of dropped equation");

				a = new QAction(menu);
				a->setText(actionText);
				data.setValue((int)DropAction_t::ExistingAsOperand);
				a->setData(data);

				// Build tooltip
				shared_ptr<Equation> newEquation = droppedComplexEquation->clone();
				newEquation->setOperand(0, signalEquation); // Throws StatesException - Remaining natures always have operand 0 - ignored

				a->setToolTip(tr("New equation would be: ") + "<br /><i>" + newEquation->getText() + "</i>");
				newEquation.reset();

				menu->addAction(a);

			}

			a = menu->addAction(tr("Cancel"));
			data.setValue((int)CommonAction_t::Cancel);
			a->setData(data);

			menu->popup(this->mapToGlobal(event->position().toPoint()));

			connect(menu, &QMenu::triggered, this, &GraphicEquation::treatMenuEventHandler);
		}

		event->acceptProposedAction();
	}
}


void GraphicEquation::treatMenuEventHandler(QAction* action)
{
	shared_ptr<Equation> newEquation;
	shared_ptr<Equation> complexEquation;
	bool valid;

	QVariant data = action->data();
	int dataValue = data.toInt();

	shared_ptr<Variable> signalEquation  = this->equation.lock();

	switch (dataValue)
	{
	case CommonAction_t::Cancel:
		break;

	case DropAction_t::ReplaceExisting:

		// Nothing should be done after that line because it
		// will cause parent to rebuild, deleting this
		replaceEquation(this->droppedEquation);

		break;

	case DropAction_t::ExistingAsOperand:

		complexEquation = dynamic_pointer_cast<Equation>(this->droppedEquation);
		newEquation = complexEquation->clone();
		newEquation->setOperand(0, signalEquation); // Throws StatesException - Extract op aways has operand 0 - ignored

		// Nothing should be done after that line because it
		// will cause parent to rebuild, deleting this
		this->replaceEquation(newEquation);

		break;

	case ContextAction_t::DeleteEquation:

		valid = false;

		if (signalEquation != nullptr)
		{
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete equation?") + "<br />" + tr("Content is:") + " " + signalEquation->getText(), QMessageBox::Ok | QMessageBox::Cancel);
			if (reply == QMessageBox::StandardButton::Ok)
				valid = true;
		}
		else
			valid = true;

		if (valid)
		{
			replaceEquation(nullptr);
		}

		break;

	case ContextAction_t::IncrementOperandCount:

		if (signalEquation != nullptr)
		{
			complexEquation = dynamic_pointer_cast<Equation>(signalEquation);

			complexEquation->increaseOperandCount(); // Throws StatesException - The menu only provides this option when applicable - ignored

			this->buildEquation();
		}

		break;

	case ContextAction_t::DecrementOperandCount:

		if (signalEquation != nullptr)
		{

			valid = false;

			complexEquation = dynamic_pointer_cast<Equation>(signalEquation);
			shared_ptr<Variable> signalEquation = complexEquation->getOperand(complexEquation->getOperandCount() - 1); // Throws StatesException - Constrained by operand count - ignored

			if (signalEquation != nullptr)
			{
				QMessageBox::StandardButton reply;
				reply = QMessageBox::question(this, tr("User confirmation needed"), tr("Delete last oprand?") + "<br />" + tr("Content is:") + " " + signalEquation->getText(), QMessageBox::Ok | QMessageBox::Cancel);

				if (reply == QMessageBox::StandardButton::Ok)
					valid = true;
			}
			else
			{
				valid = true;
			}

			if (valid)
			{
				complexEquation->decreaseOperandCount(); // Throws StatesException - The menu only provides this option when applicable - ignored

				this->buildEquation();
			}
		}

		break;

	case ContextAction_t::EditRange:
		if (editorWidget != nullptr)
		{
			editorWidget->setEdited(true);
		}
		break;
	case ContextAction_t::ExtractSwitchSingle:
		complexEquation = dynamic_pointer_cast<Equation>(signalEquation);
		if ( (complexEquation != nullptr) && (complexEquation->getFunction() == OperatorType_t::extractOp) )
		{
			complexEquation->setRange(complexEquation->getRangeL());
		}
		break;
	case ContextAction_t::ExtractSwitchRange:
		complexEquation = dynamic_pointer_cast<Equation>(signalEquation);
		if ( (complexEquation != nullptr) && (complexEquation->getFunction() == OperatorType_t::extractOp) )
		{
			complexEquation->setRange(complexEquation->getRangeL(), 0);
		}
		break;
	case ContextAction_t::EditValue:
		if (editorWidget != nullptr)
		{
			editorWidget->setEdited(true);
		}

		break;
	}
}

void GraphicEquation::treatRangeLeftBoundChanged(int newIndex)
{
	shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (this->equation.lock());

	if ( (complexEquation != nullptr) && (complexEquation->getFunction() == OperatorType_t::extractOp) )
	{
		complexEquation->setRange(newIndex, complexEquation->getRangeR());
	}
}

void GraphicEquation::treatRangeRightBoundChanged(int newIndex)
{
	shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (this->equation.lock());

	if ( (complexEquation != nullptr) && (complexEquation->getFunction() == OperatorType_t::extractOp) )
	{
		complexEquation->setRange(complexEquation->getRangeL(), newIndex);
	}
}

void GraphicEquation::treatConstantValueChanged(LogicValue newValue)
{
	shared_ptr<Equation> complexEquation = dynamic_pointer_cast<Equation> (this->equation.lock());

	if ( (complexEquation != nullptr) && (complexEquation->getFunction() == OperatorType_t::constant) )
	{
		complexEquation->setConstantValue(newValue); // Throws StatesException - Equation is constant and thus can take a value - ignored
	}
}

void GraphicEquation::updateBorder()
{
	if (this->mouseIn)
		this->setHilightedBorderColor();
	else
		this->setDefaultBorderColor();
}
