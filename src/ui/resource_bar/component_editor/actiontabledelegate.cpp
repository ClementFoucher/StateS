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
#include "actiontabledelegate.h"

// Qt classes
#include <QRegularExpressionValidator>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "actiononvariable.h"
#include "dynamiclineeditor.h"


ActionTableDelegate::ActionTableDelegate(componentId_t actuatorId, QWidget* parent) :
    QStyledItemDelegate(parent)
{
	this->actuatorId = actuatorId;
}

QWidget* ActionTableDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto actuator = machine->getActuatorComponent(this->actuatorId);
	if (actuator == nullptr) return nullptr;

	auto action = actuator->getAction(index.row());
	if (action == nullptr) return nullptr;


	QRegularExpression re("[01]{0," + QString::number(action->getActionSize()) + "}");

	return new DynamicLineEditor(QString(), false, new QRegularExpressionValidator(re), parent);
}
