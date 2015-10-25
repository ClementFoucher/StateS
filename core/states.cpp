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

// Current class header
#include "states.h"

// Qt classes
#include <QPixmap>
#include <QPainter>
#include <QSvgRenderer>

// StateS classes
#include "statesui.h"
#include "fsm.h"

QPixmap StateS::getPixmapFromSvg(const QString& path)
{
    QSvgRenderer svgRenderer(path);
    QPixmap pixmap(svgRenderer.defaultSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    svgRenderer.render(&painter);

    return pixmap;
}

QString StateS::getVersion()
{
    return "0.3.8";
}

StateS::StateS()
{
}

StateS::~StateS()
{
    // Clear references to machine in UI before deletion
    if (this->statesUi != nullptr)
        statesUi->setMachine(nullptr);
}

void StateS::run()
{
    // Create interface
    this->statesUi = unique_ptr<StatesUi>(new StatesUi());

    connect(this->statesUi.get(), &StatesUi::newFsmRequestEvent,                  this, &StateS::generateNewFsm);
    connect(this->statesUi.get(), &StatesUi::clearMachineRequestEvent,            this, &StateS::clearMachine);
    connect(this->statesUi.get(), &StatesUi::loadMachineRequestEvent,             this, &StateS::loadMachine);
    connect(this->statesUi.get(), &StatesUi::saveMachineRequestEvent,             this, &StateS::saveCurrentMachine);
    connect(this->statesUi.get(), &StatesUi::saveMachineInCurrentFileRequestEvent,this, &StateS::saveCurrentMachineInCurrentFile);

    // Generate initial machine
    this->machine  = shared_ptr<Fsm>(new Fsm());
    this->statesUi->setMachine(this->machine);

    // Display interface
    statesUi->show();
}

/*
 * Replace existing machine with a newly created FSM.
 * This is the 'New' action.
 */
void StateS::generateNewFsm()
{
    this->clearMachine();

    this->machine = shared_ptr<Fsm>(new Fsm());
    this->statesUi->setMachine(this->machine);
}

/*
 * Just delete current machine.
 * This is the 'Close' action.
 */
void StateS::clearMachine()
{
    this->statesUi->setMachine(nullptr);
    this->machine.reset();
    this->currentFilePath = QString::null;
}

/*
 * Load a machine from a saved file.
 * This is the 'load' action.
 */
void StateS::loadMachine(const QString& path)
{
    // TODO: check for path correctness

    this->clearMachine();

    this->currentFilePath = path;

    shared_ptr<Fsm> fsm = shared_ptr<Fsm>(new Fsm());
    if (this->currentFilePath != QString::null)
        fsm->loadFromFile(this->currentFilePath);

    this->machine = fsm;

    this->statesUi->setMachine(this->machine, this->currentFilePath);
}

/*
 * Save current machine to file.
 * This is the 'save as' action.
 */
void StateS::saveCurrentMachine(const QString& path)
{
    this->currentFilePath = path;

    this->saveCurrentMachineInCurrentFile();

    // TODO: check for path correctness
    this->statesUi->setCurrentFilePath(this->currentFilePath);
}

/*
 * Save current machine to currently registered save file.
 * This is the 'save' action.
 */
void StateS::saveCurrentMachineInCurrentFile()
{
    // TODO: check for path correctness
    if (this->currentFilePath != QString::null)
        this->machine->saveMachine(this->currentFilePath);
}
