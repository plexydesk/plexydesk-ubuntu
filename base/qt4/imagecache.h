/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@kde.org>
*  Authored By  : Siraj Razick <siraj@kde.org>
*                 PhobosK <phobosk@kbfx.net>
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

#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include <QtCore>
#include <QtGui>

#include <plexy.h>
#include <plexyconfig.h>

#include "plexydeskuicore_global.h"

namespace PlexyDesk
{
class PLEXYDESKUICORE_EXPORT ImageCache : public QObject
{
    Q_OBJECT

public:
    ImageCache();
    virtual ~ImageCache();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
    bool isCached(QString &filename)  const;
    void addToCached(QString &imgfile, QString &filename, QString &themename);

    QPixmap get(const QString &name);

protected:
    void load(const QString &themename);
    void clear();
    bool drawSvg(QPainter *painter,
    QRectF rect, const QString &str, const QString &elementId);

Q_SIGNALS:
    void ready();

private:
    class Private;
    Private *const d;
};
}

#endif
