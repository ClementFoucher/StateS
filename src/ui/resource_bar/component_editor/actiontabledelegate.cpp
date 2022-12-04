/*
 * Copyright © 2016-2020 Clément Foucher
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
#include "actiontabledelegate.h"

// Qt classes
#include <QRegularExpressionValidator>

// StateS classes
#include "machineactuatorcomponent.h"
#include "dynamiclineedit.h"
#include "actiononsignal.h"


ActionTableDelegate::ActionTableDelegate(shared_ptr<MachineActuatorComponent> actuator, QWidget *parent) :
    QStyledItemDelegate(parent)
{
	this->actuator = actuator;
}

QWidget* ActionTableDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
	QWidget* editor = nullptr;

	if(index.column() == 2)
	{
		shared_ptr<MachineActuatorComponent> l_actuator = this->actuator.lock();

		if (l_actuator != nullptr)
		{
			shared_ptr<ActionOnSignal> action = l_actuator->getAction(index.row()); // Throws StatesException - Ignored: list generated from action list
			QRegularExpression re("[01]{0," + QString::number(action->getActionSize()) + "}");

			editor = new DynamicLineEdit(QString(), false, new QRegularExpressionValidator(re), parent);
		}
	}

	return editor;
}
