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

#ifndef RANGEEXTRACTORWIDGET_H
#define RANGEEXTRACTORWIDGET_H

// Parent
#include "editableequation.h"

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QLabel;

// StateS classes
class LineEditWithUpDownButtons;
class Equation;


class RangeExtractorWidget : public EditableEquation
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit RangeExtractorWidget(shared_ptr<Equation> equation, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	virtual bool validEdit()            override;
	virtual bool cancelEdit()           override;
	virtual void setEdited(bool edited) override;

signals:
	void rangeLChanged(int newValue);
	void rangeRChanged(int newValue);

protected:
	virtual void mousePressEvent      (QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent       (QMouseEvent* event) override;
	virtual void mouseReleaseEvent    (QMouseEvent* event) override;
	virtual void wheelEvent           (QWheelEvent* event) override;

private:
	void update();
	void reset();

	/////
	// Object variables
private:
	weak_ptr<Equation> equation;

	QLabel* rangeLText = nullptr;
	QLabel* rangeRText = nullptr;

	LineEditWithUpDownButtons* rangeLEditor = nullptr;
	LineEditWithUpDownButtons* rangeREditor = nullptr;

	bool editMode = false;

	bool inMouseEvent = false;

};

#endif // RANGEEXTRACTORWIDGET_H
