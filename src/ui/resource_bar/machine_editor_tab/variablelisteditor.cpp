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
#include "variablelisteditor.h"

// C++ classes
#include <algorithm>

// Qt classes
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QHeaderView>

#include <QDebug>

// StateS classes
#include "statestypes.h"
#include "dynamiclineedit.h"
#include "variable.h"
#include "dynamictableitemdelegate.h"
#include "tablewidgetwithresizeevent.h"
#include "contextmenu.h"
#include "machinemanager.h"
#include "machine.h"
#include "statesexception.h"
#include "exceptiontypes.h"


VariableListEditor::VariableListEditor(VariableNature_t editorType, QWidget* parent) :
    QWidget(parent)
{
	this->editorType = editorType;

	QVBoxLayout* layout = new QVBoxLayout(this);
	QStringList headerTexts;

	if (editorType == VariableNature_t::input)
	{
		this->variablesList = new TableWidgetWithResizeEvent(0, 3, this);

		headerTexts.append(tr("Input"));
		headerTexts.append(tr("Initial value"));

		connect(machineManager.get(), &MachineManager::machineInputListChangedEvent, this, &VariableListEditor::updateList);

		this->newVariablesPrefix = tr("Input");
	}
	else if (editorType == VariableNature_t::output)
	{
		this->variablesList = new TableWidgetWithResizeEvent(0, 2, this);

		headerTexts.append(tr("Output"));

		connect(machineManager.get(), &MachineManager::machineOutputListChangedEvent, this, &VariableListEditor::updateList);

		this->newVariablesPrefix = tr("Output");
	}
	else if (editorType == VariableNature_t::internal)
	{
		this->variablesList = new TableWidgetWithResizeEvent(0, 3, this);

		headerTexts.append(tr("Variable"));
		headerTexts.append(tr("Initial value"));

		connect(machineManager.get(), &MachineManager::machineLocalVariableListChangedEvent, this, &VariableListEditor::updateList);

		this->newVariablesPrefix = tr("Variable");
	}
	else if (editorType == VariableNature_t::constant)
	{
		this->variablesList = new TableWidgetWithResizeEvent(0, 3, this);

		headerTexts.append(tr("Constant"));
		headerTexts.append(tr("Value"));

		connect(machineManager.get(), &MachineManager::machineConstantListChangedEvent, this, &VariableListEditor::updateList);

		this->newVariablesPrefix = tr("Constant");
	}

	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &VariableListEditor::updateList);

	this->newVariablesPrefix += " #";

	headerTexts.insert(1, tr("Size"));

	this->variablesList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->variablesList->verticalHeader()->setEnabled(false);

	this->variablesList->setHorizontalHeaderLabels(headerTexts);

	this->variablesList->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->listDelegate = new DynamicTableItemDelegate(variablesList);
	this->variablesList->setItemDelegate(listDelegate);

	// Don't allow to adjust height
	this->variablesList->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	layout->addWidget(this->variablesList);

	// Buttons

	this->buttonLayout = new QGridLayout();

	this->buttonAdd    = new QPushButton(tr("Add"),    this);
	this->buttonRemove = new QPushButton(tr("Remove"), this);
	this->buttonUp     = new QPushButton("↥",          this);
	this->buttonDown   = new QPushButton("↧",          this);

	// Row 0 reserved for OK/Cancel buttons
	// Row 1 for edition buttons
	// Size 1 for arrows, 20 for other buttons = 42
	// TODO: correct these sizes
	this->buttonLayout->addWidget(this->buttonUp,     1, 0,  1, 1);
	this->buttonLayout->addWidget(this->buttonDown,   1, 1,  1, 1);
	this->buttonLayout->addWidget(this->buttonAdd,    1, 2,  1, 20);
	this->buttonLayout->addWidget(this->buttonRemove, 1, 22, 1, 20);

	layout->addLayout(this->buttonLayout);

	connect(this->buttonUp,     &QAbstractButton::clicked, this, &VariableListEditor::raiseSelectedVariables);
	connect(this->buttonDown,   &QAbstractButton::clicked, this, &VariableListEditor::lowerSelectedVariables);
	connect(this->buttonAdd,    &QAbstractButton::clicked, this, &VariableListEditor::beginAddVariable);
	connect(this->buttonRemove, &QAbstractButton::clicked, this, &VariableListEditor::removeSelectedVariables);

	// To enable/disable buttons when a signal is selected
	connect(this->variablesList, &QTableWidget::itemSelectionChanged,  this, &VariableListEditor::updateButtonsEnableState);

	connect(this->variablesList, &TableWidgetWithResizeEvent::resized, this, &VariableListEditor::handleListResizedEvent);

	this->updateList();
}

