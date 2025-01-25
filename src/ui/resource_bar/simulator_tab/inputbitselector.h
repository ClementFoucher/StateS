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

#ifndef INPUTBITSELECTOR_H
#define INPUTBITSELECTOR_H

// Parent
#include <QFrame>

// C++classes
#include <memory>
using namespace std;

// Qt classes
class QLabel;

// StateS classes
class Variable;


class InputBitSelector : public QFrame
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit InputBitSelector(shared_ptr<Variable> signalToCommand, uint bitNumber, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void enterEvent           (QEnterEvent* event) override;
	virtual void leaveEvent           (QEvent* event)      override;
	virtual void mousePressEvent      (QMouseEvent*)       override;
	virtual void mouseMoveEvent       (QMouseEvent*)       override;
	virtual void mouseReleaseEvent    (QMouseEvent*)       override;
	virtual void mouseDoubleClickEvent(QMouseEvent*)       override;

private slots:
	void signalValueChangedEventHandler();

	/////
	// Object variables
private:
	weak_ptr<Variable> signalToCommand;
	uint bitNumber = 0;

	// QWidget with parent
	QLabel* bitValue = nullptr;

};

#endif // INPUTBITSELECTOR_H
