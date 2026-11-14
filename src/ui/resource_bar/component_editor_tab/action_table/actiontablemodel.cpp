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
	this->columnsRoles.append(ColumnRole_t::behavior);
	this->columnsRoles.append(ColumnRole_t::valueProvider);
	this->columnsRoles.append(ColumnRole_t::condition);

	connect(actuator.get(), &MachineActuatorComponent::actionChangedAsAResultOfAnotherChangeEvent, this, &ActionTableModel::refreshPersistentEditorsEvent);
}

int ActionTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;


	return this->columnsRoles.count();
}

int ActionTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;

	auto actuator = this->getActuator();
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
	case ColumnRole_t::behavior:
		return this->getBehaviorColumnData(index, role);
		break;
	case ColumnRole_t::valueProvider:
		return this->getValueProviderColumnData(index, role);
		break;
	case ColumnRole_t::condition:
		return this->getConditionColumnData(index, role);
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
	case ColumnRole_t::behavior:
		return this->setBehaviorColumnData(index, value, role);
		break;
	case ColumnRole_t::valueProvider:
		return this->setValueProviderColumnData(index, value, role);
		break;
	case ColumnRole_t::condition:
		return this->setConditionColumnData(index, value, role);
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
			case ColumnRole_t::behavior:
				return QVariant(tr("Behavior"));
				break;
			case ColumnRole_t::valueProvider:
				return QVariant(tr("Value"));
				break;
			case ColumnRole_t::condition:
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
			case ColumnRole_t::behavior:
				return "BEHAVIOR";
				break;
			case ColumnRole_t::valueProvider:
				return "VALUEPROVIDER";
				break;
			case ColumnRole_t::condition:
				return "CONDITION";
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


	Qt::ItemFlags flags = Qt::ItemIsEnabled    |
	                      Qt::ItemIsSelectable |
	                      Qt::ItemNeverHasChildren;

	auto columnRole = this->columnsRoles.at(index.column());
	switch (columnRole)
	{
	case ColumnRole_t::variableName:
		break;
	case ColumnRole_t::behavior:
	{
		auto action = this->getAction(index.row());
		if (action == nullptr) return Qt::NoItemFlags;


		if (std::popcount(action->getAllowedBehaviors()) > 1)
		{
			flags |= Qt::ItemIsEditable;
		}
		break;
	}
	case ColumnRole_t::valueProvider:
	case ColumnRole_t::condition:
		flags |= Qt::ItemIsEditable;
		break;
	}

	return flags;
}

bool ActionTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
	auto actuator = this->getActuator();
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
	auto actuator = this->getActuator();
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

	auto actuator = this->getActuator();
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
	auto action = this->getAction(row);
	if (action == nullptr) return;

	auto targetVariable = this->getTargetVariable(row);
	if (targetVariable == nullptr) return;

	if (targetVariable->getType() != MachineValue::Type_t::bitVector) return;

	if ( (rangeL == action->getRangeL()) && (rangeR == action->getRangeR()) ) return;


	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Change action range
	action->setRange(rangeL, rangeR);

	// Machine has been edited
	machineManager->notifyMachineEdited();

	// Notify edition
	// Changing the range can impact value provider.
	// BIG ASSUMPTION HERE: Type column is left and Value column right
	auto leftIndex  = this->index(row, this->columnsRoles.indexOf(ColumnRole_t::variableName));
	auto rightIndex = this->index(row, this->columnsRoles.indexOf(ColumnRole_t::valueProvider));
	emit this->dataChanged(leftIndex, rightIndex);
}

std::shared_ptr<const Variable> ActionTableModel::getTargetVariable(int row) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto action = this->getAction(row);
	if (action == nullptr) return nullptr;


	return machine->getVariable(action->getTargetVariableId());
}

bool ActionTableModel::hasCondition(int row) const
{
	auto action = this->getAction(row);
	if (action == nullptr) return false;


	return (action->getCondition() != nullptr);
}

int ActionTableModel::getActionRangeL(int row) const
{
	auto action = this->getAction(row);
	if (action == nullptr) return -1;


	return action->getRangeL();
}

int ActionTableModel::getActionRangeR(int row) const
{
	auto action = this->getAction(row);
	if (action == nullptr) return -1;


	return action->getRangeR();
}

MachineValue::Type_t ActionTableModel::getExpectedProvidedValueType(int row) const
{
	auto action = this->getAction(row);
	if (action == nullptr) return MachineValue::Type_t::nullType;


	return action->getExpectedProvidedValueType();
}

uint ActionTableModel::getExpectedProvidedValueBitVectorSize(int row) const
{
	auto action = this->getAction(row);
	if (action == nullptr) return 0;


	return action->getExpectedProvidedValueBitVectorSize();
}

