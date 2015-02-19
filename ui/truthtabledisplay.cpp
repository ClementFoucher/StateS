/*
 * Copyright © 2014-2015 Clément Foucher
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
#include "truthtabledisplay.h"

// Qt classes
#include <QHeaderView>

// StateS classes
#include "truthtable.h"
#include "signal.h"


TruthTableDisplay::TruthTableDisplay(const TruthTable *truthTable, QWidget* parent) :
    QTableWidget(parent)
{
    QStringList labelsList;

    QVector<Signal*> sigs = truthTable->getSignals();
    QVector<QVector<LogicValue>> inputTable = truthTable->getInputTable();
    QVector<LogicValue> outputTable = truthTable->getOutputTable();

    // Headers
    for (int i = 0 ; i < sigs.count() ; i++)
    {
        this->insertColumn(i);
        labelsList.append(sigs[i]->getName());
    }

    this->insertColumn(sigs.count());
    labelsList.append(tr("Result"));

    this->setHorizontalHeaderLabels(labelsList);

    // Table, row by row
    for(int i = 0 ; i < inputTable.count() ; i++)
    {
        this->insertRow(i);

        for (int j = 0 ; j < inputTable[i].count() ; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(inputTable[i][j].toString());

            this->setItem(i, j, item);
        }

        QTableWidgetItem* item = new QTableWidgetItem(outputTable[i].toString());

        this->setItem(i, sigs.count(), item);
    }
}


