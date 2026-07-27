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
#include <QFile>
#include <QTextStream>
#include <QFont>

class SensorWidget : public QGroupBox {
    Q_OBJECT
public:
    SensorWidget(const QString& name, const QString& unit,
                 double threshold, QWidget* parent = nullptr);
    void updateValue(double value);

private:
    QLabel* nameLabel;
    QLabel* valueLabel;
    QLabel* statusLabel;
    QProgressBar* bar;
    QString unit;
    double threshold;
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

    SensorWidget* tempWidget;
    SensorWidget* cpuWidget;
    SensorWidget* ramWidget;
    SensorWidget* diskWidget;

    QLabel* stateLabel;
    QLabel* pollLabel;
    QLabel* alarmLabel;
    QLabel* timeLabel;

    QPushButton* startStopBtn;
    QTimer* timer;

    int pollCount;
    int alarmCount;
    bool monitoring;
};

#endif // MAINWINDOW_H