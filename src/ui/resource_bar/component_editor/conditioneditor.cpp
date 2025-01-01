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

#include <QDebug>

// StateS classes
#include "fsmtransition.h"
#include "truthtabledisplay.h"
#include "equation.h"
#include "truthtable.h"
#include "machine.h"
#include "equationeditor.h"
#include "contextmenu.h"
#include "fsmgraphictransition.h"
#include "machinemanager.h"
#include "graphicmachine.h"
#include "fsmundocommand.h"


ConditionEditor::ConditionEditor(componentId_t transitionId, QWidget* parent) :
    QWidget(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(transitionId));
	if (transition == nullptr) return;

	auto graphicTransition = dynamic_cast<FsmGraphicTransition*>(graphicMachine->getGraphicComponent(transitionId));
	if (graphicTransition == nullptr) return;


	this->transitionId = transitionId;
	connect(transition.get(), &FsmTransition::conditionChangedEvent, this, &ConditionEditor::updateContent);

	this->layout = new QGridLayout(this);

	QLabel* conditionTitle = new QLabel(tr("Condition"), this);
	conditionTitle->setAlignment(Qt::AlignCenter);
	this->layout->addWidget(conditionTitle, 0, 0, 1, 2);

	this->conditionText = new QLabel("…", this);
	this->layout->addWidget(this->conditionText, 1, 0, 1, 2);

	this->buttonSetCondition = new QPushButton(tr("Set condition"), this);
	connect(this->buttonSetCondition, &QAbstractButton::clicked, this, &ConditionEditor::editCondition);
	this->layout->addWidget(this->buttonSetCondition, 3, 0, 1, 1);

	QPushButton* buttonClearCondition = new QPushButton(tr("Clear condition"), this);
	connect(buttonClearCondition, &QAbstractButton::clicked, this, &ConditionEditor::clearCondition);
	this->layout->addWidget(buttonClearCondition, 3, 1, 1, 1);

	QHBoxLayout* positionLayout = new QHBoxLayout();
	QLabel* positionLabel = new QLabel(tr("Condition position"), this);
	positionLayout->addWidget(positionLabel);
	this->conditionTextPositionSlider = new QSlider(Qt::Horizontal, this);
	this->conditionTextPositionSlider->setMinimum(1);
	this->conditionTextPositionSlider->setMaximum(99);
	this->conditionTextPositionSlider->setValue(graphicTransition->getConditionLineSliderPosition()*100);
	connect(this->conditionTextPositionSlider, &QSlider::valueChanged, this, &ConditionEditor::conditionTextPositionSliderChanged);
	positionLayout->addWidget(this->conditionTextPositionSlider);
	this->layout->addLayout(positionLayout, 4, 0, 1, 2);

	this->buttonExpandTruthTable = new QPushButton(tr("Display truth table"), this);
	connect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);
	this->layout->addWidget(this->buttonExpandTruthTable, 5, 0, 1, 2);

	this->updateContent();
}

ConditionEditor::~ConditionEditor()
{
	delete truthTableDisplay; // In case not shown
}

void ConditionEditor::editCondition()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
	if (transition == nullptr) return;


	if (machine->getReadableSignals().count() != 0)
	{
		EquationEditor* eqEdit = new EquationEditor(transition->getCondition());

		int result = eqEdit->exec();

		if (result == QDialog::DialogCode::Accepted)
		{
			shared_ptr<Signal> tmp = eqEdit->getResultEquation();

			transition->setCondition(tmp);
			machineManager->notifyMachineEdited();
		}

		delete eqEdit;
	}
	else
	{
		ContextMenu* menu = ContextMenu::createErrorMenu(tr("No compatible signal!"));
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

void ConditionEditor::treatMenuSetCondition(QAction* action)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
	if (transition == nullptr) return;


	for (shared_ptr<Signal> currentVariable : machine->getReadableSignals())
	{
		if (currentVariable->getName() == action->text())
		{
			transition->setCondition(currentVariable);
			break;
		}
	}
}

