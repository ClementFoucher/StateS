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

#ifndef TRUTHTABLEOUTPUTTABLEMODEL_H
#define TRUTHTABLEOUTPUTTABLEMODEL_H

// Parent class
#include <QAbstractTableModel>

// C++ classes
using namespace std;
#include <memory>

// StateS classes
class TruthTable;


class TruthTableOutputTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TruthTableOutputTableModel(shared_ptr<TruthTable> truthTable, QList<int> highlights, QObject* parent = nullptr);

    virtual int columnCount(const QModelIndex& parent) const                              override;
    virtual int rowCount(const QModelIndex& parent) const                                 override;
    virtual QVariant data(const QModelIndex& index, int role) const                       override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const                           override;

private:
    weak_ptr<TruthTable> truthTable;
    QList<int> highlights;
};

#endif // TRUTHTABLEOUTPUTTABLEMODEL_H
