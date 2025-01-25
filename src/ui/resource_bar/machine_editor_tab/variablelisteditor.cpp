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
		this->signalsList = new TableWidgetWithResizeEvent(0, 3, this);

		headerTexts.append(tr("Input"));
		headerTexts.append(tr("Initial value"));

		connect(machineManager.get(), &MachineManager::machineInputListChangedEvent, this, &VariableListEditor::updateList);

		this->newSignalsPrefix = tr("Input");
	}
	else if (editorType == VariableNature_t::output)
	{
		this->signalsList = new TableWidgetWithResizeEvent(0, 2, this);

		headerTexts.append(tr("Output"));

		connect(machineManager.get(), &MachineManager::machineOutputListChangedEvent, this, &VariableListEditor::updateList);

		this->newSignalsPrefix = tr("Output");
	}
	else if (editorType == VariableNature_t::internal)
	{
		this->signalsList = new TableWidgetWithResizeEvent(0, 3, this);

		headerTexts.append(tr("Variable"));
		headerTexts.append(tr("Initial value"));

		connect(machineManager.get(), &MachineManager::machineLocalVariableListChangedEvent, this, &VariableListEditor::updateList);

		this->newSignalsPrefix = tr("Variable");
	}
	else if (editorType == VariableNature_t::constant)
	{
		this->signalsList = new TableWidgetWithResizeEvent(0, 3, this);

		headerTexts.append(tr("Constant"));
		headerTexts.append(tr("Value"));

		connect(machineManager.get(), &MachineManager::machineConstantListChangedEvent, this, &VariableListEditor::updateList);

		this->newSignalsPrefix = tr("Constant");
	}

	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &VariableListEditor::updateList);

	this->newSignalsPrefix += " #";

	headerTexts.insert(1, tr("Size"));

	this->signalsList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->signalsList->verticalHeader()->setEnabled(false);

	this->signalsList->setHorizontalHeaderLabels(headerTexts);

	this->signalsList->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->listDelegate = new DynamicTableItemDelegate(signalsList);
	this->signalsList->setItemDelegate(listDelegate);

	// Don't allow to adjust height
	this->signalsList->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	layout->addWidget(this->signalsList);

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

	connect(this->buttonUp,     &QAbstractButton::clicked, this, &VariableListEditor::raiseSelectedSignals);
	connect(this->buttonDown,   &QAbstractButton::clicked, this, &VariableListEditor::lowerSelectedSignals);
	connect(this->buttonAdd,    &QAbstractButton::clicked, this, &VariableListEditor::beginAddSignal);
	connect(this->buttonRemove, &QAbstractButton::clicked, this, &VariableListEditor::removeSelectedSignals);

	// To enable/disable buttons when a signal is selected
	connect(this->signalsList, &QTableWidget::itemSelectionChanged,  this, &VariableListEditor::updateButtonsEnableState);

	connect(this->signalsList, &TableWidgetWithResizeEvent::resized, this, &VariableListEditor::handleListResizedEvent);

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
		this->removeSelectedSignals();
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
	QPoint correctedPos = signalsList->mapFromParent(event->pos());
	correctedPos.setX(correctedPos.x() - signalsList->verticalHeader()->width());
	correctedPos.setY(correctedPos.y() - signalsList->horizontalHeader()->height());
	QTableWidgetItem* cellUnderMouse = signalsList->itemAt(correctedPos);

	if (cellUnderMouse != nullptr)
	{
		QList<QString> list = this->getSelectedSignals();
		if (list.count() == 1)
		{
			int row = cellUnderMouse->row();
			this->currentSignalName = signalsList->item(row, 0);
			this->currentSignalSize = signalsList->item(row, 1);
			if (this->signalsList->columnCount() == 3)
				this->currentSignalValue = signalsList->item(row, 2);

			shared_ptr<Variable> currentSignal = associatedSignals[cellUnderMouse].lock();

			if (currentSignal != nullptr)
			{
				this->currentSignal = currentSignal;

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

				if (cellUnderMouse->row() != this->signalsList->rowCount()-1)
				{
					actionToAdd = menu->addAction(tr("Down"));
					data.setValue((int)ContextAction_t::Down);
					actionToAdd->setData(data);
				}

				menu->addSeparator();

				actionToAdd = menu->addAction(tr("Rename variable"));
				data.setValue((int)ContextAction_t::RenameSignal);
				actionToAdd->setData(data);

				actionToAdd = menu->addAction(tr("Resize variable"));
				data.setValue((int)ContextAction_t::ResizeSignal);
				actionToAdd->setData(data);

				if (this->editorType != VariableNature_t::output)
				{
					actionToAdd = menu->addAction(tr("Change variable value"));
					data.setValue((int)ContextAction_t::ChangeSignalValue);
					actionToAdd->setData(data);
				}

				menu->addSeparator();

				actionToAdd = menu->addAction(tr("Delete variable"));
				data.setValue((int)ContextAction_t::DeleteSignal);
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
			data.setValue((int)ContextAction_t::DeleteSignal);
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
	if ( ( this->signalSelectionToRestore.isEmpty() ) && ( this->signalsList->selectedItems().count() != 0 ) )
	{
		for (QModelIndex index : this->signalsList->selectionModel()->selectedRows())
		{
			this->signalSelectionToRestore.append(this->signalsList->item(index.row(), 0)->text());
		}
	}

	this->signalsList->clearContents();
	this->signalsList->setRowCount(0);
	this->associatedSignals.clear();

	this->switchMode(ListMode_t::standard);

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	disconnect(this->signalsList,  &QTableWidget::itemSelectionChanged, this, &VariableListEditor::updateButtonsEnableState);

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
		this->signalsList->insertRow(this->signalsList->rowCount());

		// Signal name
		QTableWidgetItem* currentItem = new QTableWidgetItem(sig->getName());
		Qt::ItemFlags currentFlags = currentItem->flags();
		currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
		this->signalsList->setItem(this->signalsList->rowCount()-1, 0, currentItem);
		this->associatedSignals[currentItem] = sig;

		// Signal size
		currentItem = new QTableWidgetItem(QString::number(sig->getSize()));
		currentFlags = currentItem->flags();
		currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
		this->signalsList->setItem(this->signalsList->rowCount()-1, 1, currentItem);
		this->associatedSignals[currentItem] = sig;

		// Signal (initial) value
		if (this->signalsList->columnCount() == 3)
		{
			currentItem = new QTableWidgetItem(sig->getInitialValue().toString());
			currentFlags = currentItem->flags();
			currentItem->setFlags(currentFlags & ~Qt::ItemIsEditable);
			this->signalsList->setItem(this->signalsList->rowCount()-1, 2, currentItem);
			this->associatedSignals[currentItem] = sig;
		}

		// Select signal if it was selected before list clear
		if ( ( ! this->signalSelectionToRestore.isEmpty() ) && ( this->signalSelectionToRestore.contains(sig->getName()) ) )
		{
			// Obtain current selection
			QItemSelection selectedItems = this->signalsList->selectionModel()->selection();

			// Select new row
			this->signalsList->selectRow(this->signalsList->rowCount()-1);

			// Merge selections
			selectedItems.merge(this->signalsList->selectionModel()->selection(), QItemSelectionModel::Select);
			this->signalsList->clearSelection();
			this->signalsList->selectionModel()->select(selectedItems, QItemSelectionModel::Select);
		}
	}

	this->signalSelectionToRestore.clear();
	this->updateButtonsEnableState();
	connect(this->signalsList,  &QTableWidget::itemSelectionChanged, this, &VariableListEditor::updateButtonsEnableState);
}

void VariableListEditor::updateButtonsEnableState()
{
	this->buttonRemove->setEnabled(false);
	this->buttonUp->setEnabled(false);
	this->buttonDown->setEnabled(false);

	if (this->currentMode == ListMode_t::standard)
	{
		this->buttonAdd->setEnabled(true);

		if (this->signalsList->selectedItems().count() != 0)
		{
			this->buttonRemove->setEnabled(true);

			// Up/down buttons : only enable when relevant
			QModelIndexList rows = this->signalsList->selectionModel()->selectedRows();

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

				if (sortedRows.last() != this->signalsList->rowCount()-1)
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
		this->signalsList->scrollToItem(this->currentTableItem);
	}
}

void VariableListEditor::beginAddSignal()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	QString initialName = machine->getUniqueSignalName(this->newSignalsPrefix);

	this->signalsList->insertRow(signalsList->rowCount());

	this->currentSignalName = new QTableWidgetItem(initialName);
	this->signalsList->setItem(this->signalsList->rowCount()-1, 0, this->currentSignalName);

	this->currentSignalSize = new QTableWidgetItem("1");
	Qt::ItemFlags currentFlags = this->currentSignalSize->flags();
	this->currentSignalSize->setFlags(currentFlags & ~Qt::ItemIsEditable);
	this->signalsList->setItem(this->signalsList->rowCount()-1, 1, this->currentSignalSize);

	if (this->signalsList->columnCount() == 3)
	{
		this->currentSignalValue = new QTableWidgetItem("0");
		currentFlags = this->currentSignalValue->flags();
		this->currentSignalValue->setFlags(currentFlags & ~Qt::ItemIsEditable);
		this->signalsList->setItem(this->signalsList->rowCount()-1, 2, this->currentSignalValue);
	}

	this->currentTableItem = this->currentSignalName;
	this->signalsList->selectRow(this->currentTableItem->row());
	this->setFocus(); // Prevent temporarily loosing focus when disabling buttons on mode switch
	switchMode(ListMode_t::addingSignal);

	connect(this->signalsList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingSignalSwitchField);
	connect(this->signalsList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingSignalCurrentItemChanged);
}

void VariableListEditor::addingSignalSwitchField(QTableWidgetItem* newItem)
{
	// Ignore clicks on disabled cells
	if ( (newItem->flags() & Qt::ItemIsEnabled) == 0 ) return;

	this->signalsList->closePersistentEditor(this->currentTableItem);

	// Resize value if needed
	if ( (this->currentTableItem == this->currentSignalSize) || (this->currentTableItem == this->currentSignalValue) )
	{
		this->fixSignalSize();
	}

	Qt::ItemFlags currentFlags = this->currentTableItem->flags();
	this->currentTableItem->setFlags(currentFlags & ~Qt::ItemIsEditable);

	this->currentTableItem = newItem;

	this->editCurrentCell();
}

void VariableListEditor::addingSignalCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem*)
{
	if (current != nullptr)
	{
		this->addingSignalSwitchField(current);
	}
}

