#include <stddef.h>  // defines NULL
#include "singletonrender.h"

// Global static pointer used to ensure a single instance of the class.
SingletonRender *SingletonRender::m_pInstance = NULL;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/

SingletonRender *SingletonRender::instance() {
    if (!m_pInstance)  // Only allow one instance of class to be generated.
        m_pInstance = new SingletonRender;

    return m_pInstance;
}


// will have to be called from a paint event!
void SingletonRender::drawLine(double start_x, double start_y, double end_x , double end_y){
    qDebug() << "drawline";
    QPainter painter(this->ui->meshField);
    QLineF line(start_x, start_y, end_x , end_y);
    painter.setPen(Qt::blue);
    painter.drawLine(line);
}


//bug right now:
// paint events are never called
void SingletonRender::paintEvent(QPaintEvent *event)
{
    qDebug() << "draw event";
   /*
    foreach (QLine* d, this->allDrawnLines) {
        QPainter painter(this->ui->meshField);
        painter.setPen(QPen(Qt::black, 12, Qt::DashDotLine, Qt::RoundCap));
        painter.drawLine(d);
    }*/
}



SingletonRender::SingletonRender() {
    // initialize all maps
    this->allDrawnNodes = QMap<int, DrawObject *>();
    this->allDrawnLines = QMap<int, QLine *>();
    this->allImages = QMap<QString, QPixmap *>();

    // load all images
    if (loadImages() == true) {
        qDebug() << "images loaded successfully";
    } else {
        qDebug() << "images loading failed";
    }
}

// loads all images in the ../DataIimages folder.
// saves them in the map "allImages"
bool SingletonRender::loadImages() {
    bool result = true;
    QStringList listOfFiles;

    // go into the right Directory using a Qdir object
    QDir directory;
    directory.cdUp();
    directory.cd("Data/Images/");

    // fill the list of Files with all filenames insode this directory
    if (directory.exists() == true) {
        // qDebug() << "directory.exists() == true";
        listOfFiles = directory.entryList();
    } else {
        qDebug() << "directory.exists() == false";
        result = false;
    }

    // remove all filenames who are not a png file
    listOfFiles = listOfFiles.filter(".png");

    // loads all images;
    // if one coudlnt be loaded it will return false
    for (int i = 0; i < listOfFiles.size(); ++i) {
        if (result == true) {
            QPixmap *temp = new QPixmap();

            // set the result true if loeaded succesfully, false if not
            result =
                    temp->load(directory.absolutePath().append("/" + listOfFiles.at(i)));

            // if failed print a debug message
            if (result == false) {
                qDebug() << "loaded image: "
                         << directory.absolutePath().append(listOfFiles.at(i)) << " "
                         << result;
            }

            allImages.insert(listOfFiles.at(i), temp);
        }
    }
    return result;
}

void SingletonRender::setUi(Ui::MainWindow *ui) { this->ui = ui; }

void SingletonRender::renderNode(Node *nodeToRender, int nodeID) {
    if (!allDrawnNodes.contains(nodeID)) {
        // create a Drawobject
        DrawObject *NodeDrawObject = new DrawObject(nodeID, this->ui->meshField);

        if (allImages.contains("background.png")) {
            // Draw the Background
            NodeDrawObject->setPixmap(*allImages["background.png"]);
            // Set the geometry to the right size
            NodeDrawObject->setGeometry(0, 0, 50, 50);
        } else {
            qDebug() << "background.png did not load correctly!";
        }

        allDrawnNodes.insert(nodeID, NodeDrawObject);
    }
    // TODO should use layouts instead of hardcoded position!
    allDrawnNodes.value(nodeID)
            ->move(nodeToRender->position_x, nodeToRender->position_y);

    allDrawnNodes.value(nodeID)->show();
}

void SingletonRender::renderMesh(Mesh *workMesh) {
    QMap<int, Node *> temp = workMesh->nodesInMash;

    // calls render method for every node in the mesh
    foreach (int ID, temp.keys()) {
        // Only do it if a Node with this ID exists in the mesh
        if (temp.contains(ID)) {
            renderNode(temp.value(ID), ID);
        } else {
            qDebug() << "tried to render a node that doesnt exist!";
        }
    }
}

void SingletonRender::renderNodeType(Node *nodeToRender, QWidget *parent,
                                     int position) {
    // TODO code dublication in renderNode and renderNodeType!
    QLabel *NodeDrawObject = new QLabel(parent);

    // Zeichne den hintergrund:
    NodeDrawObject->setPixmap(*allImages["background.png"]);
    NodeDrawObject->setGeometry(0, 0, 50, 50);

    // TODO should use layouts instead of hardcoded position!
    NodeDrawObject->move(5, 5 + position * 60);

    NodeDrawObject->show();
}

void SingletonRender::renderCatalogContent(QVector<Node> NodeVektor,
                                           QWidget *CatalogParent) {
    int position = 0;
    // TODO scroll weite sollte nicht hard coded sein
    CatalogParent->setMinimumHeight(NodeVektor.size() * 60 + 10);
    foreach (Node nodeTyp, NodeVektor) {
        renderNodeType(&nodeTyp, CatalogParent, position);
        position++;
    }
}