void VariableListEditor::keyPressEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key::Key_Escape)
	{
		this->cancelCurrentEdit();
		transmitEvent = false;
	}
	else if (event->key() == Qt::Key::Key_Delete)
	{
		this->removeSelectedVariables();
		transmitEvent = false;
	}
	else if (event->key() == Qt::Key::Key_Tab)
	{
		transmitEvent = false;
	}

	if (transmitEvent == true)
	{
		QWidget::keyPressEvent(event);
	}
}

void VariableListEditor::keyReleaseEvent(QKeyEvent* event)
{
	bool transmitEvent = true;

	if (event->key() == Qt::Key::Key_Escape)
	{
		transmitEvent = false;
	}
	else if (event->key() == Qt::Key::Key_Delete)
	{
		transmitEvent = false;
	}
	else if (event->key() == Qt::Key::Key_Tab)
	{
		transmitEvent = false;
	}

	if (transmitEvent)
		QWidget::keyPressEvent(event);
}

void VariableListEditor::contextMenuEvent(QContextMenuEvent* event)
{
	QPoint correctedPos = variablesList->mapFromParent(event->pos());
	correctedPos.setX(correctedPos.x() - variablesList->verticalHeader()->width());
	correctedPos.setY(correctedPos.y() - variablesList->horizontalHeader()->height());
	QTableWidgetItem* cellUnderMouse = variablesList->itemAt(correctedPos);

	if (cellUnderMouse != nullptr)
	{
		QList<QString> list = this->getSelectedVariables();
		if (list.count() == 1)
		{
			int row = cellUnderMouse->row();
			this->currentVariableName = variablesList->item(row, 0);
			this->currentVariableSize = variablesList->item(row, 1);
			if (this->variablesList->columnCount() == 3)
				this->currentVariableValue = variablesList->item(row, 2);

			shared_ptr<Variable> currentSignal = associatedVariables[cellUnderMouse].lock();

			if (currentSignal != nullptr)
			{
				this->currentVariable = currentSignal;

				ContextMenu* menu = new ContextMenu();
				menu->addTitle(tr("Action on variable") + " <i>" + currentSignal->getName() + "</i>");

				QVariant data;
				QAction* actionToAdd = nullptr;

				if (cellUnderMouse->row() != 0)
				{
					actionToAdd = menu->addAction(tr("Up"));
					data.setValue((int)ContextAction_t::Up);
					actionToAdd->setData(data);
				}

				if (cellUnderMouse->row() != this->variablesList->rowCount()-1)
				{
					actionToAdd = menu->addAction(tr("Down"));
					data.setValue((int)ContextAction_t::Down);
					actionToAdd->setData(data);
				}

				menu->addSeparator();

				actionToAdd = menu->addAction(tr("Rename variable"));
				data.setValue((int)ContextAction_t::RenameVariable);
				actionToAdd->setData(data);

				actionToAdd = menu->addAction(tr("Resize variable"));
				data.setValue((int)ContextAction_t::ResizeVariable);
				actionToAdd->setData(data);

				if (this->editorType != VariableNature_t::output)
				{
					actionToAdd = menu->addAction(tr("Change variable value"));
					data.setValue((int)ContextAction_t::ChangeVariableValue);
					actionToAdd->setData(data);
				}

				menu->addSeparator();

				actionToAdd = menu->addAction(tr("Delete variable"));
				data.setValue((int)ContextAction_t::DeleteVariable);
				actionToAdd->setData(data);

				actionToAdd = menu->addAction(tr("Cancel"));
				data.setValue((int)ContextAction_t::Cancel);
				actionToAdd->setData(data);

				menu->popup(this->mapToGlobal(event->pos()));

				connect(menu, &QMenu::triggered, this, &VariableListEditor::processMenuEventHandler);
			}
		}
		else if (list.count() != 0)
		{
			ContextMenu* menu = new ContextMenu();
			menu->addTitle(tr("Action on all selected variables"));

			QVariant data;
			QAction* actionToAdd = nullptr;

			if (this->buttonUp->isEnabled())
			{
				actionToAdd = menu->addAction(tr("Up"));
				data.setValue((int)ContextAction_t::Up);
				actionToAdd->setData(data);
			}

			if (this->buttonDown->isEnabled())
			{
				actionToAdd = menu->addAction(tr("Down"));
				data.setValue((int)ContextAction_t::Down);
				actionToAdd->setData(data);
			}

			menu->addSeparator();

			actionToAdd = menu->addAction(tr("Delete variables"));
			data.setValue((int)ContextAction_t::DeleteVariable);
			actionToAdd->setData(data);

			actionToAdd = menu->addAction(tr("Cancel"));
			data.setValue((int)ContextAction_t::Cancel);
			actionToAdd->setData(data);

			menu->popup(this->mapToGlobal(event->pos()));

			connect(menu, &QMenu::triggered, this, &VariableListEditor::processMenuEventHandler);
		}
	}
}

