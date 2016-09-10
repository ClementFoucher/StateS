/*
 * Copyright © 2016 Clément Foucher
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

// StateS classes
#include "machineactuatorcomponent.h"
#include "StateS_signal.h"
#include "actiononsignal.h"


ActionTableModel::ActionTableModel(shared_ptr<MachineActuatorComponent> actuator, QObject* parent) :
    QAbstractTableModel(parent)
{
    this->actuator = actuator;

    connect(actuator.get(), &MachineActuatorComponent::actionListChangedEvent, this, &ActionTableModel::refreshList);
}

int ActionTableModel::columnCount(const QModelIndex& parent) const
{
    int columns = 0;

    if (!parent.isValid())
    {
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

        if (l_actuator != nullptr)
        {
            if (l_actuator->getActions().count() != 0)
            {
                columns = 3;
            }
            else
            {
                // Sigle "No action" cell
                columns = 1;
            }
        }
    }

    return columns;
}

int ActionTableModel::rowCount(const QModelIndex& parent) const
{
    int rows = 0;

    if (!parent.isValid())
    {
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

        if (l_actuator != nullptr)
        {
            if (l_actuator->getActions().count() != 0)
            {
                rows = l_actuator->getActions().count();
            }
            else
            {
                // Sigle "No action" cell
                rows = 1;
            }
        }
    }

    return rows;
}

QVariant ActionTableModel::data(const QModelIndex& index, int role) const
{
    QVariant variant = QVariant();

    if (index.isValid())
    {
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();
        if (l_actuator != nullptr)
        {
            if (l_actuator->getActions().count() != 0)
            {
                shared_ptr<ActionOnSignal> action = l_actuator->getAction(index.row()); // Throws StatesException - Ignored: model generated from action list

                if (role == Qt::DisplayRole)
                {
                    if (index.column()  == 1)
                    {
                        shared_ptr<Signal> signal = action->getSignalActedOn();

                        // Build name
                        QString nameText = signal->getName();

                        if (signal->getSize() > 1)
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

                        variant = QVariant(nameText);
                    }
                    else if (index.column()  == 2)
                    {
                        variant = QVariant(action->getActionValue().toString());
                    }

                }
                else if (role == Qt::EditRole)
                {
                    if (index.column()  == 2)
                    {
                        variant = QVariant(action->getActionValue().toString());
                    }
                }
            }
            else
            {
                if (role == Qt::DisplayRole)
                {
                    // Sigle "No action" cell
                    variant = QVariant(tr("No action."));
                }
            }

        }
    }

    return variant;
}

bool ActionTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result = false;

    if ( (index.isValid()) && (role == Qt::EditRole) )
    {
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

        if (l_actuator != nullptr)
        {
            shared_ptr<ActionOnSignal> action = l_actuator->getAction(index.row()); // Throws StatesException - Ignored: model generated from action list

            if (action->isActionValueEditable())
            {
                LogicValue newValue = LogicValue::fromString(value.toString());

                // Do not change current value if no new value is provided
                if (! newValue.isNull())
                {
                    action->setActionValue(newValue); // Throws StatesException - Ignored: A validator ensures correct input value
                    result = true;
                }
            }
        }
    }

    return result;
}

QVariant ActionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant variant = QVariant();

    if (role == Qt::DisplayRole)
    {
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

        if (l_actuator != nullptr)
        {
            if (l_actuator->getActions().count() != 0)
            {
                if (orientation == Qt::Horizontal)
                {
                    if (section == 0)
                        variant = QVariant(tr("Type"));
                    else if (section == 1)
                        variant = QVariant(tr("Signal"));
                    else if (section == 2)
                        variant = QVariant(tr("Value"));
                }
                else
                {
                    variant = QVariant(section+1);
                }
            }
            else
            {
                // Sigle "No action" cell
                variant = QVariant(QString::null);
            }
        }
    }

    return variant;
}

Qt::ItemFlags ActionTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::NoItemFlags;

    if (index.isValid())
    {
        shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

        if (l_actuator != nullptr)
        {
            flags = Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;

            if (l_actuator->getActions().count() != 0)
            {
                if (index.column() != 2)
                {
                    flags |= Qt::ItemIsEnabled;
                }
                else
                {
                    shared_ptr<ActionOnSignal> action = l_actuator->getAction(index.row()); // Throws StatesException - Ignored: model generated from action list

                    if (action->isActionValueEditable())
                        flags |= (Qt::ItemIsEnabled | Qt::ItemIsEditable);
                }
            }
        }
    }

    return flags;
}

void ActionTableModel::refreshList()
{
    emit layoutChanged();
}
