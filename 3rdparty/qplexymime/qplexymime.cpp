/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Dariusz Mikulski <dariusz.mikulski@gmail.com>
*  Date: 02.05.2010
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

#include <QtDebug>
#include <QFile>
#include <QtConcurrentRun>
#include <QDir>

#include <plexyconfig.h>
#include "qplexymime.h"


QPlexyMime::QPlexyMime(QObject *parent)
    : QObject(parent)
{
    QString xmlPath;

#if  defined (Q_WS_MAC)  || defined (Q_WS_WIN)
    xmlPath = QDir::toNativeSeparators(PlexyDesk::Config::getInstance()->plexydeskBasePath() +
                "/share/plexy/mime/freedesktop.org.xml");
#else
    xmlPath = "/usr/share/mime/packages/freedesktop.org.xml";
#endif

    QFile sourceDocument(xmlPath);
    sourceDocument.open(QIODevice::ReadOnly);

    mInternalQuery = QString("declare namespace ns = 'http://www.freedesktop.org/standards/shared-mime-info';\ndeclare variable $internalFile external;\n");

    mInternalOutput = sourceDocument.readAll();
    sourceDocument.close();

#ifdef QT_NO_CONCURRENT
    mMutex = new QMutex(QMutex::NonRecursive);
#endif

    qRegisterMetaType<MimePairType>("MimePairType");
    qRegisterMetaType<MimeWithLangType>("MimeWithLangType");
    qRegisterMetaType<MimeWithListType>("MimeWithListType");

    mFileNameMimeCache = new QCache<QString, QString>(100);
    mGenericExtMimeCache = new QCache<QString, QString>(100);
    mGenericMimeIconNameCache = new QCache<QString, QString>(100);
    mExpandedAcronymMimeAcronymCache = new QCache<QString, QString>(100);
    mDescriptionCacheWithLang = new QCache<MimePairType, QString>(100);
    mDescriptionCache = new QCache<QString, QStringList>(100);
    mSubclassOfMimeCache = new QCache<QString, QString>(100);
    mAcronymMimeCache = new QCache<QString, QString>(100);
    mAliasMimeCache = new QCache<QString, QString>(100);
}

QPlexyMime::~QPlexyMime()
{
#ifdef QT_NO_CONCURRENT
    delete mMutex;
#endif
    delete mFileNameMimeCache;
    delete mGenericExtMimeCache;
    delete mGenericMimeIconNameCache;
    delete mExpandedAcronymMimeAcronymCache;
    delete mDescriptionCacheWithLang;
    delete mDescriptionCache;
    delete mSubclassOfMimeCache;
    delete mAcronymMimeCache;
    delete mAliasMimeCache;
}

QString QPlexyMime::evaluate(const QString &tmpQuery)
{
    QBuffer outputBuffer;
    QByteArray output = getInternalOutput();
    outputBuffer.setBuffer(&output);
    if(!outputBuffer.open(QIODevice::ReadOnly))
    {
        return QString();
    }

    QXmlQuery query;
    query.bindVariable("internalFile", &outputBuffer);

    QString tmp = getInternalQuery() + tmpQuery;
    query.setQuery(tmp);

    if(!query.isValid())
    {
        qDebug() << "Invalid query!" << tmpQuery;
        outputBuffer.close();
        return QString();
    }

    QString result;
    query.evaluateTo(&result);
    outputBuffer.close();

    return result.simplified();
}

QStringList QPlexyMime::evaluateToList(const QString &tmpQuery)
{
    QBuffer outputBuffer;
    QByteArray output = getInternalOutput();
    outputBuffer.setBuffer(&output);
    if(!outputBuffer.open(QIODevice::ReadOnly))
    {
        return QStringList();
    }

    QXmlQuery query;
    query.bindVariable("internalFile", &outputBuffer);

    QString tmp = getInternalQuery() + tmpQuery;
    query.setQuery(tmp);

    if(!query.isValid())
    {
        qDebug() << "Invalid query!" << tmpQuery;
        outputBuffer.close();
        return QStringList();
    }

    QStringList result;
    query.evaluateTo(&result);
    outputBuffer.close();

    return result;
}

