/*
 * Copyright © 2014-2023 Clément Foucher
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

#ifndef DYNAMICLINEEDIT_H
#define DYNAMICLINEEDIT_H

// Parent
#include <QLineEdit>

// Qt classes
class QValidator;


class DynamicLineEdit : public QLineEdit
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit DynamicLineEdit(const QString& content, bool selfManaged, QValidator* validator, QWidget* parent = nullptr);
	explicit DynamicLineEdit(const QString& content, bool selfManaged, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void markAsErroneous();
	void resetView();

signals:
	void newTextAvailableEvent(const QString& text);
	void userCancelEvent();
	void upKeyPressed();
	void downKeyPressed();

protected:
	virtual void focusInEvent   (QFocusEvent* event) override;
	virtual void keyPressEvent  (QKeyEvent*   event) override;
	virtual void keyReleaseEvent(QKeyEvent*   event) override;

private slots:
	void userValidatedEventHandler();

	/////
	// Object variables
private:
	bool erroneous = false;
	bool selfManaged = false;

};

#endif // DYNAMICLINEEDIT_H
