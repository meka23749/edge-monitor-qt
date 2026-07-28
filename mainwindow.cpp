#include "mainwindow.h"
#include <QApplication>
#include <QRandomGenerator>
#include <QTextStream>
#include <QProcess>

/* ===== SensorWidget ===== */
SensorWidget::SensorWidget(const QString& name, const QString& unit,
                           double threshold, QWidget* parent)
    : QGroupBox(parent), unit(unit), threshold(threshold), alarm(false)
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
    valueLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #0abde3;");
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

    alarm = value > threshold;

    if (alarm) {
        valueLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #ee5a24;");
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
        valueLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #0abde3;");
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
    setMinimumSize(900, 800);
    setStyleSheet("background-color: #1a1a2e; color: #eeeeee;");

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);
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

    /* GPIO Status */
    gpioBox = new QGroupBox("GPIO Status");
    gpioBox->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 2px solid #3a3a5c;"
        "border-radius: 8px; margin-top: 12px; padding: 15px; padding-top: 25px;"
        "background-color: #1e1e3a; color: #cccccc; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px;"
        "padding: 0 5px; color: #0abde3; }"
        );
    QHBoxLayout* gpioLayout = new QHBoxLayout(gpioBox);

    for (int i = 0; i < GPIO_COUNT; i++) {
        QVBoxLayout* pinLayout = new QVBoxLayout();
        gpioLabels[i] = new QLabel("--");
        gpioLabels[i]->setStyleSheet("font-size: 13px; color: #cccccc; font-weight: bold;");
        gpioLabels[i]->setAlignment(Qt::AlignCenter);

        gpioStateLabels[i] = new QLabel("--");
        gpioStateLabels[i]->setAlignment(Qt::AlignCenter);
        gpioStateLabels[i]->setFixedWidth(70);

        pinLayout->addWidget(gpioLabels[i]);
        pinLayout->addWidget(gpioStateLabels[i]);
        gpioLayout->addLayout(pinLayout);
    }

    mainLayout->addWidget(gpioBox);

    /* Chart */
    tempSeries = new QLineSeries();
    tempSeries->setName("Temperature");
    tempSeries->setColor(QColor("#ee5a24"));

    cpuSeries = new QLineSeries();
    cpuSeries->setName("CPU %");
    cpuSeries->setColor(QColor("#0abde3"));

    ramSeries = new QLineSeries();
    ramSeries->setName("RAM %");
    ramSeries->setColor(QColor("#2ecc71"));

    QChart* chart = new QChart();
    chart->addSeries(tempSeries);
    chart->addSeries(cpuSeries);
    chart->addSeries(ramSeries);
    chart->setTitle("Sensor History");
    chart->setTitleBrush(QBrush(QColor("#cccccc")));
    chart->setBackgroundBrush(QBrush(QColor("#16213e")));
    chart->legend()->setLabelColor(QColor("#cccccc"));
    chart->setAnimationOptions(QChart::NoAnimation);

    axisX = new QValueAxis();
    axisX->setTitleText("Time (s)");
    axisX->setRange(0, MAX_HISTORY * 2);
    axisX->setLabelFormat("%d");
    axisX->setLabelsColor(QColor("#888888"));
    axisX->setTitleBrush(QBrush(QColor("#888888")));
    axisX->setGridLineColor(QColor("#2a2a4a"));

    axisY = new QValueAxis();
    axisY->setTitleText("Value");
    axisY->setRange(0, 100);
    axisY->setLabelsColor(QColor("#888888"));
    axisY->setTitleBrush(QBrush(QColor("#888888")));
    axisY->setGridLineColor(QColor("#2a2a4a"));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    tempSeries->attachAxis(axisX);
    tempSeries->attachAxis(axisY);
    cpuSeries->attachAxis(axisX);
    cpuSeries->attachAxis(axisY);
    ramSeries->attachAxis(axisX);
    ramSeries->attachAxis(axisY);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setFixedHeight(200);
    chartView->setStyleSheet("background-color: #16213e; border-radius: 8px;");

    mainLayout->addWidget(chartView);

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

/* ===== Read system metrics ===== */
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

