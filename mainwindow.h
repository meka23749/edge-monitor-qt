#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDateTime>
#include <QFont>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QFile>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QProcess>

#define MAX_HISTORY 30
#define GPIO_COUNT 4

class SensorWidget : public QGroupBox {
    Q_OBJECT
public:
    SensorWidget(const QString& name, const QString& unit,
                 double threshold, QWidget* parent = nullptr);
    void updateValue(double value);
    bool isAlarm() const { return alarm; }

private:
    QLabel* valueLabel;
    QLabel* statusLabel;
    QProgressBar* bar;
    QString unit;
    double threshold;
    bool alarm;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    void updateSensors();
    void toggleMonitoring();

private:
    double readCpuTemp();
    double readCpuUsage();
    double readRamUsage();
    double readDiskUsage();
    void updateChart();
    void logToCsv(double temp, double cpu, double ram, double disk);

    SensorWidget* tempWidget;
    SensorWidget* cpuWidget;
    SensorWidget* ramWidget;
    SensorWidget* diskWidget;
    /* GPIO */
    QGroupBox* gpioBox;
    QLabel* gpioLabels[GPIO_COUNT];
    QLabel* gpioStateLabels[GPIO_COUNT];
    void updateGpio();
    void updateLeds(bool alarm);
    bool readGpioPin(int pin);

    QLabel* stateLabel;
    QLabel* pollLabel;
    QLabel* alarmLabel;
    QLabel* timeLabel;

    QPushButton* startStopBtn;
    QTimer* timer;

    /* Chart */
    QChartView* chartView;
    QLineSeries* tempSeries;
    QLineSeries* cpuSeries;
    QLineSeries* ramSeries;
    QValueAxis* axisX;
    QValueAxis* axisY;

    int pollCount;
    int alarmCount;
    bool monitoring;
};

#endif