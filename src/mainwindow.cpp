#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include<QFileDialog>
#include <QFile>
#include <QTextStream>
#include <ros/package.h>
double ToG    = 57.295779513;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  firstLoad=true;
  ui->setupUi(this);
  setUpHighlighter();
  //init status bar
  ui->outputText->parentWindow=this;
  ui->statusBar->showMessage(tr("Ready"));
  //--------init toolbar------------
  //ui->statusBar->setStyleSheet("QStatusBar{background:rgb(50,50,50);}");
  ui->mainToolBar->setMovable(false);
  ui->mainToolBar->setStyleSheet("QToolButton:hover {background-color:darkgray} QToolBar {background: rgb(179, 204, 204);border: none;}");
  //--------------------------------

  runIcon.addPixmap(QPixmap(":/img/run.png"));
  stopIcon.addPixmap(QPixmap(":/img/stop.png"));
  errorIcon.addPixmap(QPixmap(":/img/error.png"));
  RerrorIcon.addPixmap(QPixmap(":/img/error_red.png"));


  QPalette windowPalette=this->palette();
  windowPalette.setColor(QPalette::Active,QPalette::Window,QColor(82,82,82));
  windowPalette.setColor(QPalette::Inactive,QPalette::Window,QColor(82,82,82));
  this->setPalette(windowPalette);
  //--------------------------------
  initFileData();
  this->updateRobot();
  QObject::connect(this,      SIGNAL(setvaluesSubs()),            this, SLOT(updatevalues()));
  connect(ui->actionInfoR,    SIGNAL(triggered(bool)),            this, SLOT(updatevalues()));
  connect(ui->actionNewFile,  SIGNAL(triggered(bool)),            this, SLOT(newFile()));
  connect(ui->actionOpen,     SIGNAL(triggered(bool)),            this, SLOT(openFile()));
  connect(ui->actionSave_File,SIGNAL(triggered(bool)),            this, SLOT(saveFile()));
  connect(ui->actionUndo,     SIGNAL(triggered(bool)),            this, SLOT(undo()));
  connect(ui->actionRedo,     SIGNAL(triggered(bool)),            this, SLOT(redo()));
  connect(ui->editor,         SIGNAL(textChanged()),              this, SLOT(changeSaveState()));
  connect(ui->actionRun,      SIGNAL(triggered(bool)),            this, SLOT(run()));
  connect(ui->actionAbout,    SIGNAL(triggered(bool)),            this, SLOT(about()));

  connect(ui->pushButton,     SIGNAL(clicked()),                  this, SLOT(pid_value()));
  connect(ui->pushButton,     SIGNAL(clicked()),                  this, SLOT(pid_value_2()));


  fileSaved=true;
//  msg1.points.resize(1);
//  msg1.points[0].positions.resize(6);
//  msg1.points[1].positions.resize(6);

    startTime = ros::Time::now();
    joint_1_plot = 0.0;
    joint_2_plot = 0.0;
    joint_3_plot = 0.0;
    joint_4_plot = 0.0;
    joint_5_plot = 0.0;
    joint_6_plot = 0.0;

    vel_1_plot = 0.0;
    vel_2_plot = 0.0;
    vel_3_plot = 0.0;
    vel_4_plot = 0.0;
    vel_5_plot = 0.0;
    vel_6_plot = 0.0;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGraph()));
    timer->start(5);


  ui->graph_canvas->yAxis->setTickLabels(false);
  connect(ui->graph_canvas->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->graph_canvas->yAxis, SLOT(setRange(QCPRange))); // left axis only mirrors inner right axis
  ui->graph_canvas->yAxis2->setVisible(true);
  ui->graph_canvas->axisRect()->addAxis(QCPAxis::atRight);
  ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(30); // add some padding to have space for tags
  ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(30); // add some padding to have space for tags
  ui->graph_canvas->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)1);

 


    this->initializeGraph();
    connect(ui->graph_canvas, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
    connect(ui->graph_canvas, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->graph_canvas, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));

    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(removeAllGraphs()));

    ui->graph_canvas->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->graph_canvas,  SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    connect(ui->comboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_currentIndexChanged(int)));
   // connect(ui->comboBox_2,      SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_2_currentIndexChanged(int)));


  msgolder.points.resize(1);
  msgolder.points[0].positions.resize(6);

   joint_pub =       nh_.advertise<trajectory_msgs::JointTrajectory>("set_joint_trajectory_gz", 10);
// joint_sub_limit = nh_.subscribe("/joint_limits",10,&MainWindow::jointsizeCallback, this);
   pid_value_pub   = nh_.advertise<trajectory_msgs::JointTrajectory>("pid_value", 10);

   // joint_pub1 =       nh_.advertise<std_msgs::Float64>("/robot/joint_1_position_controller/command", 100);
   // joint_pub2 =       nh_.advertise<std_msgs::Float64>("/robot/joint_2_position_controller/command", 100);
   // joint_pub3 =       nh_.advertise<std_msgs::Float64>("/robot/joint_3_position_controller/command", 100);
   // joint_pub4 =       nh_.advertise<std_msgs::Float64>("/robot/joint_4_position_controller/command", 100);
   // joint_pub5 =       nh_.advertise<std_msgs::Float64>("/robot/joint_5_position_controller/command", 100);
   // joint_pub6 =       nh_.advertise<std_msgs::Float64>("/robot/joint_6_position_controller/command", 100);

   // Joint0.data = 0.0;
   // Joint1.data = 0.0;
   // Joint2.data = 0.0;
   // Joint3.data = 0.0;
   // Joint4.data = 0.0;
   // Joint5.data = 0.0;

   // joint_pub1.publish(Joint0);
   // joint_pub2.publish(Joint1);
   // joint_pub3.publish(Joint2);
   // joint_pub4.publish(Joint3);
   // joint_pub5.publish(Joint4);
   // joint_pub6.publish(Joint5);


   joint_sub_gazebo= nh_.subscribe("/gazebo_client/joint_values_gazebo",10,&MainWindow::joint_Gz_Callback, this);

   joint_sub_gazebo2= nh_.subscribe("/robot/joint_1_position_controller/state" ,10,&MainWindow::joint_Gz_Callback2, this);
   joint_sub_gazebo3= nh_.subscribe("/robot/joint_2_position_controller/state" ,10,&MainWindow::joint_Gz_Callback3, this);
   joint_sub_gazebo4= nh_.subscribe("/robot/joint_3_position_controller/state" ,10,&MainWindow::joint_Gz_Callback4, this);
   joint_sub_gazebo5= nh_.subscribe("/robot/joint_4_position_controller/state" ,10,&MainWindow::joint_Gz_Callback5, this);
   joint_sub_gazebo6= nh_.subscribe("/robot/joint_5_position_controller/state" ,10,&MainWindow::joint_Gz_Callback6, this);
   joint_sub_gazebo7= nh_.subscribe("/robot/joint_6_position_controller/state" ,10,&MainWindow::joint_Gz_Callback7, this);


   spinner = boost::shared_ptr<ros::AsyncSpinner>(new ros::AsyncSpinner(1));
   spinner->start();
//   boost::thread* publisher_thread_;
   isloop=false;
   limit.data.resize(12);
   limit.data[0] = -3.22886 *ToG;
   limit.data[1] = -2.70526 *ToG;
   limit.data[2] = -2.26893 *ToG;
   limit.data[3] = -6.10865 *ToG;
   limit.data[4] = -2.26893 *ToG;
   limit.data[5] = -6.10865 *ToG;

   limit.data[6] = 3.22886 *ToG;
   limit.data[7] = 0.610865 *ToG;
   limit.data[8] = 2.68781  *ToG;
   limit.data[9] = 6.10865  *ToG;
   limit.data[10] = 2.26893 *ToG;
   limit.data[11] = 6.10865 *ToG;
   publisher_thread_ = new boost::thread(boost::bind(&MainWindow::runloop, this));



}

MainWindow::~MainWindow()
{
  if(publisher_thread_ != NULL)
    {
      publisher_thread_->interrupt();
      publisher_thread_->join();

      delete publisher_thread_;
    }
  delete ui;
}


