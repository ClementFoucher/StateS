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

#ifndef VARIABLELISTEDITOR_H
#define VARIABLELISTEDITOR_H

// Parent
#include <QWidget>

// Qt classes
#include <QMap>
class QGridLayout;
class QPushButton;
class QTableWidgetItem;

// StateS classes
#include "statestypes.h"
class DynamicTableItemDelegate;
class TableWidgetWithResizeEvent;


class VariableListEditor : public QWidget
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class ListMode_t
	{
		initMode,
		standard,
		addingVariable,
		renamingVariable,
		resizingVariable,
		changingVariableInitialValue
	};
	enum class ContextAction_t : int
	{
		Cancel              = 0,
		DeleteVariable      = 1,
		Up                  = 2,
		Down                = 3,
		RenameVariable      = 4,
		ResizeVariable      = 5,
		ChangeVariableValue = 6
	};

	/////
	// Constructors/destructors
public:
	explicit VariableListEditor(VariableNature_t editorType, QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	virtual void keyPressEvent   (QKeyEvent*         event) override;
	virtual void keyReleaseEvent (QKeyEvent*         event) override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
	// General
	void updateList();
	void updateButtonsEnableState();
	void handleListResizedEvent();

	// Handle variable add
	void beginAddVariable();
	void addingVariableSwitchField(QTableWidgetItem* newItem);
	void addingVariableCurrentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void endAddVariable();

	// Handle variable edit
	void beginEditVariable(QTableWidgetItem* characteristicToEdit);
	void endRenameVariable();
	void endResizeVariable();
	void endChangeVariableInitialValue();

	// Add/edit common
	void validateCurrentEdit();
	void cancelCurrentEdit();

	// Other
	void raiseSelectedVariables();
	void lowerSelectedVariables();
	void removeSelectedVariables();
	void processMenuEventHandler(QAction* action);

private:
	void switchMode(ListMode_t newMode);
	void editCurrentCell(bool erroneous = false);
	void fixVariableSize();
	QList<QString> getSelectedVariables();

	/////
	// Object variables
private:
	VariableNature_t editorType;
	QString newVariablesPrefix;

	ListMode_t currentMode = ListMode_t::initMode;

	// Widgets
	QGridLayout* buttonLayout = nullptr;

	TableWidgetWithResizeEvent* variablesList = nullptr;
	DynamicTableItemDelegate*   listDelegate  = nullptr;

	QPushButton* buttonAdd    = nullptr;
	QPushButton* buttonRemove = nullptr;
	QPushButton* buttonCancel = nullptr;
	QPushButton* buttonOK     = nullptr;
	QPushButton* buttonUp     = nullptr;
	QPushButton* buttonDown   = nullptr;

	// Cell under edition
	QTableWidgetItem* currentTableItem = nullptr;
	QStringList variableSelectionToRestore;

	// Variable begin created
	QTableWidgetItem* currentVariableName  = nullptr;
	QTableWidgetItem* currentVariableSize  = nullptr;
	QTableWidgetItem* currentVariableValue = nullptr;

	// Used to know which variable is associated to each cell in table
	QMap<QTableWidgetItem*, componentId_t> associatedVariablesIds;

};

#endif // VARIABLELISTEDITOR_H
