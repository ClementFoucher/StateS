/*
 * Copyright © 2016-2023 Clément Foucher
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

// C++ classes
#include <memory>
using namespace std;

// Qt classes
#include <QRegularExpressionValidator>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "machineactuatorcomponent.h"
#include "dynamiclineedit.h"
#include "actiononsignal.h"


ActionTableDelegate::ActionTableDelegate(componentId_t actuatorId, QWidget *parent) :
    QStyledItemDelegate(parent)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(actuatorId));
	if (actuator == nullptr) return;


	this->actuatorId = actuatorId;
}

QWidget* ActionTableDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
	// Editor only applies to colummn 2
	if (index.column() != 2) return nullptr;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return nullptr;

	auto actuator = dynamic_pointer_cast<MachineActuatorComponent>(machine->getComponent(this->actuatorId));
	if (actuator == nullptr) return nullptr;


	shared_ptr<ActionOnSignal> action = actuator->getAction(index.row()); // Throws StatesException - Ignored: list generated from action list
	QRegularExpression re("[01]{0," + QString::number(action->getActionSize()) + "}");

	return new DynamicLineEdit(QString(), false, new QRegularExpressionValidator(re), parent);
}
