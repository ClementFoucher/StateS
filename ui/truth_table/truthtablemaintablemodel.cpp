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
#include "truthtablemaintablemodel.h"

// Qt classes
#include <QColor>
#include <QTableView>

// StateS classes
#include "truthtable.h"
#include "signal.h"
#include "statesexception.h"
#include "truthtableinputtablemodel.h"
#include "truthtableoutputtablemodel.h"


TruthTableMainTableModel::TruthTableMainTableModel(shared_ptr<TruthTable> truthTable, QList<int> highlights, QObject* parent) :
    QAbstractTableModel(parent)
{
    this->truthTable = truthTable;

    this->inputTable = new QTableView();
    this->inputTable->setModel(new TruthTableInputTableModel(truthTable, highlights));

    this->outputTable = new QTableView();
    this->outputTable->setModel(new TruthTableOutputTableModel(truthTable, highlights));
}

int TruthTableMainTableModel::columnCount(const QModelIndex& parent) const
{
    int columns;

    if (!parent.isValid())
    {
        shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

        if (l_truthTable != nullptr)
        {
            columns = 2;
        }
        else
        {
            // Default to single-cell table with warning message
            columns = 1;
        }
    }
    else
    {
        columns = 0;
    }

    return columns;
}

int TruthTableMainTableModel::rowCount(const QModelIndex& parent) const
{
    int rows;

    if (!parent.isValid())
    {
        rows = 1;
    }
    else
    {
        rows = 0;
    }

    return rows;
}

QVariant TruthTableMainTableModel::data(const QModelIndex& index, int role) const
{
    QVariant variant = QVariant();

    if (index.isValid())
    {
        if (role == Qt::DisplayRole)
        {
            shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

            if (l_truthTable != nullptr)
            {
           /*     if (index.column() == 0)
                {
                    variant = QVariant(this->inputTable);
                }
                else
                {
                    variant = QVariant(this->outputTable);
                }*/
            }
            else
            {
                // Default to single-cell table with warning message
                variant = QVariant(tr("Error! No table loaded."));
            }
        }
    }

    return variant;
}

QVariant TruthTableMainTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant variant = QVariant();

    if (role == Qt::DisplayRole)
    {
        shared_ptr<TruthTable> l_truthTable = this->truthTable.lock();

        if (l_truthTable != nullptr)
        {
            if (orientation == Qt::Horizontal)
            {
                if (section < (int)l_truthTable->getInputCount())
                {
                    try
                    {
                        variant = QVariant(l_truthTable->getInputs()[section]->getName());
                    }
                    catch (const StatesException& e)
                    {
                        if ( (e.getSourceClass() == "TruthTable") && (e.getEnumValue() == TruthTable::TruthTableErrorEnum::reference_expired) )
                        {
                            // Truth table is no longer valid: we should invalidate model
                            //this->truthTable.reset();
                        }
                        else
                            throw;
                    }
                }
                else if ( (section - (int)l_truthTable->getInputCount()) < (int)l_truthTable->getOutputCount())
                {
                    QVector<QString> equationTable = l_truthTable->getOutputsEquations();

                    if (equationTable.count() == 1)
                    {
                        variant = QVariant(tr("Result"));
                    }
                    else
                    {
                        variant = QVariant(tr("Result:") + " " + equationTable[section - l_truthTable->getInputCount()]);
                    }
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

Qt::ItemFlags TruthTableMainTableModel::flags(const QModelIndex& index) const
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
