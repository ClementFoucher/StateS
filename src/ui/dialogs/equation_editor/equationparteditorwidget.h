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

#ifndef EQUATIONPARTEDITORWIDGET_H
#define EQUATIONPARTEDITORWIDGET_H

// Parent
#include <QFrame>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QDrag;

// StateS classes
#include "statestypes.h"
#include "logicvalue.h"
class EquationEditorWidget;
class Equation;


class EquationPartEditorWidget : public QFrame
{
	Q_OBJECT

	/////
	// Type declarations
protected:
	// Mask for action type:  0xF000
	// Mask for action value: 0x0FFF
	enum class ActionType_t : uint
	{
		ContextMenuAction = 0x1000,
		DropAction        = 0x2000
	};
	enum class DropAction_t : uint
	{
		// Common actions
		Cancel              = 0x00,
		ReplaceExisting     = 0x01,
		// Specific action applying to all but null
		ExistingAsOperand   = 0x02,
		// Action specific to equations
		RemoveInverter      = 0x04
	};
	enum class ContextAction_t : uint
	{
		// Common actions
		Cancel               = 0x00,
		Remove               = 0x01,
		Edit                 = 0x02,
		Invert               = 0x04,
		RemoveInverter       = 0x08,
		// Actions specific to equations
		IncreaseOperandCount = 0x10,
		DecreaseOperandCount = 0x20,
		ExtractSwitchSingle  = 0x40,
		ExtractSwitchRange   = 0x80,
		AddExtractor         = 0x100
	};

	/////
	// Static variables
protected:
	static const QString passiveStyleSheet;
	static const QString activeStyleSheet;
	static const QString erroneousStyleSheet;
	static const QString editableStyleSheet;

	/////
	// Constructors/destructors
public:
	explicit EquationPartEditorWidget(uint rankInParentOperands, bool isTemplate, QWidget* parent = nullptr); // Parent is mandatory if this is an operand of another equation

	/////
	// Object functions
public:
	// Functions for editable parts
	virtual void beginEdit();
	virtual bool validEdit();
	virtual bool cancelEdit();

protected:

	// Reimplemented Qt functions
	virtual void enterEvent(QEnterEvent*) override;
	virtual void leaveEvent(QEvent*)      override;

	virtual void mousePressEvent(QMouseEvent* event) override;

	virtual void contextMenuEvent(QContextMenuEvent* event) override;

	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dragLeaveEvent(QDragLeaveEvent*)       override;
	virtual void dropEvent     (QDropEvent*      event) override;

	virtual void showEvent(QShowEvent* event) override;

	// Pure virtual functions implemented by children
	virtual uint    getAllowedMenuActions()   const = 0;
	virtual uint    getAllowedDropActions()   const = 0;
	virtual QString getText()                 const = 0;
	virtual QString getToolTipText()          const = 0;
	virtual bool    getReplaceWithoutAsking() const = 0;
	virtual bool    getIsErroneous()          const = 0;

	virtual QDrag* buildDrag() = 0;

	virtual void processSpecificMenuAction(ContextAction_t action) = 0;
	virtual void processSpecificDropAction(DropAction_t    action) = 0;

	// Functions for editable parts
	virtual bool getIsEditable() const;

	// Functions accessible to children
	void replaceByTempValue();
	bool isRootEquation() const;

private:
	void setHighlighted(bool highlighted);
	void updateBorderColor();

	void processMenuAction(ContextAction_t action);
	void processDropAction(DropAction_t action);

private slots:
	void processMenuEventHandler(QAction* action);
	void menuHidingEventHandler();

	/////
	// Object variables
protected:
	// Persistent values
	uint rankInParentOperands = 0xDEF; // Special value easy to identify in debug
	bool isTemplate;
	EquationEditorWidget* parentEquationEditor = nullptr;

	// Temporary storage
	OperandSource_t      tempValueNature;
	shared_ptr<Equation> tempEquation;
	componentId_t        tempVariableId = nullId;
	LogicValue           tempConstant   = LogicValue();

private:
	bool isHighlighted = false;
	bool isBeingReplacedByDrop = false;

};

#endif // EQUATIONPARTEDITORWIDGET_H
