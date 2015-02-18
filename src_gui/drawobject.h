#ifndef DRAWOBJECT_H
#define DRAWOBJECT_H

#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QVector>
#include <QBitmap>
#include <QPushButton>

class DrawObject : public QWidget {

   Q_OBJECT

public:
    int ID;
    QVector<QLabel*> labelvector;

    DrawObject(int id, QPoint position, int width , int height, QWidget *parent);

    QVector<QPushButton*> buttonvector;



    QBitmap  mainMask;
    QImage mainMaskAsImage;
    QPainter painter;
    QPixmap overAllPicture;

    void addPicture (QPixmap *pic, QPoint position);
    void addPicture(QPixmap *pic, QPoint position, QString typeName);
    void addGateButton(QPixmap *pic, QPoint position, QString gateName);

    //Modify the mask of the drawobject inserting the mask of the pixmap at given position
    void modifyMask(QPixmap *pic, QPoint position);

    QPixmap getPicture ();

    void highlightMask();
    int getPixel(const QImage &img, const int x, const int y) const;
    void setPixel(QImage &img, const int x, const int y, const int pixel);
    void printMask();
signals:

    void released(int nodeID, QString gateName, QPoint position);

public slots:

    void releasedOnGate(QString gateName, QPoint position);

private:

void updateOverAllPicture(QPixmap *newPicture, QPoint position);



};


#endif // DRAWOBJECT_H
