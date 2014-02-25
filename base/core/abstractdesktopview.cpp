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

#include <config.h>

#include <QDir>
#include <QtDebug>
#include <QGLWidget>
#include <QFutureWatcher>
#include <QSharedPointer>
#include <QPropertyAnimation>
#include <QGraphicsGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QDomDocument>

#include <abstractdesktopwidget.h>
#include <controllerinterface.h>
#include <pluginloader.h>

#include "controllerinterface.h"
#include "abstractdesktopview.h"

/**
  \class PlexyDesk::AbstractDesktopView

  \brief Base class for creating DesktopViews

  \fn PlexyDesk::AbstractDesktopView::enableOpenGL()

  \param state Method to enable OpenGL rendering on the view, internally
               it assigns a QGLWidget to the viewport.

  \fn PlexyDesk::AbstractDesktopView::showLayer()

   \fn  PlexyDesk::AbstractDesktopView::addExtension()
   \brief Adds an Widget Extension to Plexy Desktop, give the widget
   name in string i.e "clock" or "radio", the internals will
   take care of the loading the widget if a plugin matching the name is
   found.

   \param name String name of the widget as specified by the desktop file

   \param layerName Name of the layer you want add the widget to

**/
namespace PlexyDesk
{

class AbstractDesktopView::PrivateAbstractDesktopView
{
public:
    PrivateAbstractDesktopView() {}
    ~PrivateAbstractDesktopView()
    {
        if (mSessionTree)
            delete mSessionTree;

        mControllerMap.clear();

        if (mDesktopWidget)
            delete mDesktopWidget;
    }

    QMap<QString, ControllerPtr > mControllerMap;
    AbstractDesktopWidget *mBackgroundItem;
    QDomDocument *mSessionTree;
    QDomElement mRootElement;
    QDesktopWidget *mDesktopWidget;
    QString mBackgroundControllerName;
};

AbstractDesktopView::AbstractDesktopView(QGraphicsScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent), d(new PrivateAbstractDesktopView)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    d->mDesktopWidget = new QDesktopWidget();
    d->mBackgroundItem = 0;

    d->mSessionTree = new QDomDocument("Session");
    d->mSessionTree->appendChild(d->mSessionTree->createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));
    d->mRootElement = d->mSessionTree->createElement("session");
    d->mSessionTree->appendChild(d->mRootElement);

    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setFrameStyle(QFrame::NoFrame);
    scene->setStickyFocus(false);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setFocusPolicy(Qt::StrongFocus);
    if(viewport()) {
        viewport()->setFocusPolicy(Qt::StrongFocus);
    }
}

AbstractDesktopView::~AbstractDesktopView()
{
    delete d;
}

void AbstractDesktopView::enableOpenGL(bool state)
{
    if (state) {
        setViewport(new QGLWidget(QGLFormat(
                        QGL::DoubleBuffer )));
        setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    } else {
        setViewport(new QWidget);
        setCacheMode(QGraphicsView::CacheNone);
        setOptimizationFlags(QGraphicsView::DontSavePainterState);
        setOptimizationFlag(QGraphicsView::DontClipPainter);
        setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    }
}

bool AbstractDesktopView::setBackgroundController(const QString &controllerName)
{
    //TODO: error handling
    // delete the current background source before setting a new one

    QSharedPointer<ControllerInterface> controller =
            (PlexyDesk::PluginLoader::getInstance()->controller(controllerName));

    d->mControllerMap[controllerName] = controller;

    if (!controller.data()) {
        qWarning() << Q_FUNC_INFO << "Error loading extension" << controllerName;
        return false;
    }

    for (int i = 0 ; i < d->mDesktopWidget->screenCount() ; i++) {
        //qDebug() << Q_FUNC_INFO << i;
        d->mBackgroundItem = (AbstractDesktopWidget*) controller->defaultView();

        scene()->addItem(d->mBackgroundItem);

        d->mBackgroundItem->setContentRect(d->mDesktopWidget->screenGeometry(i));

        d->mBackgroundItem->show();
        d->mBackgroundItem->setZValue(-1);

        controller->setViewport(this);
        controller->setControllerName(controllerName);

        if(scene()) {
            scene()->setFocusItem(d->mBackgroundItem, Qt::MouseFocusReason);
        }
    }
    return true;
}

