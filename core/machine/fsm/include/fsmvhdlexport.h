/*
 * Copyright © 2014-2015 Clément Foucher
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

#ifndef FSMVHDLEXPORT_H
#define FSMVHDLEXPORT_H

// Qt classes
#include <QString>

// StateS classes
class Fsm;
class Equation;
class Signal;


class FsmVhdlExport
{
public:
    static void exportFSM(Fsm* machine, QString path, bool resetLogicPositive, bool prefixIOs);

private:
    static QString correctName(QString name);
    static QString equationText(Signal *equation, Fsm *fsm, bool prefixIOs);
};

#endif // FSMVHDLEXPORT_H
