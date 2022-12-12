#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QDialog>
#include <QLabel>
#include "customitems.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QGridLayout *g = new QGridLayout(this->centralWidget());
    g -> addWidget(ui -> selectSingleFileButton, 0, 0, 1, 1);
    g -> addWidget(ui -> selectMultipleFilesButton, 0, 1, 1, 1);
    g -> addWidget(ui -> selectFolderButton, 0, 2, 1, 1);
    g -> addWidget(ui -> clearButton, 0, 3, 1, 1);
    g -> addWidget(ui -> dataHolder, 1, 0, 4, 4);
    ui -> dataHolder -> setColumnCount(5);
    ui -> dataHolder -> setHorizontalHeaderLabels({"имя", "размер", "разрешение", "глубина цвета", "сжатие"});
    ui -> dataHolder -> horizontalHeader() -> setSectionResizeMode(QHeaderView::Stretch);
    statusBar()->showMessage("Ничего не выбрано");
    setWindowTitle("Характеристики изображений");
    dirFilter << "*.jpg" << "*.gif" << "*.tif" << "*.bmp" << "*.png" << "*.pcx";
    compression["bmp"] = "без сжатия";
    compression["gif"] = "LZW сжатие";
    compression["png"] = "Deflate сжатие";
    compression["jpg"] = "JPEG сжатие";
    compression["tiff"] = "ZIP/LZW/JPEG сжатие";
    compression["pcx"] = "RLE сжатие";
    connect(ui -> dataHolder, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(tableItemClicked(int,int)));
}

void MainWindow::tableItemClicked(int row, int column)
{
    for (int i = 0; i < ui->dataHolder->columnCount(); ++i)
    {
        ui->dataHolder-> item(row, i) -> setSelected(false);
    }
    QDialog *d = new QDialog(this);
    QHBoxLayout diagLayout(d);
    QPixmap px(dynamic_cast<NameItem*>(ui -> dataHolder -> item(row, 0)) -> filePath);
    QLabel label;
    label.setPixmap(px);
    diagLayout.addWidget(&label);
    d->exec();

}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectFolderButton_clicked()
{
    QString newDirPath = QFileDialog::getExistingDirectory(this, "Выберите папку", lastDir);
    if (!newDirPath.isEmpty())
    {
        QDir newDir(newDirPath);
        newDir.setNameFilters(dirFilter);
        QFileInfoList fileList(newDir.entryInfoList(QDir::Filter::Files));
        QStringList filepaths;
        for (const auto& x : fileList)
        {
            filepaths.append(x.absoluteFilePath());
        }
        AppendDataIntoTable(filepaths, true);
    }
}

void MainWindow::on_selectSingleFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выбрать изображение", lastDir, "Выбрать изображения (*.jpg *.gif *.tif *.bmp *.png *.pcx)");
    if (!fileName.isNull())
    {
        AppendDataIntoTable({fileName});
    }
}

void MainWindow::on_selectMultipleFilesButton_clicked()
{
    QStringList newFileNames = QFileDialog::getOpenFileNames(this, "Выбрать изображения", lastDir, "Выбрать изображения (*.jpg *.gif *.tif *.bmp *.png *.pcx)");
    if (!newFileNames.isEmpty())
    {
         AppendDataIntoTable(newFileNames);
    }
}
void MainWindow::AppendDataIntoTable(const QStringList &list, bool isDir)
{
    ui -> dataHolder -> sortByColumn(-1, Qt::AscendingOrder);
    int cnt = 0;
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo info(list[i]);
        if (fileDirs[info.absolutePath()].find(info.fileName()) == fileDirs[info.absolutePath()].end())
        {
            QImage image(list[i]);
            if (!image.isNull())
            {
                lastDir = info.absolutePath();
                fileDirs[lastDir].insert(info.fileName());
                ui -> dataHolder -> insertRow(ui -> dataHolder -> rowCount());
                int pos = ui -> dataHolder -> rowCount() - 1;
                ui -> dataHolder -> setItem(pos, 0, new NameItem(info.fileName(), list[i]));
                ui -> dataHolder -> setItem(pos, 1, new SizeItem(QString::number(image.size().width()) + " X " + QString::number(image.size().height()), image.size().width(), image.size().height()));
                ui -> dataHolder -> setItem(pos, 2, new QTableWidgetItem(QString::number(static_cast<int>(std::min(image.dotsPerMeterX(), image.dotsPerMeterY()) / 39.37))));
                ui -> dataHolder -> setItem(pos, 3, new QTableWidgetItem(QString::number(image.depth())));
                ui -> dataHolder -> setItem(pos, 4, new QTableWidgetItem(compression[info.suffix()]));
            }
            cnt++;
        }
    }
    if (isDir)
        statusBar()->showMessage("Выбрана папка: " + QDir(lastDir).dirName() + ", добавлен(о) " + QString::number(cnt) + " файл(ов), пропущено дубликатов: " + QString::number(list.size() - cnt));
    else
        statusBar()->showMessage("Добавлен(о) " + QString::number(cnt) + " файл(ов), пропущено дубликатов: " + QString::number(list.size() - cnt), messageTimeout);
}
void MainWindow::on_clearButton_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui -> dataHolder -> selectedItems();
    if (selectedItems.size() == 0)
    {
        if (ui -> dataHolder -> rowCount() != 0)
        {
            statusBar() -> showMessage("Удалено " + QString::number(ui -> dataHolder -> rowCount()) + " файлов", messageTimeout);
            ui -> dataHolder -> clearContents();
            ui -> dataHolder -> setRowCount(0);
            fileDirs.clear();
        }
        else
        {
            statusBar() -> showMessage("Нечего удалять", messageTimeout);
        }
    }
    else
    {
        int columns = ui->dataHolder->columnCount();
        for (int i = selectedItems.size() / columns - 1; i >= 0; --i)
        {
            int row = selectedItems[columns * i]->row();
            QFileInfo info(dynamic_cast<NameItem*>(ui -> dataHolder -> item(row, 0)) -> filePath);
            fileDirs[info.absolutePath()].erase(info.fileName());
            ui -> dataHolder -> removeRow(row);
        }
        statusBar() -> showMessage("Удалено " + QString::number(selectedItems.size() / columns) + " файлов", messageTimeout);
    }
}