void VariableListEditor::endAddSignal()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	disconnect(this->signalsList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingSignalCurrentItemChanged);
	disconnect(this->signalsList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingSignalSwitchField);
	this->signalsList->closePersistentEditor(this->currentTableItem);

	QString finalName;

	if (this->currentTableItem == this->currentSignalName)
	{
		DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
		finalName = editor->text();
	}
	else
	{
		finalName = this->currentSignalName->text();
	}

	// Overwrite selection to select new signal name
	LogicValue initialValue = LogicValue::getNullValue();

	if (this->currentSignalValue != nullptr)
	{
		try
		{
			initialValue = LogicValue::fromString(this->currentSignalValue->text()); // Throws StatesException
			uint size = (uint)this->currentSignalSize->text().toInt();

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
		initialValue = LogicValue(this->currentSignalSize->text().toInt());
	}

	// If success, list is reloaded through events,
	// which resets mode.
	shared_ptr<Variable> newSignal = machine->addSignal(this->editorType, finalName, initialValue);
	machineManager->notifyMachineEdited();

	// If adding signal failed, continue editing signal name
	if (newSignal == nullptr)
	{
		this->fixSignalSize();

		this->currentTableItem = this->currentSignalName;
		this->currentSignalName->setText(finalName);

		connect(this->signalsList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingSignalCurrentItemChanged);
		connect(this->signalsList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingSignalSwitchField);
		this->editCurrentCell(true);
	}
}


