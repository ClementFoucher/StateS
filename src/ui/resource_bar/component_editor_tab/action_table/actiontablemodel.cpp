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
#include "equation.h"


ActionTableModel::ActionTableModel(ComponentId actuatorId, QObject* parent) :
	QAbstractTableModel(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = machine->getActuatorComponent(actuatorId);
	if (actuator == nullptr) return;


	this->actuatorId = actuatorId;

	// WARNING: changing columns order requires to
	// update the dataChanged() signal parameters.
	this->columnsRoles.append(ColumnRole_t::variableName);
	this->columnsRoles.append(ColumnRole_t::actionType);
	this->columnsRoles.append(ColumnRole_t::actionValue);
	this->columnsRoles.append(ColumnRole_t::actionCondition);

	connect(actuator.get(), &MachineActuatorComponent::actionFixedEvent, this, &ActionTableModel::refreshPersistentEditorsEvent);
}

int ActionTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;


	return this->columnsRoles.count();
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


	auto columnRole = this->columnsRoles.at(index.column());
	switch (columnRole)
	{
	case ColumnRole_t::variableName:
		return this->getVariableNameColumnData(index, role);
		break;
	case ColumnRole_t::actionType:
		return this->getActionTypeColumnData(index, role);
		break;
	case ColumnRole_t::actionValue:
		return this->getActionValueColumnData(index, role);
		break;
	case ColumnRole_t::actionCondition:
		return this->getActionConditionColumnData(index, role);
		break;
	}

	return QVariant();
}

bool ActionTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() == false) return false;

	if (role != Qt::EditRole) return false;


	auto columnRole = this->columnsRoles.at(index.column());
	switch (columnRole)
	{
	case ColumnRole_t::variableName:
		// Nothing to do: read-only column
		break;
	case ColumnRole_t::actionType:
		return this->setActionTypeColumnData(index, value, role);
		break;
	case ColumnRole_t::actionValue:
		return this->setActionValueColumnData(index, value, role);
		break;
	case ColumnRole_t::actionCondition:
		return this->setActionConditionColumnData(index, value, role);
		break;
	}

	return false;
}

QVariant ActionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		if (role == Qt::DisplayRole)
		{
			auto columnRole = this->columnsRoles.at(section);
			switch (columnRole)
			{
			case ColumnRole_t::variableName:
				return QVariant(tr("Variable"));
				break;
			case ColumnRole_t::actionType:
				return QVariant(tr("Action"));
				break;
			case ColumnRole_t::actionValue:
				return QVariant(tr("Value"));
				break;
			case ColumnRole_t::actionCondition:
				return QVariant(tr("Condition"));
				break;
			}
		}
		else if (role == Qt::UserRole)
		{
			auto columnRole = this->columnsRoles.at(section);
			switch (columnRole)
			{
			case ColumnRole_t::variableName:
				return "VARIABLENAME";
				break;
			case ColumnRole_t::actionType:
				return "ACTIONTYPE";
				break;
			case ColumnRole_t::actionValue:
				return "ACTIONVALUE";
				break;
			case ColumnRole_t::actionCondition:
				return "ACTIONCONDITION";
				break;
			}
		}
	}
	else // (orientation == Qt::Vertical)
	{
		if (role == Qt::DisplayRole)
		{
			return QVariant(section+1);
		}
	}

	return QVariant();
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
	for (uint rank = row ; rank < static_cast<uint>(row + count) ; rank++)
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

void ActionTableModel::setRange(int row, int rangeL, int rangeR)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return;

	auto action = actuator->getAction(row);
	if (action == nullptr) return;

	auto variable = machine->getVariable(action->getVariableActedOnId());
	if (variable->getType() != MachineValue::Type_t::bitVector) return;

	if ( (rangeL == action->getActionRangeL()) && (rangeR == action->getActionRangeR()) ) return;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Change action range
	action->setActionRange(rangeL, rangeR);

	// Machine has been edited
	machineManager->notifyMachineEdited();
}

std::shared_ptr<const Variable> ActionTableModel::getVariable(int row) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return nullptr;

	auto action = actuator->getAction(row);
	if (action == nullptr) return nullptr;


	return machine->getVariable(action->getVariableActedOnId());
}

bool ActionTableModel::hasCondition(int row) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return false;

	auto action = actuator->getAction(row);
	if (action == nullptr) return false;


	return (action->getActionCondition() != nullptr);
}

int ActionTableModel::getActionRangeL(int row) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return -1;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return -1;

	auto actionActedOn = actuator->getAction(row);
	if (actionActedOn == nullptr) return -1;


	return actionActedOn->getActionRangeL();
}

int ActionTableModel::getActionRangeR(int row) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return -1;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return -1;

	auto actionActedOn = actuator->getAction(row);
	if (actionActedOn == nullptr) return -1;


	return actionActedOn->getActionRangeR();
}