QVariant ActionTableModel::getVariableNameColumnData(const QModelIndex& index, int role) const
{
	if ( (role != Qt::DisplayRole) &&
	     (role != Qt::ForegroundRole)
	   )
		return QVariant();


	if (role == Qt::DisplayRole)
	{
		auto action = this->getAction(index.row());
		if (action == nullptr) return QVariant();

		auto targetVariable = this->getTargetVariable(index.row());
		if (targetVariable == nullptr) return QVariant();


		// Build name
		QString nameText = targetVariable->getName();

		if (targetVariable->getType() == MachineValue::Type_t::bitVector)
		{
			int rangeL = action->getRangeL();
			int rangeR = action->getRangeR();

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

QVariant ActionTableModel::getBehaviorColumnData(const QModelIndex& index, int role) const
{
	if ( (role != Qt::EditRole)       &&
	     (role != Qt::DisplayRole)    &&
	     (role != Qt::DecorationRole) &&
	     (role != Qt::ForegroundRole)
	   )
		return QVariant();

	auto action = this->getAction(index.row());
	if (action == nullptr) return QVariant();


	if (role == Qt::EditRole)
	{
		uint allowedBehaviors = action->getAllowedBehaviors();
		uint currentBehavior  = static_cast<uint>(action->getBehavior());
		uint32_t returnValue = (allowedBehaviors&0xFFFF) << 16 | (currentBehavior&0xFFFF);
		return QVariant(returnValue);
	}
	else if (std::popcount(action->getAllowedBehaviors()) == 1)
	{
		// Only display text if there is just one allowed behavior.
		// When more than one allowed behavior, there is a persistent
		// editor in this column.
		if (role == Qt::DisplayRole)
		{
			return ActionOnVariable::getBehaviorText(action->getBehavior());
		}
		else if (role == Qt::DecorationRole)
		{
			auto actionBehavior = action->getBehavior();
			switch (actionBehavior)
			{
			case ActionOnVariable::Behavior_t::pulse:
			case ActionOnVariable::Behavior_t::continuous:
			{
				// Return inverted icon for boolean actions whose provided value is false
				bool invert = false;
				if (action->getExpectedProvidedValueType() == MachineValue::Type_t::boolean)
				{
					auto valueProvider = action->getValueProvider();
					if (valueProvider != nullptr)
					{
						if (valueProvider->getInitialValue() == BooleanValue::falseValue())
						{
							invert = true;
						}
					}
				}

				return QVariant(ActionOnVariable::getBehaviorIcon(actionBehavior, invert));
				break;
			}
			case ActionOnVariable::Behavior_t::assign:
			case ActionOnVariable::Behavior_t::none:
				return QVariant(ActionOnVariable::getBehaviorIcon(actionBehavior));
				break;
			}
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

QVariant ActionTableModel::getValueProviderColumnData(const QModelIndex& index, int role) const
{
	if (role != Qt::EditRole) return QVariant();

	auto action = this->getAction(index.row());
	if (action == nullptr) return QVariant();


	// Only one role possible, so avoid test on role
	return QVariant::fromValue(action->getValueProvider());
}

QVariant ActionTableModel::getConditionColumnData(const QModelIndex& index, int role) const
{
	if (role != Qt::EditRole) return QVariant();

	auto action = this->getAction(index.row());
	if (action == nullptr) return QVariant();


	// Only one role possible, so avoid test on role
	auto condition = action->getCondition();
	if (condition != nullptr)
	{
		return QVariant::fromValue(condition);
	}

	return QVariant();
}

bool ActionTableModel::setBehaviorColumnData(const QModelIndex& index, const QVariant& value, int role)
{
	auto actuator = this->getActuator();
	if (actuator == nullptr) return false;

	auto action = this->getAction(index.row());
	if (action == nullptr) return false;


	// Get new behavior
	auto newBehavior = static_cast<ActionOnVariable::Behavior_t>(value.toUInt());

	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Change behavior
	action->setBehavior(newBehavior);

	// Machine has been edited
	QString undoDescription = "ACTION_CHANGE_BEHAVIOR__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
	machineManager->notifyMachineEdited(undoDescription);

	// Notify edition
	emit this->dataChanged(index, index);

	return true;
}

bool ActionTableModel::setValueProviderColumnData(const QModelIndex& index, const QVariant& value, int role)
{
	auto actuator = this->getActuator();
	if (actuator == nullptr) return false;

	auto action = this->getAction(index.row());
	if (action == nullptr) return false;


	// Get new value provider
	auto newValueProvider = value.value<std::shared_ptr<Equation>>();

	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Edit value provider
	action->setValueProvider(newValueProvider);

	// Machine has been edited
	QString undoDescription = "ACTION_CHANGE_VALUE_PROVIDER__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
	machineManager->notifyMachineEdited(undoDescription);

	// Notify edition
	emit this->dataChanged(index, index);

	return true;
}

bool ActionTableModel::setConditionColumnData(const QModelIndex& index, const QVariant& value, int role)
{
	auto actuator = this->getActuator();
	if (actuator == nullptr) return false;

	auto action = this->getAction(index.row());
	if (action == nullptr) return false;


	// Get new condition
	auto newCondition = value.value<std::shared_ptr<Equation>>();

	// Machine is about to be edited
	machineManager->notifyMachineAboutToBeDiffEdited();

	// Edit condition
	action->setCondition(newCondition);

	// Machine has been edited
	QString undoDescription = "ACTION_CHANGE_CONDITION__" + QString::number(static_cast<uint32_t>(actuator->getId())) + "_" + QString::number(index.row());
	machineManager->notifyMachineEdited(undoDescription);

	// Notify edition
	emit this->dataChanged(index, index);

	return false;
}

inline std::shared_ptr<MachineActuatorComponent> ActionTableModel::getActuator() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;


	return machine->getActuatorComponent(this->actuatorId);
}

inline std::shared_ptr<ActionOnVariable> ActionTableModel::getAction(int row) const
{
	auto actuator = this->getActuator();
	if (actuator == nullptr) return nullptr;


	return actuator->getAction(row);
}
