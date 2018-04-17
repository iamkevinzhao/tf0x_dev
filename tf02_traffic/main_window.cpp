#include "main_window.h"
#include "ui_main_window.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QDebug>
#include "tf0x_common/distance_over_time_chart.h"
#include <QValueAxis>

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  this->setWindowTitle("TF02 Traffic");
  this->resize(1000, 800);
  chart_ = new tf0x_common::DistanceOverTimeAndSwitchValueChart();

  // chart_->createDefaultAxes();
  // chart->setTitle("Simple line chart example");

  QChartView *chartView = new QChartView(chart_);
  chartView->setRenderHint(QPainter::Antialiasing);

  QChartView* chartview = new QChartView;

  ui->ChartVerticalLayout->addWidget(chartView);
//  chartView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//  chartView->setMaximumSize(400, 400);
//  chartView->setMinimumSize(400, 400);
  // chartView->setVisible(false);

//  QCameraViewfinder* camera_view = new QCameraViewfinder;
//  QCamera* camera = new QCamera(QCameraInfo::defaultCamera());
//  camera->setViewfinder(camera_view);
//  camera->start();
  // ui->VideoVerticalLayout->addWidget(camera_view);
//  ui->ChartVerticalLayout->addWidget(camera_view);
//  camera_view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//  camera_view->setMaximumSize(400, 400);
//  camera_view->setMinimumSize(400, 400);
  // camera_view->setMinimumWidth(800);
//  ui->gridLayout_2->addWidget(camera_view);
//  ui->gridLayout_2->addWidget(chartView);

  traffic_count_.reset(new tf02_common::TrafficCount);
  traffic_count_->Initialize();
  traffic_count_->Reset();

  rate_elapsed_timer_.start();

  timer_id_ = startTimer(10);

  driver_.Initialize();
  elapsed_timer_.start();
}

MainWindow::~MainWindow()
{
  delete ui;
  delete chart_;
}

void MainWindow::timerEvent(QTimerEvent *event) {
  if (event->timerId() != timer_id_) {
    return QMainWindow::timerEvent(event);
  }
  double dist;
  if (!driver_.ReadDistance(dist)) {
    return;
  }
  auto elapsed = elapsed_timer_.elapsed();
  chart_->AddPoint(dist, elapsed);
  if (traffic_count_) {
    if (traffic_count_->Probe(dist)) {
      chart_->AddSwitchValuePoint(true, elapsed);
    } else {
      chart_->AddSwitchValuePoint(false, elapsed);
    }
    ui->TotalCountLabel->setText(QString::number(traffic_count_->Total()));
    auto rate = traffic_count_->Total() / (rate_elapsed_timer_.elapsed() / 60000.0);
    rate = std::floor(rate);
    ui->RateLabel->setText(QString::number(int(rate)) + " / Min");
  }
}

void MainWindow::on_ResetPushButton_clicked()
{
  if (traffic_count_) {
    traffic_count_->Reset();
  }
  rate_elapsed_timer_.restart();
}