/* ===== Update chart ===== */
void MainWindow::updateChart()
{
    double timePoint = pollCount * 2.0;

    tempSeries->append(timePoint, readCpuTemp());
    cpuSeries->append(timePoint, readCpuUsage());
    ramSeries->append(timePoint, readRamUsage());

    /* Keep only last MAX_HISTORY points */
    while (tempSeries->count() > MAX_HISTORY) {
        tempSeries->remove(0);
        cpuSeries->remove(0);
        ramSeries->remove(0);
    }

    /* Scroll X axis */
    if (timePoint > MAX_HISTORY * 2) {
        axisX->setRange(timePoint - MAX_HISTORY * 2, timePoint);
    }
}

/* ===== Log to CSV ===== */
void MainWindow::logToCsv(double temp, double cpu, double ram, double disk)
{
    QFile file("monitor_log.csv");
    bool isNew = !file.exists();

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        if (isNew) {
            stream << "timestamp,cpu_temp,cpu_usage,ram_usage,disk_usage,state\n";
        }
        QString state = (temp > 70.0 || cpu > 80.0 || ram > 85.0) ? "ALARM" : "OK";
        stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << ","
               << QString::number(temp, 'f', 1) << ","
               << QString::number(cpu, 'f', 1) << ","
               << QString::number(ram, 'f', 1) << ","
               << QString::number(disk, 'f', 1) << ","
               << state << "\n";
        file.close();
    }
}

/* ===== Read GPIO pin via libgpiod v2 ===== */
bool MainWindow::readGpioPin(int pin)
{
#ifdef Q_OS_LINUX
    /* BeagleY-AI GPIO mapping:
     * GPIO8  = chip 1, line 0
     * GPIO7  = chip 1, line 9
     * GPIO22 = chip 2, line 41
     * GPIO17 = chip 3, line 8
     */
    int chip = -1, line = -1;
    switch (pin) {
    case 8:  chip = 1; line = 0;  break;
    case 7:  chip = 1; line = 9;  break;
    case 22: chip = 2; line = 41; break;
    case 17: chip = 3; line = 8;  break;
    default: return false;
    }

    QProcess proc;
    proc.start("gpioget", QStringList() << "-c" << QString::number(chip) << QString::number(line));
    proc.waitForFinished(500);
    QString output = proc.readAllStandardOutput().trimmed();
    return output.contains("active");
#endif
    return QRandomGenerator::global()->bounded(2) == 1;
}

/* ===== Update GPIO display ===== */
void MainWindow::updateGpio()
{
    int pins[GPIO_COUNT] = {8, 7, 22, 17};
    QString pinNames[GPIO_COUNT] = {"GPIO8", "GPIO7", "GPIO22", "GPIO17"};

    for (int i = 0; i < GPIO_COUNT; i++) {
        bool state = readGpioPin(pins[i]);
        gpioLabels[i]->setText(pinNames[i]);

        if (state) {
            gpioStateLabels[i]->setText("HIGH");
            gpioStateLabels[i]->setStyleSheet(
                "background-color: #0a3d2e; color: #2ecc71;"
                "border-radius: 10px; padding: 3px 12px;"
                "font-size: 11px; font-weight: bold;"
                );
        } else {
            gpioStateLabels[i]->setText("LOW");
            gpioStateLabels[i]->setStyleSheet(
                "background-color: #2a2a4a; color: #888888;"
                "border-radius: 10px; padding: 3px 12px;"
                "font-size: 11px; font-weight: bold;"
                );
        }
    }
}

void MainWindow::updateLeds(bool alarm)
{
#ifdef Q_OS_LINUX
    QFile actLed("/sys/class/leds/ACT/brightness");
    QFile pwrLed("/sys/class/leds/PWR/brightness");

    if (actLed.open(QIODevice::WriteOnly)) {
        actLed.write(alarm ? "0" : "1");
        actLed.close();
    }
    if (pwrLed.open(QIODevice::WriteOnly)) {
        pwrLed.write(alarm ? "1" : "0");
        pwrLed.close();
    }
#endif
}

/* ===== Update all sensors ===== */
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

    /* Update chart */
    updateChart();
    updateGpio();

    /* Log to CSV */
    logToCsv(temp, cpu, ram, disk);

    bool anyAlarm = (tempWidget->isAlarm() || cpuWidget->isAlarm() ||
                     ramWidget->isAlarm() || diskWidget->isAlarm());
    if (anyAlarm) alarmCount++;

    updateLeds(anyAlarm);

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