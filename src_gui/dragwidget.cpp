#include "dragwidget.h"
#include <QMouseEvent>
#include <QPoint>
#include <QDrag>
#include <QDebug>
#include <Qt>
#include <QMimeData>
#include <QIODevice>
#include <QPixmap>
#include <QLabel>
#include "drawobject.h"
#include <QPainter>

DragWidget::DragWidget(QWidget *parent) :
    QWidget(parent)
{
}

void DragWidget::mousePressEvent(QMouseEvent *event){
    qDebug() << "click";
    QLabel *child = static_cast<QLabel*>(childAt(event->pos()));
    if(!child){
        qDebug() << "no child";
        return;
    }
    qDebug() << "child";
    QPoint hotSpot = event->pos() - child->pos();

    QByteArray Data ;
    QDataStream dataStream(&Data, QIODevice::WriteOnly);
    dataStream << child << QPoint(hotSpot);

    //something about mime data...
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/customthingy", Data);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(*child->pixmap());
    drag->setHotSpot(hotSpot);
/*
    QPixmap tempPixmap = child->pixmap();
    QPainter painter;
    painter.begin(&tempPixmap);
    painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 127));
    painter.end();
*/
   // child->setPixmap(tempPixmap);

    if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
        qDebug() << "never happens?";
        //child->close();
    else {
        qDebug() << "drag end";
     child->show();
     //child->setPixmap(pixmap);
    }

}

void DragWidget::dragEnterEvent(QDragEnterEvent *event){
    qDebug()<<"dragEnterEvent";
    if (event->mimeData()->hasFormat("application/customthingy")) {
             if (event->source() == this) {
                 event->setDropAction(Qt::MoveAction);
                 event->accept();
             } else {
                 event->acceptProposedAction();
             }
         } else {
             event->ignore();
         }
}
void DragWidget::dragMoveEvent(QDragMoveEvent *event){
    qDebug()<<"1";
}
void DragWidget::dropEvent(QDropEvent *event){
    qDebug()<<"1";
}


