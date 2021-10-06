#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutexLocker>
#include "game.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class GameThread : public QThread
{
    Q_OBJECT
public:
    GameThread(Game& game) : QThread(), mGame(game) {}
private:
    void run() override
    {
        Game::Position pos;
        Game::UiSign uiSign = Game::UiSign::None;
        mGame.ExecuteCpuMove(pos, uiSign);
        if (!isInterruptionRequested())
        {
            emit CpuResultReady(pos.mX, pos.mY, static_cast<int>(uiSign));
        }
    }
signals:
    void CpuResultReady(int posx, int posY, int uiSign);
private:
    Game& mGame;
};

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum
    {
        OptionsIndex = 0,
        GameIndex = 1,
    };

    enum Status
    {
        None,
        SetOptions,
        UserMove,
        UserWon,
        CpuMove,
        CpuWon,
        Tied,
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void on_actionNew_Game_triggered();
    void on_psStart_clicked();

    void ExecuteCpuMove();
    void CpuResultReady(int posx, int posY, int uiSign);

private:
    void GoToOptions();
    void GoToGame();
    void PlayerClicked();
    void SetStatus(Status status);
    void ProcessButton(QPushButton* pb, Game::UiSign sign);

private:
    Ui::MainWindow *ui;
    QList<QPushButton*> mButtons;
    GameThread mWorker;
    QMutex mUserMutex;
    Game mGame;

};
#endif // MAINWINDOW_H