// Begin edit signal is trigerred by double-click on table item
void VariableListEditor::beginEditSignal(QTableWidgetItem* characteristicToEdit)
{
	this->currentTableItem = characteristicToEdit;

	if (characteristicToEdit->column() == 0)
	{
		switchMode(ListMode_t::renamingSignal);
	}
	else if (characteristicToEdit->column() == 1)
	{
		switchMode(ListMode_t::resizingSignal);
	}
	else if (characteristicToEdit->column() == 2)
	{
		switchMode(ListMode_t::changingSignalInitialValue);
	}
}

void VariableListEditor::endRenameSignal()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

	QString finalName = editor->text();

	shared_ptr<Variable> currentSignal = this->associatedSignals[currentTableItem].lock();

	if ( (currentSignal != nullptr) && (finalName != currentSignal->getName()) )
	{
		// Overwrite selection to select new signal name
		this->signalSelectionToRestore.append(finalName);
		bool success = machine->renameSignal(currentSignal->getName(), finalName);

		if (success == true)
		{
			machineManager->notifyMachineEdited();
		}
		else
		{
			this->signalSelectionToRestore.clear();
			this->editCurrentCell(true);
		}
	}
	else
	{
		// Reset list
		updateList();
	}
}

void VariableListEditor::endResizeSignal()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	try
	{
		DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

		uint finalSize = (uint)editor->text().toInt();

		shared_ptr<Variable> currentSignal = this->associatedSignals[currentTableItem].lock();

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

void VariableListEditor::endChangeSignalInitialValue()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	try
	{
		DynamicLineEdit* editor = this->listDelegate->getCurentEditor();

		LogicValue newInitialValue = LogicValue::fromString(editor->text()); // Throws StatesException

		shared_ptr<Variable> currentSignal = this->associatedSignals[currentTableItem].lock();

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
		if ( (e.getSourceClass() == "Signal") && (e.getEnumValue() == SignalError_t::size_mismatch) )
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
	if (this->currentMode == ListMode_t::addingSignal)
	{
		this->endAddSignal();
	}
	else if (this->currentMode == ListMode_t::renamingSignal)
	{
		this->endRenameSignal();
	}
	else if (this->currentMode == ListMode_t::resizingSignal)
	{
		this->endResizeSignal();
	}
	else if (this->currentMode == ListMode_t::changingSignalInitialValue)
	{
		this->endChangeSignalInitialValue();
	}
}

void VariableListEditor::cancelCurrentEdit()
{
	if (this->currentMode != ListMode_t::standard)
	{
		if (this->currentMode == ListMode_t::addingSignal)
		{
			disconnect(this->signalsList, &QTableWidget::currentItemChanged, this, &VariableListEditor::addingSignalCurrentItemChanged);
			disconnect(this->signalsList, &QTableWidget::itemClicked,        this, &VariableListEditor::addingSignalSwitchField);
		}

		// Reset list
		this->updateList();
	}
}

void VariableListEditor::raiseSelectedSignals()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	// Update list to make sure selection order matches list order
	this->updateList();

	QList<int>  signalsRanks;

	QModelIndexList rows = this->signalsList->selectionModel()->selectedRows();
	for (QModelIndex index : rows)
	{
		signalsRanks.append(index.row());
	}

	if (rows.isEmpty() == false)
	{
		for (int i = 0 ; i < rows.count() ; i++)
		{
			if ( (signalsRanks.at(i) != 0) && ( ! this->signalsList->item(signalsRanks.at(i)-1, 0)->isSelected() ) )
			{
				// Actually lower upper signals rater than raising signal itself
				machine->changeSignalRank(this->signalsList->item(signalsRanks.at(i)-1, 0)->text(), signalsRanks.at(i));
			}
		}
		machineManager->notifyMachineEdited();
	}
}

