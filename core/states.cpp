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
    return "0.3.7";
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
    this->machine = shared_ptr<Fsm>(new Fsm());
    this->statesUi = unique_ptr<StatesUi>(new StatesUi(this->machine));

    connect(this->statesUi.get(), &StatesUi::newFsmRequestEvent,       this, &StateS::generateNewFsm);
    connect(this->statesUi.get(), &StatesUi::clearMachineRequestEvent, this, &StateS::clearMachine);
    connect(this->statesUi.get(), &StatesUi::loadMachineRequestEvent,  this, &StateS::loadMachine);
    connect(this->statesUi.get(), &StatesUi::getCurrentFileEvent,      this, &StateS::getCurrentFile);
    connect(this->statesUi.get(), &StatesUi::machineSavedEvent,        this, &StateS::updateSaveFilePath);

    statesUi->show();
}

void StateS::generateNewFsm()
{
    this->machine = shared_ptr<Fsm>(new Fsm());
    this->statesUi->setMachine(this->machine);
    this->currentFile.clear();
}

void StateS::clearMachine()
{
    this->statesUi->setMachine(nullptr);
    this->machine.reset();
    this->currentFile.clear();
}

void StateS::loadMachine(const QString& path)
{
    this->generateNewFsm();
    this->currentFile = path;
    dynamic_pointer_cast<Fsm>(this->machine)->loadFromFile(path);
}

void StateS::updateSaveFilePath(const QString &path)
{
    this->currentFile = path;
}

QString StateS::getCurrentFile()
{
    return this->currentFile;
}
