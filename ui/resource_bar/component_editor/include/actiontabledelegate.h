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
#ifndef ACTIONTABLEDELEGATE_H
#define ACTIONTABLEDELEGATE_H

// Parent
#include <QStyledItemDelegate>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
class MachineActuatorComponent;


/**
 * @brief The ActionTableDelegate class provides an editor
 * for the action value cells in an ActionEditor table.
 */
class ActionTableDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit ActionTableDelegate(shared_ptr<MachineActuatorComponent> actuator, QWidget* parent = nullptr);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const override;

private:
	weak_ptr<MachineActuatorComponent> actuator;

};

#endif // ACTIONTABLEDELEGATE_H
