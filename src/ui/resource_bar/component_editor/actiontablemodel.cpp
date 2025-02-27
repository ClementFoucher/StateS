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

// C++ classes
using namespace std;
#include <memory>

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

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(actuatorId));
	if (actuator == nullptr) return;


	this->actuatorId = actuatorId;
	connect(actuator.get(), &MachineActuatorComponent::actionListChangedEvent, this, &ActionTableModel::refreshList);
}

int ActionTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid() == true) return 0;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
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

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
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

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return QVariant();


	if (actuator->getActions().count() != 0)
	{
		auto action = actuator->getAction(index.row());
		if (action == nullptr) return QVariant();


		if (role == Qt::DisplayRole)
		{
			auto variableId = action->getVariableActedOnId();

			auto variable = machine->getVariable(variableId);
			if (variable == nullptr) return QVariant();


			if (index.column()  == 1)
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
			else if (index.column() == 2)
			{
				switch (action->getActionType())
				{
				case ActionOnVariableType_t::reset:
				case ActionOnVariableType_t::set:
				case ActionOnVariableType_t::activeOnState:
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
				}
			}
		}
		else if (role == Qt::EditRole)
		{
			if (index.column()  == 2)
			{
				return QVariant(action->getActionValue().toString());
			}
		}
	}
	else
	{
		if (role == Qt::DisplayRole)
		{
			// Sigle "No action" cell
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

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return false;

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return false;


	if (action->isActionValueEditable() == true)
	{
		auto newValue = LogicValue::fromString(value.toString());

		// Do not change current value if no new value is provided
		if (newValue.isNull() == false)
		{
			// Make sure new value size fits the action size
			auto actionValue = action->getActionValue();

			if (actionValue.isNull() == false)
			{
				newValue.resize(actionValue.getSize());

				action->setActionValue(newValue);
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

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
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

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return Qt::NoItemFlags;


	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

	if (actuator->getActions().count() != 0)
	{
		if (index.column() != 2)
		{
			flags |= Qt::ItemIsEnabled;
		}
		else
		{
			auto action = actuator->getAction(index.row());
			if (action == nullptr) return Qt::NoItemFlags;


			if (action->isActionValueEditable())
			{
				flags |= (Qt::ItemIsEnabled | Qt::ItemIsEditable);
			}
		}
	}

	return flags;
}

void ActionTableModel::refreshList()
{
	emit layoutChanged();
}
