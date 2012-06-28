#include "ZBarReaderTest.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>

#include <time.h>

#include <zbar.h>
#include <zbar/Image.h>

using namespace zbar;
using namespace std;

ZBarReaderTest::ZBarReaderTest(QWidget *parent) :
    QMainWindow(parent),
    mTotalRead(0),
    mLogFile("zbar-reader-report.txt")
{
    setupUi(this);

    init();

    generateInputFileCheck->setChecked(false);
    enableGenerateInputFile();
    readFromFileCheck->setChecked(false);
    enableReadFromFile();
}

ZBarReaderTest::~ZBarReaderTest()
{
}

void ZBarReaderTest::init()
{
    connect(generateInputFileCheck, SIGNAL(toggled(bool)), this, SLOT(enableGenerateInputFile()));
    connect(readFromFileCheck, SIGNAL(toggled(bool)), this, SLOT(enableReadFromFile()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(selectInputDir()));
    connect(browseBtn, SIGNAL(clicked()), this, SLOT(selectInputFile()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(decodeAction, SIGNAL(triggered()), this, SLOT(decode()));
    connect(decodeIterativeAction, SIGNAL(triggered()), this, SLOT(decodeIterative()));
    connect(writeFileBtn, SIGNAL(clicked()), this, SLOT(writeInputFile()));
}

void ZBarReaderTest::enableGenerateInputFile()
{
    bool enable = generateInputFileCheck->isChecked();

    numberOfImagesLabel->setEnabled(enable);
    numberOfImagesEdit->setEnabled(enable);
    writeFileBtn->setEnabled(enable);

    QDir inputDir(mInputDir);
    QStringList files = inputDir.entryList(QStringList() << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp");
    totalImagesEdit->setText(QString::number(files.size()));
}

void ZBarReaderTest::enableReadFromFile()
{
    bool enable = readFromFileCheck->isChecked();

    inputFileLabel->setEnabled(enable);
    inputFileEdit->setEnabled(enable);
    browseBtn->setEnabled(enable);
}

void ZBarReaderTest::selectInputDir()
{
    QFileDialog fileDialog(this, tr("Select input image directory"), QApplication::applicationDirPath());
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly);
    if (fileDialog.exec()) {
        mInputDir = fileDialog.selectedFiles().takeFirst();
        inputDirEdit->setText(mInputDir);
    }
    enableGenerateInputFile();
}

void ZBarReaderTest::selectInputFile()
{
    QFileDialog fileDialog(this, tr("Select input file"), QApplication::applicationDirPath());
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    if (fileDialog.exec()) {
        mInputFile = fileDialog.selectedFiles().takeFirst();
        inputFileEdit->setText(mInputFile);
    }
}

void ZBarReaderTest::decode()
{
    mTotalRead = 0;
    QStringList files;
    if (!readFromFileCheck->isChecked()) {
        QDir inputDir(mInputDir);
        files = inputDir.entryList(QStringList() << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp");
        files.sort();
        mLogFile.setFileName(QString("zbar-reader-report-%1-%2-decode.txt").arg(files.size()).arg(QDir(mInputDir).dirName()));
    } else {
        QFile f(mInputFile);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Unable to open input file"), QMessageBox::Ok);
            return;
        }
        QString content = f.readAll();
        files = content.split("\n", QString::SkipEmptyParts);
        qDebug () << files;
        mLogFile.setFileName(QString("zbar-reader-report-%1-%2-decode.txt").arg(files.size()).arg(QDir(mInputDir).dirName()));
    }
    if (!mLogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to open log file"));
        return;
    }
    QTextStream stream(&mLogFile);

    int totalFiles = files.size();

    QVector<QRgb>colorTable;
    for (int x = 0; x < 256; x++)
        colorTable << qRgb(x,x,x);

    QTime timer;
    timer.start();
    for (int x = 0; x < files.size(); x++) {
        QString line = QString::number(x + 1) + " / " + QString::number(totalFiles) + " : " + files[x].split("/").takeLast();
        QImage img;
        if (!readFromFileCheck->isChecked())
            img = QImage(mInputDir + "/" + files[x]);
        else
            img = QImage(files[x]);
        if (img.isNull()) {
            line += " | Invalid image";
            continue;
        }
        QImage tmp = img.convertToFormat(QImage::Format_Indexed8, colorTable);

        QString result = decode(tmp);
        if (result.isEmpty())
            line += " | Unable to decode";
        else
            line += result;

//        qDebug () << line;
        stream << line << endl;
        resultOutputEdit->append(line);
        resultOutputEdit->textCursor ().movePosition (QTextCursor::End);
        QCoreApplication::sendPostedEvents();
    }
    int time = (double)timer.elapsed() / 1000.;
    QString res = QString("Total read: %1\nTime elapsed: %2 s.\n").arg(mTotalRead).arg(time);
//    qDebug () << res;
    stream << res;
    mLogFile.close();
    resultOutputEdit->append(res);
}

void ZBarReaderTest::decodeIterative()
{
    mTotalRead = 0;
    QStringList files;
    if (!readFromFileCheck->isChecked()) {
        QDir inputDir(mInputDir);
        files = inputDir.entryList(QStringList() << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp");
        files.sort();
        mLogFile.setFileName(QString("zbar-reader-report-%1-%2-decode.txt").arg(files.size()).arg(QDir(mInputDir).dirName()));
    } else {
        QFile f(mInputFile);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Unable to open input file"), QMessageBox::Ok);
            return;
        }
        QString content = f.readAll();
        files = content.split("\n", QString::SkipEmptyParts);
        qDebug () << files;
        mLogFile.setFileName(QString("zbar-reader-report-%1-%2-force-decode.txt").arg(files.size()).arg(QDir(mInputDir).dirName()));
    }
    if (!mLogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to open log file"));
        return;
    }
    QTextStream stream(&mLogFile);

    QVector<QRgb>colorTable;
    for (int x = 0; x < 256; x++)
        colorTable << qRgb(x,x,x);

    QTime timer;
    timer.start();
    int totalFiles = files.size();
    for (int x = 0; x < totalFiles; x++) {
        QString line = QString::number(x + 1) + " / " + QString::number(totalFiles) + " : " + files[x].split("/").takeLast();
        QImage img(mInputDir + "/" + files[x]);
        if (img.isNull()) {
            line += " | Invalid image";
            continue;
        }
        QImage tmp = img.convertToFormat(QImage::Format_Indexed8, colorTable);

        QString result = decode(tmp);
        if (result.isEmpty())
            result = decode(tmp, true);
        if (result.isEmpty())
            line += " | Unable to decode";
        else
            line += result;

//        qDebug () << line.split("/").takeLast();
        stream << line << endl;
        resultOutputEdit->append(line);
        resultOutputEdit->textCursor ().movePosition (QTextCursor::End);
        QCoreApplication::sendPostedEvents();
    }
    int time = (double)timer.elapsed() / 1000.;
    QString res = QString("Total read: %1\nTime elapsed: %2 s.\n").arg(mTotalRead).arg(time);
//    qDebug () << res;
    stream << res;
    mLogFile.close();
    resultOutputEdit->append(res);
}

QString ZBarReaderTest::decode(const QImage &image, bool useHints)
{
    Image zbarImg(image.width(), image.height(), "Y800", image.bits(), image.bytesPerLine() * image.height());

    QString line;
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    if (!useHints) {
      scanner.scan(zbarImg);
    } else {
        int fib1 = 1, fib2 = 1;
        int fib;
        for (fib = fib1 + fib2; fib < 11; fib = fib1 + fib2) {
            int roiWidth =  (1. / fib) * image.width(), roiHeight = (1. / fib) * image.height();
            for (int x = 0; x < image.width(); x += 50) {
                for (int y = 0; y < image.height(); y += 50) {
//                  qDebug () << "roiWidth" << roiWidth << "roiHeight" << roiHeight;
                  if (roiWidth + x > image.width())
                      roiWidth = image.width() - x;
                  if (roiHeight + y > image.height())
                      roiHeight = image.height() - y;
                    QImage img = image.copy(x, y, roiWidth, roiHeight);
                    Image i(img.width(), img.height(), "Y800", img.bits(), img.bytesPerLine() * img.height());
                    scanner.scan(i);
                }
            }
            int temp = fib1;
            fib1 = fib;
            fib2 = temp;
        }
    }
    SymbolSet s = scanner.get_results();
    int resSize = s.get_size();
    if (resSize > 0) {
        mTotalRead++;
        for (SymbolIterator symbol = scanner.get_results().symbol_begin(); symbol != scanner.get_results().symbol_end(); ++symbol) {
            if (QString::compare("QR-Code ", QString(symbol->get_type_name().data()).trimmed()))
                line += " | " + QString(symbol->get_data().data()).replace("\n", "<br>");
        }
    }
    return line;
}

void ZBarReaderTest::writeInputFile()
{
    QDir inputDir(mInputDir);
    if (!inputDir.exists()) {
        QMessageBox::critical(this, "Error", "Invalid input directory", QMessageBox::Ok);
        return;
    }
    QStringList files = inputDir.entryList(QStringList() << "*.png" << "*.jpeg" << "*.jpg" << "*.bmp");
    files.sort();
    totalImagesEdit->setText(QString::number(files.size()));
    QFile inputFile(QString("%1-%2.txt").arg(inputDir.path().simplified()).arg(numberOfImagesEdit->text().toInt()));
    if (!inputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Unable to create input file.");
        return;
    }
    QTextStream stream(&inputFile);
    qsrand(time(NULL));
    QStringList list;
    do {
        QString file = mInputDir + "/" + files[rand() % files.size()] + "\n";
        if (!list.contains(file)) {
            list << file;
            stream << file;
        }
    } while (list.size() < numberOfImagesEdit->text().toInt());

    inputFile.close();
}
