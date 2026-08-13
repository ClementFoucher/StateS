/*
 * Copyright © 2016-2026 Clément Foucher
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
#include "actiontablemodel.h"

// Qt
#include <QBrush>
#include <QIcon>

// StateS
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "variable.h"
#include "actiononvariable.h"


ActionTableModel::ActionTableModel(ComponentId actuatorId, QObject* parent) :
	QAbstractTableModel(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = machine->getActuatorComponent(actuatorId);
	if (actuator == nullptr) return;


	this->actuatorId = actuatorId;

	this->columnsRoles.append(ColumnRole_t::actionType);
	this->columnsRoles.append(ColumnRole_t::variableName);
	this->columnsRoles.append(ColumnRole_t::actionValue);

	connect(actuator.get(), &MachineActuatorComponent::actionFixedEvent, this, &ActionTableModel::refreshPersistentEditorsEvent);
}

int ActionTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;


	return 3;
}

int ActionTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return 0;


	return actuator->getActions().count();
}

QVariant ActionTableModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() == false) return QVariant();

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QVariant();

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return QVariant();


	auto action = actuator->getAction(index.row());
	if (action == nullptr) return QVariant();

	auto variableId = action->getVariableActedOnId();

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return QVariant();


	auto columnRole = this->columnsRoles.at(index.column());
	switch (columnRole)
	{
	case ColumnRole_t::actionType:
	{
		uint allowedActionTypes = action->getAllowedActionTypes();

		if (role == Qt::EditRole)
		{
			uint currentActionType  = static_cast<uint>(action->getActionType());
			uint32_t returnValue = (allowedActionTypes&0xFFFF) << 16 | (currentActionType&0xFFFF);
			return QVariant(returnValue);
		}
		else if (std::popcount(allowedActionTypes) == 1)
		{
			// Only display text if there is just one allowed action type.
			// When more than one allowed action, there is a persistent
			// editor in this column.
			if (role == Qt::DisplayRole)
			{
				return QVariant(action->getCurrentActionTypeText());
			}
			else if (role == Qt::DecorationRole)
			{
				return QVariant(action->getCurrentActionTypeIcon());
			}
			else if (role == Qt::ForegroundRole)
			{
				QBrush brush;
				brush.setColor(Qt::gray);

				return QVariant(brush);
			}
		}
		break;
	}
	case ColumnRole_t::variableName:
	{
		if (role == Qt::DisplayRole)
		{
			// Build name
			QString nameText = variable->getName();

			if (variable->getType() == MachineValue::Type_t::bitVector)
			{
				int rangeL = action->getActionRangeL();
				int rangeR = action->getActionRangeR();

				if (rangeL != -1)
				{
					nameText += "[" + QString::number(rangeL);

					if (rangeR != -1)
					{
						nameText += ".." + QString::number(rangeR);
					}

					nameText += "]";
				}
			}

			return QVariant(nameText);
		}
		else if (role == Qt::ForegroundRole)
		{
			QBrush brush;
			brush.setColor(Qt::gray);

			return QVariant(brush);
		}
		break;
	}
	case ColumnRole_t::actionValue:
	{
		if (role == Qt::DisplayRole)
		{
			switch (action->getActionType())
			{
			case ActionOnVariable::Type_t::reset:
			case ActionOnVariable::Type_t::set:
			case ActionOnVariable::Type_t::continuous:
			case ActionOnVariable::Type_t::pulse:
			case ActionOnVariable::Type_t::assign:
				return QVariant(action->getActionValue().toDisplayString());
				break;
			case ActionOnVariable::Type_t::increment:
				return QVariant(variable->getName() + " + 1");
				break;
			case ActionOnVariable::Type_t::decrement:
				return QVariant(variable->getName() + " - 1");
				break;
			case ActionOnVariable::Type_t::none:
				// Nothing
				break;
			}
		}
		else if (role == Qt::EditRole)
		{
			switch (action->getActionType())
			{
			case ActionOnVariable::Type_t::reset:
			case ActionOnVariable::Type_t::set:
			case ActionOnVariable::Type_t::continuous:
			case ActionOnVariable::Type_t::pulse:
			case ActionOnVariable::Type_t::assign:
				if (action->isActionValueEditable() == true)
				{
					auto actionValue = action->getActionValue();

					// Provide various fields representing value
					QString text;

					// First field is type
					switch (actionValue.getType())
					{
					case MachineValue::Type_t::boolean:
						text += "BOOLEAN";
						break;
					case MachineValue::Type_t::bitVector:
						text += "BITVECTOR";
						break;
					case MachineValue::Type_t::nullType:
						text += "NULL";
						break;
					}

					// Second field is variable value
					text += ":" + actionValue.toRawString();

					// Third field (only for bit vectors) is variable size
					if (actionValue.getType() == MachineValue::Type_t::bitVector)
					{
						auto bitVectorValue = actionValue.getBitVectorValue();
						text += ":" + QString::number(bitVectorValue.getSize());
					}

					return text;
				}
				break;
			case ActionOnVariable::Type_t::increment:
				return QVariant(variable->getName() + " + 1");
				break;
			case ActionOnVariable::Type_t::decrement:
				return QVariant(variable->getName() + " - 1");
				break;
			case ActionOnVariable::Type_t::none:
				// Nothing
				break;
			}
		}
		else if (role == Qt::ForegroundRole)
		{
			if (action->isActionValueEditable() == false)
			{
				QBrush brush;
				brush.setColor(Qt::gray);

				return QVariant(brush);
			}
			else
			{
				return QVariant();
			}
		}
		break;
	}
	}

	return QVariant();
}

bool ActionTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() == false) return false;

	if (role != Qt::EditRole) return false;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return false;

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return false;


	auto columnRole = this->columnsRoles.at(index.column());
	switch (columnRole)
	{
	case ColumnRole_t::actionType:
	{
		ActionOnVariable::Type_t newActionType = static_cast<ActionOnVariable::Type_t>(value.toUInt());

		// Machine is about to be edited
		machineManager->notifyMachineAboutToBeDiffEdited();

		// Change action type
		action->setActionType(newActionType);

		// Machine has been edited
		QString undoDescription = "ACTION_CHANGE_TYPE__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
		machineManager->notifyMachineEdited(undoDescription);

		// Changing the action type can impact action value:
		// We have to notify about it
		auto rightColumnIndex = this->index(index.row(), this->columnCount()-1);
		emit this->dataChanged(rightColumnIndex, rightColumnIndex);

		return true;
		break;
	}
	case ColumnRole_t::variableName:
		// Nothing to do: read-only column
		break;
	case ColumnRole_t::actionValue:
		if (action->isActionValueEditable() == true)
		{
			auto valueAsString = value.toString();

			MachineValue newValue{};
			switch (action->getActionValue().getType())
			{
			case MachineValue::Type_t::boolean:
			{
				newValue = BooleanValue::fromRawString(valueAsString);
				break;
			}
			case MachineValue::Type_t::bitVector:
			{
				auto newBitVectorVariableValue = BitVectorValue::fromRawString(valueAsString);

				auto oldVariableValueSize = action->getActionValue().getBitVectorValue().getSize();
				if (newBitVectorVariableValue.getSize() != oldVariableValueSize)
				{
					newBitVectorVariableValue.resize(oldVariableValueSize);
				}

				newValue = newBitVectorVariableValue;
				break;
			}
			case MachineValue::Type_t::nullType:
				break;
			}

			// Do not change current value if no new value is provided
			if (newValue.isNull() == false)
			{
				// Machine is about to be edited
				machineManager->notifyMachineAboutToBeDiffEdited();

				// Edit action value
				action->setActionValue(newValue);

				// Machine has been edited
				QString undoDescription = "ACTION_CHANGE_VALUE__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
				machineManager->notifyMachineEdited(undoDescription);

				return true;
			}
		}
		break;
	}

	return false;
}

QVariant ActionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole) return QVariant();

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QVariant();

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return QVariant();


	if (orientation == Qt::Horizontal)
	{
		auto columnRole = this->columnsRoles.at(section);
		switch (columnRole)
		{
		case ColumnRole_t::actionType:
			return QVariant(tr("Type"));
			break;
		case ColumnRole_t::variableName:
			return QVariant(tr("Variable"));
			break;
		case ColumnRole_t::actionValue:
			return QVariant(tr("Value"));
			break;
		}
	}
	else
	{
		return QVariant(section+1);
	}
}

Qt::ItemFlags ActionTableModel::flags(const QModelIndex& index) const
{
	if (index.isValid() == false) return Qt::NoItemFlags;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return Qt::NoItemFlags;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return Qt::NoItemFlags;


	Qt::ItemFlags flags = Qt::ItemIsEnabled    |
	                      Qt::ItemIsSelectable |
	                      Qt::ItemNeverHasChildren;

	auto columnRole = this->columnsRoles.at(index.column());
	if (columnRole == ColumnRole_t::actionValue)
	{
		auto action = actuator->getAction(index.row());
		if (action == nullptr) return Qt::NoItemFlags;


		if (action->isActionValueEditable())
		{
			flags |= Qt::ItemIsEditable;
		}
	}

	return flags;
}

bool ActionTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return false;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Remove actions
	this->beginRemoveRows(parent, row, row+count-1);
	for (uint rank = row ; rank < (uint)(row + count) ; rank++)
	{
		actuator->removeAction(row);
	}
	this->endRemoveRows();

	// Machine has been edited
	QString undoDescription = "ACTION_REMOVE__" + QString::number(static_cast<uint32_t>(actuator->getId()));
	machineManager->notifyMachineEdited(undoDescription);

	return true;
}

bool ActionTableModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return false;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Move actions
	if (sourceRow < destinationChild)
	{
		// Is lowering actions
		bool doMove = this->beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild+count);
		if (doMove == false) return false;


		int offset = destinationChild-sourceRow;

		// Iterate in reverse order
		for (int rank = sourceRow+count-1 ; rank >= sourceRow ; rank--)
		{
			actuator->changeActionRank(rank, rank+offset);
		}
	}
	else
	{
		// Is raising actions
		bool doMove = this->beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
		if (doMove == false) return false;


		int offset = sourceRow-destinationChild;

		// Iterate in normal order
		for (int rank = sourceRow ; rank < sourceRow+count ; rank++)
		{
			actuator->changeActionRank(rank, rank-offset);
		}
	}
	this->endMoveRows();

	// Machine has been edited
	QString undoDescription = "ACTION_REORDER__" + QString::number(static_cast<uint32_t>(actuator->getId()));
	machineManager->notifyMachineEdited(undoDescription);

	return true;
}

void ActionTableModel::addAction(const QString& variableName)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return;


	// Find variable from name
	for (auto variableId : machine->getWrittableVariablesIds())
	{
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) continue;


		if (variable->getName() == variableName)
		{
			// Machine is about to be edited
			machineManager->notifyMachineAboutToBeDiffEdited();

			// Add action
			this->beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
			actuator->addAction(variableId);
			this->endInsertRows();

			// Machine has been edited
			machineManager->notifyMachineEdited();

			break;
		}
	}
}