bool QPlexyMime::getGenericMime(const QString &mimeType, QFileInfo *fileInfo)
{
    if (mimeType.isEmpty())
    {
        Q_EMIT cannotFound("Mime type is empty.", mimeType);
        return false;
    }

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    QString *result = new QString;
    if(!mGenericExtMimeCache->contains(mimeType))
    {
        QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type[@type='%1']/ns:glob/@pattern/string()").arg(mimeType);
        *result = evaluate(tmpQuery);
        mGenericExtMimeCache->insert(mimeType, result);
    }
    else
        result = mGenericExtMimeCache->object(mimeType);

#ifdef QT_NO_CONCURRENT
    mMutex->unlock();
#endif

    if(result->isEmpty())
    {
        Q_EMIT cannotFound("Cannot find mime type.", mimeType);
        return false;
    }

    QString extension(*result);
    extension = extension.remove(0, 1);
    QString filename = QString("test") + extension;

    fileInfo->setFile(filename.simplified());

    return true;
}

void QPlexyMime::fromFileName(const QString &fileName)
{
#ifndef QT_NO_CONCURRENT
    QtConcurrent::run(this, &QPlexyMime::internalFromFileName, fileName).waitForFinished();
#else
    internalFromFileName(fileName);
#endif
}

void QPlexyMime::internalFromFileName(const QString &fileName)
{
    QFileInfo fileInfo;
    fileInfo.setFile(fileName);
    QString ext = fileInfo.suffix();

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    QString *result = new QString;
    if(!mFileNameMimeCache->contains(ext))
    {
        QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../@type/string()").arg(ext);

        *result = evaluate(tmpQuery);
        mFileNameMimeCache->insert(ext, result);
    }
    else
        result = mFileNameMimeCache->object(ext);

#ifdef QT_NO_CONCURRENT
    mMutex->unlock();
#endif

    if(result->isEmpty())
    {
        qWarning("Cannot find mime for specified filename.");
        Q_EMIT cannotFound("Cannot find mime for specified filename.", fileName);
        return;
    }

    MimePairType pairMime(fileName, result->simplified());

    Q_EMIT fromFileNameMime(pairMime);

    return;
}

void QPlexyMime::genericIconName(const QString &mimeType)
{
#ifndef QT_NO_CONCURRENT
    QtConcurrent::run(this, &QPlexyMime::internalGenericIconName, mimeType).waitForFinished();
#else
    internalGenericIconName(mimeType);
#endif
}

void QPlexyMime::internalGenericIconName(const QString &mimeType)
{
    QFileInfo fileInfo;

    if(!getGenericMime(mimeType, &fileInfo))
    {
        return;
    }

    QString ext = fileInfo.suffix();

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    QString *result = new QString;
    if(!mGenericMimeIconNameCache->contains(mimeType))
    {
        QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../ns:generic-icon/@name/string()").arg(ext);
        *result = evaluate(tmpQuery);
        mGenericMimeIconNameCache->insert(mimeType, result);
    }
    else
        result = mGenericMimeIconNameCache->object(mimeType);

#ifdef QT_NO_CONCURRENT
    mMutex->unlock();
#endif

    if(result->isEmpty())
    {
        Q_EMIT cannotFound("Cannot find icon name for specified mime.", mimeType);
        return;
    }

    MimePairType pairMime(mimeType, result->simplified());

    Q_EMIT genericIconNameMime(pairMime);
}

void QPlexyMime::expandedAcronym(const QString &mimeType)
{
#ifndef QT_NO_CONCURRENT
    QtConcurrent::run(this, &QPlexyMime::internalExpandedAcronym, mimeType).waitForFinished();
#else
    internalExpandedAcronym(mimeType);
#endif
}