void MainWindow::initializeGraph() {
    //Make legend visible
    //ui.graph_canvas->legend->setVisible(true);
    //ui.graph_canvas->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);

    //Add the graphs

   jGraph0 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 0));
   jGraph1 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 0));
   jGraph2 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 0));
   jGraph3 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 0));
   jGraph4 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 0));
   jGraph5 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 0));

   vGraph0 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 1));
   vGraph1 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 1));
   vGraph2 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 1));
   vGraph3 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 1));
   vGraph4 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 1));
   vGraph5 = ui->graph_canvas->addGraph(ui->graph_canvas->xAxis, ui->graph_canvas->axisRect()->axis(QCPAxis::atRight, 1));

   QPen PenJ1;
   PenJ1.setColor(QColor(30, 40, 255, 150));
   PenJ1.setStyle(Qt::DotLine);
   PenJ1.setWidthF(2);
   jGraph0->setPen(PenJ1);
   jGraph0->setName("Joint_1");
   jTag0 = new AxisTag(jGraph0->valueAxis());
   jTag0->setPen(jGraph0->pen());

   QPen PenJ2;
   PenJ2.setColor(Qt::red);
   PenJ2.setStyle(Qt::DotLine);
   PenJ2.setWidthF(2);
   jGraph1->setPen(PenJ2);
   jGraph1->setName("Joint_2");
   jTag1 = new AxisTag(jGraph1->valueAxis());
   jTag1->setPen(jGraph1->pen());

   QPen PenJ3;
   PenJ3.setColor(Qt::yellow);
   PenJ3.setStyle(Qt::DotLine);
   PenJ3.setWidthF(2);
   jGraph2->setPen(PenJ3);
   jGraph2->setName("Joint_3");
   jTag2 = new AxisTag(jGraph2->valueAxis());
   jTag2->setPen(jGraph2->pen());

   QPen PenJ4;
   PenJ4.setColor(QColor(115,182,209));
   PenJ4.setStyle(Qt::DotLine);
   PenJ4.setWidthF(2);
   jGraph3->setPen(PenJ4);
   jGraph3->setName("Joint_4");
   jTag3 = new AxisTag(jGraph3->valueAxis());
   jTag3->setPen(jGraph3->pen());

   QPen PenJ5;
   PenJ5.setColor(Qt::black);
   PenJ5.setStyle(Qt::DotLine);
   PenJ5.setWidthF(2);
   jGraph4->setPen(PenJ5);
   jGraph4->setName("Joint_5");
   jTag4 = new AxisTag(jGraph4->valueAxis());
   jTag4->setPen(jGraph4->pen());

   QPen PenJ6;
   PenJ6.setColor(Qt::green);
   PenJ6.setStyle(Qt::DotLine);
   PenJ6.setWidthF(2);
   jGraph5->setPen(PenJ6);
   jGraph5->setName("Joint_6");
   jTag5 = new AxisTag(jGraph5->valueAxis());
   jTag5->setPen(jGraph5->pen());

   //Velocity

   QPen Penv1;
   Penv1.setColor(Qt::black);
   Penv1.setStyle(Qt::DotLine);
   Penv1.setWidthF(2);
   vGraph0->setPen(Penv1);
   vGraph0->setName("C_Joint_1");
   vTag0 = new AxisTag(vGraph0->valueAxis());
   vTag0->setPen(vGraph0->pen());

   QPen Penv2;
   Penv2.setColor(Qt::yellow);
   Penv2.setStyle(Qt::DotLine);
   Penv2.setWidthF(2);
   vGraph1->setPen(Penv2);
   vGraph1->setName("C_Joint_2");
   vTag1 = new AxisTag(vGraph1->valueAxis());
   vTag1->setPen(vGraph1->pen());

   QPen Penv3;
   Penv3.setColor(QColor(115,182,209));
   Penv3.setStyle(Qt::DotLine);
   Penv3.setWidthF(2);
   vGraph2->setPen(Penv3);
   vGraph2->setName("C_Joint_3");
   vTag2 = new AxisTag(vGraph2->valueAxis());
   vTag2->setPen(vGraph2->pen());

   QPen Penv4;
   Penv4.setColor(Qt::green);
   Penv4.setStyle(Qt::DotLine);
   Penv4.setWidthF(2);
   vGraph3->setPen(Penv4);
   vGraph3->setName("C_Joint_4");
   vTag3 = new AxisTag(vGraph3->valueAxis());
   vTag3->setPen(vGraph3->pen());

   QPen Penv5;
   Penv5.setColor(QColor(30, 40, 255, 150));
   Penv5.setStyle(Qt::DotLine);
   Penv5.setWidthF(2);
   vGraph4->setPen(Penv5);
   vGraph4->setName("C_Joint_5");
   vTag4 = new AxisTag(vGraph4->valueAxis());
   vTag4->setPen(vGraph4->pen());

   QPen Penv6;
   Penv6.setColor(Qt::red);
   Penv6.setStyle(Qt::DotLine);
   Penv6.setWidthF(2);
   vGraph5->setPen(Penv6);
   vGraph5->setName("C_Joint_6");
   vTag5 = new AxisTag(vGraph5->valueAxis());
   vTag5->setPen(vGraph5->pen());

   // mGraph11->setPen(QPen(QColor(0, 120, 0)));
   // mGraph2->setPen(QPen(QColor(0, 180, 60)));
   // mTag1 = new AxisTag(mGraph1->valueAxis());
   // mTag1->setPen(mGraph1->pen());
   // mTag11 = new AxisTag(mGraph11->valueAxis());
   // mTag11->setPen(mGraph11->pen());
   // mTag2 = new AxisTag(mGraph2->valueAxis());
   // mTag2->setPen(mGraph2->pen());

   // mGraph1 = ui->graph_canvas->addGraph();
    //mGraph1 =ui->graph_canvas->graph(0)->setName("Joint_1");
    // QPen blueDotPen;
    // blueDotPen.setColor(QColor(30, 40, 255, 150));
    // blueDotPen.setStyle(Qt::DotLine);
    // blueDotPen.setWidthF(4);
    // mGraph1 =ui->graph_canvas->graph(0)->setPen(blueDotPen);

    // mGraph2 = ui->graph_canvas->addGraph();
    // mGraph2 = ui->graph_canvas->graph(1)->setName("Joint_2");
    // QPen redDotPen;
    // redDotPen.setColor(Qt::red);
    // redDotPen.setStyle(Qt::DotLine);
    // redDotPen.setWidthF(4);
    // mGraph2 = ui->graph_canvas->graph(1)->setPen(redDotPen);

    // ui->graph_canvas->addGraph();
    // ui->graph_canvas->graph(2)->setName("Joint_3");
    // QPen yellowDotPen;
    // yellowDotPen.setColor(Qt::yellow);
    // yellowDotPen.setStyle(Qt::DotLine);
    // yellowDotPen.setWidthF(4);
    // ui->graph_canvas->graph(2)->setPen(yellowDotPen);

    // ui->graph_canvas->addGraph();
    // ui->graph_canvas->graph(3)->setName("Joint_4");
    // QPen blackDotPen;
    // blackDotPen.setColor(Qt::black);
    // blackDotPen.setStyle(Qt::DotLine);
    // blackDotPen.setWidthF(4);
    // ui->graph_canvas->graph(3)->setPen(blackDotPen);

    // ui->graph_canvas->addGraph();
    // ui->graph_canvas->graph(4)->setName("Joint_5");
    // QPen greenDotPen;
    // greenDotPen.setColor(Qt::green);
    // greenDotPen.setStyle(Qt::DotLine);
    // greenDotPen.setWidthF(4);
    // ui->graph_canvas->graph(4)->setPen(greenDotPen);

    // ui->graph_canvas->addGraph();
    // ui->graph_canvas->graph(5)->setName("Joint_6");
    // QPen otherDotPen;
    // otherDotPen.setColor(QColor(115,182,209));
    // otherDotPen.setStyle(Qt::DotLine);
    // otherDotPen.setWidthF(4);
    // ui->graph_canvas->graph(5)->setPen(otherDotPen);

    // give the axes some labels:
    ui->graph_canvas->xAxis->setLabel("Time (s)");
    ui->graph_canvas->yAxis->setLabel("Joint move (°) | (°/s)");
    ui->graph_canvas->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->graph_canvas->legend->setFont(legendFont);
    ui->graph_canvas->legend->setSelectedFont(legendFont);
    ui->graph_canvas->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items


    //For user interaction
    // main_window_ui_->graph_canvas->setInteraction(QCP::iRangeDrag, true);
    // main_window_ui_->graph_canvas->setInteraction(QCP::iRangeZoom, true);
    ui->graph_canvas->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);

    connect(ui->graph_canvas, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));

    x_org = 0;

    //Plot the graph
    ui->graph_canvas->replot();
}

