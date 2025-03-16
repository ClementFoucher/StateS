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
#include "conditioneditor.h"

// Qt classes
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

// StateS classes
#include "fsmtransition.h"
#include "truthtabledisplay.h"
#include "equation.h"
#include "truthtable.h"
#include "machine.h"
#include "equationeditor.h"
#include "contextmenu.h"
#include "graphicfsmtransition.h"
#include "machinemanager.h"
#include "graphicmachine.h"
#include "fsmundocommand.h"
#include "operand.h"


ConditionEditor::ConditionEditor(componentId_t transitionId, QWidget* parent) :
	QWidget(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(transitionId));
	if (transition == nullptr) return;

	auto graphicTransition = dynamic_cast<GraphicFsmTransition*>(graphicMachine->getGraphicComponent(transitionId));
	if (graphicTransition == nullptr) return;


	this->transitionId = transitionId;
	connect(transition.get(), &FsmTransition::conditionChangedEvent, this, &ConditionEditor::updateContent);

	this->layout = new QGridLayout(this);

	this->conditionText = new QLabel("…", this);
	this->layout->addWidget(this->conditionText, 0, 0, 1, 2);

	this->buttonSetCondition = new QPushButton(tr("Set condition"), this);
	connect(this->buttonSetCondition, &QAbstractButton::clicked, this, &ConditionEditor::editCondition);
	this->layout->addWidget(this->buttonSetCondition, 2, 0, 1, 1);

	QPushButton* buttonClearCondition = new QPushButton(tr("Clear condition"), this);
	connect(buttonClearCondition, &QAbstractButton::clicked, this, &ConditionEditor::clearCondition);
	this->layout->addWidget(buttonClearCondition, 2, 1, 1, 1);

	QHBoxLayout* positionLayout = new QHBoxLayout();
	QLabel* positionLabel = new QLabel(tr("Condition position"), this);
	positionLayout->addWidget(positionLabel);
	this->conditionTextPositionSlider = new QSlider(Qt::Horizontal, this);
	this->conditionTextPositionSlider->setMinimum(1);
	this->conditionTextPositionSlider->setMaximum(99);
	this->conditionTextPositionSlider->setValue(graphicTransition->getConditionLineSliderPosition()*100);
	connect(this->conditionTextPositionSlider, &QSlider::valueChanged, this, &ConditionEditor::conditionTextPositionSliderChanged);
	positionLayout->addWidget(this->conditionTextPositionSlider);
	this->layout->addLayout(positionLayout, 3, 0, 1, 2);

	this->buttonToggleTruthTable = new QPushButton(tr("Display truth table"), this);
	connect(this->buttonToggleTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);
	this->layout->addWidget(this->buttonToggleTruthTable, 4, 0, 1, 2);

	this->updateContent();
}

ConditionEditor::~ConditionEditor()
{
	delete this->truthTableDisplay; // In case not shown
}

void ConditionEditor::editCondition()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
	if (transition == nullptr) return;


	if (machine->getReadableVariablesIds().count() != 0)
	{
		EquationEditor* eqEdit = new EquationEditor(transition->getCondition());

		int result = eqEdit->exec();

		if (result == QDialog::DialogCode::Accepted)
		{
			auto newEquation = eqEdit->getResultEquation();

			transition->setCondition(newEquation);
			machineManager->notifyMachineEdited();
		}

		delete eqEdit;
	}
	else
	{
		auto textList = QStringList();
		textList.append(tr("No compatible variable!"));
		textList.append(tr("First declare the machine variables in Machine tab."));
		auto menu = ContextMenu::createErrorMenu(textList);
		menu->popup(buttonSetCondition->mapToGlobal(QPoint(buttonSetCondition->width(), -menu->sizeHint().height())));
	}
}

void ConditionEditor::clearCondition()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
	if (transition == nullptr) return;


	transition->clearCondition();
	machineManager->notifyMachineEdited();
}

