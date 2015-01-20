#include "data.h"
#include "singletonrender.h"
#include "ui_mainwindow.h"



Data::Data(QObject *parent) : QObject(parent) {

  mesh = new Mesh();
  nodeCatalog = new NodeCatalog();
}



void Data::setUi(Ui::MainWindow *ui){
    this->ui = ui;
}


void Data::addNodeToMesh(Node *newNode){
    this->mesh->addNode(newNode);
    singletonRender::instance()->renderMesh(this->mesh, this->ui->meshField);

}

void Data::moveNodeInMesh(QPoint *Position, int numberOfNode) {

    this->mesh->nodes.at(numberOfNode)->setPosition(Position->x(), Position->y());
    singletonRender::instance()->renderMesh(this->mesh, this->ui->meshField);

}
