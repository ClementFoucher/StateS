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

#ifndef CELLWITHEDITANDCLEARBUTTONSEDITOR_H
#define CELLWITHEDITANDCLEARBUTTONSEDITOR_H

// Parent
#include <QWidget>

// Qt
class QHBoxLayout;
class QPushButton;


class CellWithEditAndClearButtonsEditor : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit CellWithEditAndClearButtonsEditor(QWidget* parent = nullptr);

	/////
	// Object functions
protected:
	void resizeEvent(QResizeEvent* event) override;

	void setMainWidget(QWidget* mainWidget);
	void updateClearButtonVisibility();

private slots:
	virtual void editButtonClickedEventHandler()  = 0;
	virtual void clearButtonClickedEventHandler() = 0;

private:
	virtual bool shouldClearButtonBeVisibile() = 0;

	/////
	// Signals
signals:
	void contentChangedEvent();

	/////
	// Object variables
private:
	QHBoxLayout* mainLayout = nullptr;

	QWidget*     mainWidget  = nullptr;
	QPushButton* buttonEdit  = nullptr;
	QPushButton* buttonClear = nullptr;

};

#endif // CELLWITHEDITANDCLEARBUTTONSEDITOR_H