void QPlexyMime::internalExpandedAcronym(const QString &mimeType)
{
    QFileInfo fileInfo;

    if(!getGenericMime(mimeType, &fileInfo))
    {
        return;
    }

    QString ext = fileInfo.suffix();

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    QString *result = new QString;
    if(!mExpandedAcronymMimeAcronymCache->contains(mimeType))
    {
        QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../ns:expanded-acronym/string()").arg(ext);
        *result = evaluate(tmpQuery);
        mExpandedAcronymMimeAcronymCache->insert(mimeType, result);
    }
    else
        result = mExpandedAcronymMimeAcronymCache->object(mimeType);

#ifdef QT_NO_CONCURRENT
    mMutex->unlock();
#endif

    if(result->isEmpty())
    {
        Q_EMIT cannotFound("Cannot find expanded acronym for specified mime.", mimeType);
        return;
    }

    MimePairType pairMime(mimeType, result->simplified());

    Q_EMIT expandedAcronymMime(pairMime);
}

void QPlexyMime::description(const QString &mimeType, const QString &lang)
{
#ifndef QT_NO_CONCURRENT
    QtConcurrent::run(this, &QPlexyMime::internalDescription, mimeType, lang).waitForFinished();
#else
    internalDescription(mimeType, lang);
#endif
}

void QPlexyMime::internalDescription(const QString &mimeType, const QString &lang)
{
    QFileInfo fileInfo;

    if(!getGenericMime(mimeType, &fileInfo))
    {
        return;
    }

    QString ext = fileInfo.suffix();

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    if(lang.isEmpty())
    {
        QStringList *result = new QStringList;

        if(!mDescriptionCache->contains(mimeType))
        {
            QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../ns:comment/string()").arg(ext);
            *result = evaluateToList(tmpQuery);
            mDescriptionCache->insert(mimeType, result);
        }
        else
            result = mDescriptionCache->object(mimeType);

#ifdef QT_NO_CONCURRENT
        mMutex->unlock();
#endif

        if(result->isEmpty())
        {
            Q_EMIT cannotFound("Cannot find description for specified mime type.", qMakePair(mimeType, lang));
            return;
        }

        MimeWithListType mimeWithList(mimeType, *result);

        Q_EMIT descriptionWithList(mimeWithList);
    }
    else
    {
        QString *result = new QString;

        if(!mDescriptionCacheWithLang->contains(qMakePair(mimeType, lang)))
        {
            QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../ns:comment[@xml:lang='%2']/string()").arg(ext).arg(lang);
            *result = evaluate(tmpQuery);
            mDescriptionCacheWithLang->insert(qMakePair(mimeType, lang), result);
        }
        else
            result = mDescriptionCacheWithLang->object(qMakePair(mimeType, lang));

#ifdef QT_NO_CONCURRENT
        mMutex->unlock();
#endif

        if(result->isEmpty())
        {
            Q_EMIT cannotFound("Cannot find description for specified mime type and language.", mimeType);
            return;
        }

        MimePairType pairMime(mimeType, lang);
        MimeWithLangType hashMimeWithLang(pairMime, result->simplified());

        Q_EMIT descriptionWithLang(hashMimeWithLang);
    }
}

void QPlexyMime::subclassOfMime(const QString &mimeType)
{
#ifndef QT_NO_CONCURRENT
    QtConcurrent::run(this, &QPlexyMime::internalSubclassOfMime, mimeType).waitForFinished();
#else
    internalSubclassOfMime(mimeType);
#endif
}

