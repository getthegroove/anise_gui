#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QCoreApplication>
#include <QVector>
#include <QCursor>
#include <QShortcut>
#include <QSpinBox>
#include <limits>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "anisecommunicator.h"
#include "settingshandler.h"
#include "qdebugstreamredirector.h"
#include "mesh.h"
#include "singletonrender.h"
#include "nodecatalog.h"
#include "data.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    // Important to do first! SingletonRender is used in initialize GUI
    SingletonRender::instance()->setUi(this->ui);


    initializeGUI();

    //activates deleteItem when deletebutton is pressed
    //deleteItem handles itself whether object is focussed or not nothing is deleted
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut, SIGNAL(activated()), Data::instance(), SLOT(deleteItem()));
    connect(ui->delete_button, SIGNAL(clicked()), Data::instance(), SLOT(deleteItem()));
}

void MainWindow::initializeGUI() {
    // general initialization of the mainwindow
    ui->setupUi(this);                   // generate gui from xml
    menuBar()->setNativeMenuBar(false);  // schaltet natives ubuntu menu aus
    // new Q_DebugStream(std::cout, ui->qDebug_out); // leitet qDebug ins
    // logfenster
    // Q_DebugStream::registerQDebugMessageHandler();

    // make the mesh editor accept drops
    ui->mesh_edt_area->setAcceptDrops(true);

    // setup viewports
    ui->Node_Catalog->setupViewport(ui->nodeCatalogContent);
    ui->mesh_edt_area->setupViewport(ui->meshField);

    // hides the tab-widget
    ui->tabWidget->hide();

    // prepare propertytable
    ui->tableWidget->hide();
    ui->tableWidget->setColumnCount(2);
    // initialize all data content
    Data::instance()->initialize(this);
}


MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_actionLoad_triggered() {



    qDebug() << "Trying to open FileDialog";
    QString fileName = QFileDialog::getOpenFileName(
                this, "Load previously saved mesh", "",
                "Mesh-Files (*.mesh *.json);; All Files (*.*)");

    qDebug() << "Path to File loaded\nPath is" << fileName
             << "\nnow let's load it to the FileHandler!";

    if (fileName == "") return;
    this->on_actionNew_triggered();
    QJsonObject *obj = JsonFileHandler::readFile(fileName);
    JsonFileHandler::extractNodesAndConnections(*obj);
    SingletonRender::instance()->renderMesh(Data::instance()->getMesh());
}

void MainWindow::on_actionSet_framework_path_triggered() {
    qDebug() << "Try to open Folder Chooser";
    QFileDialog dialog(this);

    QString fileName =
            QFileDialog::getOpenFileName(this, "Set your framework path", "", "");

    SettingsHandler::storeSetting("frameworkpath", fileName);
    AniseCommunicator::setFrameworkPath(fileName);
}

void MainWindow::on_actionNew_triggered() {
    // TODO ask if old is wished to thrown away if not saved
    // qDebug() << "Mesh before NEW:\t Nodes: " <<
    // Data::instance()->mesh->getAllNodes().size();
    Data::instance()->newMeshProject();
    // qDebug() << "Mesh after NEW:\t Nodes: " <<
    // Data::instance()->mesh->getAllNodes().size();
}

void MainWindow::on_actionLoad_Catalog_triggered() {

    //the old ndoes should be deleted!

    QString out = AniseCommunicator::getAllNodeTypes();
    //Data::instance()->getNodeCatalog()->Content.clear();
    JsonFileHandler::parseNodeTypesFromAnise(out);
    //SingletonRender::instance()->renderCatalogContent(Data::instance()->getNodeCatalog()->Content.values().toVector());


}

void MainWindow::on_actionSave_triggered() {
    Mesh *theMesh = Data::instance()->getMesh();
    QString fileName = QFileDialog::getSaveFileName(this, "Save current project to...", "",
                                         "Mesh-Files (*.mesh *.json);;All Files(*)");
    if(!(fileName.endsWith(".json",Qt::CaseInsensitive) || fileName.endsWith(".mesh", Qt::CaseInsensitive)))
        fileName += ".mesh";
    JsonFileHandler::saveMesh(fileName, theMesh);

}

