/*
 * Copyright © 2014-2021 Clément Foucher
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

#ifndef REACTIVEBUTTON_H
#define REACTIVEBUTTON_H

// Parent
#include <QPushButton>


class ReactiveButton : public QPushButton
{
	Q_OBJECT

public:
	explicit ReactiveButton(QWidget* parent = nullptr);
	explicit ReactiveButton(const QString& text, QWidget* parent = nullptr);

signals:
	void mouseEnterEvent();
	void mouseLeaveEvent();
	void keyboardFocusInEvent();
	void keyboardFocusOutEvent();

protected:
	void enterEvent(QEnterEvent*) override;
	void leaveEvent(QEvent*) override;
	void focusInEvent (QFocusEvent*) override;
	void focusOutEvent(QFocusEvent*) override;

};

#endif // REACTIVEBUTTON_H
