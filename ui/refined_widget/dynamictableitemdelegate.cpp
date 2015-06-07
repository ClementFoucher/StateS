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
#include "dynamictableitemdelegate.h"

// Qt classes
#include <QTableWidgetItem>

// StateS classes
#include "dynamiclineedit.h"


DynamicTableItemDelegate::DynamicTableItemDelegate(QObject* parent) :
    QItemDelegate(parent)
{
}

/**
 * @brief DynamicTableItemDelegate::createEditor
 * This deletes the previous editor.
 * @param parent
 * @param index
 * @return
 */
QWidget* DynamicTableItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& index ) const
{
    this->latestEditor = shared_ptr<DynamicLineEdit>(new DynamicLineEdit(index.model()->data(index, Qt::EditRole).toString(), false, this->validator, parent));

    return this->latestEditor.get();
}

void DynamicTableItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QString content = index.model()->data(index, Qt::EditRole).toString();

    DynamicLineEdit* lineEdit = static_cast<DynamicLineEdit*>(editor);
    lineEdit->setText(content);
}

void DynamicTableItemDelegate::setValidator(shared_ptr<QValidator> validator)
{
    this->validator = validator;
}

shared_ptr<DynamicLineEdit> DynamicTableItemDelegate::getCurentEditor() const
{
    return this->latestEditor;
}