void AbstractDesktopView::addController(const QString &controllerName, bool firstRun)
{
    if (d->mControllerMap.keys().contains(controllerName))
        return;

    QSharedPointer<ControllerInterface> controller =
            (PlexyDesk::PluginLoader::getInstance()->controller(controllerName));

    if (!controller.data()) {
        qWarning() << Q_FUNC_INFO << "Error loading extension" << controllerName;
        return;
    }

    d->mControllerMap[controllerName] = controller;

    AbstractDesktopWidget *defaultView = controller->defaultView();
    QGraphicsItem *viewItem = qobject_cast<QGraphicsItem*>(defaultView);
    if (!viewItem)
        return;

    scene()->addItem(viewItem);

    connect(defaultView, SIGNAL(closed(PlexyDesk::AbstractDesktopWidget*)), this, SLOT(onWidgetClosed(PlexyDesk::AbstractDesktopWidget*)));

    defaultView->show();

    controller->setViewport(this);
    controller->setControllerName(controllerName);

    if (scene()) {
        scene()->setFocusItem(d->mBackgroundItem, Qt::MouseFocusReason);
    }

    qDebug() << Q_FUNC_INFO << firstRun;
    if (!firstRun)
        saveItemStateToSession(controller->controllerName(), defaultView->widgetID(), 0);
}

QStringList AbstractDesktopView::currentControllers() const
{
    QStringList rv = d->mControllerMap.keys();
    rv.removeOne(d->mBackgroundControllerName);

    return rv;
}

ControllerPtr AbstractDesktopView::controller(const QString &name)
{
    return d->mControllerMap[name];
}

void AbstractDesktopView::setControllerRect(const QString &controllerName, const QRectF &rect)
{
    if (d->mControllerMap[controllerName]) {
        d->mControllerMap[controllerName]->setViewRect(rect);
    }

    QDomElement widget = d->mSessionTree->createElement("widget");
    widget.setAttribute("controller", controllerName);
    QDomElement geometry = d->mSessionTree->createElement("geometry");
    geometry.setAttribute("x", rect.x());
    geometry.setAttribute("y", rect.y());
    geometry.setAttribute("width", rect.width());
    geometry.setAttribute("height", rect.height());
    widget.appendChild(geometry);
    d->mRootElement.appendChild(widget);

    Q_EMIT sessionUpdated(d->mSessionTree->toString());
}

QSharedPointer<ControllerInterface> AbstractDesktopView::controllerByName(const QString &name)
{
    return d->mControllerMap[name];
}

void AbstractDesktopView::dropEvent(QDropEvent *event)
{
    if (this->scene()) {
        QList<QGraphicsItem *> items = scene()->items(event->pos());

        Q_FOREACH(QGraphicsItem *item, items) {

            QGraphicsObject *itemObject = item->toGraphicsObject();

            if (!itemObject) {
                continue;
            }

            AbstractDesktopWidget *widget = qobject_cast<AbstractDesktopWidget*> (itemObject);

            if (!widget || !widget->controller())
                continue;

            widget->controller()->handleDropEvent(widget, event);
            return;
        }
    }

    scene()->update(this->sceneRect());
    event->acceptProposedAction();
}

void AbstractDesktopView::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();

}

void AbstractDesktopView::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void AbstractDesktopView::addWidgetToView(AbstractDesktopWidget *widget)
{
    if (!widget)
        return;

    connect(widget, SIGNAL(closed(PlexyDesk::AbstractDesktopWidget*)), this, SLOT(onWidgetClosed(PlexyDesk::AbstractDesktopWidget*)));
    QGraphicsItem *item = (AbstractDesktopWidget*) widget;
    scene()->addItem(item);
    item->setPos(QCursor::pos());
    item->show();

    if (widget->controller())
    saveItemStateToSession(widget->controller()->controllerName(), widget->widgetID(), 0);
}

