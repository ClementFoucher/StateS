/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "variabletablemodel.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "variabletableview.h"


VariableTableModel::VariableTableModel(VariableNature_t editorNature, QObject* parent) :
	QAbstractTableModel(parent)
{
	connect(machineManager.get(), &MachineManager::machineUpdatedEvent, this, &VariableTableModel::machineUpdatedEventHandler);

	this->editorNature = editorNature;

	this->columnsRoles.append(ColumnRole::name);
	this->columnsRoles.append(ColumnRole::size);
	switch (this->editorNature)
	{
	case VariableNature_t::output:
	case VariableNature_t::internal:
		this->columnsRoles.append(ColumnRole::memorized);
		break;
	case VariableNature_t::input:
	case VariableNature_t::constant:
		break;
	}
	this->columnsRoles.append(ColumnRole::value);
}

int VariableTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;


	return machine->getVariablesIds(this->editorNature).count();
}

int VariableTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;


	return this->columnsRoles.count();
}

QVariant VariableTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		if (role == Qt::DisplayRole)
		{
			auto columnRole = this->columnsRoles.at(section);
			switch (columnRole)
			{
			case ColumnRole::name:
				switch (this->editorNature)
				{
				case VariableNature_t::input:
					return tr("Input");
					break;
				case VariableNature_t::internal:
					return tr("Variable");
					break;
				case VariableNature_t::output:
					return tr("Output");
					break;
				case VariableNature_t::constant:
					return tr("Constant");
					break;
				}
				break;
			case ColumnRole::size:
				return tr("Size");
				break;
			case ColumnRole::memorized:
				return tr("Memorized");
				break;
			case ColumnRole::value:
				switch (this->editorNature)
				{
				case VariableNature_t::input:
					return tr("Initial value in simulator");
					break;
				case VariableNature_t::internal:
				case VariableNature_t::output:
					return tr("Initial/default value");
					break;
				case VariableNature_t::constant:
					return tr("Value");
					break;
				}
				break;
			}
		}
		else if (role == Qt::UserRole)
		{
			auto columnRole = this->columnsRoles.at(section);
			switch (columnRole)
			{
			case ColumnRole::name:
				return "NAME";
				break;
			case ColumnRole::size:
				return "SIZE";
				break;
			case ColumnRole::memorized:
				return "MEMORIZED";
				break;
			case ColumnRole::value:
				return "VALUE";
				break;
			}
		}
	}
	else // (orientation == Qt::Vertical)
	{
		if (role == Qt::DisplayRole)
		{
			return section+1;
		}
	}

	return QVariant();
}

QVariant VariableTableModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() == false) return QVariant();

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QVariant();

	auto variableId = machine->getVariableId(this->editorNature, index.row());
	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return QVariant();


	auto columnRole = this->columnsRoles.at(index.column());
	if (role == Qt::DisplayRole)
	{
		switch (columnRole)
		{
		case ColumnRole::name:
			return variable->getName();
			break;
		case ColumnRole::size:
			return QString::number(variable->getSize());
			break;
		case ColumnRole::memorized:
			// No display role for this column
			// as all rows have a permanent editor.
			break;
		case ColumnRole::value:
			return variable->getInitialValue().toString();
			break;
		}
	}
	else if (role == Qt::EditRole)
	{
		switch (columnRole)
		{
		case ColumnRole::name:
			return variable->getName();
			break;
		case ColumnRole::size:
			return QString::number(variable->getSize());
			break;
		case ColumnRole::memorized:
			return variable->getMemorized();
			break;
		case ColumnRole::value:
			{
				// Provide various fields representing value
				QString text;
				auto variableValue = variable->getInitialValue();

				// First field is variable value
				text = variableValue.toString();

				// Second field is variable size
				text += ":" + QString::number(variableValue.getSize());

				return text;
			}
			break;
		}
	}

	return QVariant();
}

bool VariableTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto variableId = machine->getVariableId(this->editorNature, index.row());
	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return false;


	bool dataSucessfullyChanged = false;
	auto columnRole = this->columnsRoles.at(index.column());
	if (role == Qt::EditRole)
	{
		switch (columnRole)
		{
		case ColumnRole::name:
		{
			auto valueAsString = value.toString();
			dataSucessfullyChanged = machine->renameVariable(variableId, valueAsString);

			if (dataSucessfullyChanged == false)
			{
				this->failedVariableName = valueAsString;
			}
			break;
		}
		case ColumnRole::size:
		{
			bool ok;
			auto valueAsInt = value.toInt(&ok);

			if ( (ok == true) && (valueAsInt > 0) )
			{
				variable->setSize(valueAsInt);

				// Check that size vas correctly changed
				if ((int)variable->getSize() == valueAsInt)
				{
					dataSucessfullyChanged = true;
				}
			}
			break;
		}
		case ColumnRole::memorized:
			variable->setMemorized(value.toBool());
			dataSucessfullyChanged = true;
			break;
		case ColumnRole::value:
		{
			auto valueAsString = value.toString();
			auto newVariableValue = LogicValue::fromString(valueAsString);

			auto oldVariableValue = variable->getInitialValue();
			if (oldVariableValue.isNull() == false)
			{
				auto oldVariableValueSize = oldVariableValue.getSize();
				if (newVariableValue.getSize() != oldVariableValueSize)
				{
					newVariableValue.resize(oldVariableValueSize);
				}
			}

			if (newVariableValue.isNull() == false)
			{
				variable->setInitialValue(newVariableValue);

				// Check that value vas correctly changed
				if (variable->getInitialValue() == newVariableValue)
				{
					dataSucessfullyChanged = true;
				}
			}
			break;
		}
		}
	}

	if (dataSucessfullyChanged == true)
	{
		emit this->dataChanged(index, index);

		// Machine has been edited
		machineManager->notifyMachineEdited();
	}

	return dataSucessfullyChanged;
}

