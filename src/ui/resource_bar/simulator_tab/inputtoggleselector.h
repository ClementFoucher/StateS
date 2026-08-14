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

#ifndef INPUTTOGGLESELECTOR_H
#define INPUTTOGGLESELECTOR_H

// Parent
#include <QFrame>

// Stdlib
#include <memory>

// Qt
class QLabel;

// StateS
#include "componentid.h"
class SimulatedVariable;


class InputToggleSelector : public QFrame
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit InputToggleSelector(ComponentId variableId, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	std::shared_ptr<SimulatedVariable> getVariable() const;

	virtual void enterEvent(QEnterEvent* event) override;
	virtual void leaveEvent(QEvent*      event) override;

private slots:
	virtual void variableValueChangedEventHandler() = 0;

	/////
	// Object variables
protected:
	ComponentId variableId = nullId;

	QLabel* toggleValue = nullptr;

};

#endif // INPUTTOGGLESELECTOR_H
