/*
 * Copyright © 2014-2026 Clément Foucher
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

#ifndef RANGEEDITOR_H
#define RANGEEDITOR_H

// Parent
#include <QWidget>

// Stdlib
#include <memory>

// Qt
class QLabel;

// StateS
class LineWithUpDownButtonsEditor;
class Equation;


class RangeEditor : public QWidget
{
	Q_OBJECT

	/////
	// Type declarations
public:
	enum class Mode_t
	{
		compact_mode,
		editor_mode
	};

	/////
	// Constructors/destructors
public:
	explicit RangeEditor(std::shared_ptr<Equation> equation, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setExtractSingleBit();
	void setExtractRange();

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void wheelEvent     (QWheelEvent* event) override;

	virtual void keyPressEvent(QKeyEvent* event) override;

private slots:
	void lEditorValueChangedEventHandler(int newValue);
	void rEditorValueChangedEventHandler(int newValue);

private:
	void setMode(Mode_t newMode, bool saveChanges);

	void setRangeL(uint newValue);
	void setRangeR(uint newValue);

	void clear();
	void build();

	/////
	// Object variables
private:
	std::shared_ptr<Equation> equation;
	bool isValid = false;
	unsigned int vectorSize;

	QLabel* rangeLText = nullptr;
	QLabel* rangeRText = nullptr;

	LineWithUpDownButtonsEditor* rangeLEditor = nullptr;
	LineWithUpDownButtonsEditor* rangeREditor = nullptr;

	Mode_t mode = Mode_t::compact_mode;

};

#endif // RANGEEDITOR_H