void MainWindow::mouseMoved(QMouseEvent *event) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);

    double x = ui->graph_canvas->xAxis->pixelToCoord(event->x());
    double y = ui->graph_canvas->yAxis->pixelToCoord(event->y());

    oss << "Graph values: x: " << x << "\ty: " << y ;
    ui->label_4->setText(QString::fromStdString(oss.str()));
}

void MainWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);
  
  if (ui->graph_canvas->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {

    if (ui->graph_canvas->graphCount() > 0)
      menu->addAction("Clear all graphs", this, SLOT(removeAllGraphs()));
    //END
    std::cout <<"esta fue la ultima linea de code bajo ROS, Yeser A. Morales" << std::endl;
  }
  
  menu->popup(ui->graph_canvas->mapToGlobal(pos));
}

void MainWindow::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      ui->graph_canvas->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);

      ui->graph_canvas->replot();
    }
  }
}

void MainWindow::removeAllGraphs()
{
   // main_window_ui_->graph_canvas->clearGraphs();
   ui->graph_canvas->graph(0)->data()->clear();
   ui->graph_canvas->graph(1)->data()->clear();
   ui->graph_canvas->graph(2)->data()->clear();
   ui->graph_canvas->graph(3)->data()->clear();
   ui->graph_canvas->graph(4)->data()->clear();
   ui->graph_canvas->graph(5)->data()->clear();
   ui->graph_canvas->graph(6)->data()->clear();
   ui->graph_canvas->graph(7)->data()->clear();
   ui->graph_canvas->graph(8)->data()->clear();
   ui->graph_canvas->graph(9)->data()->clear();
   ui->graph_canvas->graph(10)->data()->clear();
   ui->graph_canvas->graph(11)->data()->clear();

   ui->graph_canvas->replot();
}

void MainWindow::updateGraph() {
    double x_val = (ros::Time::now() - startTime).toSec();

    // double x_org1 = x_org;
    // if (x_val - x_org1 > 20) {
    //     main_window_ui_->graph_canvas->graph(0)->removeDataBefore(x_org1 + 1);
    //     main_window_ui_->graph_canvas->graph(1)->removeDataBefore(x_org1 + 1);
    //     x_org++;
    // }

    ui->graph_canvas->graph(0)->addData(x_val, joint_1_plot);//Set Point
    ui->graph_canvas->graph(1)->addData(x_val, joint_2_plot);//Output
    ui->graph_canvas->graph(2)->addData(x_val, joint_3_plot);
    ui->graph_canvas->graph(3)->addData(x_val, joint_4_plot);
    ui->graph_canvas->graph(4)->addData(x_val, joint_5_plot);
    ui->graph_canvas->graph(5)->addData(x_val, joint_6_plot);

    ui->graph_canvas->graph(6)->addData(x_val, vel_1_plot);//Set Point
    ui->graph_canvas->graph(7)->addData(x_val, vel_2_plot);//Output
    ui->graph_canvas->graph(8)->addData(x_val, vel_3_plot);
    ui->graph_canvas->graph(9)->addData(x_val, vel_4_plot);
    ui->graph_canvas->graph(10)->addData(x_val, vel_5_plot);
    ui->graph_canvas->graph(11)->addData(x_val, vel_6_plot);
  

 // ui->graph_canvas->xAxis->rescale();
  jGraph0->rescaleValueAxis(false, true);
  jGraph1->rescaleValueAxis(false, true);
  jGraph2->rescaleValueAxis(false, true);
  jGraph3->rescaleValueAxis(false, true);
  jGraph4->rescaleValueAxis(false, true);
  jGraph5->rescaleValueAxis(false, true);

  vGraph0->rescaleValueAxis(false, true);
  vGraph1->rescaleValueAxis(false, true);
  vGraph2->rescaleValueAxis(false, true);
  vGraph3->rescaleValueAxis(false, true);
  vGraph4->rescaleValueAxis(false, true);
  vGraph5->rescaleValueAxis(false, true);
  
  ui->graph_canvas->xAxis->setRange(ui->graph_canvas->xAxis->range().upper, 30, Qt::AlignRight);
  
  // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
  double jgraph1Value = jGraph0->dataMainValue(jGraph0->dataCount()-1);
  double jgraph2Value = jGraph1->dataMainValue(jGraph1->dataCount()-1);
  double jgraph3Value = jGraph2->dataMainValue(jGraph2->dataCount()-1);
  double jgraph4Value = jGraph3->dataMainValue(jGraph3->dataCount()-1);
  double jgraph5Value = jGraph4->dataMainValue(jGraph4->dataCount()-1);
  double jgraph6Value = jGraph5->dataMainValue(jGraph5->dataCount()-1);

  double vgraph1Value = vGraph0->dataMainValue(vGraph0->dataCount()-1);
  double vgraph2Value = vGraph1->dataMainValue(vGraph1->dataCount()-1);
  double vgraph3Value = vGraph2->dataMainValue(vGraph2->dataCount()-1);
  double vgraph4Value = vGraph3->dataMainValue(vGraph3->dataCount()-1);
  double vgraph5Value = vGraph4->dataMainValue(vGraph4->dataCount()-1);
  double vgraph6Value = vGraph5->dataMainValue(vGraph5->dataCount()-1);

  jTag0->updatePosition(jgraph1Value);
  jTag1->updatePosition(jgraph2Value);
  jTag2->updatePosition(jgraph3Value);
  jTag3->updatePosition(jgraph4Value);
  jTag4->updatePosition(jgraph5Value);
  jTag5->updatePosition(jgraph6Value);

  vTag0->updatePosition(vgraph1Value);
  vTag1->updatePosition(vgraph2Value);
  vTag2->updatePosition(vgraph3Value);
  vTag3->updatePosition(vgraph4Value);
  vTag4->updatePosition(vgraph5Value);
  vTag5->updatePosition(vgraph6Value);

  jTag0->setText(QString::number(jgraph1Value, 'f', 4));
  jTag1->setText(QString::number(jgraph2Value, 'f', 4));
  jTag2->setText(QString::number(jgraph3Value, 'f', 4));
  jTag3->setText(QString::number(jgraph4Value, 'f', 4));
  jTag4->setText(QString::number(jgraph5Value, 'f', 4));
  jTag5->setText(QString::number(jgraph6Value, 'f', 4));

  vTag0->setText(QString::number(vgraph1Value, 'f', 4));
  vTag1->setText(QString::number(vgraph2Value, 'f', 4));
  vTag2->setText(QString::number(vgraph3Value, 'f', 4));
  vTag3->setText(QString::number(vgraph4Value, 'f', 4));
  vTag4->setText(QString::number(vgraph5Value, 'f', 4));
  vTag5->setText(QString::number(vgraph6Value, 'f', 4));
  
 // mPlot->replot();
   ui->graph_canvas->rescaleAxes();

    ui->graph_canvas->replot();
}

void MainWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed
  
  if (ui->graph_canvas->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->graph_canvas->axisRect()->setRangeZoom(ui->graph_canvas->xAxis->orientation());
  else if (ui->graph_canvas->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->graph_canvas->axisRect()->setRangeZoom(ui->graph_canvas->yAxis->orientation());
  else
    ui->graph_canvas->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged
  
  if (ui->graph_canvas->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->graph_canvas->axisRect()->setRangeDrag(ui->graph_canvas->xAxis->orientation());
  else if (ui->graph_canvas->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->graph_canvas->axisRect()->setRangeDrag(ui->graph_canvas->yAxis->orientation());
  else
    ui->graph_canvas->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::on_comboBox_currentIndexChanged(int index=0)
{
  switch (index){

   case 0:
 {
    ui->graph_canvas->graph(0)->setVisible(true);
    ui->graph_canvas->graph(1)->setVisible(true);
    ui->graph_canvas->graph(2)->setVisible(true);
    ui->graph_canvas->graph(3)->setVisible(true);
    ui->graph_canvas->graph(4)->setVisible(true);
    ui->graph_canvas->graph(5)->setVisible(true);
    ui->graph_canvas->graph(6)->setVisible(true);
    ui->graph_canvas->graph(7)->setVisible(true);
    ui->graph_canvas->graph(8)->setVisible(true);
    ui->graph_canvas->graph(9)->setVisible(true);
    ui->graph_canvas->graph(10)->setVisible(true);
    ui->graph_canvas->graph(11)->setVisible(true);    

    jTag0->setVisibleArrow(true);
    jTag1->setVisibleArrow(true);
    jTag2->setVisibleArrow(true);
    jTag3->setVisibleArrow(true);
    jTag4->setVisibleArrow(true);
    jTag5->setVisibleArrow(true);
    vTag0->setVisibleArrow(true);
    vTag1->setVisibleArrow(true);
    vTag2->setVisibleArrow(true);
    vTag3->setVisibleArrow(true);
    vTag4->setVisibleArrow(true);
    vTag5->setVisibleArrow(true);


   break;
 }
   case 1:
  {
    ui->graph_canvas->graph(0)->setVisible(true);
    ui->graph_canvas->graph(1)->setVisible(false);
    ui->graph_canvas->graph(2)->setVisible(false);
    ui->graph_canvas->graph(3)->setVisible(false);
    ui->graph_canvas->graph(4)->setVisible(false);
    ui->graph_canvas->graph(5)->setVisible(false);
    ui->graph_canvas->graph(6)->setVisible(true);
    ui->graph_canvas->graph(7)->setVisible(false);
    ui->graph_canvas->graph(8)->setVisible(false);
    ui->graph_canvas->graph(9)->setVisible(false);
    ui->graph_canvas->graph(10)->setVisible(false);
    ui->graph_canvas->graph(11)->setVisible(false); 

    jTag0->setVisibleArrow(true);
    jTag1->setVisibleArrow(false);
    jTag2->setVisibleArrow(false);
    jTag3->setVisibleArrow(false);
    jTag4->setVisibleArrow(false);
    jTag5->setVisibleArrow(false);
    vTag0->setVisibleArrow(true);
    vTag1->setVisibleArrow(false);
    vTag2->setVisibleArrow(false);
    vTag3->setVisibleArrow(false);
    vTag4->setVisibleArrow(false);
    vTag5->setVisibleArrow(false);



   break;
  }
   case 2:
 {
    ui->graph_canvas->graph(0)->setVisible(false);
    ui->graph_canvas->graph(1)->setVisible(true);
    ui->graph_canvas->graph(2)->setVisible(false);
    ui->graph_canvas->graph(3)->setVisible(false);
    ui->graph_canvas->graph(4)->setVisible(false);
    ui->graph_canvas->graph(5)->setVisible(false);
    ui->graph_canvas->graph(6)->setVisible(false);
    ui->graph_canvas->graph(7)->setVisible(true);
    ui->graph_canvas->graph(8)->setVisible(false);
    ui->graph_canvas->graph(9)->setVisible(false);
    ui->graph_canvas->graph(10)->setVisible(false);
    ui->graph_canvas->graph(11)->setVisible(false); 

    jTag0->setVisibleArrow(false);
    jTag1->setVisibleArrow(true);
    jTag2->setVisibleArrow(false);
    jTag3->setVisibleArrow(false);
    jTag4->setVisibleArrow(false);
    jTag5->setVisibleArrow(false);
    vTag0->setVisibleArrow(false);
    vTag1->setVisibleArrow(true);
    vTag2->setVisibleArrow(false);
    vTag3->setVisibleArrow(false);
    vTag4->setVisibleArrow(false);
    vTag5->setVisibleArrow(false);
   break;
 }

    case 3:
 {
    ui->graph_canvas->graph(0)->setVisible(false);
    ui->graph_canvas->graph(1)->setVisible(false);
    ui->graph_canvas->graph(2)->setVisible(true);
    ui->graph_canvas->graph(3)->setVisible(false);
    ui->graph_canvas->graph(4)->setVisible(false);
    ui->graph_canvas->graph(5)->setVisible(false);
    ui->graph_canvas->graph(6)->setVisible(false);
    ui->graph_canvas->graph(7)->setVisible(false);
    ui->graph_canvas->graph(8)->setVisible(true);
    ui->graph_canvas->graph(9)->setVisible(false);
    ui->graph_canvas->graph(10)->setVisible(false);
    ui->graph_canvas->graph(11)->setVisible(false); 

    jTag0->setVisibleArrow(false);
    jTag1->setVisibleArrow(false);
    jTag2->setVisibleArrow(true);
    jTag3->setVisibleArrow(false);
    jTag4->setVisibleArrow(false);
    jTag5->setVisibleArrow(false);
    vTag0->setVisibleArrow(false);
    vTag1->setVisibleArrow(false);
    vTag2->setVisibleArrow(true);
    vTag3->setVisibleArrow(false);
    vTag4->setVisibleArrow(false);
    vTag5->setVisibleArrow(false);
   break;
 }

    case 4:
 {
    ui->graph_canvas->graph(0)->setVisible(false);
    ui->graph_canvas->graph(1)->setVisible(false);
    ui->graph_canvas->graph(2)->setVisible(false);
    ui->graph_canvas->graph(3)->setVisible(true);
    ui->graph_canvas->graph(4)->setVisible(false);
    ui->graph_canvas->graph(5)->setVisible(false);
    ui->graph_canvas->graph(6)->setVisible(false);
    ui->graph_canvas->graph(7)->setVisible(false);
    ui->graph_canvas->graph(8)->setVisible(false);
    ui->graph_canvas->graph(9)->setVisible(true);
    ui->graph_canvas->graph(10)->setVisible(false);
    ui->graph_canvas->graph(11)->setVisible(false); 

    jTag0->setVisibleArrow(false);
    jTag1->setVisibleArrow(false);
    jTag2->setVisibleArrow(false);
    jTag3->setVisibleArrow(true);
    jTag4->setVisibleArrow(false);
    jTag5->setVisibleArrow(false);
    vTag0->setVisibleArrow(false);
    vTag1->setVisibleArrow(false);
    vTag2->setVisibleArrow(false);
    vTag3->setVisibleArrow(true);
    vTag4->setVisibleArrow(false);
    vTag5->setVisibleArrow(false);
   break;
 }
    case 5:
 {
    ui->graph_canvas->graph(0)->setVisible(false);
    ui->graph_canvas->graph(1)->setVisible(false);
    ui->graph_canvas->graph(2)->setVisible(false);
    ui->graph_canvas->graph(3)->setVisible(false);
    ui->graph_canvas->graph(4)->setVisible(true);
    ui->graph_canvas->graph(5)->setVisible(false);
    ui->graph_canvas->graph(6)->setVisible(false);
    ui->graph_canvas->graph(7)->setVisible(false);
    ui->graph_canvas->graph(8)->setVisible(false);
    ui->graph_canvas->graph(9)->setVisible(false);
    ui->graph_canvas->graph(10)->setVisible(true);
    ui->graph_canvas->graph(11)->setVisible(false); 

    jTag0->setVisibleArrow(false);
    jTag1->setVisibleArrow(false);
    jTag2->setVisibleArrow(false);
    jTag3->setVisibleArrow(false);
    jTag4->setVisibleArrow(true);
    jTag5->setVisibleArrow(false);
    vTag0->setVisibleArrow(false);
    vTag1->setVisibleArrow(false);
    vTag2->setVisibleArrow(false);
    vTag3->setVisibleArrow(false);
    vTag4->setVisibleArrow(true);
    vTag5->setVisibleArrow(false);
   break;
 }
    case 6:
 {
    ui->graph_canvas->graph(0)->setVisible(false);
    ui->graph_canvas->graph(1)->setVisible(false);
    ui->graph_canvas->graph(2)->setVisible(false);
    ui->graph_canvas->graph(3)->setVisible(false);
    ui->graph_canvas->graph(4)->setVisible(false);
    ui->graph_canvas->graph(5)->setVisible(true);
    ui->graph_canvas->graph(6)->setVisible(false);
    ui->graph_canvas->graph(7)->setVisible(false);
    ui->graph_canvas->graph(8)->setVisible(false);
    ui->graph_canvas->graph(9)->setVisible(false);
    ui->graph_canvas->graph(10)->setVisible(false);
    ui->graph_canvas->graph(11)->setVisible(true); 

    jTag0->setVisibleArrow(false);
    jTag1->setVisibleArrow(false);
    jTag2->setVisibleArrow(false);
    jTag3->setVisibleArrow(false);
    jTag4->setVisibleArrow(false);
    jTag5->setVisibleArrow(true);
    vTag0->setVisibleArrow(false);
    vTag1->setVisibleArrow(false);
    vTag2->setVisibleArrow(false);
    vTag3->setVisibleArrow(false);
    vTag4->setVisibleArrow(false);
    vTag5->setVisibleArrow(true);
   break;
 }

    case 7:
 {
    // ui->graph_canvas->graph(0)->setVisible(false);
    // ui->graph_canvas->graph(1)->setVisible(false);
    // ui->graph_canvas->graph(2)->setVisible(false);
    // ui->graph_canvas->graph(3)->setVisible(false);
    // ui->graph_canvas->graph(4)->setVisible(false);
    // ui->graph_canvas->graph(5)->setVisible(true);
    ui->graph_canvas->graph(6)->setVisible(false);
    ui->graph_canvas->graph(7)->setVisible(false);
    ui->graph_canvas->graph(8)->setVisible(false);
    ui->graph_canvas->graph(9)->setVisible(false);
    ui->graph_canvas->graph(10)->setVisible(false);
    ui->graph_canvas->graph(11)->setVisible(false); 

    // jTag0->setVisibleArrow(false);
    // jTag1->setVisibleArrow(false);
    // jTag2->setVisibleArrow(false);
    // jTag3->setVisibleArrow(false);
    // jTag4->setVisibleArrow(false);
    // jTag5->setVisibleArrow(true);
    vTag0->setVisibleArrow(false);
    vTag1->setVisibleArrow(false);
    vTag2->setVisibleArrow(false);
    vTag3->setVisibleArrow(false);
    vTag4->setVisibleArrow(false);
    vTag5->setVisibleArrow(false);
   break;
 }


  }
}

void MainWindow::pid_value(){
     std::string joint;
     QString vJoint;
     trajectory_msgs::JointTrajectory pidmsg;
        pidmsg.points.clear();
        pidmsg.joint_names.clear();
        pidmsg.points.resize(1);
        pidmsg.points[0].positions.resize(5);
        pidmsg.points[0].velocities.resize(1);


  pidmsg.points[0].positions[0] = ui->doubleSpinBox->   value();
  pidmsg.points[0].positions[1] = ui->doubleSpinBox_2-> value();
  pidmsg.points[0].positions[2] = ui->doubleSpinBox_3-> value();
  pidmsg.points[0].positions[3] = ui->doubleSpinBox_4-> value();
  pidmsg.points[0].positions[4] = ui->doubleSpinBox_5-> value();

  pidmsg.points[0].velocities[0] = ui->comboBox_2->currentIndex();




  vJoint = ui->comboBox_2->currentText();
  joint  = vJoint.toUtf8().constData();
  pidmsg.joint_names.push_back(joint);

  pid_value_pub.publish(pidmsg);

}


void MainWindow::pid_value_2()

//on_comboBox_2_currentIndexChanged(int index=0)
{
  int indexPID;
  bool enableAW = false;
  dynamic_reconfigure::ReconfigureRequest srv_req;
  dynamic_reconfigure::ReconfigureResponse srv_resp;
  dynamic_reconfigure::BoolParameter enable_param;
  dynamic_reconfigure::DoubleParameter double_param_p;
  dynamic_reconfigure::DoubleParameter double_param_i;
  dynamic_reconfigure::DoubleParameter double_param_d;  
  dynamic_reconfigure::DoubleParameter double_param_imax;  
  dynamic_reconfigure::DoubleParameter double_param_imin;  

  dynamic_reconfigure::Config conf;


  
  enable_param.name = "antiwindup";
  enable_param.value = ui->checkBox->checkState(); // enableAW;
  conf.bools.push_back(enable_param);

  double_param_p.name = "p";
  double_param_p.value = ui->doubleSpinBox->   value();
  conf.doubles.push_back(double_param_p);

  double_param_i.name = "i";
  double_param_i.value = ui->doubleSpinBox_2->   value();
  conf.doubles.push_back(double_param_i);

  double_param_d.name = "d";
  double_param_d.value = ui->doubleSpinBox_3->   value();
  conf.doubles.push_back(double_param_d);

  double_param_imax.name = "i_clamp_max";
  double_param_imax.value = ui->doubleSpinBox_4->   value();
  conf.doubles.push_back(double_param_imax);

  double_param_imin.name = "i_clamp_min";
  double_param_imin.value = ui->doubleSpinBox_5->   value();
  conf.doubles.push_back(double_param_imin);

  srv_req.config = conf;
  indexPID = ui->comboBox_2->currentIndex();

  switch (indexPID){
   case 0:
  {
      if (ros::service::call("/robot/joint_1_position_controller/pid/set_parameters", srv_req, srv_resp)) {
        ROS_INFO("call to set joint_1_position_controller parameters succeeded");
      } else {
        ROS_INFO("call to set joint_1_position_controller parameters failed");
      }

   break;
  }

   case 1:
 {
      if (ros::service::call("/robot/joint_2_position_controller/pid/set_parameters", srv_req, srv_resp)) {
        ROS_INFO("call to set joint_2_position_controller parameters succeeded");
      } else {
        ROS_INFO("call to set joint_2_position_controller parameters failed");
      }
   
   break;
 }
   case 2:
  {
      if (ros::service::call("/robot/joint_3_position_controller/pid/set_parameters", srv_req, srv_resp)) {
        ROS_INFO("call to set joint_3_position_controller parameters succeeded");
      } else {
        ROS_INFO("call to set joint_3_position_controller parameters failed");
      }    
    
   break;
  }
   case 3:
 {
      if (ros::service::call("/robot/joint_4_position_controller/pid/set_parameters", srv_req, srv_resp)) {
        ROS_INFO("call to set joint_4_position_controller parameters succeeded");
      } else {
        ROS_INFO("call to set joint_4_position_controller parameters failed");
      }
         
   break;
 }
   case 4:
 {
      if (ros::service::call("/robot/joint_5_position_controller/pid/set_parameters", srv_req, srv_resp)) {
        ROS_INFO("call to set joint_5_position_controller parameters succeeded");
      } else {
        ROS_INFO("call to set joint_5_position_controller parameters failed");
      }
  
   break;
   }

   case 5:
 {
      if (ros::service::call("/robot/joint_6_position_controller/pid/set_parameters", srv_req, srv_resp)) {
        ROS_INFO("call to set joint_6_position_controller parameters succeeded");
      } else {
        ROS_INFO("call to set joint_6_position_controller parameters failed");
      }
  
   break;
   }   
  
  }
}


void MainWindow::setUpHighlighter(){
  QFont font;
  font.setFamily("Courier");
  font.setFixedPitch(true);
  font.setPointSize(14);
  font.setBold(true);
  ui->editor->setFont(font);
  ui->editor->setTabStopWidth(fontMetrics().width(QLatin1Char('9'))*4);
  highlighter=new Highlighter(ui->editor->document());
}

void MainWindow::resizeEvent(QResizeEvent *event){
  QMainWindow::resizeEvent(event);
  ui->editor->setGeometry(10,0,width()-20,height()-ui->statusBar->height()-ui->mainToolBar->height()-80-15);
  ui->outputText->setGeometry(10,ui->editor->height()+10,this->width()-20,80);
}
void MainWindow::initFileData(){
  fileName=tr("Untitled.rrun");
  filePath=QString::fromUtf8( ros::package::getPath("interpreter_gui").c_str()) + "/Script/Untitled.rrun"; //QDir::homePath()+"/ros_qtc_plugin/src/interpreter_gui/Script";//tr("/home/yesser/ros_qtc_plugin/src/interpreter_gui"); //QCoreApplication::applicationDirPath()
  fileSaved=true;
  isRunning=false;
}
void MainWindow::undo(){
  ui->editor->undo();
}
void MainWindow::redo(){
  ui->editor->redo();
}
void MainWindow::saveFile(){
  QString savePath=QFileDialog::getSaveFileName(this,tr("Elija la ruta y nombre del archivo a guardar"),filePath,tr("Rrun File(*.rrun)"));
  if(!savePath.isEmpty()){
      QFile out(savePath);
      out.open(QIODevice::WriteOnly|QIODevice::Text);
      QTextStream str(&out);
      str<<ui->editor->toPlainText();
      out.close();
      fileSaved=true;
      QRegularExpression re(tr("(?<=\\/)\\w+\\.rrun"));
      fileName=re.match(savePath).captured();
      filePath=savePath;
      this->setWindowTitle(tr("Robot Script Editor - ")+fileName);
    }
  ui->actionError_Datos->setIcon(errorIcon);

}
void MainWindow::newFile(){
  MainWindow *newWindow=new MainWindow();
  QRect newPos=this->geometry();
  newWindow->setGeometry(newPos.x()+10,newPos.y()+10,newPos.width(),newPos.height());
  newWindow->show();
  ui->actionError_Datos->setIcon(errorIcon);
}
void MainWindow::openFile(){
  if(!fileSaved){
      if(QMessageBox::Save==QMessageBox::question(this,tr("Archivo no guardado"),tr("Guardar Archivo Actual"),QMessageBox::Save,QMessageBox::Cancel))
        saveFile();
    }
  QString openPath=QFileDialog::getOpenFileName(this,tr("Seleccione el archivo para abrir"),filePath,tr("Rrun File(*.rrun)"));
  if(!openPath.isEmpty()){
      QFile in(openPath);
      in.open(QIODevice::ReadOnly|QIODevice::Text);
      QTextStream str(&in);
      ui->editor->setPlainText(str.readAll());
      QRegularExpression re(tr("(?<=\\/)\\w+\\.rrun"));
      fileName=re.match(openPath).captured();
      this->setWindowTitle(tr("Robot Script Editor - ")+fileName);
      filePath=openPath;
      fileSaved=true;
    }
  ui->actionError_Datos->setIcon(errorIcon);
}
void MainWindow::run(){
  //    trajectory_point1.positions.clear();

  trajectory_msgs::JointTrajectory msg;
  msg.points.resize(1);
  msg.points[0].positions.resize(6);
  isloop =false;
  std::cout<< "step1" << std::endl;

  int it=0;
//       for (int j = 0; j < 6; j++) {
        msg1.points.clear();
        msg1.joint_names.clear();
        msg1.points.resize(1);
        msg1.points[0].positions.resize(6);
        msg1.points[0].velocities.resize(6);
        msg1.points[0].velocities[0] = 0;
        msg1.points[0].velocities[1] = 0;
        msg1.points[0].velocities[2] = 0;
        msg1.points[0].velocities[3] = 0;
        msg1.points[0].velocities[4] = 0;
        msg1.points[0].velocities[5] = 0;

//       } //for
//       joint_pub.publish(msg1) ; //publish nodo


  if(isRunning){
//      process.terminate();
      ui->actionRun->setIcon(runIcon);
      return;
    }
  if(!fileSaved){
//      if(QMessageBox::Save==QMessageBox::question(this,tr("Archivo no guardado"),tr("Guardar Archivo Actual？"),QMessageBox::Save,QMessageBox::Cancel))
//        saveFile();
    QFile out(filePath);
    out.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream str(&out);
    str<<ui->editor->toPlainText();
    out.close();
    fileSaved=true;

    }
  if(fileSaved){
    //if(process!=nullptr)delete process;
    isRunning=true;
    ui->statusBar->showMessage(tr("Programa En Ejecucion..."));
    ui->outputText->clear();
    output.clear();
    error.clear();
    QString buildPath;
    QRegularExpression re(tr(".*(?=\\.rrun)"));
    buildPath=re.match(filePath).captured();
//    qDebug()<<filePath;
//    process.start("bash", QStringList() << "-c" << QString(tr("g++ ")+filePath+tr(" -o ")+buildPath+tr(";")+buildPath));
//    process.waitForStarted();

    ui->outputText->setFocus();
    ui->actionRun->setIcon(stopIcon);

    std::ifstream streamcount(filePath.toStdString()); // OR "/home/yesser/ros_qtc_plugin/src/interpreter_gui/Script/firstScript.rrun"

    int points = std::count(std::istreambuf_iterator<char>(streamcount),std::istreambuf_iterator<char>(),'\n');
    std::cout<< "\n points in trajectory " << points << std::endl;

    std::ifstream stream(filePath.toStdString());
    if(stream.good()){
      std::string linea;
      while(!stream.eof()&&isRunning){
        linea.clear();
        std::getline(stream, linea);
        ROS_INFO("Proceso %s",linea.c_str());
        it++;
        msg = this->comandos(linea,it);
      }

      std::cout << "\n point = " << msg.points.size() << " " << msg  <<std::endl;
      joint_pub.publish(msg) ; //publish nodo
    }

    this->runFinished(0);


//    if (!isloop){
//    }

   } //file save
}

// void MainWindow::jointsizeCallback(const std_msgs::Float32MultiArray::ConstPtr &msglimit) //Valores de los limtes de los joints
// {
//   std::cout<<"DATAOK"<< std::endl;

// //  limit.data.resize(12);

//     limit = *msglimit;

//     Q_EMIT setvaluesSubs();


// }

void MainWindow::joint_Gz_Callback2(const control_msgs::JointControllerState &msg) //Valores de los limtes de los joints
{
  std::cout<<"gazebo joints"<< std::endl;
control_msgs::JointControllerState ms1;
//ms1.process_value_dot
    joint_1_plot = msg.process_value*ToG;
//    joint_2_plot = msg.points[0].positions[2]*ToG;
//    joint_3_plot = msg.points[0].positions[3]*ToG;
//    joint_4_plot = msg.points[0].positions[4]*ToG;
//    joint_5_plot = msg.points[0].positions[5]*ToG;
//    joint_6_plot = msg.points[0].positions[6]*ToG;

    vel_1_plot = msg.process_value_dot*ToG;
//    vel_2_plot = msg.points[1].positions[2];
//    vel_3_plot = msg.points[1].positions[3];
//    vel_4_plot = msg.points[1].positions[4];
//    vel_5_plot = msg.points[1].positions[5];
//    vel_6_plot = msg.points[1].positions[6];

  std::cout<<joint_1_plot<<"\n"<<joint_2_plot<<std::endl;

     // joint_positions_["joint_1"]= msg.points[0].positions[0]/ToG;
     // joint_positions_["joint_2"]= msg.points[0].positions[1]/ToG;
     // joint_positions_["joint_3"]= msg.points[0].positions[2]/ToG;
     // joint_positions_["joint_4"]= msg.points[0].positions[3]/ToG;
     // joint_positions_["joint_5"]= msg.points[0].positions[4]/ToG;
     // joint_positions_["joint_6"]= msg.points[0].positions[5]/ToG;


}

void MainWindow::joint_Gz_Callback3(const control_msgs::JointControllerState &msg) //Valores de los limtes de los joints
{

    joint_2_plot = msg.process_value*ToG;
    vel_2_plot = msg.process_value_dot*ToG;

}

void MainWindow::joint_Gz_Callback4(const control_msgs::JointControllerState &msg) //Valores de los limtes de los joints
{

    joint_3_plot = msg.process_value*ToG;
    vel_3_plot = msg.process_value_dot*ToG;

}

void MainWindow::joint_Gz_Callback5(const control_msgs::JointControllerState &msg) //Valores de los limtes de los joints
{

    joint_4_plot = msg.process_value*ToG;
    vel_4_plot = msg.process_value_dot*ToG;

}

void MainWindow::joint_Gz_Callback6(const control_msgs::JointControllerState &msg) //Valores de los limtes de los joints
{

    joint_5_plot = msg.process_value*ToG;
    vel_5_plot = msg.process_value_dot*ToG;

}

void MainWindow::joint_Gz_Callback7(const control_msgs::JointControllerState &msg) //Valores de los limtes de los joints
{

    joint_6_plot = msg.process_value*ToG;
    vel_6_plot = msg.process_value_dot*ToG;

}

void MainWindow::joint_Gz_Callback(const trajectory_msgs::JointTrajectory &msg) //Valores de los limtes de los joints
{
  std::cout<<"gazebo joints"<< std::endl;
// //ms1.process_value_dot
//    joint_1_plot = msg.points[0].positions[0]*ToG;
//    joint_2_plot = msg.points[0].positions[1]*ToG;
//    joint_3_plot = msg.points[0].positions[2]*ToG;
//    joint_4_plot = msg.points[0].positions[3]*ToG;
//    joint_5_plot = msg.points[0].positions[4]*ToG;
//    joint_6_plot = msg.points[0].positions[5]*ToG;

//    vel_1_plot = msg.points[1].positions[0];
//    vel_2_plot = msg.points[1].positions[1];
//    vel_3_plot = msg.points[1].positions[2];
//    vel_4_plot = msg.points[1].positions[3];
//    vel_5_plot = msg.points[1].positions[4];
//    vel_6_plot = msg.points[1].positions[5];

//   std::cout<<joint_1_plot<<"\n"<<joint_2_plot<<std::endl;

     // joint_positions_["joint_1"]= msg.points[0].positions[0]/ToG;
     // joint_positions_["joint_2"]= msg.points[0].positions[1]/ToG;
     // joint_positions_["joint_3"]= msg.points[0].positions[2]/ToG;
     // joint_positions_["joint_4"]= msg.points[0].positions[3]/ToG;
     // joint_positions_["joint_5"]= msg.points[0].positions[4]/ToG;
     // joint_positions_["joint_6"]= msg.points[0].positions[5]/ToG;


}


void MainWindow::updatevalues(){
  std::cout<<"Signal is OK"<< std::endl;
  std::string info;
  ui->outputText->clear();
  info = std::string("Valores en Grados de Cada Joint de el Robot Cargado a ROS \n");
  this->updateOutput(info);
  for (int i =0; i<6;i++)
  {
    info = std::string(">>  Joint_" + std::to_string(i+1) + "   " + std::to_string(limit.data[i]) + " Grados (°)    " + " +" + std::to_string(limit.data[i+6]) + " Grados (°) ");

    this->updateOutput(info);
  }


}


trajectory_msgs::JointTrajectory MainWindow::comandos(std::string &comando, int &ii){
  int i =ii;
  std::string info;
  std::vector<double> jointvalues(6);
  trajectory_msgs::JointTrajectoryPoint comandoP;
  comandoP.positions.resize(6);
  comandoP.velocities.resize(6);




 //Full Zero (Core Dumped)
//      for (int j = 0; j < 6; j++) {
//      msg1.points[0].positions[j] = 0.00;
//      } //for
//Heuristics end file

  std::cout << "\n I have iterator " << i <<std::endl;

  std::vector<std::string> partes=split(comando, ' ');

  if(partes.size()==0){
    std::cout<< "Line empty"<< std::endl;
//    msg1.points[0].positions[0] = 0.00;
      }else if (partes[1] == std::string("G") && partes[3] == std::string("V")){
        comandoP.velocities[0] = std::stod(partes[4]);
        comandoP.velocities[1] = std::stod(partes[4]);
        comandoP.velocities[2] = std::stod(partes[4]);
        comandoP.velocities[3] = std::stod(partes[4]);
        comandoP.velocities[4] = std::stod(partes[4]);
        comandoP.velocities[5] = std::stod(partes[4]);
        switch(partes[0][6]){
          case '1':
            if(partes.size()>1){
              std::cout << "I have it 1 " << partes[2]  <<std::endl;

              info =std::string(">>  Ejecutando Moviento del Joint 1  a  " +partes[2] + " Grados (°)");
              this->updateOutput(info);

              comandoP.positions[0] = std::stod(partes[2]);
              // Joint0.data = std::stod(partes[2])/ToG;
              // joint_pub1.publish(Joint0);

              if(comandoP.positions[0]>limit.data[0] && comandoP.positions[0]<limit.data[6] ){
                std::cout<< "is ok for now"<< std::endl;
                comandoP.positions[1] = msg1.points[i-1].positions[1];
                comandoP.positions[2] = msg1.points[i-1].positions[2];
                comandoP.positions[3] = msg1.points[i-1].positions[3];
                comandoP.positions[4] = msg1.points[i-1].positions[4];
                comandoP.positions[5] = msg1.points[i-1].positions[5];
                msg1.joint_names.push_back("joint_1");
                msg1.points.push_back(comandoP);
              }
              else {
                comandoP.positions[0] = 0.00;
                comandoP.velocities[0] = 0.00;
                msg1.joint_names.push_back("joint_1");
                msg1.points.push_back(comandoP);
                this->updateError();
                   }
            }
            break;
          case '2':
            if(partes.size()>1){
              std::cout << "I have it  2 " << partes[2]  <<std::endl;
              info =std::string(">>  Ejecutando Moviento del Joint 2  a  " +partes[2] + " Grados (°)");
              this->updateOutput(info);

              comandoP.positions[1] = std::stod(partes[2]);
              // Joint1.data = std::stod(partes[2])/ToG;
              // joint_pub2.publish(Joint1);
//              comandoP.velocities[0] = std::stod(partes[4]);
//              comandoP.velocities[1] = std::stod(partes[4]);
//              comandoP.velocities[2] = std::stod(partes[4]);
//              comandoP.velocities[3] = std::stod(partes[4]);
//              comandoP.velocities[4] = std::stod(partes[4]);
//              comandoP.velocities[5] = std::stod(partes[4]);

//              msg1.points[i-1].positions[1]. = std::stod(partes[1]);

              if(comandoP.positions[1]>limit.data[1] && comandoP.positions[1]<limit.data[7] ){
                std::cout<< "is ok for now"<< std::endl;
                comandoP.positions[0] = msg1.points[i-1].positions[0];
                comandoP.positions[2] = msg1.points[i-1].positions[2];
                comandoP.positions[3] = msg1.points[i-1].positions[3];
                comandoP.positions[4] = msg1.points[i-1].positions[4];
                comandoP.positions[5] = msg1.points[i-1].positions[5];
                msg1.joint_names.push_back("joint_2");
                msg1.points.push_back(comandoP);
              }
              else {
                comandoP.positions[1] = 0.00;
                comandoP.velocities[1] = 0.00;
                msg1.joint_names.push_back("joint_2");
                msg1.points.push_back(comandoP);
//                msg1.points[i-1].positions[1] = 0.00;
                this->updateError();
                   }
            }
            break;

        case '3':
          if(partes.size()>1){
            std::cout << "I have it  3 " << partes[1]  <<std::endl;

            info =std::string(">>  Ejecutando Moviento del Joint 3  a  " +partes[2] + " Grados (°)");
            this->updateOutput(info);

            comandoP.positions[2] = std::stod(partes[2]);
              // Joint2.data = std::stod(partes[2])/ToG;
              // joint_pub3.publish(Joint2);
            if(comandoP.positions[2]>limit.data[2] && comandoP.positions[2]<limit.data[8] ){
              std::cout<< "is ok for now"<< std::endl;
              comandoP.positions[0] = msg1.points[i-1].positions[0];
              comandoP.positions[1] = msg1.points[i-1].positions[1];
              comandoP.positions[3] = msg1.points[i-1].positions[3];
              comandoP.positions[4] = msg1.points[i-1].positions[4];
              comandoP.positions[5] = msg1.points[i-1].positions[5];
              msg1.joint_names.push_back("joint_3");
              msg1.points.push_back(comandoP);
              }
            else {
              comandoP.positions[2] = 0.00;
              comandoP.velocities[2] = 0.00;
              msg1.joint_names.push_back("joint_3");
              msg1.points.push_back(comandoP);
              this->updateError();
                 }
          }
          break;

        case '4':
          if(partes.size()>1){
            std::cout << "I have it  4 " << partes[1]  <<std::endl;

            info =std::string(">>  Ejecutando Moviento del Joint 4  a  " +partes[2] + " Grados (°)");
            this->updateOutput(info);

            comandoP.positions[3] = std::stod(partes[2]);
              // Joint3.data = std::stod(partes[2])/ToG;
              // joint_pub4.publish(Joint3);
            if(comandoP.positions[3]>limit.data[3] && comandoP.positions[3]<limit.data[9] ){
              std::cout<< "is ok for now"<< std::endl;
              comandoP.positions[0] = msg1.points[i-1].positions[0];
              comandoP.positions[1] = msg1.points[i-1].positions[1];
              comandoP.positions[2] = msg1.points[i-1].positions[2];
              comandoP.positions[4] = msg1.points[i-1].positions[4];
              comandoP.positions[5] = msg1.points[i-1].positions[5];
              msg1.joint_names.push_back("joint_4");
              msg1.points.push_back(comandoP);
              }
            else {
              comandoP.positions[3] = 0.00;
              comandoP.velocities[3] = 0.00;
              msg1.joint_names.push_back("joint_4");
              msg1.points.push_back(comandoP);
              this->updateError();
                 }
          }
          break;

        case '5':
          if(partes.size()>1){
            std::cout << "I have it  5 " << partes[1]  <<std::endl;

            info =std::string(">>  Ejecutando Moviento del Joint 5  a  " +partes[2] + " Grados (°)");
            this->updateOutput(info);

            comandoP.positions[4] = std::stod(partes[2]);
              // Joint4.data = std::stod(partes[2])/ToG;
              // joint_pub5.publish(Joint4);
            if(comandoP.positions[4]>limit.data[4] && comandoP.positions[4]<limit.data[10] ){
              std::cout<< "is ok for now"<< std::endl;
              comandoP.positions[0] = msg1.points[i-1].positions[0];
              comandoP.positions[1] = msg1.points[i-1].positions[1];
              comandoP.positions[2] = msg1.points[i-1].positions[2];
              comandoP.positions[3] = msg1.points[i-1].positions[3];
              comandoP.positions[5] = msg1.points[i-1].positions[5];
              msg1.joint_names.push_back("joint_5");
              msg1.points.push_back(comandoP);
              }
            else {
              comandoP.positions[4] = 0.00;
              comandoP.velocities[4] = 0.00;
              msg1.joint_names.push_back("joint_5");
              msg1.points.push_back(comandoP);
              this->updateError();
                 }
          }
          break;

        case '6':
          if(partes.size()>1){
            std::cout << "I have it  6 " << partes[1]  <<std::endl;

            info =std::string(">>  Ejecutando Moviento del Joint 6  a  " +partes[2] + " Grados (°)");
            this->updateOutput(info);

            comandoP.positions[5] = std::stod(partes[2]);
              // Joint5.data = std::stod(partes[2])/ToG;
              // joint_pub6.publish(Joint5);
            if(comandoP.positions[5]>limit.data[5] && comandoP.positions[5]<limit.data[11] ){
              std::cout<< "is ok for now"<< std::endl;
              comandoP.positions[0] = msg1.points[i-1].positions[0];
              comandoP.positions[1] = msg1.points[i-1].positions[1];
              comandoP.positions[2] = msg1.points[i-1].positions[2];
              comandoP.positions[3] = msg1.points[i-1].positions[3];
              comandoP.positions[4] = msg1.points[i-1].positions[4];
              msg1.joint_names.push_back("joint_6");
              msg1.points.push_back(comandoP);
              }
            else {
              comandoP.positions[5] = 0.00;
              comandoP.velocities[5] = 0.00;
              msg1.joint_names.push_back("joint_6");
              msg1.points.push_back(comandoP);
              this->updateError();
                 }
          }
          break;

          case 'm':
            //m union posicion_radianes
            //Mueve la unión hasta la posición en radianes indicada
            if(partes.size()>2){
//              robot->mover(partes[1],std::stod(partes[2]));
            }
            break;

          case 'w':
            //w [union]
            //Espera hasta que la unión halla llegado a su destino. Si no se indica la unión se espera hata terminar todas
            ROS_INFO("eseparndo");
            break;
          case 's':
            //s milisegundos
            //Duerme los milisegundos indicados.

            //de momento nos dormimos durante el tiempo indicado.
            std::string tiempo(partes[1]);
            ROS_INFO("durmiendo %d",std::stoi(tiempo));
            boost::this_thread::sleep(boost::posix_time::milliseconds(std::stoi(tiempo)));
            //usleep(std::stoi(tiempo));
            ROS_INFO("despierto");
            break;/**/
        }
      }

  if(partes.size()==0){
    std::cout<< "Line empty"<< std::endl;
  }else{
        if(partes[0]== std::string("wait()"))
        {
          std::cout<< "I have wait()"<< std::endl;
//          isloop =true;
          info =std::string(">>  Instrucción wait() a  " + partes[1] + " ms");
          this->updateOutput(info);
          comandoP.velocities[0] = std::stod(partes[1]);
//          comandoP.velocities[1] = std::stod(partes[1]);
//          comandoP.velocities[2] = std::stod(partes[1]);
//          comandoP.velocities[3] = std::stod(partes[1]);
//          comandoP.velocities[4] = std::stod(partes[1]);
//          comandoP.velocities[5] = std::stod(partes[1]);
          comandoP.positions[0] = msg1.points[i-1].positions[0];
          comandoP.positions[1] = msg1.points[i-1].positions[1];
          comandoP.positions[2] = msg1.points[i-1].positions[2];
          comandoP.positions[3] = msg1.points[i-1].positions[3];
          comandoP.positions[4] = msg1.points[i-1].positions[4];
          comandoP.positions[5] = msg1.points[i-1].positions[5];
          msg1.joint_names.push_back("wait");
          msg1.points.push_back(comandoP);
        }
        else {
          std::cout<< "I don't have wait()"<< std::endl;
             }
          }

  if(partes.size()==0){
    std::cout<< "Line empty"<< std::endl;
  }else{
        if(partes[0]== std::string("loop()"))
        {
          std::cout<< "I have loop()"<< std::endl;
          isloop =true;
          info =std::string(">>  Ejecutando todos los movimientos con Comportamiento Ciclico \nDesactiva la instrucción loop() para detener");
          this->updateOutput(info);
          // trajectory_msgs::JointTrajectory msg;
          // msg.points.push_back(comandoP);
        }
        else {
          std::cout<< "I don't have loop()"<< std::endl;
             }
          }




//  for (int k = 0; k < 6; k++) {
//   msg1.points[0].positions[k] =jointvalues[k]; //array [i]
//                              }
//  msgolder = msg1;
return msg1;
}

void MainWindow::runloop(){
  boost::mutex::scoped_lock state_pub_lock(state_pub_mutex_);
  ros::Rate loop_rate(10);

//  msgolder.points.resize(1);
//  msgolder.points[0].positions.resize(6);

  while (true){

//    for (int j = 0; j < 6; j++) {
//    msgolder.points[0].positions[j] = 0.0;
//    joint_pub.publish(msgolder);
//    } //for
    if(isloop)
    {
    std::cout<< "I'm alive"<< std::endl;
    joint_pub.publish(msg1);
    }
//  for (int i = 0; i < 6; i++) {
//  msgolder.points[0].positions[i] = msg1.points[0].positions[i];
//  joint_pub.publish(msgolder);
//  } //for
    try {
      boost::this_thread::interruption_point();
    } catch(const boost::thread_interrupted& o) {
      break; // quit the thread's loop
    }

  loop_rate.sleep();

  }//while
}


std::vector<std::string> MainWindow::split(const std::string &c, char d){
  std::vector<std::string> resultado;

  std::stringstream cs(c);
  std::string parte;
  while(std::getline(cs, parte, d)){
    resultado.push_back(parte);
  }

  return resultado;
}

void MainWindow::runFinished(int code){
  ui->actionRun->setIcon(runIcon);
  isRunning=false;
  qDebug()<<tr("exit code=")<<code;
  ui->statusBar->showMessage(tr("Ready"));

}
void MainWindow::updateOutput(std::string &info)
{
  output=QString::fromLocal8Bit(info.c_str());
  //ui->outputText->setPlainText(output+tr("\n")+error);
  ui->outputText->setPlainText(ui->outputText->toPlainText()+output+tr("... \n"));//+tr("\n"));
}
void MainWindow::updateError(){
  error=QString("Valores Fuera del espacio de trabajo \n Regresando Robot a su estado Inicial");
//  //ui->outputText->setPlainText(output+tr("\n")+error);
  ui->outputText->setPlainText(ui->outputText->toPlainText()+error);//+tr("\n"));
//  ui->actionError_Datos->isEnabled();
  ui->actionError_Datos->setIcon(RerrorIcon);
  isRunning=false;
}

void MainWindow::updateRobot(){
  error=QString("Por Favor cargue un modelo de Robot a ROS");
  ui->outputText->setPlainText(ui->outputText->toPlainText()+error);//+tr("\n"));
  ui->actionError_Datos->setIcon(errorIcon);
  isRunning=false;
}

void MainWindow::inputData(QString data){
//  if(isRunning)process.write(data.toLocal8Bit());
}
void MainWindow::closeEvent(QCloseEvent *event){
  if(!fileSaved){
      if(QMessageBox::Save==QMessageBox::question(this,tr("¿Salir sin guardar?"),tr("El archivo actual no se ha guardado"),QMessageBox::Save,QMessageBox::Cancel))
        saveFile();
      fileSaved=true;
    }
}
void MainWindow::about(){
  QMessageBox::information(this,tr("About"),tr(" Yeser M. v1.1 \n Universidad Nacional de Ingenieria \nmyalfredo03@ieee.org"),QMessageBox::Ok);
}

//CODE Heuristic
//    msg1.points.resize(points+1);
//    for (std::size_t k=0; k==points; ++k){
//      if(true){
//    msg1.points[k].positions.resize(6);
//    }
//    }
//msg1.points[i-1].positions[2] = std::stod(partes[1]);


//MainWindow::comandos Gazebo es capaz de mover el joint de una forma progresiva por lo tanto no requiero 
// Guardar el valor anterior de la posicion del joint mas que solo aplicar el avance en cada msg.point.x