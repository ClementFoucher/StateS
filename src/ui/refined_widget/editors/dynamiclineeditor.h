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

#ifndef DYNAMICLINEEDITOR_H
#define DYNAMICLINEEDITOR_H

// Parent
#include <QLineEdit>


/**
 * @brief The DynamicLineEditor class provides a line edit
 * which is highlighted when edition starts, can be marked
 * as erroneous and provides a signal when user cancels
 * edit using the escape key.
 */
class DynamicLineEditor : public QLineEdit
{
	Q_OBJECT

	/////
	// Static variables
protected:
	static const QString editStyle;
	static const QString errorStyle;

	/////
	// Constructors/destructors
public:
	explicit DynamicLineEditor(const QString& content, QWidget* parent = nullptr);
	explicit DynamicLineEditor(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setErroneous(bool erroneous);
	bool getIsErroneous() const;

signals:
	void userCancelEvent();

protected:
	virtual void focusInEvent   (QFocusEvent* event) override;
	virtual void keyPressEvent  (QKeyEvent*   event) override;
	virtual void keyReleaseEvent(QKeyEvent*   event) override;

	/////
	// Object variables
protected:
	bool erroneous = false;

};

#endif // DYNAMICLINEEDITOR_H