Qt::ItemFlags VariableTableModel::flags(const QModelIndex& index) const
{
	if (index.isValid() == false) return Qt::NoItemFlags;


	return Qt::ItemIsSelectable     |
	       Qt::ItemNeverHasChildren |
	       Qt::ItemIsEnabled        |
	       Qt::ItemIsEditable;
}

bool VariableTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;


	// List variables to be removed
	QList<componentId_t> variablesToRemoveIds;
	for (int rank = row ; rank < row + count ; rank++)
	{
		auto variableId = machine->getVariableId(this->editorNature, rank);
		if (variableId == nullId) continue;


		variablesToRemoveIds.append(variableId);
	}

	// Do remove variables
	this->beginRemoveRows(parent, row, row+count-1);
	for (auto variableToRemoveId : variablesToRemoveIds)
	{
		machine->removeVariable(variableToRemoveId);
	}
	this->endRemoveRows();

	// Machine has been edited
	QString undoDescription = "VARIABLE_REMOVE__";
	switch (this->editorNature)
	{
	case VariableNature_t::input:
		undoDescription += "INPUTS";
		break;
	case VariableNature_t::output:
		undoDescription += "OUTPUTS";
		break;
	case VariableNature_t::internal:
		undoDescription += "INTERNAL_VARIABLES";
		break;
	case VariableNature_t::constant:
		undoDescription += "CONSTANTSS";
		break;
	}
	machineManager->notifyMachineEdited(undoDescription);

	return true;
}

bool VariableTableModel::insertRows(int row, int count, const QModelIndex& parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;


	this->beginInsertRows(parent, row, row+count-1);

	bool atLeastOneInsertion = false;
	for (int i = 0 ; i < count ; i++)
	{
		// Determine a unique variable name
		QString namePrefix;
		switch (this->editorNature)
		{
		case VariableNature_t::input:
			namePrefix = tr("Input");
			break;
		case VariableNature_t::output:
			namePrefix = tr("Output");
			break;
		case VariableNature_t::internal:
			namePrefix = tr("Variable");
			break;
		case VariableNature_t::constant:
			namePrefix = tr("Constant");
			break;
		}
		namePrefix += " #";

		QString initialName;
		uint suffix = 0;
		auto variablesIds = machine->getAllVariablesIds();
		bool nameIsValid;
		do
		{
			initialName = namePrefix + QString::number(suffix);
			nameIsValid = true;
			for (auto& existingVariableId : variablesIds)
			{
				auto existingVariable = machine->getVariable(existingVariableId);
				if (existingVariable == nullptr) continue;


				if (existingVariable->getName() == initialName)
				{
					suffix++;
					nameIsValid = false;
					break;
				}
			}
		} while (nameIsValid == false);

		// Add variable
		auto newVarId = machine->addVariable(this->editorNature, initialName);

		// Make internal variables memorized by default
		if (this->editorNature == VariableNature_t::internal)
		{
			auto variable = machine->getVariable(newVarId);
			if (variable != nullptr)
			{
				variable->setMemorized(true);
			}
		}

		if (newVarId != nullId)
		{
			atLeastOneInsertion = true;
		}
	}
	this->endInsertRows();

	if (atLeastOneInsertion == true)
	{
		// Machine has been edited
		machineManager->notifyMachineEdited();
	}

	return atLeastOneInsertion;
}

bool VariableTableModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;


	if (sourceRow < destinationChild)
	{
		// Is lowering variables
		bool doMove = this->beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild+count);
		if (doMove == false) return false;


		// Get an initial snapshop of variables and their ranks
		auto variablesIds = machine->getVariablesIds(this->editorNature);
		int offset = destinationChild-sourceRow;

		// Iterate in reverse order
		for (int rank = sourceRow+count-1 ; rank >= sourceRow ; rank--)
		{
			auto variableId = variablesIds[rank];
			machine->changeVariableRank(variableId, rank+offset);
		}
	}
	else
	{
		// Is raising variables
		bool doMove = this->beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
		if (doMove == false) return false;


		// Get an initial snapshop of variables and their ranks
		auto variablesIds = machine->getVariablesIds(this->editorNature);
		int offset = sourceRow-destinationChild;

		// Iterate in normal order
		for (int rank = sourceRow ; rank < sourceRow+count ; rank++)
		{
			auto variableId = variablesIds[rank];
			machine->changeVariableRank(variableId, rank-offset);
		}
	}
	this->endMoveRows();

	// Machine has been edited
	QString undoDescription;
	auto table = dynamic_cast<VariableTableView*>(this->parent());
	if (table != nullptr)
	{
		// Build undo reason string
		auto selection = table->selectionModel()->selectedRows();
		QStringList selectedVariables;
		for (auto& selectedRow : selection)
		{
			auto variableName = this->data(selectedRow, Qt::DisplayRole).toString();
			selectedVariables.append(variableName);
		}
		selectedVariables.sort();
		undoDescription = "VARIABLE_REORDER_";
		for (auto& variableName : selectedVariables)
		{
			undoDescription += "_" + variableName;
		}
	}
	machineManager->notifyMachineEdited(undoDescription);

	return true;
}

QString VariableTableModel::getFailedVariableName()
{
	auto failedName = this->failedVariableName;
	this->failedVariableName = QString();
	return failedName;
}

void VariableTableModel::machineUpdatedEventHandler()
{
	this->beginResetModel();
	this->endResetModel();
}
