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

// Current class header
#include "equationparteditorwidget.h"

// Qt classes
#include <QInputEvent>
#include <QDrag>

// StateS classes
#include "equationeditorwidget.h"
#include "contextmenu.h"
#include "equationpartmimedata.h"


const QString EquationPartEditorWidget::passiveStyleSheet   = "EquationPartEditorWidget {border: 1px solid lightgrey; border-radius: 10px}";
const QString EquationPartEditorWidget::activeStyleSheet    = "EquationPartEditorWidget {border: 1px solid blue; border-radius: 10px}";
const QString EquationPartEditorWidget::erroneousStyleSheet = "EquationPartEditorWidget {border: 1px solid red; border-radius: 10px}";
const QString EquationPartEditorWidget::editableStyleSheet  = "EquationPartEditorWidget {border: 1px solid green; border-radius: 10px}";


EquationPartEditorWidget::EquationPartEditorWidget(uint rankInParentOperands, bool isTemplate, QWidget* parent) :
	QFrame(parent)
{
	this->isTemplate = isTemplate;

	if (this->isTemplate == false)
	{
		this->parentEquationEditor = dynamic_cast<EquationEditorWidget*>(parent);
		this->rankInParentOperands = rankInParentOperands;

		// Accept drops for equation parts only
		if (this->isRootEquation() == false)
		{
			this->setAcceptDrops(true);
		}
	}
}

void EquationPartEditorWidget::beginEdit()
{
	// Provide a default implementation so that non-editable parts
	// don't have to care about these functions.
}

bool EquationPartEditorWidget::validEdit()
{
	// Provide a default implementation so that non-editable parts
	// don't have to care about these functions.
	return false;
}

bool EquationPartEditorWidget::cancelEdit()
{
	// Provide a default implementation so that non-editable parts
	// don't have to care about these functions.
	return false;
}

void EquationPartEditorWidget::enterEvent(QEnterEvent*)
{
	if (this->parentEquationEditor != nullptr)
	{
		this->parentEquationEditor->setHighlighted(false);
	}

	this->setHighlighted(true);
}

void EquationPartEditorWidget::leaveEvent(QEvent*)
{
	if (this->parentEquationEditor != nullptr)
	{
		this->parentEquationEditor->setHighlighted(true);
	}

	this->setHighlighted(false);
}

void EquationPartEditorWidget::mousePressEvent(QMouseEvent* event)
{
	bool acceptEvent = false;

	if (this->isTemplate == true)
	{
		if (event->button() == Qt::LeftButton)
		{
			// Generate drag-n-drop object on mouse pressed
			auto drag = this->buildDrag();
			drag->exec();

			acceptEvent = true;
		}
	}
	else
	{
		if (event->button() == Qt::LeftButton)
		{
			if (this->getIsEditable() == true)
			{
				this->beginEdit();

				acceptEvent = true;
			}
		}
	}

	if (acceptEvent == false)
	{
		event->ignore();
	}
}

void EquationPartEditorWidget::contextMenuEvent(QContextMenuEvent* event)
{
	if (this->isTemplate == true) return;

	if (this->isRootEquation() == true) return;


	auto menu = new ContextMenu();
	menu->addTitle(tr("Operand:") +  " <i>" + this->getText() + "</i>");

	QVariant data;
	QAction* addedAction = nullptr;

	uint allowedActions = this->getAllowedMenuActions();

	if ((allowedActions & (uint)ContextAction_t::Remove) != 0)
	{
		addedAction = menu->addAction(tr("Remove"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::Remove);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::Edit) != 0)
	{
		addedAction = menu->addAction(tr("Edit"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::Edit);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::ExtractSwitchSingle) != 0)
	{
		addedAction = menu->addAction(tr("Extract single bit"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::ExtractSwitchSingle);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::ExtractSwitchRange) != 0)
	{
		addedAction = menu->addAction(tr("Extract sub-vector"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::ExtractSwitchRange);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::AddExtractor) != 0)
	{
		addedAction = menu->addAction(tr("Extract sub-vector or single bit"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::AddExtractor);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::IncreaseOperandCount) != 0)
	{
		addedAction = menu->addAction(tr("Add one operand to that operator"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::IncreaseOperandCount);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::DecreaseOperandCount) != 0)
	{
		addedAction = menu->addAction(tr("Remove one operand from that operator"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::DecreaseOperandCount);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::Invert) != 0)
	{
		addedAction = menu->addAction(tr("Invert"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::Invert);
		addedAction->setData(data);
	}

	if ((allowedActions & (uint)ContextAction_t::RemoveInverter) != 0)
	{
		addedAction = menu->addAction(tr("Remove the inverter"));
		data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::RemoveInverter);
		addedAction->setData(data);
	}

	addedAction = menu->addAction(tr("Cancel"));
	data.setValue((int)ActionType_t::ContextMenuAction | (int)ContextAction_t::Cancel);
	addedAction->setData(data);

	menu->popup(this->mapToGlobal(event->pos()));

	connect(menu, &QMenu::triggered,   this, &EquationPartEditorWidget::processMenuEventHandler);
	connect(menu, &QMenu::aboutToHide, this, &EquationPartEditorWidget::menuHidingEventHandler);
}