void MainWindow::updatePropertyTable(int nodeID) {
    QTableWidget *table = ui->tableWidget;
    if (nodeID >= 0 &&
            Data::instance()->getMesh()->nodesInMesh.contains(nodeID)) {
        deleteTable();
        Node *n = Data::instance()->getMesh()->getNodeByID(nodeID);
        QMap<QString, Node::parameter> *map = n->getParams();
        table->setRowCount(map->size() + 5);
        int i = 0;
        // create the entries all nodes have
        QTableWidgetItem *name = new QTableWidgetItem(),
                *ID = new QTableWidgetItem(),
                *type = new QTableWidgetItem();

        // set the data
        ID->setData(0, nodeID);
        name->setData(0, n->getName());
        type->setData(0, n->getType());

        // asign to table:

        //NodeID
        table->setItem(i, 0, new QTableWidgetItem("NodeID"));
        table->setItem(i, 1, ID);
        table->item(i, 0)->setFlags(table->item(0, 0)->flags() ^
                                    (Qt::ItemIsEnabled | Qt::ItemIsSelectable));
        i++;

        //Node Class
        table->setItem(i, 0, new QTableWidgetItem("Node Class"));
        table->setItem(i, 1, type);
        table->item(i, 0)->setFlags(table->item(i, 0)->flags() ^
                                    (Qt::ItemIsEnabled | Qt::ItemIsSelectable));
        i++;

        //input Gates

        bool moreThanOne = n->getInputGates()->size()>1;
        if(!n->getInputGates()->isEmpty()){
            QString types;
            for(Gate *g : *n->getInputGates())
                for(QString s : g->getTypes())
                    types += s + ", ";
            types.chop(2);
            QTableWidgetItem *in = new QTableWidgetItem();
            in->setText(types);
            in->setFlags(in->flags() ^ (Qt::ItemIsEditable | Qt::ItemIsSelectable));
            table->setItem(i,0, new QTableWidgetItem("Input" + QString((moreThanOne?"s":""))));
            table->setItem(i,1,in);
            table->item(i,1)->setToolTip(types);
            i++;
        }

        //output gates
        moreThanOne = n->getOutputGates()->size()>1;
        if(!n->getInputGates()->isEmpty()){
            QString types;
            for(Gate *g : *n->getOutputGates())
                for(QString s : g->getTypes())
                    types += s + ", ";
            types.chop(2);
            QTableWidgetItem *out = new QTableWidgetItem;
            out->setText(types);
            out->setFlags(out->flags() ^ (Qt::ItemIsEditable | Qt::ItemIsSelectable));
            table->setItem(i,0, new QTableWidgetItem("Output" + QString((moreThanOne?"s":""))));
            table->setItem(i,1,out);
            table->item(i,1)->setToolTip(types);
            i++;
        }
        table->setItem(i, 0, new QTableWidgetItem("Node Name"));
        table->setItem(i, 1, name);
        i++;

        // make name editable
        name->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable |
                       Qt::ItemIsSelectable);

        // avoid editing keys, and nodeID/type
        ID->setFlags(ID->flags() ^ (Qt::ItemIsEditable | Qt::ItemIsSelectable |
                                    Qt::ItemIsEnabled));
        type->setFlags(type->flags() ^ (Qt::ItemIsEditable | Qt::ItemIsSelectable |
                                        Qt::ItemIsEnabled));

        for (int j = 0; j < i; j++)
            table->item(j, 0)->setFlags(table->item(j, 0)->flags() ^
                                        (Qt::ItemIsEditable | Qt::ItemIsSelectable));
        QStringList keys = map->keys();
        int count = i;
        for (int j = 0; j < keys.size(); j++) {
            count++;
            // create key item
            QString key = keys.value(j);
            QTableWidgetItem *key_item = new QTableWidgetItem((*map)[key].name);
            key_item->setData(Qt::UserRole,key);
            key_item->setFlags(key_item->flags() ^
                               (Qt::ItemIsEditable | Qt::ItemIsSelectable));
            key_item->setToolTip((*map)[key].descr);
            table->setItem(i + j, 0, key_item);

            // create value item
            QVariant value = map->value(key).value;
            QTableWidgetItem *value_item = new QTableWidgetItem();
            value_item->setToolTip((*map)[key].descr);
            //QSpinBox *spinner;
            switch (value.userType()) {
            case QVariant::Bool:       
                // value_item->setData(0, value);
                value_item->setCheckState(value.toBool() ? Qt::Checked
                                                         : Qt::Unchecked);
                table->setItem(i + j, 1, value_item);
                break;
            /*case QVariant::Int:
                spinner = new QSpinBox(table);
                spinner->setMaximum(std::numeric_limits<int>::max());
                spinner->setMinimum(std::numeric_limits<int>::min());
                spinner->setValue(value.toInt());
                table->setCellWidget(3 + i, 1, spinner);
                break;
            case QVariant::UInt:
                spinner = new QSpinBox(table);
                spinner->setMaximum(std::numeric_limits<int>::max());
                spinner->setMinimum(0);
                spinner->setValue(value.toInt());
                table->setCellWidget(3 + i, 1, spinner);
                break;*/
            default:

                value_item->setData(Qt::UserRole, value);
                value_item->setText(value.toString());
                table->setItem(i + j, 1, value_item);
                break;
            }
        }
        table->setRowCount(count);
        connect(table, SIGNAL(itemChanged(QTableWidgetItem*)), Data::instance()->getMesh(), SLOT(updateNode(QTableWidgetItem*)));
        table->resizeColumnsToContents();
        if(ui->details->checkState() == Qt::Checked)
            table->show();
    } else if (table->isVisible()) {
        deleteTable();
    }
}