/**
 * @brief VariableListEditor::updateList
 * UpdateList totally resets the widget.
 * The list is filled with signals existing
 * in machine at this time.
 * This can be called to take into account the
 * creation/edition of a signal, or to cancel
 * a creation/edition that has not been validated.
 */
void VariableListEditor::updateList()
{
	// Save previous selection if not overwritten
	if ( ( this->variableSelectionToRestore.isEmpty() ) && ( this->variablesList->selectedItems().count() != 0 ) )
	{
		for (QModelIndex index : this->variablesList->selectionModel()->selectedRows())
		{
			this->variableSelectionToRestore.append(this->variablesList->item(index.row(), 0)->text());
		}
	}

	this->variablesList->clearContents();
	this->variablesList->setRowCount(0);
	this->associatedVariables.clear();

	this->switchMode(ListMode_t::standard);

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	disconnect(this->variablesList,  &QTableWidget::itemSelectionChanged, this, &VariableListEditor::updateButtonsEnableState);

	// Get signals I have to deal with
	QList<shared_ptr<Variable>> signalsToAdd;

	if (this->editorType == VariableNature_t::input)
	{
		signalsToAdd = machine->getInputsAsSignals();
	}
	else if (this->editorType == VariableNature_t::output)
	{
		signalsToAdd = machine->getOutputsAsSignals();
	}
	else if (this->editorType == VariableNature_t::internal)
	{
		signalsToAdd = machine->getLocalVariables();
	}
	else if (this->editorType == VariableNature_t::constant)
	{
		signalsToAdd = machine->getConstants();
	}

	for (shared_ptr<Variable> sig : signalsToAdd)
	{
		this->variablesList->insertRow(this->variablesList->rowCount());

		// Signal name
		QTableWidgetItem* currentItem = new QTableWidgetItem(sig->getName());
		Qt::ItemFlags currentFlags = currentItem->flags();
		currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
		this->variablesList->setItem(this->variablesList->rowCount()-1, 0, currentItem);
		this->associatedVariables[currentItem] = sig;

		// Signal size
		currentItem = new QTableWidgetItem(QString::number(sig->getSize()));
		currentFlags = currentItem->flags();
		currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
		this->variablesList->setItem(this->variablesList->rowCount()-1, 1, currentItem);
		this->associatedVariables[currentItem] = sig;

		// Signal (initial) value
		if (this->variablesList->columnCount() == 3)
		{
			currentItem = new QTableWidgetItem(sig->getInitialValue().toString());
			currentFlags = currentItem->flags();
			currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
			this->variablesList->setItem(this->variablesList->rowCount()-1, 2, currentItem);
			this->associatedVariables[currentItem] = sig;
		}

		// Select signal if it was selected before list clear
		if ( ( ! this->variableSelectionToRestore.isEmpty() ) && ( this->variableSelectionToRestore.contains(sig->getName()) ) )
		{
			// Obtain current selection
			QItemSelection selectedItems = this->variablesList->selectionModel()->selection();

			// Select new row
			this->variablesList->selectRow(this->variablesList->rowCount()-1);

			// Merge selections
			selectedItems.merge(this->variablesList->selectionModel()->selection(), QItemSelectionModel::Select);
			this->variablesList->clearSelection();
			this->variablesList->selectionModel()->select(selectedItems, QItemSelectionModel::Select);
		}
	}

	this->variableSelectionToRestore.clear();
	this->updateButtonsEnableState();
	connect(this->variablesList,  &QTableWidget::itemSelectionChanged, this, &VariableListEditor::updateButtonsEnableState);
}

