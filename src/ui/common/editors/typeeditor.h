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

#ifndef TYPEEDITOR_H
#define TYPEEDITOR_H

// Parent
#include <QWidget>

// Qt
class QLabel;

// StateS
#include "machinevalue.h"
class SelfManagedDynamicLineEditor;
class DiscreetComboBox;


class TypeEditor : public QWidget
{
	Q_OBJECT

	/////
	// Type declarations
private:
	enum class indexType : int
	{
		boolean   = 0,
		bitVector = 1
	};

	/////
	// Constructors/destructors
public:
	explicit TypeEditor(QWidget* parent = nullptr);
	virtual ~TypeEditor();

	/////
	// Object functions
public:
	void setCurrentType(MachineValue::Type_t type);
	MachineValue::Type_t getCurrentType() const;

	void setBitVectorSize(uint size);
	uint getBitVectorSize() const;

	void triggerEditBitVectorSize();
	void setErroneous(bool erroneous);
	bool getIsErroneous() const;

	void setIgnoreWheelEvents(bool ignoreWheelEvents);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;

	virtual bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
	void selectedTypeChangedEventHandler(int);
	void sizeChangedEventHandler(QString);
	void sizeEditCanceledEventHandler();

	/////
	// Signals
signals:
	void typeChangedEvent(TypeEditor* me);

	/////
	// Object variables
private:
	uint bitVectorSize = 2; // Default to 2 if no value provided

	DiscreetComboBox* typeComboBox = nullptr;

	// Bit vector size related widgets
	SelfManagedDynamicLineEditor* sizeLineEdit   = nullptr;
	QLabel*                       sizeLabel      = nullptr;
	QLabel*                       openingBracket = nullptr;
	QLabel*                       closingBracket = nullptr;

};

#endif // TYPEEDITOR_H
