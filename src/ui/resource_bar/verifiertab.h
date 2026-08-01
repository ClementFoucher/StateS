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

#ifndef VERIFIERTAB_H
#define VERIFIERTAB_H

// Parent
#include <QWidget>

// Stdlib
#include <memory>
using namespace std;

// Qt
class QListWidgetItem;
class QVBoxLayout;
class QCheckBox;
class QLabel;
class QListWidget;
class QPushButton;

// StateS
class FsmVerifier;
class TruthTableDisplay;
class HintWidget;


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
	void beginCheck();
	void cancelCheck();
	void checkFinished();

	void resetContent();

	void timeout();

	void proofRequested(QListWidgetItem* item);

	/////
	// Object variables
private:
	unique_ptr<FsmVerifier> verifier;
	QTimer* timer = nullptr;

	QVBoxLayout* mainLayout = nullptr;

	// Permanent widgets
	QCheckBox*   checkVhdlExport = nullptr;
	QPushButton* buttonVerify    = nullptr;

	// Verification in progress widgets
	QLabel*      verificationRunningLabel = nullptr;
	QPushButton* buttonCancel             = nullptr;
	QLabel*      timingWarning            = nullptr;

	// Verification results widgets
	QPushButton*       buttonClear       = nullptr;
	QLabel*            listTitle         = nullptr;
	QListWidget*       list              = nullptr;
	TruthTableDisplay* truthTableDisplay = nullptr;
	HintWidget*        hintBox           = nullptr;

};

#endif // VERIFIERTAB_H
