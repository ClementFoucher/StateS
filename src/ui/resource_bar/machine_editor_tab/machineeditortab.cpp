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
#include "machineeditortab.h"

// Qt classes
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QTabWidget>

// StateS classes
#include "machinemanager.h"
#include "variablelisteditor.h"
#include "machinecomponentvisualizer.h"
#include "collapsiblewidgetwithtitle.h"
#include "hintwidget.h"
#include "selfmanageddynamiclineeditor.h"
#include "machineundocommand.h"
#include "machine.h"


MachineEditorTab::MachineEditorTab(shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent) :
    QWidget(parent)
{
	this->machineComponentView = machineComponentView;

	////
	// Build object

	connect(machineManager.get(), &MachineManager::machineNameChangedEvent, this, &MachineEditorTab::updateMachineName);
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent,     this, &MachineEditorTab::updateMachineName);

	////
	// Visual rendering

	//
	// Title

	QLabel* title = new QLabel("<b>" + tr("Machine editor") + "</b>");
	title->setAlignment(Qt::AlignCenter);

	//
	// Machine name
	auto machineNameEditor = new QGroupBox(tr("Machine name"));
	auto machineNameEditorLayout = new QVBoxLayout(machineNameEditor);

	this->machineName = new SelfManagedDynamicLineEditor();
	connect(this->machineName, &SelfManagedDynamicLineEditor::newTextAvailableEvent, this, &MachineEditorTab::nameTextChangedEventHandler);
	connect(this->machineName, &SelfManagedDynamicLineEditor::userCancelEvent,       this, &MachineEditorTab::updateMachineName);
	machineNameEditorLayout->addWidget(this->machineName);

	//
	// Machine variables
	auto variablesEditor = new QGroupBox(tr("Variable editor"));
	auto variablesEditorLayout = new QVBoxLayout(variablesEditor);

	auto variablesTabs = new QTabWidget();
	variablesTabs->insertTab(0, new VariableListEditor(VariableNature_t::input),    tr("Inputs"));
	variablesTabs->insertTab(1, new VariableListEditor(VariableNature_t::output),   tr("Outputs"));
	variablesTabs->insertTab(2, new VariableListEditor(VariableNature_t::internal), tr("Variables"));
	variablesTabs->insertTab(3, new VariableListEditor(VariableNature_t::constant), tr("Constants"));
	variablesTabs->setCurrentIndex(0);
	variablesEditorLayout->addWidget(variablesTabs);

	//
	// Hint
	QString hintTitle = tr("Hint:") + " " + tr("Machine editor");

	QString hintText = "<p><b>" + tr("Names:") + "</b></p>"
	                 + "<p>"
	                 + tr("Allowed characters for machine and variables names are alphanumerical ones, space and")
	                 + " {'_', '@', '#', '-'}."
	                 + "</p>"
	                 + "<p><b>" + tr("Variables editor:") + "</b></p>"
	                 + "<p>"
	                 + tr("Switch between variable natures using tabs.")
	                 + "<br>"
	                 + tr("Double-click on a field to edit it.")
	                 + "</p>";

	auto hintDisplay = new HintWidget(hintTitle, hintText, this);

	//
	// Machine visualization
	this->machineDisplay = new CollapsibleWidgetWithTitle(tr("Component visualization"), machineComponentView.get());

	//
	// Build complete rendering
	auto mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(title);
	mainLayout->addWidget(machineNameEditor);
	mainLayout->addWidget(variablesEditor);
	mainLayout->addWidget(hintDisplay);
	mainLayout->addWidget(this->machineDisplay);

	////
	// Fill content

	this->updateMachineName();

	// Set focus to variable editor in order to prevent machine name being edited by default
	variablesTabs->setFocus();
}

void MachineEditorTab::showEvent(QShowEvent* e)
{
	// Ensure we get the view back
	shared_ptr<MachineComponentVisualizer> l_machineComponentView = this->machineComponentView.lock();

	if (l_machineComponentView != nullptr)
	{
		this->machineDisplay->setContent(tr("Component visualization"), l_machineComponentView.get());
	}

	QWidget::showEvent(e);
}

/**
 * @brief MachineEditorTab::mousePressEvent
 * Used to allow validation of machine name wherever we click,
 * otherwise clicks inside this widget that don't give focus to
 * another widget won't validate input.
 */
void MachineEditorTab::mousePressEvent(QMouseEvent* e)
{
	this->machineName->clearFocus();

	QWidget::mousePressEvent(e);
}

void MachineEditorTab::nameTextChangedEventHandler(const QString& newName)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto oldName = machine->getName();
	if (newName == oldName) return;

	bool accepted = machine->setName(newName);

	if (accepted == true)
	{
		// Name gets cleaned by machine: make sure the new name is
		// different from the old one before building an undo command.
		if (machine->getName() != oldName)
		{
			MachineUndoCommand* undoCommand = new MachineUndoCommand(oldName);
			machineManager->notifyMachineEdited(undoCommand);
		}
	}
	else
	{
		this->machineName->setErroneous(true);
	}
}

void MachineEditorTab::updateMachineName()
{
	this->machineName->resetView();

	auto machine = machineManager->getMachine();
	if (machine != nullptr)
	{
		this->machineName->setText(machine->getName());
		this->machineName->setEnabled(true);
	}
	else
	{
		this->machineName->setText("<i>(" + tr("No machine") + ")</i>");
		this->machineName->setEnabled(false);
	}
}
