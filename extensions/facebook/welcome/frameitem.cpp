#include "frameitem.h"

void Frame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    render.render(painter, rect());
}


