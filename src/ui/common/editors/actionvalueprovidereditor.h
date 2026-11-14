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

#ifndef ACTIONVALUEPROVIDEREDITOR_H
#define ACTIONVALUEPROVIDEREDITOR_H

// Parent
#include "cellwitheditandclearbuttonseditor.h"

// Qt
class QLabel;

// StateS
#include "machinevalue.h"
class Equation;
class EquationEditorDialog;
class ValueEditor;


class ActionValueProviderEditor : public CellWithEditAndClearButtonsEditor
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionValueProviderEditor(QWidget* parent = nullptr);

	/////
	// Object functions
public:
	void setValueProvider(std::shared_ptr<const Equation> valueProvider);
	std::shared_ptr<Equation> getValueProvider() const;

	void setExpectedType(MachineValue::Type_t expectedType, uint bitVectorExpectedSize = 0);

private slots:
	virtual void editButtonClickedEventHandler()  override;
	virtual void clearButtonClickedEventHandler() override;

	void equationEditorDialogClosedEventHandler(int result);
	void constantEditorValueChangedEventHandler();

private:
	virtual bool shouldClearButtonBeVisibile() override;

	void updateDisplay();
	void setTextAsMainWidget();
	void setConstantEditorAsMainWidget();

	bool isCurrentValueProviderConstant() const;

	/////
	// Object variables
private:
	std::shared_ptr<Equation> valueProvider;

	MachineValue::Type_t expectedType = MachineValue::Type_t::nullType;
	uint expectedBitVectorSize = 0;

	QLabel*      valueProviderText = nullptr;
	ValueEditor* constantEditor    = nullptr;

	EquationEditorDialog* equationEditorDialog = nullptr;

};

#endif // ACTIONVALUEPROVIDEREDITOR_H
