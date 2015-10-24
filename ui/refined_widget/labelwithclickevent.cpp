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
#include "labelwithclickevent.h"

// Qt classes
#include <QMouseEvent>


LabelWithClickEvent::LabelWithClickEvent(const QString &text) :
    QLabel(text)
{

}

/*
 * Normal QLabel intercepts mouseReleaseEvent (why?)
 * This allows to redirect all mouse events to a specific signal.
 * TODO: Should we still transmit event to parent when redirecting signal?
 *
 */
bool LabelWithClickEvent::event(QEvent *e)
{
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(e);

    if (mouseEvent != nullptr)
    {
        return clicked(mouseEvent);
    }
    else
        return QLabel::event(e);
}
