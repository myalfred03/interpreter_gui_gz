#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "highlighter.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <ros/ros.h>
#include <trajectory_msgs/JointTrajectory.h>
#include <std_msgs/Float32MultiArray.h>
#include <boost/bind.hpp>
#include <std_msgs/String.h>
#include "axistag.h"

//#include <moveit/robot_trajectory/robot_trajectory.h>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  ros::Publisher joint_pub,pid_value_pub;
  // ros::Subscriber joint_sub_limit;
  ros::Subscriber joint_sub_gazebo;

  ros::NodeHandle nh_;
  std_msgs::Float32MultiArray limit ;
  void runloop();

  void initializeGraph();
  double x_org;
  ros::Time startTime;
  double joint_1_plot, joint_2_plot, joint_3_plot, joint_4_plot, joint_5_plot, joint_6_plot;
  double vel_1_plot, vel_2_plot, vel_3_plot, vel_4_plot, vel_5_plot, vel_6_plot;

  QTimer *timer;




//  void publishJointStates();

private:
  QIcon runIcon;
  QIcon stopIcon;
  QIcon errorIcon;
  QIcon RerrorIcon;
  Ui::MainWindow *ui;
  Highlighter *highlighter;
  AxisTag *mTag1;
  AxisTag *mTag11;
  AxisTag *mTag2;
  QPointer<QCPGraph> mGraph1;
  QPointer<QCPGraph> mGraph11;
  QPointer<QCPGraph> mGraph2;


//  QProcess process;
  void setUpHighlighter();
  QString fileName;
  QString filePath;
  bool fileSaved;
  bool isRunning;
  bool isloop;
  //bool fileEdited;
  void initFileData();

  //remember Points[i].positions.resize(x)
  //moveit_core/robot_trajectory/src/robottrajectory.cpp
//  moveit_msgs::RobotTrajectory   trajectory;


  trajectory_msgs::JointTrajectory comandos(std::string &comando, int &i);
  trajectory_msgs::JointTrajectory msg1, msgolder;
  std::vector<std::string> split(const std::string &c, char d);
  void trajectory(const trajectory_msgs::JointTrajectory &msg);

  //--------Based on works of Robolegs------------------
  boost::shared_ptr<ros::AsyncSpinner> spinner;
  //----------------------------------------------------

  boost::thread* publisher_thread_;
  boost::mutex state_pub_mutex_;

  std::vector<double> jointvaluesOLD = {0.0,0.0,0.0,0.0,0.0,0.0};
  bool firstLoad;
  //-----------------------------


  //---------code running data---
  QString output;
  QString error;
  //-----------------------------

  //---------Read Data to File for move robot-------
//  std::ifstream file("~/Documents/Untitled.rrun");
  //------------------------------------------------
public slots:

  void mouseMoved(QMouseEvent * event);
  void updateGraph();
  void mouseWheel();
  void removeAllGraphs();
  void changeSaveState(){
    //qDebug()<<"changed";
    if(firstLoad&&fileSaved){
        this->setWindowTitle(tr("Robot Editor Script - ")+fileName);
        firstLoad=false;
        return;
      }
    fileSaved=false;
    this->setWindowTitle(tr("Robot Editor Script - ")+fileName+tr("*"));
  }
  // void jointsizeCallback(const std_msgs::Float32MultiArray::ConstPtr &msglimit);
  void joint_Gz_Callback(const trajectory_msgs::JointTrajectory &msg);
  void newFile();
  void saveFile();
  void openFile();
  void undo();
  void redo();
  void run();

  //------------------------------
  void runFinished(int code);
  void updateError();
  void updateRobot();
  void about();
  void updateOutput(std::string &info);

public Q_SLOTS:
  void updatevalues();
  void on_comboBox_currentIndexChanged(int index);
  void pid_value();


Q_SIGNALS:
  void setvaluesSubs();

public:
  void inputData(QString data);
protected:
  void resizeEvent(QResizeEvent* event)override;
  void closeEvent(QCloseEvent* event)override;
};

#endif // MAINWINDOW_H
