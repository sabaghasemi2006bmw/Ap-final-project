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
    refreshDiscountTable();
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

void MainWindow::refreshDiscountTable()
{
    // 1. Get the Table Widget (Assume you named it 'table_discounts' in Designer)
    QTableWidget *table = ui->table_discounts;

    // 2. Setup Headers (Only needs to be done once, but safe to repeat)
    if (table->columnCount() == 0) {
        QStringList headers = {"Code", "Percentage", "Expiration Date", "Status"};
        table->setColumnCount(headers.size());
        table->setHorizontalHeaderLabels(headers);

        // Styling
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers); // Read-only
    }

    // 3. Clear Old Data
    table->setRowCount(0);

    // 4. Get Data from Database
    QVector<DiscountCode> codes = db->getAllDiscounts();

    // 5. Populate Table
    QDate today = QDate::currentDate();

    for (const DiscountCode &dc : codes) {
        int row = table->rowCount();
        table->insertRow(row);

        // Col 0: Code
        QTableWidgetItem *itemCode = new QTableWidgetItem(dc.code);
        itemCode->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 0, itemCode);

        // Col 1: Percentage
        QTableWidgetItem *itemPercent = new QTableWidgetItem(QString::number(dc.percentage) + "%");
        itemPercent->setTextAlignment(Qt::AlignCenter);
        // Color code high discounts
        if (dc.percentage >= 50) itemPercent->setForeground(Qt::darkGreen);
        table->setItem(row, 1, itemPercent);

        // Col 2: Expiration Date
        QTableWidgetItem *itemDate = new QTableWidgetItem(dc.expirationDate.toString("yyyy-MM-dd"));
        itemDate->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 2, itemDate);

        // Col 3: Status (Valid vs Expired)
        QTableWidgetItem *itemStatus = new QTableWidgetItem();
        itemStatus->setTextAlignment(Qt::AlignCenter);

        if (dc.expirationDate < today) {
            itemStatus->setText("Expired");
            itemStatus->setForeground(Qt::red);
        } else {
            itemStatus->setText("Active");
            itemStatus->setForeground(Qt::blue);
        }
        table->setItem(row, 3, itemStatus);
    }
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


void MainWindow::on_pushButton_clicked()
{
    QString code = ui->lineEdit_3->text().trimmed().toUpper();
    int percent = ui->spinBox->value();
    QDate expiry = ui->dateEdit->date();

    if (code.isEmpty()) return;

    DiscountCode dc;
    dc.code = code;
    dc.percentage = percent;
    dc.expirationDate = expiry;

    db->addDiscount(dc);

    QMessageBox::information(this, "Success", "Coupon Added: " + code);

    refreshDiscountTable();
}

