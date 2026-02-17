#include "mainwindow.h"
#include "ad.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

QPixmap decodeBase64(const QString &base64) {
    if (base64.isEmpty()) return QPixmap();
    QPixmap img;
    img.loadFromData(QByteArray::fromBase64(base64.toLatin1()));
    return img;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(1200,800);
    this->move(400,0);
    ui->App_control->setCurrentIndex(1);
    ////////////////////////////////////////////////////////////////////////
    QPixmap server_page(":/Images/conncet_back.png");
    QPalette palette1;
    palette1.setBrush(QPalette::Window, QBrush(server_page));
    ui->connect->setPalette(palette1);
    ui->connect->setAutoFillBackground(true);

    QPixmap main_page(":/Images/main_back.png");
    QPalette palette2;
    palette2.setBrush(QPalette::Window, QBrush(main_page));
    ui->Ads->setPalette(palette2);
    ui->Ads->setAutoFillBackground(true);

    db = new DatabaseManager(this);

    setupTables();
    populateTables();
}

void MainWindow::setupTables()
{
    QStringList headers = {"Image", "ID", "Title", "Seller", "Price", "Date", "Status"};

    // 2. Styling CSS (Modern Look)
    QString tableStyle =
        "QTableWidget { "
        "   background-color: #ffffff; "
        "   alternate-background-color: #f2f2f2; " // Zebra striping
        "   gridline-color: #e0e0e0; "
        "   selection-background-color: #3d8ec9; " // Nice Blue selection
        "   selection-color: white; "
        "   font-size: 13px; "
        "   border: 1px solid #dcdcdc; "
        "}"
        "QHeaderView::section { "
        "   background-color: #f8f9fa; " // Light gray header
        "   padding: 6px; "
        "   border: 1px solid #dcdcdc; "
        "   font-weight: bold; "
        "   font-size: 13px; "
        "   color: #333; "
        "}";

    // --- Helper to Apply Settings to a Table ---
    auto configureTable = [&](QTableWidget* table) {
        table->setColumnCount(headers.size());
        table->setHorizontalHeaderLabels(headers);
        table->setStyleSheet(tableStyle);

        // UX Improvements
        table->setAlternatingRowColors(true);
        table->setSelectionMode(QAbstractItemView::ExtendedSelection);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->verticalHeader()->setVisible(false); // Hide ugly row numbers
        table->setShowGrid(false); // Clean look
        table->setFocusPolicy(Qt::NoFocus); // Removes dotted line on click

        // Dimensions
        table->verticalHeader()->setDefaultSectionSize(90); // Taller rows for images
        table->setIconSize(QSize(80, 80));

        // Column Resizing
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // Image
        table->setColumnWidth(0, 100);
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed); // ID
        table->setColumnWidth(1, 50); // Make ID column small
    };

    // --- Apply to Both Tables ---
    configureTable(ui->Pending_table);
    configureTable(ui->AllAds_Table);

}

void MainWindow::populateTables()
{
    ui->Pending_table->setRowCount(0);
    ui->AllAds_Table->setRowCount(0);

    // Disable sorting while populating to prevent crashes
    ui->Pending_table->setSortingEnabled(false);
    ui->AllAds_Table->setSortingEnabled(false);

    QVector<Ad> allAds = db->getAllAds();

    for (const Ad &ad : allAds) {
        QTableWidget *targetTable = nullptr;
        if (ad.status == AdStatus::PENDING) targetTable = ui->Pending_table;
        else targetTable = ui->AllAds_Table;

        int row = targetTable->rowCount();
        targetTable->insertRow(row);

        QTableWidgetItem *imgItem = new QTableWidgetItem();
        QPixmap pix = decodeBase64(ad.imageBase64); // Your helper function
        if (!pix.isNull()) {
            imgItem->setIcon(QIcon(pix.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        } else {
            imgItem->setText("No Image");
            imgItem->setTextAlignment(Qt::AlignCenter);
        }
        targetTable->setItem(row, 0, imgItem);

        targetTable->setItem(row, 1, new QTableWidgetItem(ad.id));

        targetTable->setItem(row, 2, new QTableWidgetItem(ad.title));

        targetTable->setItem(row, 3, new QTableWidgetItem(ad.sellerUsername));

        QString priceStr = QLocale(QLocale::English).toString((qlonglong)ad.price);
        targetTable->setItem(row, 4, new QTableWidgetItem(priceStr + " Tokens"));

        targetTable->setItem(row, 5, new QTableWidgetItem(ad.date));

        QTableWidgetItem *statusItem = new QTableWidgetItem();
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setFont(QFont("Segoe UI", 10, QFont::Bold)); // Bold font

        if (ad.status == AdStatus::PENDING) {
            statusItem->setText("PENDING");
            statusItem->setForeground(QBrush(QColor(255, 140, 0))); // Dark Orange
        } else if(ad.status == AdStatus::APPROVED) {
            statusItem->setText("APPROVED");
            statusItem->setForeground(QBrush(QColor(34, 139, 34))); // Forest Green
        }
        else if(ad.status == AdStatus::REJECTED) {
            statusItem->setText("REJECTED");
            statusItem->setForeground(QBrush(QColor(255, 20, 20))); // Red
        }
        else if(ad.status == AdStatus::SOLD) {
            statusItem->setText("SOLD");
            statusItem->setForeground(QBrush(QColor(0, 0, 139))); // Blue
        }

        targetTable->setItem(row, 6, statusItem);
    }

    ui->Pending_table->setSortingEnabled(true);
    ui->AllAds_Table->setSortingEnabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Approve_button_clicked()
{
    // 1. Get list of selected rows
    // Using selectionModel is safer for multiple rows
    QModelIndexList selection = ui->Pending_table->selectionModel()->selectedRows();

    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Action", "Please select at least one ad.");
        return;
    }

    // 2. Loop through every selected row
    for (const QModelIndex &index : selection) {
        int row = index.row();
        // ID is now in Column 1 (Column 0 is image)
        QString adId = ui->Pending_table->item(row, 1)->text();

        db->updateAdStatus(adId, AdStatus::APPROVED);
    }
    db->saveAds();
    db->loadAds();
    // 3. Refresh only once at the end
    populateTables();
    QMessageBox::information(this, "Success", QString("Approved %1 ads.").arg(selection.count()));
}


void MainWindow::on_Reject_button_clicked()
{
    QModelIndexList selection = ui->Pending_table->selectionModel()->selectedRows();

    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Action", "Please select at least one ad.");
        return;
    }

    for (const QModelIndex &index : selection) {
        int row = index.row();
        QString adId = ui->Pending_table->item(row, 1)->text(); // ID is in Column 1

        db->updateAdStatus(adId, AdStatus::REJECTED);
    }

    db->loadAds();
    populateTables();
    QMessageBox::information(this, "Success", QString("Rejected %1 ads.").arg(selection.count()));
}


void MainWindow::on_Reject_button_2_clicked()
{
    db->loadAds();
    populateTables();
}

