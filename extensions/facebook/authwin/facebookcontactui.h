#ifndef FACEBOOKCONTACTUI_H
#define FACEBOOKCONTACTUI_H

#include <plexy.h>
#include <desktopwidget.h>

#include "facebookcontactcard.h"

class FacebookContactUI : public PlexyDesk::DesktopWidget
{
    Q_OBJECT
public:
    explicit FacebookContactUI(const QRectF &rect);
    virtual ~FacebookContactUI();

    void setFacebookContactData(QHash<QString, QVariant> data);

    void addContact(const QVariantMap &data);

Q_SIGNALS:
    void addContactCard(QString);

public Q_SLOTS:
    void onViewClicked(QString);

private:
        virtual void paintFrontView(QPainter *painter, const QRectF &rect);

private:
    class PrivateFacebookContactUI;
    PrivateFacebookContactUI *const d;
};

#endif // FACEBOOKCONTACTUI_H
