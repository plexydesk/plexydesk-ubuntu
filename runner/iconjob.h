#ifndef ICONJOB_H
#define ICONJOB_H

#include <pendingjob.h>
#include <QPixmap>
#include <QSharedPointer>

class IconProvider;

namespace PlexyDesk
{
class IconJob : public PendingJob
{
    Q_OBJECT
public:
    IconJob(QObject *parent);
    virtual ~IconJob();
    void requestIcon(const QString &name, const QString &size);
    QStringList getSubDir(const QString &path);
    QPixmap iconPixmap() const;
Q_SIGNALS:
    void newJob();
public Q_SLOTS:
    void handleJob();
private:
    class Private;
    Private *const d;
    friend class IconProvider;
};

typedef QSharedPointer<IconJob> IconJobPtr;
}
#endif // ICONJOB_H
