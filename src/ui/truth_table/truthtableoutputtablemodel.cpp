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
#include "truthtableoutputtablemodel.h"

// Qt classes
#include <QColor>

// StateS classes
#include "truthtable.h"
#include "logicvalue.h"


TruthTableOutputTableModel::TruthTableOutputTableModel(shared_ptr<TruthTable> truthTable, QList<int> highlights, QObject* parent) :
    QAbstractTableModel(parent)
{
	this->truthTable = truthTable;
	this->highlights = highlights;
}

int TruthTableOutputTableModel::columnCount(const QModelIndex& parent) const
{
	int columns = 0;

	if (!parent.isValid())
	{
		shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

		if (l_truthTable != nullptr)
		{
			columns = l_truthTable->getOutputCount();
		}
	}

	return columns;
}

int TruthTableOutputTableModel::rowCount(const QModelIndex& parent) const
{
	int rows = 0;

	if (!parent.isValid())
	{
		shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

		if (l_truthTable != nullptr)
		{
			rows = l_truthTable->getRowsCount();
		}
	}

	return rows;
}

QVariant TruthTableOutputTableModel::data(const QModelIndex& index, int role) const
{
	QVariant variant = QVariant();

	if (index.isValid())
	{
		if (role == Qt::DisplayRole)
		{
			shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

			if (l_truthTable != nullptr)
			{
				if (index.column() < (int)l_truthTable->getOutputCount())
				{
					auto outputValue = l_truthTable->getOutputValue(index.row(), index.column());
					variant = QVariant(outputValue.toString());
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
						variant = QVariant(QColor(0, 255, 0, 25));
					}
					else
					{
						variant = QVariant(QColor(0, 255, 0, 50));
					}
				}
			}
		}
		else if (role == Qt::ForegroundRole)
		{
			if (this->highlights.contains(index.row()))
			{
				shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

				if (l_truthTable != nullptr)
				{
					if (index.column() < (int)l_truthTable->getOutputCount())
					{
						variant = QVariant(QColor(255, 0, 0, 255));
					}
				}
			}
		}
	}

	return variant;
}

QVariant TruthTableOutputTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant variant = QVariant();

	if (role == Qt::DisplayRole)
	{
		shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

		if (l_truthTable != nullptr)
		{
			if (orientation == Qt::Horizontal)
			{
				if (section < (int)l_truthTable->getOutputCount())
				{
					auto equationText = l_truthTable->getOutputEquationText(section);
					variant = QVariant(equationText);
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

Qt::ItemFlags TruthTableOutputTableModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = Qt::NoItemFlags;

	if (index.isValid())
	{
		if (this->truthTable.expired() == false)
		{
			flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
		}
	}

	return flags;
}
