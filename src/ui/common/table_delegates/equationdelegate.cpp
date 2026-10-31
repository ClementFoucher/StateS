/*
 * Copyright © 2026 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
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

// Current class header
#include "equationdelegate.h"

// Qt
#include <QTableView>

// StateS
#include "equationeditorcell.h"


QWidget* EquationDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	auto editor = new EquationEditorCell(parent);

	// For now, just enable condition coloring.
	// TODO: if this delegate is used to edit equations
	// that are not conditions, find a better way to do this.
	editor->setIsConditionEditor(true);

	return editor;
}

void EquationDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto equationEditor = dynamic_cast<EquationEditorCell*>(editor);
	if (equationEditor == nullptr) return;


	auto cellData = index.data(Qt::EditRole);
	auto equation = cellData.value<std::shared_ptr<const Equation>>();
	equationEditor->setEquation(equation);

	connect(equationEditor, &EquationEditorCell::equationChangedEvent, this, &EquationDelegate::equationChangedEventHandler);
}

void EquationDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto equationEditor = dynamic_cast<EquationEditorCell*>(editor);
	if (equationEditor == nullptr) return;


	auto newCondition = equationEditor->getEquation();
	auto variant = QVariant::fromValue(newCondition);
	model->setData(index, variant, Qt::EditRole);
}

void EquationDelegate::equationChangedEventHandler(EquationEditorCell* editor)
{
	emit this->commitData(editor);

	auto tableView = dynamic_cast<QTableView*>(this->parent());
	if (tableView != nullptr)
	{
		tableView->resizeColumnsToContents();
	}
}
