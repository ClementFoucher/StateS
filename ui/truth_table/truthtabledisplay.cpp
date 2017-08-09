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
#include "truthtabledisplay.h"

// Qt classes
#include <QHeaderView>
#include <QScrollBar>

// StateS classes
#include "truthtableinputtablemodel.h"
#include "truthtableoutputtablemodel.h"
#include "truthtable.h"


TruthTableDisplay::TruthTableDisplay(shared_ptr<TruthTable> truthTable, QList<int> highlights, QWidget* parent) :
    QTableWidget(parent)
{
	this->insertColumn(0);
	this->insertColumn(1);
	this->insertRow(0);

	this->inputTable = new QTableView();
	this->inputTable->setModel(new TruthTableInputTableModel(truthTable, highlights));
	this->inputTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	this->outputTable = new QTableView();
	this->outputTable->setModel(new TruthTableOutputTableModel(truthTable, highlights));
	this->outputTable->verticalHeader()->hide();

	this->inputTable ->setVerticalScrollBarPolicy  (Qt::ScrollBarAlwaysOff);
	this->inputTable ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->outputTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	this->inputTable ->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->outputTable->setSelectionBehavior(QAbstractItemView::SelectRows);

	this->inputTable ->setSelectionMode(QAbstractItemView::SingleSelection);
	this->outputTable->setSelectionMode(QAbstractItemView::SingleSelection);


	this->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	this->verticalHeader()  ->setSectionResizeMode(QHeaderView::Fixed);
	this->verticalHeader()->hide();

	this->setCellWidget(0, 0, this->inputTable);
	this->setCellWidget(0, 1, this->outputTable);

	QStringList labelsList;
	if (truthTable->getInputCount() > 1)
		labelsList.append(tr("Inputs"));
	else
		labelsList.append(tr("Input"));
	if (truthTable->getOutputCount() > 1)
		labelsList.append(tr("Outputs"));
	else
		labelsList.append(tr("Result"));
	this->setHorizontalHeaderLabels(labelsList);

	this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

	connect(this->inputTable ->horizontalHeader(), &QHeaderView::sectionResized, this, &TruthTableDisplay::subTableResized);
	connect(this->outputTable->horizontalHeader(), &QHeaderView::sectionResized, this, &TruthTableDisplay::subTableResized);

	connect(this->inputTable ->verticalScrollBar(), &QScrollBar::valueChanged, this->outputTable->verticalScrollBar(), &QScrollBar::setValue);
	connect(this->outputTable->verticalScrollBar(), &QScrollBar::valueChanged, this->inputTable ->verticalScrollBar(), &QScrollBar::setValue);

	connect(this->inputTable ->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TruthTableDisplay::updateSelection);
	connect(this->outputTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TruthTableDisplay::updateSelection);
}

void TruthTableDisplay::resizeEvent(QResizeEvent* event)
{
	this->resizeCells();

	QTableWidget::resizeEvent(event);
}

void TruthTableDisplay::showEvent(QShowEvent* event)
{
	this->resizeCells();

	QTableWidget::showEvent(event);
}

void TruthTableDisplay::resizeCells()
{
	int inputsHSize = this->inputTable->horizontalHeader()->length();
	inputsHSize += this->inputTable->verticalHeader()->sizeHint().width();
	inputsHSize += this->inputTable->contentsMargins().left();
	inputsHSize += this->inputTable->contentsMargins().right();
	this->horizontalHeader()->resizeSection(0, inputsHSize);

	int outputsHSize = this->outputTable->horizontalHeader()->length();
	outputsHSize += this->outputTable->contentsMargins().left();
	outputsHSize += this->outputTable->contentsMargins().right();
	if (this->outputTable->verticalScrollBar()->isVisible())
		outputsHSize += this->outputTable->verticalScrollBar()->sizeHint().width();
	this->horizontalHeader()->resizeSection(1, outputsHSize);

	int vSize = this->height();
	vSize -= this->horizontalHeader()->sizeHint().height();
	vSize -= this->contentsMargins().top();
	vSize -= this->contentsMargins().bottom();
	this->verticalHeader()->resizeSection(0, vSize);
}

void TruthTableDisplay::subTableResized(int, int, int)
{
	this->resizeCells();
}

void TruthTableDisplay::updateSelection(const QItemSelection& selected, const QItemSelection&)
{
	this->inputTable ->selectRow(selected[0].top());
	this->outputTable->selectRow(selected[0].top());
}
