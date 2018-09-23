#include <QSettings>
#include <QFile>
#include <QtDebug>
#include <QCloseEvent>

#include "appinfo.h"
#include "appconfig.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if ( meta_config_key_exists(LAST_OPENED_WINDOW_SIZE) ) {
        this->restoreGeometry( meta_config_value(LAST_OPENED_WINDOW_SIZE).toByteArray() );
    }

    if ( meta_config_key_exists(MAIN_SCREEN_LAYOUT) ) {
        ui->mainSplitter->restoreState( meta_config_value(MAIN_SCREEN_LAYOUT).toByteArray() );
    } else {
        // Set the default window size
        QList<int> splitterSizes = {0,0,0};
        int window_width = this->width();
        splitterSizes[0] = window_width / 6;     // First split is 1/6 of the screen
        splitterSizes[1] = window_width / 6;     // Second split is 1/6 of the screen
        splitterSizes[2] = window_width / 6 * 4; // Last split occupies 4/6 of the screen
        ui->mainSplitter->setSizes(splitterSizes);
    }

    // Remove margin on toolbar on Mac OS X
#ifdef Q_OS_MAC
    ui->customToolbar->layout()->setContentsMargins(0,0,0,0);
#endif
    // Hide the header of the notebook tree widget
    ui->notebookTreeWidget->header()->hide();

    QListWidget *list = ui->noteList;
    QListWidgetItem *myitem = new QListWidgetItem(list);

    myitem->setText("Hello");

    QPushButton *mybutton =  new QPushButton("Push me", this);
    myitem->setSizeHint(QSize(myitem->sizeHint().width(), 100));
    list->setItemWidget(myitem, mybutton);

    connect(ui->userButton, &QPushButton::clicked,
            this, &MainWindow::userButtonClicked);
}

MainWindow::~MainWindow()
{

    set_meta_config_value( LAST_OPENED_WINDOW_SIZE, this->saveGeometry() );
    set_meta_config_value( MAIN_SCREEN_LAYOUT, ui->mainSplitter->saveState() );

    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    m_user_window.close();
    event->accept();
}

void MainWindow::userButtonClicked()
{
    m_user_window.show();
}
