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
#include "truthtableinputtablemodel.h"

// Qt
#include <QColor>

// StateS
#include "truthtable.h"
#include "machinevalue.h"


TruthTableInputTableModel::TruthTableInputTableModel(std::shared_ptr<TruthTable> truthTable, QList<int> highlights, QObject* parent) :
	QAbstractTableModel(parent)
{
	this->truthTable = truthTable;
	this->highlights = highlights;
}

int TruthTableInputTableModel::columnCount(const QModelIndex& parent) const
{
	int columns = 0;

	if (parent.isValid() == false)
	{
		auto l_truthTable = this->truthTable.lock();

		if (l_truthTable != nullptr)
		{
			columns = l_truthTable->getInputCount();
		}
	}

	return columns;
}

int TruthTableInputTableModel::rowCount(const QModelIndex& parent) const
{
	int rows = 0;

	if (parent.isValid() == false)
	{
		auto l_truthTable = this->truthTable.lock();

		if (l_truthTable != nullptr)
		{
			rows = l_truthTable->getRowsCount();
		}
	}

	return rows;
}

QVariant TruthTableInputTableModel::data(const QModelIndex& index, int role) const
{
	QVariant variant{};

	if (index.isValid() == true)
	{
		if (role == Qt::DisplayRole)
		{
			auto l_truthTable = this->truthTable.lock();

			if (l_truthTable != nullptr)
			{
				if (index.column() < static_cast<int>(l_truthTable->getInputCount()))
				{
					auto inputValue = l_truthTable->getInputValue(index.row(), index.column());
					variant = QVariant(inputValue.toDisplayString());
				}
			}
		}
		else if (role == Qt::BackgroundRole)
		{
			if (this->truthTable.expired() == false)
			{
				if (this->highlights.contains(index.row()))
				{
					variant = QVariant(QColor(255, 0, 0, 25));
				}
				else
				{
					if ( (index.row() % 2) == 0)
					{
						variant = QVariant(QColor(0, 0, 255, 25));
					}
					else
					{
						variant = QVariant(QColor(0, 0, 255, 50));
					}
				}
			}
		}
	}

	return variant;
}

QVariant TruthTableInputTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant variant{};

	if (role == Qt::DisplayRole)
	{
		auto l_truthTable = this->truthTable.lock();

		if (l_truthTable != nullptr)
		{
			if (orientation == Qt::Horizontal)
			{
				if (section < static_cast<int>(l_truthTable->getInputCount()))
				{
					auto inputText = l_truthTable->getInputVariableText(section);
					variant = QVariant(inputText);
				}
			}
			else
			{
				variant = QVariant(section);
			}
		}
	}

	return variant;
}

Qt::ItemFlags TruthTableInputTableModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = Qt::NoItemFlags;

	if (index.isValid() == true)
	{
		if (this->truthTable.expired() == false)
		{
			flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
		}
	}

	return flags;
}
