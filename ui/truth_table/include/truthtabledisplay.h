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

#ifndef TRUTHTABLEDISPLAY_H
#define TRUTHTABLEDISPLAY_H

// Parent
#include <QTableWidget>

// C++ classes
using namespace std;
#include <memory>

// StateS classes
class TruthTable;


class TruthTableDisplay : public QTableWidget
{
    Q_OBJECT

public:
    explicit TruthTableDisplay(shared_ptr<TruthTable> truthTable, QList<int> highlights = QList<int>(), QWidget* parent = nullptr);

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void showEvent  (QShowEvent* event)   override;

private slots:
    void subTableResized(int, int, int);
    void updateSelection(const QItemSelection& selected, const QItemSelection&);

private:
    void resizeCells();

    QTableView* inputTable  = nullptr;
    QTableView* outputTable = nullptr;
};

#endif // TRUTHTABLEDISPLAY_H