void VariableListEditor::updateButtonsEnableState()
{
	this->buttonRemove->setEnabled(false);
	this->buttonUp->setEnabled(false);
	this->buttonDown->setEnabled(false);

	if (this->currentMode == ListMode_t::standard)
	{
		this->buttonAdd->setEnabled(true);

		if (this->variablesList->selectedItems().count() != 0)
		{
			this->buttonRemove->setEnabled(true);

			// Up/down buttons : only enable when relevant
			QModelIndexList rows = this->variablesList->selectionModel()->selectedRows();

			// First sort selected rows
			QVector<int> sortedRows(rows.count());
			for (int i = 0 ; i < rows.count() ; i++)
			{
				sortedRows[i] = rows[i].row();
			}

			sort(sortedRows.begin(), sortedRows.end());

			int currentRow = sortedRows[0];
			bool areSuccesive = true;

			// Then check if one or multiple groups
			for (int i = 1 ; i < sortedRows.count() ; i++)
			{
				if (sortedRows[i] == currentRow+1)
					currentRow = sortedRows[i];
				else
				{
					areSuccesive = false;
					break;
				}
			}

			if (areSuccesive == false)
			{
				// Lacunar selection can always be moved up or down
				this->buttonUp->setEnabled(true);
				this->buttonDown->setEnabled(true);
			}
			else
			{
				// If single group, check if at top or at bottom
				if (sortedRows[0] != 0)
					this->buttonUp->setEnabled(true);

				if (sortedRows.last() != this->variablesList->rowCount()-1)
					this->buttonDown->setEnabled(true);
			}
		}
	}
	else
	{
		this->buttonAdd->setEnabled(false);
	}
}

void VariableListEditor::handleListResizedEvent()
{
	if (this->currentTableItem != nullptr)
	{
		this->variablesList->scrollToItem(this->currentTableItem);
	}
}

void VariableListEditor::beginAddVariable()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	QString initialName = machine->getUniqueSignalName(this->newVariablesPrefix);

	this->variablesList->insertRow(variablesList->rowCount());

	this->currentVariableName = new QTableWidgetItem(initialName);
	this->variablesList->setItem(this->variablesList->rowCount()-1, 0, this->currentVariableName);

	this->currentVariableSize = new QTableWidgetItem("1");
	Qt::ItemFlags currentFlags = this->currentVariableSize->flags();
	this->currentVariableSize->setFlags(currentFlags & ~Qt::ItemIsEditable);
	this->variablesList->setItem(this->variablesList->rowCount()-1, 1, this->currentVariableSize);

	if (this->variablesList->columnCount() == 3)
	{
		this->currentVariableValue = new QTableWidgetItem("0");
		currentFlags = this->currentVariableValue->flags();
		this->currentVariableValue->setFlags(currentFlags & ~Qt::ItemIsEditable);
		this->variablesList->setItem(this->variablesList->rowCount()-1, 2, this->currentVariableValue);
	}

	this->currentTableItem = this->currentVariableName;
	this->variablesList->selectRow(this->currentTableItem->row());
	this->setFocus(); // Prevent temporarily loosing focus when disabling buttons on mode switch
	switchMode(ListMode_t::addingVariable);

	connect(this->variablesList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingVariableSwitchField);
	connect(this->variablesList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingVariableCurrentItemChanged);
}