void AbstractDesktopView::saveItemLocationToSession(const QString &controllerName, const QPointF &pos, const QString &widgetId)
{
    QDomNodeList widgetNodeList = d->mSessionTree->documentElement().elementsByTagName("widget");

    for(int index = 0; index < widgetNodeList.count(); index++) {
        QDomElement widgetElement = widgetNodeList.at(index).toElement();

        if (widgetElement.attribute("controller") == controllerName) {
            QDomNodeList locationElementList = widgetElement.elementsByTagName("location");

            if (locationElementList.count() <= 0 ) {
                QDomElement widget = d->mSessionTree->createElement("location");
                widget.setAttribute("id", widgetId);
                QDomElement geometry = d->mSessionTree->createElement("geometry");
                geometry.setAttribute("x", pos.x());
                geometry.setAttribute("y", pos.y());
                widget.appendChild(geometry);
                widgetElement.appendChild(widget);
                break;
            }

            bool locationTagFound = false;
            for(int index = 0; index < locationElementList.count(); index++) {
                QDomElement locationElement = locationElementList.at(index).toElement();

                if (locationElement.hasChildNodes() && locationElement.attribute("id") == widgetId) {
                    locationTagFound = true;
                    QDomElement geoElement = locationElement.firstChildElement("geometry");

                    if (!geoElement.isNull()) {
                        geoElement.setAttribute("x", pos.x());
                        geoElement.setAttribute("y", pos.y());
                    } else
                        qDebug() << Q_FUNC_INFO << "Error :" << "Missing geometry tag";

                    break;
                }
            }

            // No tag with matching id was found so we add a new location tag to the controller
            if (!locationTagFound) {
                QDomElement widget = d->mSessionTree->createElement("location");
                widget.setAttribute("id", widgetId);
                QDomElement geometry = d->mSessionTree->createElement("geometry");
                geometry.setAttribute("x", pos.x());
                geometry.setAttribute("y", pos.y());
                widget.appendChild(geometry);
                widgetElement.appendChild(widget);
            }

        } else
            continue;

    }

    Q_EMIT sessionUpdated(d->mSessionTree->toString());
}

void AbstractDesktopView::saveItemStateToSession(const QString &controllerName, const QString &widgetId, bool state)
{
    QDomNodeList widgetNodeList = d->mSessionTree->documentElement().elementsByTagName("widget");

    for(int index = 0; index < widgetNodeList.count(); index++) {
        QDomElement widgetElement = widgetNodeList.at(index).toElement();

        if (widgetElement.attribute("controller") == controllerName) {
            QDomNodeList locationElementList = widgetElement.elementsByTagName("state");

            if (locationElementList.count() <= 0 ) {
                QDomElement widget = d->mSessionTree->createElement("state");
                widget.setAttribute("id", widgetId);
                widget.setAttribute("state", state);
                widgetElement.appendChild(widget);
                break;
            }

            bool locationTagFound = false;
            for(int index = 0; index < locationElementList.count(); index++) {
                QDomElement locationElement = locationElementList.at(index).toElement();

                if (locationElement.attribute("id") == widgetId) {
                    locationTagFound = true;
                    locationElement.setAttribute("state", state);
                    break;
                }
            }

            // No tag with matching id was found so we add a new location tag to the controller
            if (!locationTagFound) {
                QDomElement widget = d->mSessionTree->createElement("state");
                widget.setAttribute("id", widgetId);
                widget.setAttribute("state", state);
                widgetElement.appendChild(widget);
            }

        } else
            continue;

    }

    Q_EMIT sessionUpdated(d->mSessionTree->toString());
}

void AbstractDesktopView::sessionDataForController(const QString &controllerName, const QString &key, const QString &value)
{
    QDomNodeList widgetNodeList = d->mSessionTree->documentElement().elementsByTagName("widget");

    for(int index = 0; index < widgetNodeList.count(); index++) {
        QDomElement widgetElement = widgetNodeList.at(index).toElement();

        if (widgetElement.attribute("controller") != controllerName)
            continue;

        if (widgetElement.hasChildNodes()) {
            QDomElement argElement = widgetElement.firstChildElement("arg");

            if (argElement.isNull()) {
                QDomElement keyTag = d->mSessionTree->createElement("arg");
                keyTag.setAttribute(key, value);
                widgetElement.appendChild(keyTag);
            } else
                argElement.setAttribute(key, value);
        }
    }

    Q_EMIT sessionUpdated(d->mSessionTree->toString());
}

