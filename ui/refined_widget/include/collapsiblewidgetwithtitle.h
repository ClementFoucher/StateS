/*
 * Copyright © 2014-2020 Clément Foucher
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

#ifndef COLLAPSIBLEWIDGETWITHTITLE_H_
#define COLLAPSIBLEWIDGETWITHTITLE_H_

// Parent
#include <QFrame>

// Qt classes
class QWidget;
class QPushButton;
class QLabel;
class QGridLayout;
class QResizeEvent;


class CollapsibleWidgetWithTitle : public QFrame
{
	Q_OBJECT

public:
	explicit CollapsibleWidgetWithTitle(const QString& title, QWidget* content, QWidget* parent = nullptr);
	explicit CollapsibleWidgetWithTitle(QWidget* parent = nullptr);

	void setContent(const QString& title, QWidget* content, bool deletePreviousContent = false);
	void setContent(const QString& title, const QString& textContent, bool deletePreviousContent = false);

	void setCollapsed(bool collapse);
	bool getCollapsed();

private slots:
	void buttonCollapseToggledEventHander(bool buttonPushed);

private:
	QGridLayout* layout         = nullptr;
	QPushButton* buttonCollapse = nullptr;
	QLabel*      title          = nullptr;
	QWidget*     content        = nullptr;

	void collapse();
	void extend();

};

#endif // COLLAPSIBLEWIDGETWITHTITLE_H_
