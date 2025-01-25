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
#include <QVBoxLayout>
#include <QTabWidget>

// StateS classes
#include "machinemanager.h"
#include "signallisteditor.h"
#include "machinecomponentvisualizer.h"
#include "collapsiblewidgetwithtitle.h"
#include "dynamiclineedit.h"
#include "machineundocommand.h"
#include "machine.h"


MachineEditorTab::MachineEditorTab(shared_ptr<MachineComponentVisualizer> machineComponentView, QWidget* parent) :
    QWidget(parent)
{
	this->machineComponentView = machineComponentView;

	QVBoxLayout* layout = new QVBoxLayout(this);

	//
	// Machine name
	QLabel* machineNameLabel = new QLabel("<b>" + tr("Machine name")  + "</b>", this);
	machineNameLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(machineNameLabel);

	this->machineName = new DynamicLineEdit(QString(), true, this);
	this->updateMachineName();

	connect(this->machineName, &DynamicLineEdit::newTextAvailableEvent, this, &MachineEditorTab::nameTextChangedEventHandler);
	connect(this->machineName, &DynamicLineEdit::userCancelEvent,       this, &MachineEditorTab::updateMachineName);

	connect(machineManager.get(), &MachineManager::machineNameChangedEvent, this, &MachineEditorTab::updateMachineName);
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent,     this, &MachineEditorTab::updateMachineName);

	layout->addWidget(this->machineName);

	//
	// Title

	QLabel* title = new QLabel("<b>" + tr("Variable editor")  + "</b>", this);
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);

	//
	// Signals

	QTabWidget* signalsTabs = new QTabWidget(this);
	layout->addWidget(signalsTabs);

	signalsTabs->insertTab(0, new SignalListEditor(VariableNature_t::input),    tr("Inputs"));
	signalsTabs->insertTab(1, new SignalListEditor(VariableNature_t::output),   tr("Outputs"));
	signalsTabs->insertTab(2, new SignalListEditor(VariableNature_t::internal), tr("Variables"));
	signalsTabs->insertTab(3, new SignalListEditor(VariableNature_t::constant), tr("Constants"));

	signalsTabs->setCurrentIndex(0);

	//
	// Hint

	QLabel* hintText = new QLabel(tr("Switch between variable types using tabs.")
	                              + "<br />"
	                              + tr("Variable length can not exceed 64 bits.")
	                              + "<br />"
	                              + tr("Allowed characters are alphanumerical ones, space and")
	                              + " {'_', '@', '#', '-'}."
	                              + "<br />"
	                              + tr("Double-click on a value to edit it.")
	                              );
	hintText->setAlignment(Qt::AlignCenter);
	hintText->setWordWrap(true);

	QString hintTitle = tr("Hint:") + " " + tr("Variable editor");
	this->hintDisplay = new CollapsibleWidgetWithTitle(hintTitle, hintText, this);
	layout->addWidget(this->hintDisplay);

	//
	// Machine visualization

	this->machineDisplay = new CollapsibleWidgetWithTitle(tr("Component visualization"), machineComponentView.get(), this);
	layout->addWidget(this->machineDisplay);

	//
	// Set tab focus order to prevent machine name being edited by default
	this->setTabOrder(signalsTabs, this->machineName);
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
 * otherwise clicks inside this widget won't validate input.
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
		MachineUndoCommand* undoCommand = new MachineUndoCommand(oldName);
		machineManager->notifyMachineEdited(undoCommand);
	}
	else
	{
		this->machineName->markAsErroneous();
	}
}

void MachineEditorTab::updateMachineName()
{
	auto machine = machineManager->getMachine();
	if (machine != nullptr)
	{
		this->machineName->setText(machine->getName());
	}
	else
	{
		this->machineName->setText("<i>(" + tr("No machine") + ")</i>");
		this->machineName->setEnabled(false);
	}
}