void AbstractDesktopView::restoreViewFromSession(const QString &sessionData, bool firstRun)
{
    if (d->mSessionTree) {
        QString errorMsg;
        d->mSessionTree->setContent(sessionData, &errorMsg);
        qDebug() << Q_FUNC_INFO <<  errorMsg;
    }

    QDomNodeList widgetNodeList = d->mSessionTree->documentElement().elementsByTagName("widget");

    for(int index = 0; index < widgetNodeList.count(); index++) {
        QDomElement widgetElement = widgetNodeList.at(index).toElement();

        addController(widgetElement.attribute("controller"), firstRun);
        QSharedPointer<ControllerInterface> iface = controllerByName(widgetElement.attribute("controller"));

        if (widgetElement.hasChildNodes()) {
            QDomElement argElement = widgetElement.firstChildElement("arg");

            if (!argElement.isNull()) {
                QDomNamedNodeMap attrMap = argElement.attributes();
                QVariantMap args;
                for (int attrIndex = 0; attrIndex < attrMap.count(); attrIndex++) {
                    QDomNode attributeNode = attrMap.item(attrIndex);
                    QString key = attributeNode.toAttr().name();
                    QString value = attributeNode.toAttr().value();
                    args[key] = QVariant(value);
                }

                if (iface) {
                    iface->revokeSession(args);
                }
            }

            QDomElement rectElement = widgetElement.firstChildElement("geometry");
            if (!rectElement.isNull()) {
                QDomAttr x = rectElement.attributeNode("x");
                QDomAttr y = rectElement.attributeNode("y");

                QDomAttr widthAttr = rectElement.attributeNode("width");
                QDomAttr heightAttr = rectElement.attributeNode("height");

                QRectF geometry (x.value().toFloat(), y.value().toFloat(), widthAttr.value().toFloat(), heightAttr.value().toFloat());

                if(iface) {
                    iface->setViewRect(geometry);
                }
            }

            //restore location for child widgets
            QDomNodeList locationElementList = widgetElement.elementsByTagName("location");

            for(int index = 0; index < locationElementList.count(); index++) {
                QDomElement locationElement = locationElementList.at(index).toElement();

                if(locationElement.isNull())
                    continue;

                if (this->scene()) {
                    QList<QGraphicsItem *> items = scene()->items();

                    Q_FOREACH(QGraphicsItem *item, items) {

                        QGraphicsObject *itemObject = item->toGraphicsObject();

                        if (!itemObject) {
                            continue;
                        }

                        AbstractDesktopWidget *widget = qobject_cast<AbstractDesktopWidget*> (itemObject);

                        if (!widget || !widget->controller())
                            continue;

                        if (locationElement.attribute("id") == widget->widgetID()) {
                            QDomElement geoElement = locationElement.firstChildElement("geometry");
                            QPointF widgetPos = QPoint(geoElement.attribute("x").toInt(), geoElement.attribute("y").toInt());
                            widget->setPos(widgetPos);
                        }
                    }
                }
            }

            //restore deleted widgets
            QDomNodeList stateElements = widgetElement.elementsByTagName("state");

            for(int index = 0; index < stateElements.count(); index++) {
                QDomElement stateElement = stateElements.at(index).toElement();

                if(stateElement.isNull())
                    continue;

                if (this->scene()) {
                    QList<QGraphicsItem *> items = scene()->items();
                    QList<AbstractDesktopWidget*> itemsToDelete;

                    Q_FOREACH(QGraphicsItem *item, items) {

                        if (!item)
                            continue;

                        QGraphicsObject *itemObject = item->toGraphicsObject();

                        if (!itemObject) {
                            continue;
                        }

                        AbstractDesktopWidget *widget = qobject_cast<AbstractDesktopWidget*> (itemObject);

                        if (!widget || !widget->controller())
                            continue;

                        if (stateElement.attribute("id") == widget->widgetID()) {
                            bool state = stateElement.attribute("state").toInt();
                            if (state) {
                                widget->hide();
                                scene()->removeItem(item);
                                itemsToDelete.append(widget);
                            }
                        }
                    }
                    //delete items
                    int count = itemsToDelete.count();
                    for (int i = 0; i < count; i++) {
                        AbstractDesktopWidget *widget = itemsToDelete.at(i);
                        if(widget) {
                            onWidgetClosed(widget);
                        }
                    }

                    itemsToDelete.clear();
                }
            }
        }
    }

}

void AbstractDesktopView::onWidgetClosed(AbstractDesktopWidget *widget)
{
    if (scene()) {
        bool deleted = 0;

        if (widget->controller()) {
           QString widgetId = widget->widgetID();
           QString controllerName = widget->controller()->controllerName();
           deleted = widget->controller()->deleteWidget(widget);
          /* if the plugin didn't delete it, we are going to delete it anyways, so always pass TRUE to saveItemStateToSession */
           saveItemStateToSession(controllerName, widgetId, 1);
        }

        if (!deleted)
            delete widget;
    }
}

}
