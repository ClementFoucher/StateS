/*
 * Copyright © 2016-2025 Clément Foucher
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

// Qt classes
#include <QBrush>
#include <QIcon>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "variable.h"
#include "actiononvariable.h"


ActionTableModel::ActionTableModel(componentId_t actuatorId, QObject* parent) :
    QAbstractTableModel(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = machine->getActuatorComponent(actuatorId);
	if (actuator == nullptr) return;


	this->actuatorId = actuatorId;
	connect(actuator.get(), &MachineActuatorComponent::actionListChangedEvent, this, &ActionTableModel::refreshList);
}

int ActionTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return 0;


	if (actuator->getActions().count() != 0)
	{
		return 3;
	}
	else
	{
		// Sigle "No action" cell
		return 1;
	}
}

int ActionTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return 0;


	if (actuator->getActions().count() != 0)
	{
		return actuator->getActions().count();
	}
	else
	{
		// Sigle "No action" cell
		return 1;
	}
}

QVariant ActionTableModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() == false) return QVariant();

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return QVariant();

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return QVariant();


	if (actuator->getActions().count() != 0)
	{
		auto action = actuator->getAction(index.row());
		if (action == nullptr) return QVariant();

		auto variableId = action->getVariableActedOnId();

		auto variable = machine->getVariable(variableId);
		if (variable == nullptr) return QVariant();


		if (index.column() == 0)
		{
			uint allowedActionTypes = action->getAllowedActionTypes();

			if (role == Qt::EditRole)
			{
				uint currentActionType  = (uint)action->getActionType();
				uint32_t returnValue = (allowedActionTypes&0xFFFF) << 16 | (currentActionType&0xFFFF);
				return QVariant(returnValue);
			}
			else if (std::popcount(allowedActionTypes) == 1)
			{
				// Only display text if there is one allowed action type.
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
		}
		else if (index.column() == 1)
		{
			if (role == Qt::DisplayRole)
			{
				// Build name
				QString nameText = variable->getName();

				if (variable->getSize() > 1)
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
		}
		else if (index.column() == 2)
		{
			if ( (role == Qt::DisplayRole) || (role == Qt::EditRole) )
			{
				switch (action->getActionType())
				{
				case ActionOnVariableType_t::reset:
				case ActionOnVariableType_t::set:
				case ActionOnVariableType_t::continuous:
				case ActionOnVariableType_t::pulse:
				case ActionOnVariableType_t::assign:
					return QVariant(action->getActionValue().toString());
					break;
				case ActionOnVariableType_t::increment:
					return QVariant(variable->getName() + " + 1");
					break;
				case ActionOnVariableType_t::decrement:
					return QVariant(variable->getName() + " - 1");
					break;
				case ActionOnVariableType_t::none:
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
		}
	}
	else
	{
		if (role == Qt::DisplayRole)
		{
			// Single "No action" cell
			return QVariant(tr("No action."));
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


	if (index.column() == 0)
	{
		ActionOnVariableType_t newActionType = (ActionOnVariableType_t)value.toUInt();
		action->setActionType(newActionType);

		// Machine has been edited
		machineManager->notifyMachineEdited();
	}
	else if (index.column() == 2)
	{
		if (action->isActionValueEditable() == true)
		{
			auto newValue = LogicValue::fromString(value.toString());

			// Do not change current value if no new value is provided
			if (newValue.isNull() == false)
			{
				// Make sure new value size fits the action size
				uint actionSize = action->getActionSize();
				newValue.resize(actionSize);

				action->setActionValue(newValue);

				// Machine has been edited
				machineManager->notifyMachineEdited();

				return true;
			}
		}
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


	if (actuator->getActions().count() != 0)
	{
		if (orientation == Qt::Horizontal)
		{
			if (section == 0)
			{
				return QVariant(tr("Type"));
			}
			else if (section == 1)
			{
				return QVariant(tr("Variable"));
			}
			else if (section == 2)
			{
				return QVariant(tr("Value"));
			}
			else
			{
				return QVariant();
			}
		}
		else
		{
			return QVariant(section+1);
		}
	}
	else
	{
		// Sigle "No action" cell
		return QVariant(QString());
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

	if (actuator->getActions().count() != 0)
	{
		if (index.column() == 2)
		{
			auto action = actuator->getAction(index.row());
			if (action == nullptr) return Qt::NoItemFlags;


			if (action->isActionValueEditable())
			{
				flags |= Qt::ItemIsEditable;
			}
		}
	}

	return flags;
}

void ActionTableModel::refreshList()
{
	emit layoutChanged();
}