void EquationPartEditorWidget::dragEnterEvent(QDragEnterEvent* event)
{
	// Check if drag object is acceptable
	const EquationPartMimeData* mimeData = dynamic_cast<const EquationPartMimeData*>(event->mimeData());
	if (mimeData == nullptr) return;


	this->setHighlighted(true);
	event->acceptProposedAction();
}

void EquationPartEditorWidget::dragLeaveEvent(QDragLeaveEvent*)
{
	this->setHighlighted(false);
}

void EquationPartEditorWidget::dropEvent(QDropEvent* event)
{
	auto mimeData = dynamic_cast<const EquationPartMimeData*>(event->mimeData());
	if (mimeData == nullptr) return;


	event->acceptProposedAction();

	// Obtain dropped content
	this->tempValueNature = mimeData->getSource();
	switch(this->tempValueNature)
	{
	case OperandSource_t::variable:
		this->tempVariableId = mimeData->getVariableId();
		break;
	case OperandSource_t::equation:
		this->tempEquation = mimeData->getEquation();
		break;
	case OperandSource_t::constant:
		this->tempConstant = mimeData->getConstant();
		break;
	}

	// Check allowed actions
	uint allowedActions = mimeData->getAvailableActions() & this->getAllowedDropActions();

	bool displayMenu = true;
	bool autoReplace = false;
	if ( ((allowedActions & (uint)DropAction_t::ReplaceExisting) != 0) && (this->getReplaceWithoutAsking() == true) )
	{
		if ( (mimeData->getSource() != OperandSource_t::equation) || ((allowedActions & (uint)DropAction_t::ExistingAsOperand) == 0) )
		{
			// Special case: only replace action available, and part allows to be replaced without asking
			// Replace without asking, except if source is equation: ask if replace or use as operand
			displayMenu = false;
			autoReplace = true;
		}
	}

	if (displayMenu == true)
	{
		// Ask what to do
		auto menu = new ContextMenu();
		auto subtitles = QStringList();
		subtitles.append(tr("Current element:") + " <i>"  + this->getText() + "</i>");
		subtitles.append(tr("Dropped element:") + " <i> " + mimeData->getText() + "</i>");

		if ((allowedActions & ~(uint)DropAction_t::ReplaceExisting) != 0)
		{
			menu->addTitle(tr("What do you want to do?"), subtitles);
		}
		else
		{
			menu->addTitle(tr("Do you want to replace that element?"), subtitles);
		}

		QVariant data;
		QAction* addedAction;

		if ((allowedActions & (uint)DropAction_t::ReplaceExisting) != 0)
		{
			addedAction = menu->addAction(tr("Replace current element by dropped element"));
			data.setValue((int)ActionType_t::DropAction | (int)DropAction_t::ReplaceExisting);
			addedAction->setData(data);
		}

		if ((allowedActions & (uint)DropAction_t::ExistingAsOperand) != 0)
		{
			addedAction = menu->addAction(tr("Make current element an operand of dropped equation"));
			data.setValue((int)ActionType_t::DropAction | (int)DropAction_t::ExistingAsOperand);
			addedAction->setData(data);
		}

		if ((allowedActions & (uint)DropAction_t::RemoveInverter) != 0)
		{
			addedAction = menu->addAction(tr("Remove inverter from current element"));
			data.setValue((int)ActionType_t::DropAction | (int)DropAction_t::RemoveInverter);
			addedAction->setData(data);
		}

		menu->addSeparator();

		addedAction = menu->addAction(tr("Cancel"));
		data.setValue((int)ActionType_t::DropAction | (int)DropAction_t::Cancel);
		addedAction->setData(data);

		menu->popup(this->mapToGlobal(event->position()).toPoint());

		connect(menu, &QMenu::triggered,   this, &EquationPartEditorWidget::processMenuEventHandler);
		connect(menu, &QMenu::aboutToHide, this, &EquationPartEditorWidget::menuHidingEventHandler);
	}
	else if (autoReplace == true)
	{
		this->isBeingReplacedByDrop = true;
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
	}
}

void EquationPartEditorWidget::showEvent(QShowEvent* event)
{
	// Force border update on first show
	this->updateBorderColor();

	// Set tool tip
	this->setToolTip(this->getToolTipText());

	QFrame::showEvent(event);
}

