#ifndef NOTES_H
#define NOTES_H

#include <QMainWindow>
#include "userwindow.h"
#include "notelistmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent (QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    UserWindow m_user_window;
    NoteListManager *m_note_list_manager;

public slots:
    void userButtonClicked();
};

#endif // NOTES_H