void VariableListEditor::addingVariableSwitchField(QTableWidgetItem* newItem)
{
	// Ignore clicks on disabled cells
	if ( (newItem->flags() & Qt::ItemIsEnabled) == 0 ) return;

	this->variablesList->closePersistentEditor(this->currentTableItem);

	// Resize value if needed
	if ( (this->currentTableItem == this->currentVariableSize) || (this->currentTableItem == this->currentVariableValue) )
	{
		this->fixVariableSize();
	}

	Qt::ItemFlags currentFlags = this->currentTableItem->flags();
	this->currentTableItem->setFlags(currentFlags & ~Qt::ItemIsEditable);

	this->currentTableItem = newItem;

	this->editCurrentCell();
}

void VariableListEditor::addingVariableCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem*)
{
	if (current != nullptr)
	{
		this->addingVariableSwitchField(current);
	}
}

void VariableListEditor::endAddVariable()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	disconnect(this->variablesList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingVariableCurrentItemChanged);
	disconnect(this->variablesList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingVariableSwitchField);
	this->variablesList->closePersistentEditor(this->currentTableItem);

	QString finalName;

	if (this->currentTableItem == this->currentVariableName)
	{
		DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
		finalName = editor->text();
	}
	else
	{
		finalName = this->currentVariableName->text();
	}

	// Overwrite selection to select new signal name
	LogicValue initialValue = LogicValue::getNullValue();

	if (this->currentVariableValue != nullptr)
	{
		try
		{
			initialValue = LogicValue::fromString(this->currentVariableValue->text()); // Throws StatesException
			uint size = (uint)this->currentVariableSize->text().toInt();

			if (initialValue.getSize() < size)
				initialValue.resize(size); // Throws StatesException
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueError_t::unsupported_char) )
			{
				qDebug() << "(SignalListEditor:) Info: Wrong input for initial value, change ignored.";
			}
			else
				throw;
		}
	}
	else
	{
		// To force correct size (actual value is ignored)
		initialValue = LogicValue(this->currentVariableSize->text().toInt());
	}

	// If success, list is reloaded through events,
	// which resets mode.
	shared_ptr<Variable> newSignal = machine->addSignal(this->editorType, finalName, initialValue);
	machineManager->notifyMachineEdited();

	// If adding signal failed, continue editing signal name
	if (newSignal == nullptr)
	{
		this->fixVariableSize();

		this->currentTableItem = this->currentVariableName;
		this->currentVariableName->setText(finalName);

		connect(this->variablesList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingVariableCurrentItemChanged);
		connect(this->variablesList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingVariableSwitchField);
		this->editCurrentCell(true);
	}
}


// Begin edit signal is trigerred by double-click on table item
void VariableListEditor::beginEditVariable(QTableWidgetItem* characteristicToEdit)
{
	this->currentTableItem = characteristicToEdit;

	if (characteristicToEdit->column() == 0)
	{
		switchMode(ListMode_t::renamingVariable);
	}
	else if (characteristicToEdit->column() == 1)
	{
		switchMode(ListMode_t::resizingVariable);
	}
	else if (characteristicToEdit->column() == 2)
	{
		switchMode(ListMode_t::changingVariableInitialValue);
	}
}