QVariant ActionTableModel::getVariableNameColumnData(const QModelIndex& index, int role) const
{
	if ( (role != Qt::DisplayRole) &&
	     (role != Qt::ForegroundRole)
	   )
		return QVariant();


	if (role == Qt::DisplayRole)
	{
		auto machine = machineManager->getMachine();
		if (machine == nullptr) return QVariant();

		auto actuator = machine->getActuatorComponent(this->actuatorId);
		if (actuator == nullptr) return QVariant();

		auto action = actuator->getAction(index.row());
		if (action == nullptr) return QVariant();

		auto variableId = action->getVariableActedOnId();
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) return QVariant();


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
	else // (role == Qt::ForegroundRole)
	{
		QBrush brush;
		brush.setColor(Qt::gray);

		return QVariant(brush);
	}
}

QVariant ActionTableModel::getActionTypeColumnData(const QModelIndex& index, int role) const
{
	if ( (role != Qt::EditRole)       &&
	     (role != Qt::DisplayRole)    &&
	     (role != Qt::DecorationRole) &&
	     (role != Qt::ForegroundRole)
	   )
		return QVariant();

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QVariant();

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return QVariant();

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return QVariant();


	if (role == Qt::EditRole)
	{
		uint allowedActionTypes = action->getAllowedActionTypes();
		uint currentActionType  = static_cast<uint>(action->getActionType());
		uint32_t returnValue = (allowedActionTypes&0xFFFF) << 16 | (currentActionType&0xFFFF);
		return QVariant(returnValue);
	}
	else if (std::popcount(action->getAllowedActionTypes()) == 1)
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

	return QVariant();
}

QVariant ActionTableModel::getActionValueColumnData(const QModelIndex& index, int role) const
{
	if ( (role != Qt::DisplayRole) &&
	     (role != Qt::EditRole)    &&
	     (role != Qt::ForegroundRole)
	   )
		return QVariant();

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QVariant();

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return QVariant();

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return QVariant();


	if (role == Qt::DisplayRole)
	{
		auto variableId = action->getVariableActedOnId();
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) return QVariant();


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
		auto variableId = action->getVariableActedOnId();
		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) return QVariant();


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
				case MachineValue::Type_t::natural:
					text += "NATURAL";
					break;
				case MachineValue::Type_t::relative:
					text += "RELATIVE";
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
	else // (role == Qt::ForegroundRole)
	{
		if (action->isActionValueEditable() == false)
		{
			QBrush brush;
			brush.setColor(Qt::gray);

			return QVariant(brush);
		}
	}

	return QVariant();
}

QVariant ActionTableModel::getActionConditionColumnData(const QModelIndex& index, int role) const
{
	if (role != Qt::EditRole) return QVariant();

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QVariant();

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return QVariant();

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return QVariant();


	// Only one role possible, so avoid test on role
	auto condition = action->getActionCondition();
	if (condition != nullptr)
	{
		return QVariant::fromValue(condition);
	}

	return QVariant();
}

bool ActionTableModel::setActionTypeColumnData(const QModelIndex& index, const QVariant& value, int role)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return false;

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return false;


	// Get new action type
	auto newActionType = static_cast<ActionOnVariable::Type_t>(value.toUInt());

	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Change action type
	action->setActionType(newActionType);

	// Machine has been edited
	QString undoDescription = "ACTION_CHANGE_TYPE__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
	machineManager->notifyMachineEdited(undoDescription);

	// Notify edition
	// Changing the action type can impact action value.
	// BIG ASSUMPTION HERE: Type column is left and Value column right
	auto leftIndex  = this->index(index.row(), this->columnsRoles.indexOf(ColumnRole_t::actionType));
	auto rightIndex = this->index(index.row(), this->columnsRoles.indexOf(ColumnRole_t::actionValue));
	emit this->dataChanged(leftIndex, rightIndex);

	return true;
}

bool ActionTableModel::setActionValueColumnData(const QModelIndex& index, const QVariant& value, int role)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return false;

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return false;

	if (action->isActionValueEditable() == false) return false;

	auto actionValue = action->getActionValue();
	if (actionValue.getType() == MachineValue::Type_t::nullType) return false;


	// Get new value
	auto valueAsString = value.toString();

	MachineValue newValue{};
	switch (action->getActionValue().getType())
	{
	case MachineValue::Type_t::boolean:
		newValue = BooleanValue::fromRawString(valueAsString);
		break;
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
	case MachineValue::Type_t::natural:
		newValue = NaturalValue::fromRawString(valueAsString);
		break;
	case MachineValue::Type_t::relative:
		newValue = RelativeValue::fromRawString(valueAsString);
		break;
	case MachineValue::Type_t::nullType:
		// Checked previously, should not happen
		break;
	}

	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Edit action value
	action->setActionValue(newValue);

	// Machine has been edited
	QString undoDescription = "ACTION_CHANGE_VALUE__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
	machineManager->notifyMachineEdited(undoDescription);

	// Notify edition
	emit this->dataChanged(index, index);

	return true;
}

bool ActionTableModel::setActionConditionColumnData(const QModelIndex& index, const QVariant& value, int role)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return false;

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return false;


	// Get new condition
	auto equation = value.value<std::shared_ptr<Equation>>();

	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Edit condition
	action->setActionCondition(equation);

	// Machine has been edited
	QString undoDescription = "ACTION_CHANGE_CONDITION__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
	machineManager->notifyMachineEdited(undoDescription);

	// Notify edition
	emit this->dataChanged(index, index);

	return false;
}
