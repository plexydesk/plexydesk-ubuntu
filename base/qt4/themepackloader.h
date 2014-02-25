#ifndef THEMEPACKLOADER_H
#define THEMEPACKLOADER_H

#include <QObject>
#include <QSettings>
#include <QPoint>
#include <abstractdesktopwidget.h>
#include <abstractdesktopview.h>

#include "plexydeskuicore_global.h"

namespace PlexyDesk
{

class PLEXYDESKUICORE_EXPORT ThemepackLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString themeName READ QString WRITE setThemeName)

public:
    explicit ThemepackLoader(const QString &themeName, QObject *parent = 0);

    virtual ~ThemepackLoader();

    void setThemeName(const QString &name);

    QStringList desktopWidgets() const;

    QString desktopBackgroundController() const;

    bool queryMultiScreen(const QString &name);

    QRectF positionForWidget(const QString &name, const QRectF &screen_rect);

    QRectF positionForBackground(const QString &name, const QRectF &screen_rect);

    QStringList widgets(const QString &type);

    QPoint widgetPos(const QString &name);

    QString hiddenQmlWidgets(const QString &name);

    QString wallpaper();

    QString qmlFilesFromTheme(const QString &themeName);

    QString qmlBackdropFromTheme();

    QVariant getProperty(const QString &themeName, const QString &prop);

    PlexyDesk::AbstractDesktopWidget::State widgetView(const QString &name);

    QString loadSessionFromDisk() const;

    void saveSessionToDisk(const QString &data);

    void setRect (const QRectF &rect);

Q_SIGNALS:

    void ready();

private:
    void scanThemepackPrefix();

    int toScreenValue(const QString &val, int max_distance);

    class ThemepackLoaderPrivate;

    ThemepackLoaderPrivate *const d;
};

} // namespace PlexyDesk

#endif // THEMEPACKLOADER_H