void ConditionEditor::updateContent()
{
	delete this->conditionWarningText;
	this->conditionWarningText = nullptr;

	bool truthTableDisplayed = this->isTruthTableDisplayed();
	if (truthTableDisplayed == true)
	{
		this->collapseTruthTable();
	}

	delete this->truthTableDisplay;
	this->truthTableDisplay = nullptr;
	this->truthTable.reset();

	this->buttonToggleTruthTable->setEnabled(false);


	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
	if (transition == nullptr) return;

	auto graphicTransition = dynamic_cast<GraphicFsmTransition*>(graphicMachine->getGraphicComponent(transitionId));
	if (graphicTransition == nullptr) return;


	int sliderPosition = graphicTransition->getConditionLineSliderPosition()*100;
	if (sliderPosition != this->conditionTextPositionSlider->value())
	{
		this->conditionTextPositionSlider->setValue(sliderPosition);
	}

	auto condition = transition->getCondition();
	if (condition != nullptr)
	{
		if (condition->getOperatorType() != OperatorType_t::identity)
		{
			this->conditionText->setText(condition->getColoredText());
		}
		else // (condition->getOperatorType() == OperatorType_t::identity)
		{
			auto operand = condition->getOperand(0);
			if (operand == nullptr) return;


			this->conditionText->setText(operand->getText());
		}

		if (condition->getSize() == 1)
		{
			if (condition->getOperatorType() != OperatorType_t::identity)
			{
				this->buttonToggleTruthTable->setEnabled(true);

				if (truthTableDisplayed)
				{
					this->expandTruthTable();
				}
			}
		}
		else
		{
			if (condition->getSize() == 0)
			{
				this->conditionWarningText = new QLabel("<span style=\"color:red;\">" +
				                                        tr("Warning: the current condition is not valid.") +
				                                        "<br />" + tr("Thus, this transition will never be crossed.") +
				                                        "<br />" + tr("Edit condition and hover over errors for more information.") +
				                                        "</span>"
				                                        );
				this->conditionWarningText->setWordWrap(true);
			}
			else
			{
				this->conditionWarningText = new QLabel("<span style=\"color:red;\">" +
				                                        tr("Warning: equation representing condition is size") + " " + QString::number(condition->getSize()) +
				                                        "<br />" + tr("Conditions must be size one to allow being treated as booleans.") +
				                                        "<br />" + tr("Thus, the current transition will never be crossed.") +
				                                        "</span>"
				                                        );
				this->conditionWarningText->setWordWrap(true);
			}

			this->conditionWarningText->setAlignment(Qt::AlignCenter);
			this->layout->addWidget(conditionWarningText, 1, 0, 1, 2);
		}
	}
	else
	{
		this->conditionText->setText("1");
	}
}

void ConditionEditor::conditionTextPositionSliderChanged(int newValue)
{
	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	auto graphicTransition = dynamic_cast<GraphicFsmTransition*>(graphicMachine->getGraphicComponent(transitionId));
	if (graphicTransition == nullptr) return;


	qreal realValue = ((qreal)newValue)/100;
	graphicTransition->setConditionLineSliderPosition(realValue);

	FsmUndoCommand* undoCommand = new FsmUndoCommand(UndoCommandId_t::fsmUndoTransitionConditionSliderPositionChangeId, graphicTransition->getLogicComponentId());
	machineManager->notifyMachineEdited(undoCommand);
}

void ConditionEditor::expandTruthTable()
{
	if (this->isTruthTableDisplayed() == true) return;


	disconnect(this->buttonToggleTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);

	if (this->truthTableDisplay != nullptr)
	{
		this->truthTableDisplay->setVisible(true);
	}
	else
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return;

		auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
		if (transition == nullptr) return;

		auto condition = transition->getCondition();
		if (condition == nullptr) return;


		if (condition->getOperatorType() != OperatorType_t::identity)
		{
			this->truthTable = shared_ptr<TruthTable>(new TruthTable(condition));

			this->truthTableDisplay = new TruthTableDisplay(this->truthTable);
			this->layout->addWidget(this->truthTableDisplay, 5, 0, 1, 2);
		}
	}

	this->buttonToggleTruthTable->setText(tr("Collapse truth table"));

	connect(this->buttonToggleTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::collapseTruthTable);

}

void ConditionEditor::collapseTruthTable()
{
	disconnect(this->buttonToggleTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::collapseTruthTable);

	this->buttonToggleTruthTable->setText(tr("Display truth table"));
	this->truthTableDisplay->setVisible(false);

	connect(this->buttonToggleTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);
}

bool ConditionEditor::isTruthTableDisplayed()
{
	if ( (this->truthTableDisplay != nullptr) && (this->truthTableDisplay->isVisible()) )
	{
		return true;
	}
	else
	{
		return false;
	}
}