void ConditionEditor::updateContent()
{
	delete this->conditionWarningText;
	this->conditionWarningText = nullptr;

	bool truthTableDisplayed = this->isTruthTableDisplayed();
	if (truthTableDisplayed)
		this->collapseTruthTable();

	delete this->truthTableDisplay;
	this->truthTableDisplay = nullptr;
	this->truthTable.reset();

	this->buttonExpandTruthTable->setEnabled(false);


	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto graphicMachine = machineManager->getGraphicMachine();
	if (graphicMachine == nullptr) return;

	auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
	if (transition == nullptr) return;

	auto graphicTransition = dynamic_cast<FsmGraphicTransition*>(graphicMachine->getGraphicComponent(transitionId));
	if (graphicTransition == nullptr) return;


	int sliderPosition = graphicTransition->getConditionLineSliderPosition()*100;
	if (sliderPosition != this->conditionTextPositionSlider->value())
	{
		this->conditionTextPositionSlider->setValue(sliderPosition);
	}

	if (transition->getCondition() != nullptr)
	{
		shared_ptr<Equation> equationOperand = dynamic_pointer_cast<Equation>(transition->getCondition());

		if (equationOperand != nullptr)
		{
			this->conditionText->setText(equationOperand->getColoredText());
		}
		else
		{
			this->conditionText->setText(transition->getCondition()->getText());
		}

		if (transition->getCondition()->getSize() == 1)
		{
			shared_ptr<Equation> equationCondition = dynamic_pointer_cast<Equation>(transition->getCondition());

			if (equationCondition != nullptr)
			{
				this->buttonExpandTruthTable->setEnabled(true);

				if (truthTableDisplayed)
					this->expandTruthTable();
			}
		}
		else
		{
			if (transition->getCondition()->getSize() == 0)
			{
				this->conditionWarningText = new QLabel("<span style=\"color:red;\">" +
				                                        tr("Warning: equation representing condition is not valid.") +
				                                        "<br />" + tr("Thus, the current transition will never be crossed.") +
				                                        "<br />" + tr("Edit condition and hover over errors for more information.") +
				                                        "</span>"
				                                        );
				this->conditionWarningText->setWordWrap(true);
			}
			else
			{
				this->conditionWarningText = new QLabel("<span style=\"color:red;\">" +
				                                        tr("Warning: equation representing condition is size") + " " + QString::number(transition->getCondition()->getSize()) +
				                                        "<br />" + tr("Conditions must be size one to allow being treated as booleans.") +
				                                        "<br />" + tr("Thus, the current transition will never be crossed.") +
				                                        "</span>"
				                                        );
				this->conditionWarningText->setWordWrap(true);
			}

			this->conditionWarningText->setAlignment(Qt::AlignCenter);
			this->layout->addWidget(conditionWarningText, 2, 0, 1, 2);
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

	auto graphicTransition = dynamic_cast<FsmGraphicTransition*>(graphicMachine->getGraphicComponent(transitionId));
	if (graphicTransition == nullptr) return;


	qreal realValue = ((qreal)newValue)/100;
	graphicTransition->setConditionLineSliderPosition(realValue);

	FsmUndoCommand* undoCommand = new FsmUndoCommand(UndoCommandId_t::fsmUndoTransitionConditionSliderPositionChangeId, graphicTransition->getLogicComponentId());
	machineManager->notifyMachineEdited(undoCommand);
}

void ConditionEditor::expandTruthTable()
{
	if (this->isTruthTableDisplayed() == true) return;


	disconnect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);

	if (this->truthTableDisplay != nullptr)
	{
		this->truthTableDisplay->setVisible(true);
	}
	else
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return;

		auto transition = dynamic_pointer_cast<FsmTransition>(machine->getComponent(this->transitionId));
		if (transition != nullptr)
		{
			shared_ptr<Equation> equation = dynamic_pointer_cast<Equation>(transition->getCondition());

			if (equation != nullptr)
			{
				this->truthTable = shared_ptr<TruthTable>(new TruthTable(equation));

				this->truthTableDisplay = new TruthTableDisplay(this->truthTable);
				this->layout->addWidget(this->truthTableDisplay, 5, 0, 1, 2);
			}
			else
			{
				// TODO
			}
		}
	}

	this->buttonExpandTruthTable->setText(tr("Collapse truth table"));

	connect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::collapseTruthTable);

}

void ConditionEditor::collapseTruthTable()
{
	disconnect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::collapseTruthTable);

	this->buttonExpandTruthTable->setText(tr("Display truth table"));
	this->truthTableDisplay->setVisible(false);

	connect(this->buttonExpandTruthTable, &QAbstractButton::clicked, this, &ConditionEditor::expandTruthTable);
}

bool ConditionEditor::isTruthTableDisplayed()
{
	if ( (this->truthTableDisplay != nullptr) && (this->truthTableDisplay->isVisible()) )
		 return true;
	else return false;
}
