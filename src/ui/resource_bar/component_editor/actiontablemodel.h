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

#ifndef ACTIONTABLEMODEL_H
#define ACTIONTABLEMODEL_H

// Parent class
#include <QAbstractTableModel>

// StateS classes
#include "statestypes.h"


/**
 * @brief The ActionTableModel class provides content
 * for action table displayed in ActionEditor based on
 * an a MachineActuatorComponen information. It allows
 * edition for action value.
 * The first column is not handled by the model but
 * by a specific widget.
 *
 * The model emits a layoutChanged() signal whenever
 * the actuator component changes so that the table
 * view model is refreshed.
 *
 * If no action is set on the current actuator,
 * it displays a single cell with text "No action.".
 */
class ActionTableModel : public QAbstractTableModel
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionTableModel(componentId_t actuatorId, QObject* parent = nullptr);

	/////
	// Object functions
public:
	virtual int columnCount(const QModelIndex& parent)                              const override;
	virtual int rowCount(const QModelIndex& parent)                                 const override;
	virtual QVariant data(const QModelIndex& index, int role)                       const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role)       override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index)                           const override;

private slots:
	void refreshList();

	/////
	// Object variables
private:
	componentId_t actuatorId = 0;

};

#endif // ACTIONTABLEMODEL_H