void VariableListEditor::endRenameVariable()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

	QString finalName = editor->text();

	shared_ptr<Variable> currentSignal = this->associatedVariables[currentTableItem].lock();

	if ( (currentSignal != nullptr) && (finalName != currentSignal->getName()) )
	{
		// Overwrite selection to select new signal name
		this->variableSelectionToRestore.append(finalName);
		bool success = machine->renameSignal(currentSignal->getName(), finalName);

		if (success == true)
		{
			machineManager->notifyMachineEdited();
		}
		else
		{
			this->variableSelectionToRestore.clear();
			this->editCurrentCell(true);
		}
	}
	else
	{
		// Reset list
		updateList();
	}
}

void VariableListEditor::endResizeVariable()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	try
	{
		DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

		uint finalSize = (uint)editor->text().toInt();

		shared_ptr<Variable> currentSignal = this->associatedVariables[currentTableItem].lock();

		if ( (currentSignal != nullptr) && (finalSize != currentSignal->getSize()) )
		{
			machine->resizeSignal(currentSignal->getName(), finalSize); // Throws StatesException
			machineManager->notifyMachineEdited();
		}
		else
		{
			// Reset list
			updateList();
		}
	}
	catch (const StatesException& e)
	{
		if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueError_t::resized_to_0) )
		{
			qDebug() << "(SignalListEditor:) Info: Wrong input for variable size, change ignored.";
			this->editCurrentCell(true);
		}
		else
			throw;
	}
}

void VariableListEditor::endChangeVariableInitialValue()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	try
	{
		DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

		LogicValue newInitialValue = LogicValue::fromString(editor->text()); // Throws StatesException

		shared_ptr<Variable> currentSignal = this->associatedVariables[currentTableItem].lock();

		if ( (currentSignal != nullptr) && (newInitialValue != currentSignal->getInitialValue()) )
		{
			if (newInitialValue.getSize() < currentSignal->getSize())
				newInitialValue.resize(currentSignal->getSize()); // Throws StatesException

			machine->changeSignalInitialValue(currentSignal->getName(), newInitialValue); // Throws StatesException
			machineManager->notifyMachineEdited();
		}
		else
		{
			updateList();
		}
	}
	catch (const StatesException& e)
	{
		if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == VariableError_t::size_mismatch) )
		{
			qDebug() << "(SignalListEditor:) Info: Wrong input for variable initial value, change ignored.";
			this->editCurrentCell(true);
		}
		else
			throw;
	}
}

void VariableListEditor::validateCurrentEdit()
{
	if (this->currentMode == ListMode_t::addingVariable)
	{
		this->endAddVariable();
	}
	else if (this->currentMode == ListMode_t::renamingVariable)
	{
		this->endRenameVariable();
	}
	else if (this->currentMode == ListMode_t::resizingVariable)
	{
		this->endResizeVariable();
	}
	else if (this->currentMode == ListMode_t::changingVariableInitialValue)
	{
		this->endChangeVariableInitialValue();
	}
}

void VariableListEditor::cancelCurrentEdit()
{
	if (this->currentMode != ListMode_t::standard)
	{
		if (this->currentMode == ListMode_t::addingVariable)
		{
			disconnect(this->variablesList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingVariableCurrentItemChanged);
			disconnect(this->variablesList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingVariableSwitchField);
		}

		// Reset list
		this->updateList();
	}
}

void VariableListEditor::raiseSelectedVariables()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	// Update list to make sure selection order matches list order
	this->updateList();

	QList<int>  signalsRanks;

	QModelIndexList rows = this->variablesList->selectionModel()->selectedRows();
	for (QModelIndex index : rows)
	{
		signalsRanks.append(index.row());
	}

	if (rows.isEmpty() == false)
	{
		for (int i = 0 ; i < rows.count() ; i++)
		{
			if ( (signalsRanks.at(i) != 0) && ( ! this->variablesList->item(signalsRanks.at(i)-1, 0)->isSelected() ) )
			{
				// Actually lower upper signals rater than raising signal itself
				machine->changeSignalRank(this->variablesList->item(signalsRanks.at(i)-1, 0)->text(), signalsRanks.at(i));
			}
		}
		machineManager->notifyMachineEdited();
	}
}

