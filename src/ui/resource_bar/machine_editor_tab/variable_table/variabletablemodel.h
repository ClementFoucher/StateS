/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VARIABLETABLEMODEL_H
#define VARIABLETABLEMODEL_H

// Parent
#include <QAbstractTableModel>

// StateS classes
#include "statestypes.h"


class VariableTableModel : public QAbstractTableModel
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class ColumnRole
	{
		name,
		size,
		memorized,
		value
	};

	/////
	// Constructors/destructors
public:
	explicit VariableTableModel(VariableNature_t editorNature, QObject* parent = nullptr);

	/////
	// Object functions
public:
	virtual int rowCount(const QModelIndex& parent = QModelIndex())    const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual QVariant data(const QModelIndex& index, int role)                 const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;

	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex())                                                       override;
	virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex())                                                       override;
	virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

	QString getFailedVariableName();

private slots:
	void machineUpdatedEventHandler();

	/////
	// Signals
signals:
	void rowsMovedEvent();

	/////
	// Object variables
private:
	VariableNature_t editorNature;
	QList<ColumnRole> columnsRoles;

	// Temporary storage to retrieve erroneous name and relaunch edit
	QString failedVariableName;

};

#endif // VARIABLETABLEMODEL_H
