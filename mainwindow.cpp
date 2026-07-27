#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QStyleFactory>
#include <QRandomGenerator>
#include <QStorageInfo>
#include <QRegularExpression>

/* ===== SensorWidget ===== */
SensorWidget::SensorWidget(const QString& name, const QString& unit,
                           double threshold, QWidget* parent)
    : QGroupBox(parent), unit(unit), threshold(threshold)
{
    setTitle(name);
    setStyleSheet(
        "QGroupBox { font-weight: bold; border: 2px solid #3a3a5c;"
        "border-radius: 8px; margin-top: 12px; padding: 15px; padding-top: 25px;"
        "background-color: #1e1e3a; color: #cccccc; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px;"
        "padding: 0 5px; color: #0abde3; }"
        );

    QVBoxLayout* layout = new QVBoxLayout(this);

    valueLabel = new QLabel("--");
    valueLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #0abde3;");
    valueLabel->setAlignment(Qt::AlignCenter);

    bar = new QProgressBar();
    bar->setRange(0, static_cast<int>(threshold * 1.5));
    bar->setValue(0);
    bar->setTextVisible(false);
    bar->setFixedHeight(8);
    bar->setStyleSheet(
        "QProgressBar { background-color: #2a2a4a; border-radius: 4px; }"
        "QProgressBar::chunk { background-color: #0abde3; border-radius: 4px; }"
        );

    statusLabel = new QLabel("OK");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(
        "background-color: #0a3d2e; color: #2ecc71;"
        "border-radius: 10px; padding: 3px 12px; font-size: 11px; font-weight: bold;"
        );
    statusLabel->setFixedWidth(80);

    QLabel* threshLabel = new QLabel(QString("Threshold: %1 %2").arg(threshold).arg(unit));
    threshLabel->setStyleSheet("color: #555555; font-size: 11px;");
    threshLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(valueLabel);
    layout->addWidget(bar);
    layout->addWidget(threshLabel);

    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->addStretch();
    statusLayout->addWidget(statusLabel);
    statusLayout->addStretch();
    layout->addLayout(statusLayout);
}

void SensorWidget::updateValue(double value)
{
    valueLabel->setText(QString("%1 %2").arg(value, 0, 'f', 1).arg(unit));
    bar->setValue(static_cast<int>(value));

    bool alarm = value > threshold;

    if (alarm) {
        valueLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #ee5a24;");
        statusLabel->setText("ALARM");
        statusLabel->setStyleSheet(
            "background-color: #3d0a0a; color: #ee5a24;"
            "border-radius: 10px; padding: 3px 12px; font-size: 11px; font-weight: bold;"
            );
        bar->setStyleSheet(
            "QProgressBar { background-color: #2a2a4a; border-radius: 4px; }"
            "QProgressBar::chunk { background-color: #ee5a24; border-radius: 4px; }"
            );
        setStyleSheet(
            "QGroupBox { font-weight: bold; border: 2px solid #ee5a24;"
            "border-radius: 8px; margin-top: 12px; padding: 15px; padding-top: 25px;"
            "background-color: #2d1f1f; color: #cccccc; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px;"
            "padding: 0 5px; color: #ee5a24; }"
            );
    } else {
        valueLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #0abde3;");
        statusLabel->setText("OK");
        statusLabel->setStyleSheet(
            "background-color: #0a3d2e; color: #2ecc71;"
            "border-radius: 10px; padding: 3px 12px; font-size: 11px; font-weight: bold;"
            );
        bar->setStyleSheet(
            "QProgressBar { background-color: #2a2a4a; border-radius: 4px; }"
            "QProgressBar::chunk { background-color: #0abde3; border-radius: 4px; }"
            );
        setStyleSheet(
            "QGroupBox { font-weight: bold; border: 2px solid #3a3a5c;"
            "border-radius: 8px; margin-top: 12px; padding: 15px; padding-top: 25px;"
            "background-color: #1e1e3a; color: #cccccc; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px;"
            "padding: 0 5px; color: #0abde3; }"
            );
    }
}

/* ===== MainWindow ===== */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), pollCount(0), alarmCount(0), monitoring(false)
{
    setWindowTitle("Industrial Edge Monitor - Qt Dashboard");
    setMinimumSize(700, 500);
    setStyleSheet("background-color: #1a1a2e; color: #eeeeee;");

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 15, 20, 15);

    /* Title */
    QLabel* title = new QLabel("Industrial Edge Monitor");
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #0abde3;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QLabel* subtitle = new QLabel("Real-time System Monitoring | C++ Qt");
    subtitle->setStyleSheet("font-size: 12px; color: #666666;");
    subtitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitle);

    /* Status bar */
    QHBoxLayout* statusBar = new QHBoxLayout();
    statusBar->setSpacing(30);

    stateLabel = new QLabel("State: IDLE");
    stateLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #0abde3;");
    pollLabel = new QLabel("Polls: 0");
    pollLabel->setStyleSheet("font-size: 13px; color: #cccccc;");
    alarmLabel = new QLabel("Alarms: 0");
    alarmLabel->setStyleSheet("font-size: 13px; color: #cccccc;");
    timeLabel = new QLabel("");
    timeLabel->setStyleSheet("font-size: 13px; color: #cccccc;");

    statusBar->addStretch();
    statusBar->addWidget(stateLabel);
    statusBar->addWidget(pollLabel);
    statusBar->addWidget(alarmLabel);
    statusBar->addWidget(timeLabel);
    statusBar->addStretch();
    mainLayout->addLayout(statusBar);

    /* Sensor grid */
    QHBoxLayout* topRow = new QHBoxLayout();
    QHBoxLayout* bottomRow = new QHBoxLayout();

    tempWidget = new SensorWidget("CPU Temperatur", "\u00B0C", 70.0);
    cpuWidget  = new SensorWidget("CPU Auslastung", "%", 80.0);
    ramWidget  = new SensorWidget("RAM Auslastung", "%", 85.0);
    diskWidget = new SensorWidget("Disk Auslastung", "%", 90.0);

    topRow->addWidget(tempWidget);
    topRow->addWidget(cpuWidget);
    bottomRow->addWidget(ramWidget);
    bottomRow->addWidget(diskWidget);

    mainLayout->addLayout(topRow);
    mainLayout->addLayout(bottomRow);

    /* Start/Stop button */
    startStopBtn = new QPushButton("START MONITORING");
    startStopBtn->setFixedHeight(40);
    startStopBtn->setStyleSheet(
        "QPushButton { background-color: #0abde3; color: white;"
        "border: none; border-radius: 8px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #0899b5; }"
        );
    connect(startStopBtn, &QPushButton::clicked, this, &MainWindow::toggleMonitoring);
    mainLayout->addWidget(startStopBtn);

    /* Footer */
    QLabel* footer = new QLabel("Industrial Edge Monitor v1.0 | Steve Meka");
    footer->setStyleSheet("font-size: 10px; color: #444444;");
    footer->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footer);

    /* Timer */
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSensors);
}

