#ifndef FACEBOOKCONTACTCARD_H
#define FACEBOOKCONTACTCARD_H

#include <plexy.h>
#include <desktopwidget.h>
#include <datasource.h>

class FacebookContactCard : public PlexyDesk::DesktopWidget
{
    Q_OBJECT
public:
    FacebookContactCard(const QRectF &rect, QGraphicsObject *parent = 0);
    virtual ~FacebookContactCard();

    void setDataSource(const QString &id, const QString &token, PlexyDesk::DataSource *dataSource);

    Q_INVOKABLE QString facebookId() const;

    Q_INVOKABLE QPixmap userImage() const;

    Q_INVOKABLE QPixmap coverImage() const;

public Q_SLOTS:
    void onDataUpdated(QVariantMap);
    void onStatusUpdated(QVariantMap);
    void onCoverReady();
    void onAvatarReady();

    void onWallPostClicked();
    void feedClicked();
    void onMessageRestested();
    void onWidgetClosed(PlexyDesk::AbstractDesktopWidget*);
    void onFeedClosed(PlexyDesk::AbstractDesktopWidget*);

private:
    virtual void paintFrontView(QPainter *painter, const QRectF &rect);
    virtual void paintDockView(QPainter *painter, const QRectF &rect);

private:
    void requestStatusMessage();

    class PrivateFacebookContactCard;
    PrivateFacebookContactCard *const d;


};

#endif // FACEBOOKCONTACTCARD_H