void VariableListEditor::lowerSelectedVariables()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	// Update list to make sure selection order matches list order
	this->updateList();

	QList<int> signalsRanks;

	QModelIndexList rows = this->variablesList->selectionModel()->selectedRows();
	for (QModelIndex index : rows)
	{
		signalsRanks.push_front(index.row());
	}

	if (rows.isEmpty() == false)
	{
		for (int i = 0 ; i < rows.count() ; i++)
		{
			if ( (signalsRanks.at(i) != this->variablesList->rowCount()-1) && ( ! this->variablesList->item(signalsRanks.at(i)+1, 0)->isSelected() ) )
			{
				// Actually raise lower signals rater than lowering signal itself
				machine->changeSignalRank(this->variablesList->item(signalsRanks.at(i)+1, 0)->text(), signalsRanks.at(i));
			}
		}
		machineManager->notifyMachineEdited();
	}
}

void VariableListEditor::removeSelectedVariables()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	QStringList selection;
	int lastSelectionIndex = -1;

	for (QModelIndex index : this->variablesList->selectionModel()->selectedRows())
	{
		selection.append(this->variablesList->item(index.row(), 0)->text());
		if (lastSelectionIndex < index.row())
			lastSelectionIndex = index.row();
	}

	// Overwrite selection to select next signal in list (if not last)
	if (lastSelectionIndex < this->variablesList->rowCount()-1)
		this->variableSelectionToRestore.append(variablesList->item(lastSelectionIndex+1,0)->text());

	if (selection.isEmpty() == false)
	{
		for (QString signalName : selection)
		{
			machine->deleteSignal(signalName);
		}
		machineManager->notifyMachineEdited();
	}
}

void VariableListEditor::processMenuEventHandler(QAction* action)
{
	QVariant data = action->data();
	ContextAction_t dataValue = ContextAction_t(data.toInt());

	switch (dataValue)
	{
	case ContextAction_t::Cancel:
		break;
	case ContextAction_t::DeleteVariable:
		this->removeSelectedVariables();
		break;
	case ContextAction_t::Up:
		this->raiseSelectedVariables();
		break;
	case ContextAction_t::Down:
		this->lowerSelectedVariables();
		break;
	case ContextAction_t::RenameVariable:
		this->beginEditVariable(this->currentVariableName);
		break;
	case ContextAction_t::ResizeVariable:
		this->beginEditVariable(this->currentVariableSize);
		break;
	case ContextAction_t::ChangeVariableValue:
		this->beginEditVariable(this->currentVariableValue);
		break;
	}
}

void VariableListEditor::switchMode(ListMode_t newMode)
{
	if (newMode != this->currentMode)
	{
		this->currentMode = newMode;

		this->updateButtonsEnableState();

		if (newMode == ListMode_t::standard)
		{
			this->currentTableItem   = nullptr;
			this->currentVariableName  = nullptr;
			this->currentVariableSize  = nullptr;
			this->currentVariableValue = nullptr;

			this->listDelegate->setValidator(nullptr);

			delete this->buttonCancel;
			delete this->buttonOK;

			this->buttonCancel = nullptr;
			this->buttonOK     = nullptr;

			connect(this->variablesList, &QTableWidget::itemDoubleClicked, this, &VariableListEditor::beginEditVariable);
		}
		else if ( (newMode == ListMode_t::addingVariable)  || (newMode == ListMode_t::renamingVariable) || (newMode == ListMode_t::resizingVariable) || (newMode == ListMode_t::changingVariableInitialValue) )
		{
			disconnect(variablesList, &QTableWidget::itemDoubleClicked, this, &VariableListEditor::beginEditVariable);

			this->buttonOK = new QPushButton(tr("OK"));
			this->buttonLayout->addWidget(this->buttonOK, 0, 0, 1, 21);
			connect(this->buttonOK, &QPushButton::clicked, this, &VariableListEditor::validateCurrentEdit);

			this->buttonCancel = new QPushButton(tr("Cancel"));
			this->buttonLayout->addWidget(this->buttonCancel, 0, 21, 1, 21);
			connect(this->buttonCancel, &QPushButton::clicked, this, &VariableListEditor::cancelCurrentEdit);

			// Disable all items in list except edited item row
			// (TODO: Should also disable other lists to avoid begin another
			// edit while current could be faulty)
			for (int i = 0 ; i < this->variablesList->rowCount() ; i++)
			{
				if (i != this->currentTableItem->row())
				{
					for (int j = 0 ; j < this->variablesList->columnCount() ; j++)
					{
						Qt::ItemFlags currentFlags = this->variablesList->item(i, j)->flags();
						this->variablesList->item(i, j)->setFlags(currentFlags & ~Qt::ItemIsEnabled);
					}
				}
			}

			this->editCurrentCell();
		}
	}
}