void MainWindow::deleteTable() {
    QTableWidget *table = ui->tableWidget;
    table->disconnect(table, SIGNAL(itemChanged(QTableWidgetItem*)), Data::instance()->getMesh(), SLOT(updateNode(QTableWidgetItem*)));
    table->hide();
    // delete all tableitems, because they aren't needed any more
    for (int col = 0; col < table->columnCount(); col++)
        for (int row = 0; row < table->rowCount(); row++)
            delete table->item(row, col);
    table->setRowCount(0);
}

void MainWindow::displayTypeInfo(const QString &type) {
    deleteTable();
    QTableWidget *table = ui->tableWidget;
    Node n = *Data::instance()->getNodeCatalog()->getPointerOfType(type);
    //i is to count the rows set up
    int i = 0;
    const QMap<QString, Node::parameter> *params = n.getParams();

    //init the table
    table->setRowCount(((params->size()) + 4));
    table->setColumnCount(2);

    //print the Node Type
    table->setItem(i, 0, new QTableWidgetItem("Type"));
    table->setItem(i, 1, new QTableWidgetItem(type));
    QFont f1 = table->item(i,1)->font(), f2 = table->item(i,0)->font();
    f1.setBold(true);
    f2.setBold(true);
    table->item(i,0)->setFont(f2);
    table->item(i++,1)->setFont(f1);


    //print input gates
    QVector<Gate*> *gates = n.getInputGates();
    bool moreThanOneGate = gates->size()>1;
    QString gateTypes;
    //if there are inputs, print them
    if(!gates->isEmpty()) {
        table->setItem(i, 0, new QTableWidgetItem("Input" + QString((moreThanOneGate?"s":""))));
        for(Gate *g : *gates)
            for(QString s : g->getTypes())
                gateTypes += s + ", ";
        gateTypes.chop(2);
        table->setItem(i++, 1, new QTableWidgetItem(gateTypes));
    }


    //print outputs
    gateTypes = "";
    gates = n.getOutputGates();
    moreThanOneGate = gates->size()>1;
    if(!gates->isEmpty()){
        for(Gate *g : *gates)
            for(QString s : g->getTypes())
                gateTypes += s + ", ";
        gateTypes.chop(2);
        table->setItem(i, 0, new QTableWidgetItem("Output" + QString((moreThanOneGate?"s":""))));
        table->setItem(i++, 1, new QTableWidgetItem(gateTypes));
    }

    QString descr = n.getDescription();
    if(descr != "")
    {
        table->setRowCount(table->rowCount() + 1);
        table->setItem(i, 0, new QTableWidgetItem("Description"));
        table->setItem(i, 1, new QTableWidgetItem(descr));
        table->item(i++,1)->setToolTip(descr);
    }
    QList<QString> keys = params->keys();
    for (int j = 0; j < keys.size(); j++) {
        table->setItem(i, 0, new QTableWidgetItem((*params)[keys[j]].name));
        table->setItem(i, 1, new QTableWidgetItem((*params)[keys[j]].type));
        table->item(i, 1)->setToolTip((*params)[keys[j]].descr);
        i++;
    }
    table->setRowCount(i);
    for (int col = 0; col < table->columnCount(); col++)
        for (int row = 0; row < table->rowCount(); row++)
            if(table->item(row,col))
                table->item(row, col)->setFlags(Qt::ItemIsEnabled);
    table->resizeColumnToContents(0);
    table->resizeColumnToContents(1);
    for(int col = 0; col < table->columnCount();col++)
        for(int row = 0;row < table->rowCount();row++)
            table->item(row,col)->setToolTip(table->item(row,col)->text());
    if(ui->details->checkState() == Qt::Checked)
        table->show();
}

void MainWindow::on_details_stateChanged(int arg1)
{
    if(arg1 && Data::instance()->getFocusedID() != -1)
        ui->tableWidget->show();
    else
        ui->tableWidget->hide();
}
