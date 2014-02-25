/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@kde.org>
*  Authored By  :
*
*  PlexyDesk is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  PlexyDesk is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with PlexyDesk. If not, see <http://www.gnu.org/licenses/lgpl.html>
*******************************************************************************/
#ifndef TIMER_DATA_I
#define TIMER_DATA_I

#include <QtCore>
#include <plexy.h>
#include <dataplugininterface.h>
#include <QtNetwork>

class TimerInterface : public QObject, public PlexyDesk::DataPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PlexyDesk::DataPluginInterface)

public :
    virtual ~TimerInterface() {}

    /* this will return a valid data plugin pointer*/
    QSharedPointer<PlexyDesk::DataSource> model();

};

#endif