void VariableListEditor::editCurrentCell(bool erroneous)
{
	//
	// Set validator

	if (this->currentMode == ListMode_t::resizingVariable)
	{
		this->listDelegate->setValidator(shared_ptr<QValidator>(new QIntValidator(1, 64)));
	}
	else if (this->currentMode == ListMode_t::changingVariableInitialValue)
	{
		shared_ptr<Variable> currentSignal = this->associatedVariables[this->currentTableItem].lock();

		if (currentSignal != nullptr)
		{
			// A string made of only '0' and '1' chars, witch length is between 0 and size
			QRegularExpression re("[01]{0," + QString::number(currentSignal->getSize()) + "}");
			this->listDelegate->setValidator(shared_ptr<QValidator>(new QRegularExpressionValidator(re)));
		}
	}
	else if (this->currentMode == ListMode_t::addingVariable)
	{
		if (this->currentTableItem == this->currentVariableSize)
		{
			this->listDelegate->setValidator(shared_ptr<QValidator>(new QIntValidator(1, 64)));
		}
		else if (this->currentTableItem == this->currentVariableValue)
		{
			uint currentSize = (uint)this->currentVariableSize->text().toInt();

			// A string made of only '0' and '1' chars, witch length is between 0 and size
			QRegularExpression re("[01]{0," + QString::number(currentSize) + "}");
			this->listDelegate->setValidator(shared_ptr<QValidator>(new QRegularExpressionValidator(re)));
		}
		else
		{
			this->listDelegate->setValidator(nullptr);
		}
	}
	else // Renaming
	{
		this->listDelegate->setValidator(nullptr);
	}

	//
	// Begin edition

	Qt::ItemFlags currentFlags = this->currentTableItem->flags();
	this->currentTableItem->setFlags(currentFlags | Qt::ItemIsEditable);

	this->variablesList->openPersistentEditor(this->currentTableItem);

	DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
	connect(editor, &DynamicLineEdit::returnPressed, this, &VariableListEditor::validateCurrentEdit);

	// Done

	if (erroneous)
	{
		editor->markAsErroneous();
	}

	editor->setFocus();
}

void VariableListEditor::fixVariableSize()
{
	if(this->currentVariableValue != nullptr)
	{
		try
		{
			LogicValue currentInitialValue = LogicValue::fromString(this->currentVariableValue->text()); // Throws StatesException

			uint newSize = (uint)this->currentVariableSize->text().toInt();
			currentInitialValue.resize(newSize); // Throws StatesException

			this->currentVariableValue->setText(currentInitialValue.toString());
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueError_t::unsupported_char) )
			{
				qDebug() << "(SignalListEditor:) Info: Wrong input for initial value, change ignored.";
			}
			else
				throw;
		}
	}
}

QList<QString> VariableListEditor::getSelectedVariables()
{
	QList<QString> selectionString;

	for (QModelIndex index : this->variablesList->selectionModel()->selectedRows())
	{
		QTableWidgetItem* currentItem = this->variablesList->item(index.row(), 1);
		if (currentItem != nullptr)
			selectionString.append(currentItem->text());
	}

	return selectionString;
}
