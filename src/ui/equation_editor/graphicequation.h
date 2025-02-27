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

#ifndef GRAPHICEQUATION_H
#define GRAPHICEQUATION_H

// Parent
#include <QFrame>

// C++ classes
#include <memory>
using namespace std;

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class Equation;
class EditableEquation;


class GraphicEquation : public QFrame
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum CommonAction_t { Cancel = 0 };
	enum DropAction_t { ReplaceExisting = 1, ExistingAsOperand = 2};
	enum ContextAction_t { DeleteEquation = 3, IncrementOperandCount = 4, DecrementOperandCount = 5, ExtractSwitchSingle = 6, ExtractSwitchRange = 7, EditRange = 8, EditValue = 9};

	/////
	// Constructors/destructors
public:
	explicit GraphicEquation(shared_ptr<Equation> equation, int operandNumber, bool isTemplate = false, QWidget* parent = nullptr);
	explicit GraphicEquation(componentId_t variableId,      int operandNumber, bool isTemplate = false, QWidget* parent = nullptr);
	explicit GraphicEquation(LogicValue constant,           int operandNumber, bool isTemplate = false, QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void replaceOperand(uint operandNumber, shared_ptr<Equation> newOperand);
	void replaceOperandConstantValue(uint operandNumber, LogicValue newValue);
	void setVariableLock(bool lock);

	shared_ptr<Equation> getLogicEquation() const;

	void forceCompleteRendering();
	bool validEdit();
	bool cancelEdit();

protected:
	virtual void enterEvent(QEnterEvent*) override;
	virtual void leaveEvent(QEvent*)      override;

	virtual void mousePressEvent      (QMouseEvent* event) override;
	virtual void mouseMoveEvent       (QMouseEvent* event) override;
	virtual void mouseReleaseEvent    (QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
	virtual void dropEvent     (QDropEvent*      event) override;

	virtual void contextMenuEvent(QContextMenuEvent* event) override;

	void enterChildrenEvent();
	void leaveChildrenEvent();

private slots:
	void treatMenuEventHandler(QAction* action);
	void treatRangeLeftBoundChanged(int newIndex);
	void treatRangeRightBoundChanged(int newIndex);
	void treatConstantValueChanged(LogicValue newValue);
	void updateBorder();

private:
	void configure();
	void setDefaultBorderColor();
	void setHilightedBorderColor();

	void replaceEquation(shared_ptr<Equation> newEquation);
	void buildEquation();
	void buildTemplateEquation();
	void buildVariableEquation();
	void buildCompleteEquation();

	GraphicEquation* parentEquation() const;

	/////
	// Object variables
private:
	bool isTemplate;
	int operandNumber;

	weak_ptr<Equation> equation;
	// Only top-level GraphicEquation holds root Equation
	shared_ptr<Equation> rootEquation;
	// Just to not loose the reference for variable/constant equations
	shared_ptr<Equation> localEquation;

	shared_ptr<Equation> droppedEquation;

	bool completeRendering = false;
	bool mouseIn = false;

	EditableEquation* editorWidget = nullptr;

	bool inMouseEvent = false;
	bool lockVariable = false;

};

#endif // GRAPHICEQUATION_H
