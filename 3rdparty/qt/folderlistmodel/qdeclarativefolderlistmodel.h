/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEFOLDERLISTMODEL_H
#define QDECLARATIVEFOLDERLISTMODEL_H

#include <qdeclarative.h>
#include <QStringList>
#include <QUrl>
#include <QAbstractListModel>
#include <QDeclarativeImageProvider>

#include <imagecache.h>

#ifndef QT_NO_DIRMODEL

#define FOLDERLISTMODEL_EXPORT Q_DECL_EXPORT

QT_MODULE(Declarative)

class QDeclarativeContext;
class QModelIndex;

class QDeclarativeFolderListModelPrivate;

class FOLDERLISTMODEL_EXPORT QDeclarativeFolderListModel : public QAbstractListModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)

    Q_PROPERTY(QUrl folder READ folder WRITE setFolder NOTIFY folderChanged)
    Q_PROPERTY(QUrl parentFolder READ parentFolder NOTIFY folderChanged)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    Q_PROPERTY(SortField sortField READ sortField WRITE setSortField)
    Q_PROPERTY(bool sortReversed READ sortReversed WRITE setSortReversed)
    Q_PROPERTY(bool showDirs READ showDirs WRITE setShowDirs)
    Q_PROPERTY(bool showDotAndDotDot READ showDotAndDotDot WRITE setShowDotAndDotDot)
    Q_PROPERTY(bool showOnlyReadable READ showOnlyReadable WRITE setShowOnlyReadable)
    Q_PROPERTY(int count READ count)

public:
    QDeclarativeFolderListModel(QObject *parent = 0);
    ~QDeclarativeFolderListModel();

    enum Roles { FileIconRole = Qt::DecorationRole,
                 FileNameRole = Qt::UserRole+1,
                 FilePathRole = Qt::UserRole+2,
                 FileMimeIconRole = Qt::UserRole+3,
                 FileTypeIconRole = Qt::UserRole+4
                 };

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    int count() const { return rowCount(QModelIndex()); }


    QUrl folder() const;
    void setFolder(const QUrl &folder);

    QUrl parentFolder() const;

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &filters);

    enum SortField { Unsorted, Name, Time, Size, Type };
    SortField sortField() const;
    void setSortField(SortField field);
    Q_ENUMS(SortField)

    bool sortReversed() const;
    void setSortReversed(bool rev);

    bool showDirs() const;
    void  setShowDirs(bool);
    bool showDotAndDotDot() const;
    void  setShowDotAndDotDot(bool);
    bool showOnlyReadable() const;
    void  setShowOnlyReadable(bool);

    Q_INVOKABLE bool isFolder(int index) const;
    Q_INVOKABLE void SetToHome();
    Q_INVOKABLE bool openFile(QString file);

    virtual void classBegin();
    virtual void componentComplete();

Q_SIGNALS:
    void folderChanged();

private Q_SLOTS:
    void refresh();
    void inserted(const QModelIndex &index, int start, int end);
    void removed(const QModelIndex &index, int start, int end);
    void handleDataChanged(const QModelIndex &start, const QModelIndex &end);

private:
    Q_DISABLE_COPY(QDeclarativeFolderListModel)
    QDeclarativeFolderListModelPrivate *d;
    PlexyDesk::ImageCache *mImageCache ;
    QVariant getMimeTypeImagePath(const QString file) const;
    QVariant getFileTypeImagePath(const QString file) const;
};

QML_DECLARE_TYPE(QDeclarativeFolderListModel)

#endif // QT_NO_DIRMODEL

#endif // QDECLARATIVEFOLDERLISTMODEL_H
