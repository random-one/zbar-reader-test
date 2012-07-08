#ifndef ZBARREADERTEST_H
#define ZBARREADERTEST_H

#include <QMainWindow>
#include <QFile>
#include <QFuture>

#include "ui_ZBarReaderTest.h"

class ZBarReaderTest : public QMainWindow, Ui::ZBarReaderTest
{
    Q_OBJECT
    
public:
    explicit ZBarReaderTest(QWidget *parent = 0);
    ~ZBarReaderTest();
    
private slots:
    void init();
    void enableGenerateInputFile();
    void enableReadFromFile();
    void selectInputDir();
    void selectInputFile();
    void decode();
    void decodeIterative();
    void writeInputFile();

private:
    QString decode(const QImage &image);
    QString decodeIterative(const QImage &image);
    void loadFiles();

private:
    QString mInputDir;
    QString mInputFile;
    int mTotalRead;
    QFile mLogFile;
    QStringList mFiles;
    QFuture <QString> *future;
    QFutureWatcher <QString> *watcher;
};

#endif // ZBARREADERTEST_H
