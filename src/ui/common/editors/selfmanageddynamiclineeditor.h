/*
 * Copyright © 2025 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELFMANAGEDDYNAMICLINEEDITOR_H
#define SELFMANAGEDDYNAMICLINEEDITOR_H

// Parent
#include "dynamiclineeditor.h"


class SelfManagedDynamicLineEditor : public DynamicLineEditor
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit SelfManagedDynamicLineEditor(const QString& content, QWidget* parent = nullptr);
	explicit SelfManagedDynamicLineEditor(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void resetView();

protected:
	virtual void keyPressEvent  (QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

private slots:
	void userValidatedEventHandler();

	/////
	// Signals
signals:
	void newTextAvailableEvent(const QString& text);
	void userCancelEvent();

};

#endif // SELFMANAGEDDYNAMICLINEEDITOR_H
