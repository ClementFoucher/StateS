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

#ifndef VERIFIERTAB_H
#define VERIFIERTAB_H

// Parent
#include <QWidget>

// C++ classes
#include <memory>
using namespace std;

// Qt classes
class QListWidget;
class QLabel;
class QListWidgetItem;
class QPushButton;

// StateS classes
#include "fsmverifier.h"
class TruthTableDisplay;
class CollapsibleWidgetWithTitle;


class VerifierTab : public QWidget
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit VerifierTab(QWidget* parent = nullptr);

	/////
	// Object functions
private slots:
	void checkNow();
	void clearDisplay();
	void setCheckVhdl(bool doCheck);

	void proofRequested(QListWidgetItem* item);

	/////
	// Object variables
private:
	unique_ptr<FsmVerifier> verifier;

	bool checkVhdl = false;

	QLabel*                     listTitle         = nullptr;
	QListWidget*                list              = nullptr;
	QPushButton*                buttonClear       = nullptr;
	TruthTableDisplay*          truthTableDisplay = nullptr;
	CollapsibleWidgetWithTitle* hintBox           = nullptr;

};

#endif // VERIFIERTAB_H