void QPlexyMime::internalSubclassOfMime(const QString &mimeType)
{
    QFileInfo fileInfo;

    if(!getGenericMime(mimeType, &fileInfo))
    {
        return;
    }

    QString ext = fileInfo.suffix();

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    QString *result = new QString;
    if(!mSubclassOfMimeCache->contains(mimeType))
    {
        QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../ns:sub-class-of/@type/string()").arg(ext);
        *result = evaluate(tmpQuery);
        mSubclassOfMimeCache->insert(mimeType, result);
    }
    else
        result = mSubclassOfMimeCache->object(mimeType);

#ifdef QT_NO_CONCURRENT
    mMutex->unlock();
#endif

    if(result->isEmpty())
    {
        Q_EMIT cannotFound("Cannot find subclass of mime for specified mime.", mimeType);
        return;
    }

    MimePairType pairMime(mimeType, result->simplified());

    Q_EMIT subclassMime(pairMime);
}

void QPlexyMime::acronym(const QString &mimeType)
{
#ifndef QT_NO_CONCURRENT
    QtConcurrent::run(this, &QPlexyMime::internalAcronym, mimeType).waitForFinished();
#else
    internalAcronym(mimeType);
#endif
}

void QPlexyMime::internalAcronym(const QString &mimeType)
{
    QFileInfo fileInfo;

    if(!getGenericMime(mimeType, &fileInfo))
    {
        return;
    }

    QString ext = fileInfo.suffix();

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    QString *result = new QString;
    if(!mAcronymMimeCache->contains(mimeType))
    {
        QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../ns:acronym/string()").arg(ext);
        *result = evaluate(tmpQuery);
        mAcronymMimeCache->insert(mimeType, result);
    }
    else
        result = mAcronymMimeCache->object(mimeType);

#ifdef QT_NO_CONCURRENT
    mMutex->unlock();
#endif

    if(result->isEmpty())
    {
        Q_EMIT cannotFound("Cannot find acronym for specified mime.", mimeType);
        return;
    }

    MimePairType pairMime(mimeType, result->simplified());

    Q_EMIT acronymMime(pairMime);
}

void QPlexyMime::alias(const QString &mimeType)
{
#ifndef QT_NO_CONCURRENT
    QtConcurrent::run(this, &QPlexyMime::internalAlias, mimeType).waitForFinished();
#else
    internalAlias(mimeType);
#endif
}

void QPlexyMime::internalAlias(const QString &mimeType)
{
    QFileInfo fileInfo;

    if(!getGenericMime(mimeType, &fileInfo))
    {
        return;
    }

    QString ext = fileInfo.suffix();

#ifdef QT_NO_CONCURRENT
    mMutex->lock();
#endif

    QString *result = new QString;
    if(!mAliasMimeCache->contains(mimeType))
    {
        QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../ns:alias/@type/string()").arg(ext);
        *result = evaluate(tmpQuery);
        mAliasMimeCache->insert(mimeType, result);
    }
    else
        result = mAliasMimeCache->object(mimeType);

#ifdef QT_NO_CONCURRENT
    mMutex->unlock();
#endif

    if(result->isEmpty())
    {
        Q_EMIT cannotFound("Cannot find alias for specified mime.", mimeType);
        return;
    }

    MimePairType pairMime(mimeType, result->simplified());

    Q_EMIT aliasMime(pairMime);
}

/*
   // TODO: rewrite
   QString QPlexyMime::mimeType (void)
   {
    QString ext = fileInfo.suffix();
    QString tmpQuery = QString("doc($internalFile)/ns:mime-info/ns:mime-type/ns:glob[@pattern='*.%1']/../@type/string()").arg(ext);

    setQuery(tmpQuery);

    QString result;
    //result = d->evaluate();

    return result.simplified();
   }

   QString QPlexyMime::fromFile (const QString& fileName)
   {
        QFileInfo fileInfo(fileName);
    if (fileInfo.isDir())
        {
                return("inode/directory");
        }

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
                return(QString());

        QString mimeType = fromFile(&file);

    file.close();

    return(mimeType);
   }

   QString QPlexyMime::fromFile (QFile *file)
   {
        return QString();
   }
 */