void VariableListEditor::lowerSelectedSignals()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	// Update list to make sure selection order matches list order
	this->updateList();

	QList<int> signalsRanks;

	QModelIndexList rows = this->signalsList->selectionModel()->selectedRows();
	for (QModelIndex index : rows)
	{
		signalsRanks.push_front(index.row());
	}

	if (rows.isEmpty() == false)
	{
		for (int i = 0 ; i < rows.count() ; i++)
		{
			if ( (signalsRanks.at(i) != this->signalsList->rowCount()-1) && ( ! this->signalsList->item(signalsRanks.at(i)+1, 0)->isSelected() ) )
			{
				// Actually raise lower signals rater than lowering signal itself
				machine->changeSignalRank(this->signalsList->item(signalsRanks.at(i)+1, 0)->text(), signalsRanks.at(i));
			}
		}
		machineManager->notifyMachineEdited();
	}
}

void VariableListEditor::removeSelectedSignals()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	QStringList selection;
	int lastSelectionIndex = -1;

	for (QModelIndex index : this->signalsList->selectionModel()->selectedRows())
	{
		selection.append(this->signalsList->item(index.row(), 0)->text());
		if (lastSelectionIndex < index.row())
			lastSelectionIndex = index.row();
	}

	// Overwrite selection to select next signal in list (if not last)
	if (lastSelectionIndex < this->signalsList->rowCount()-1)
		this->signalSelectionToRestore.append(signalsList->item(lastSelectionIndex+1,0)->text());

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
	case ContextAction_t::DeleteSignal:
		this->removeSelectedSignals();
		break;
	case ContextAction_t::Up:
		this->raiseSelectedSignals();
		break;
	case ContextAction_t::Down:
		this->lowerSelectedSignals();
		break;
	case ContextAction_t::RenameSignal:
		this->beginEditSignal(this->currentSignalName);
		break;
	case ContextAction_t::ResizeSignal:
		this->beginEditSignal(this->currentSignalSize);
		break;
	case ContextAction_t::ChangeSignalValue:
		this->beginEditSignal(this->currentSignalValue);
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
			this->currentSignalName  = nullptr;
			this->currentSignalSize  = nullptr;
			this->currentSignalValue = nullptr;

			this->listDelegate->setValidator(nullptr);

			delete this->buttonCancel;
			delete this->buttonOK;

			this->buttonCancel = nullptr;
			this->buttonOK     = nullptr;

			connect(this->signalsList, &QTableWidget::itemDoubleClicked, this, &VariableListEditor::beginEditSignal);
		}
		else if ( (newMode == ListMode_t::addingSignal)  || (newMode == ListMode_t::renamingSignal) || (newMode == ListMode_t::resizingSignal) || (newMode == ListMode_t::changingSignalInitialValue) )
		{
			disconnect(signalsList, &QTableWidget::itemDoubleClicked, this, &VariableListEditor::beginEditSignal);

			this->buttonOK = new QPushButton(tr("OK"));
			this->buttonLayout->addWidget(this->buttonOK, 0, 0, 1, 21);
			connect(this->buttonOK, &QPushButton::clicked, this, &VariableListEditor::validateCurrentEdit);

			this->buttonCancel = new QPushButton(tr("Cancel"));
			this->buttonLayout->addWidget(this->buttonCancel, 0, 21, 1, 21);
			connect(this->buttonCancel, &QPushButton::clicked, this, &VariableListEditor::cancelCurrentEdit);

			// Disable all items in list except edited item row
			// (TODO: Should also disable other lists to avoid begin another
			// edit while current could be faulty)
			for (int i = 0 ; i < this->signalsList->rowCount() ; i++)
			{
				if (i != this->currentTableItem->row())
				{
					for (int j = 0 ; j < this->signalsList->columnCount() ; j++)
					{
						Qt::ItemFlags currentFlags = this->signalsList->item(i, j)->flags();
						this->signalsList->item(i, j)->setFlags(currentFlags & ~Qt::ItemIsEnabled);
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

	if (this->currentMode == ListMode_t::resizingSignal)
	{
		this->listDelegate->setValidator(shared_ptr<QValidator>(new QIntValidator(1, 64)));
	}
	else if (this->currentMode == ListMode_t::changingSignalInitialValue)
	{
		shared_ptr<Variable> currentSignal = this->associatedSignals[this->currentTableItem].lock();

		if (currentSignal != nullptr)
		{
			// A string made of only '0' and '1' chars, witch length is between 0 and size
			QRegularExpression re("[01]{0," + QString::number(currentSignal->getSize()) + "}");
			this->listDelegate->setValidator(shared_ptr<QValidator>(new QRegularExpressionValidator(re)));
		}
	}
	else if (this->currentMode == ListMode_t::addingSignal)
	{
		if (this->currentTableItem == this->currentSignalSize)
		{
			this->listDelegate->setValidator(shared_ptr<QValidator>(new QIntValidator(1, 64)));
		}
		else if (this->currentTableItem == this->currentSignalValue)
		{
			uint currentSize = (uint)this->currentSignalSize->text().toInt();

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

	this->signalsList->openPersistentEditor(this->currentTableItem);

	DynamicLineEdit* editor = this->listDelegate->getCurentEditor();
	connect(editor, &DynamicLineEdit::returnPressed, this, &VariableListEditor::validateCurrentEdit);

	// Done

	if (erroneous)
	{
		editor->markAsErroneous();
	}

	editor->setFocus();
}

void VariableListEditor::fixSignalSize()
{
	if(this->currentSignalValue != nullptr)
	{
		try
		{
			LogicValue currentInitialValue = LogicValue::fromString(this->currentSignalValue->text()); // Throws StatesException

			uint newSize = (uint)this->currentSignalSize->text().toInt();
			currentInitialValue.resize(newSize); // Throws StatesException

			this->currentSignalValue->setText(currentInitialValue.toString());
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

QList<QString> VariableListEditor::getSelectedSignals()
{
	QList<QString> selectionString;

	for (QModelIndex index : this->signalsList->selectionModel()->selectedRows())
	{
		QTableWidgetItem* currentItem = this->signalsList->item(index.row(), 1);
		if (currentItem != nullptr)
			selectionString.append(currentItem->text());
	}

	return selectionString;
}