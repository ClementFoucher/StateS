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

#ifndef CHECKBOXHTML_H
#define CHECKBOXHTML_H

// Parent
#include <QWidget>

// Qt classes
class QCheckBox;

// StateS classes
class LabelWithClickEvent;


/**
 * @brief The CheckBoxHtml class is used to allow clicking
 * on the text to update the associated check box.
 */
class CheckBoxHtml : public QWidget
{
	Q_OBJECT

public:
	explicit CheckBoxHtml(const QString& text, Qt::AlignmentFlag boxAlign = Qt::AlignmentFlag::AlignRight, bool allowLink = false, QWidget* parent = nullptr);

	void setText(QString newText);
	void setChecked(bool check);
	bool isChecked();

signals:
	void toggled(bool checked = false);

protected:
	bool event(QEvent* e) override;

private:
	LabelWithClickEvent* label;
	QCheckBox*           checkBox;
};

#endif // CHECKBOXHTML_H
