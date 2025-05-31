/*
 * Copyright © 2014-2025 Clément Foucher
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

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

// Parent
#include <QMenu>


class ContextMenu : public QMenu
{
	Q_OBJECT

	/////
	// Static functions
public:
	static ContextMenu* createErrorMenu(const QString& text, QWidget* parent = nullptr);
	static ContextMenu* createErrorMenu(const QStringList& texts, QWidget* parent = nullptr);

	/////
	// Static variables
private:
	static const QString defaultStyle;
	static const QString errorStyle;
	static const QString listStyle;

	/////
	// Constructors/destructors
public:
	explicit ContextMenu(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void addTitle(const QString& titleText, const QStringList& subtitlesText = QStringList());
	void setListStyle();

};

#endif // CONTEXTMENU_H