bool EquationPartEditorWidget::getIsEditable() const
{
	// Provide a default implementation so that non-editable parts
	// don't have to care about these functions.
	return false;
}

void EquationPartEditorWidget::replaceByTempValue()
{
	if (this->parentEquationEditor == nullptr) return;


	switch(this->tempValueNature)
	{
	case OperandSource_t::variable:
		if (this->tempVariableId == nullId) return;


		this->parentEquationEditor->replaceOperand(this->rankInParentOperands, this->tempVariableId);
		// Do NOT do anything after, as this ceases to exist
		break;
	case OperandSource_t::equation:
		if (this->tempEquation == nullptr)
		{
			// Empty equation is possible in certain cases
			this->parentEquationEditor->clearOperand(this->rankInParentOperands);
		}
		else
		{
			this->parentEquationEditor->replaceOperand(this->rankInParentOperands, this->tempEquation);
		}
		// Do NOT do anything after, as this ceases to exist
		break;
	case OperandSource_t::constant:
		if (this->tempConstant.isNull() == true) return;


		this->parentEquationEditor->replaceOperand(this->rankInParentOperands, this->tempConstant, this->isBeingReplacedByDrop);
		// Do NOT do anything after, as this ceases to exist
		break;
	}
}

bool EquationPartEditorWidget::isRootEquation() const
{
	if ( (this->parentEquationEditor == nullptr) && (this->isTemplate == false) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void EquationPartEditorWidget::setHighlighted(bool highlighted)
{
	this->isHighlighted = highlighted;
	this->updateBorderColor();
}

void EquationPartEditorWidget::updateBorderColor()
{
	if (this->isRootEquation() == true) return;


	if (this->isHighlighted == true)
	{
		this->setStyleSheet(EquationPartEditorWidget::activeStyleSheet);
	}
	else if (this->isTemplate == true)
	{
		this->setStyleSheet(EquationPartEditorWidget::passiveStyleSheet);
	}
	else if (this->getIsErroneous() == true)
	{
		this->setStyleSheet(EquationPartEditorWidget::erroneousStyleSheet);
	}
	else if (this->getIsEditable() == true)
	{
		this->setStyleSheet(EquationPartEditorWidget::editableStyleSheet);
	}
	else
	{
		this->setStyleSheet(EquationPartEditorWidget::passiveStyleSheet);
	}
}

void EquationPartEditorWidget::processMenuAction(ContextAction_t action)
{
	switch (action)
	{
	case ContextAction_t::Remove:
		if (this->parentEquationEditor == nullptr) return;


		this->parentEquationEditor->clearOperand(this->rankInParentOperands);
		// Do NOT do anything after, as this ceases to exist
		break;
	case ContextAction_t::Edit:
		this->beginEdit();
		break;
	case ContextAction_t::IncreaseOperandCount:
	case ContextAction_t::DecreaseOperandCount:
	case ContextAction_t::ExtractSwitchRange:
	case ContextAction_t::ExtractSwitchSingle:
	case ContextAction_t::AddExtractor:
	case ContextAction_t::Invert:
	case ContextAction_t::RemoveInverter:
		this->processSpecificMenuAction(action);
		// Do NOT do anything after, as this may cease to exist
		break;
	case ContextAction_t::Cancel:
		break;
	}
}

void EquationPartEditorWidget::processDropAction(DropAction_t action)
{
	switch (action)
	{
	case DropAction_t::ReplaceExisting:
		this->isBeingReplacedByDrop = true;
		this->replaceByTempValue();
		// Do NOT do anything after, as this ceases to exist
		break;
	case DropAction_t::ExistingAsOperand:
	case DropAction_t::RemoveInverter:
		this->processSpecificDropAction(action);
		// Do NOT do anything after, as this may cease to exist
		break;
	case DropAction_t::Cancel:
		break;
	}
}

void EquationPartEditorWidget::processMenuEventHandler(QAction* action)
{
	QVariant data = action->data();
	uint dataValue = (uint)data.toInt();

	ActionType_t actionType = (ActionType_t)(dataValue & 0xF000);

	if (actionType == ActionType_t::ContextMenuAction)
	{
		ContextAction_t actionValue = (ContextAction_t)(dataValue & 0x0FFF);
		this->processMenuAction(actionValue);
	}
	else
	{
		DropAction_t actionValue = (DropAction_t)(dataValue & 0x0FFF);
		this->processDropAction(actionValue);
	}
}

void EquationPartEditorWidget::menuHidingEventHandler()
{
	// This is necessary as when menu pops up, the leave event is lost.
	// This should not cause issue with this being deleted as this function
	// is called before processing selected menu item.
	this->setHighlighted(false);
}