void MainWindow::toggleMonitoring()
{
    if (monitoring) {
        timer->stop();
        monitoring = false;
        startStopBtn->setText("START MONITORING");
        startStopBtn->setStyleSheet(
            "QPushButton { background-color: #0abde3; color: white;"
            "border: none; border-radius: 8px; font-size: 14px; font-weight: bold; }"
            "QPushButton:hover { background-color: #0899b5; }"
            );
        stateLabel->setText("State: IDLE");
        stateLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #0abde3;");
    } else {
        timer->start(2000);
        monitoring = true;
        startStopBtn->setText("STOP MONITORING");
        startStopBtn->setStyleSheet(
            "QPushButton { background-color: #ee5a24; color: white;"
            "border: none; border-radius: 8px; font-size: 14px; font-weight: bold; }"
            "QPushButton:hover { background-color: #c44719; }"
            );
        updateSensors();
    }
}

/* ===== Read real system metrics ===== */
double MainWindow::readCpuTemp()
{
#ifdef Q_OS_LINUX
    QFile file("/sys/class/thermal/thermal_zone0/temp");
    if (file.open(QIODevice::ReadOnly)) {
        QString val = file.readAll().trimmed();
        file.close();
        return val.toDouble() / 1000.0;
    }
#endif
    return 45.0 + QRandomGenerator::global()->bounded(25.0);
}

double MainWindow::readCpuUsage()
{
#ifdef Q_OS_LINUX
    static qint64 prevIdle = 0, prevTotal = 0;
    QFile file("/proc/stat");
    if (file.open(QIODevice::ReadOnly)) {
        QString line = file.readLine();
        file.close();
        QStringList parts = line.split(QRegularExpression("\\s+"));
        if (parts.size() >= 8) {
            qint64 user = parts[1].toLongLong();
            qint64 nice = parts[2].toLongLong();
            qint64 system = parts[3].toLongLong();
            qint64 idle = parts[4].toLongLong();
            qint64 total = user + nice + system + idle;
            double usage = 0;
            if (total - prevTotal > 0)
                usage = (1.0 - double(idle - prevIdle) / double(total - prevTotal)) * 100.0;
            prevIdle = idle;
            prevTotal = total;
            return usage;
        }
    }
#endif
    return QRandomGenerator::global()->bounded(60.0);
}

double MainWindow::readRamUsage()
{
#ifdef Q_OS_LINUX
    QFile file("/proc/meminfo");
    if (file.open(QIODevice::ReadOnly)) {
        qint64 total = 0, available = 0;
        while (!file.atEnd()) {
            QString line = file.readLine();
            if (line.startsWith("MemTotal:"))
                total = line.split(QRegularExpression("\\s+"))[1].toLongLong();
            if (line.startsWith("MemAvailable:"))
                available = line.split(QRegularExpression("\\s+"))[1].toLongLong();
        }
        file.close();
        if (total > 0)
            return (1.0 - double(available) / double(total)) * 100.0;
    }
#endif
    return 30.0 + QRandomGenerator::global()->bounded(40.0);
}

double MainWindow::readDiskUsage()
{
#ifdef Q_OS_LINUX
    QStorageInfo storage("/");
    if (storage.isValid())
        return (1.0 - double(storage.bytesFree()) / double(storage.bytesTotal())) * 100.0;
#endif
    return 40.0 + QRandomGenerator::global()->bounded(30.0);
}

void MainWindow::updateSensors()
{
    double temp = readCpuTemp();
    double cpu  = readCpuUsage();
    double ram  = readRamUsage();
    double disk = readDiskUsage();

    tempWidget->updateValue(temp);
    cpuWidget->updateValue(cpu);
    ramWidget->updateValue(ram);
    diskWidget->updateValue(disk);

    pollCount++;
    bool anyAlarm = (temp > 70.0 || cpu > 80.0 || ram > 85.0 || disk > 90.0);
    if (anyAlarm) alarmCount++;

    pollLabel->setText(QString("Polls: %1").arg(pollCount));
    alarmLabel->setText(QString("Alarms: %1").arg(alarmCount));
    timeLabel->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));

    if (anyAlarm) {
        stateLabel->setText("State: ALARM");
        stateLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #ee5a24;");
        alarmLabel->setStyleSheet("font-size: 13px; color: #ee5a24;");
    } else {
        stateLabel->setText("State: RUNNING");
        stateLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2ecc71;");
        alarmLabel->setStyleSheet("font-size: 13px; color: #cccccc;");
    }
}