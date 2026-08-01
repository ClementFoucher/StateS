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

#ifndef FSMVERIFIER_H
#define FSMVERIFIER_H

// Parent
#include <QThread>

// Stdlib
#include <memory>
using namespace std;

// Qt
#include <QList>

// StateS
#include "statestypes.h"
class TruthTable;


class FsmVerifier : public QThread
{
	Q_OBJECT

	/////
	// Type declarations
public:

	class Issue
	{
	public:
		QString text;
		VerifierSeverityLevel_t type = VerifierSeverityLevel_t::hint;
		shared_ptr<TruthTable> proof;
		QList<int> proofsHighlight;
	};

	/////
	// Constructors/destructors
public:
	explicit FsmVerifier() = default;

	void setCheckVhdl();

	void abort();

	/////
	// Object functions
public:
	const QList<shared_ptr<Issue>>& getIssues();

protected:
	virtual void run() override;

	/////
	// Signals
signals:
	void verificationOver();

	/////
	// Object variables
private:
	bool checkVhdl = false;
	bool doAbort = false;

	QList<shared_ptr<Issue>> issues;

};

#endif // FSMVERIFIER_H
