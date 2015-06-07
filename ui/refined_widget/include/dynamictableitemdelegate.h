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

#ifndef DYNAMICTABLEITEMDELEGATE_H
#define DYNAMICTABLEITEMDELEGATE_H

// Parent
#include <QItemDelegate>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QTableWidgetItem;
class QValidator;

// StateS classes
class DynamicLineEdit;


class DynamicTableItemDelegate  : public QItemDelegate
{
    Q_OBJECT

public:
    DynamicTableItemDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option, const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void refuseEdition();

    // This function takes ownership of the validator
    void setValidator(shared_ptr<QValidator> validator);

    DynamicLineEdit* getCurentEditor() const;

private:
    shared_ptr<QValidator> validator;

    // QWidget with a parent
    mutable DynamicLineEdit* latestEditor = nullptr;

};

#endif // DYNAMICTABLEITEMDELEGATE_H
