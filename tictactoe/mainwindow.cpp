#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QTimer>

static const char* sXPos = "xpos";
static const char* sYPos = "ypos";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mWorker(mGame)
{
    ui->setupUi(this);    
    connect(&mWorker, &GameThread::CpuResultReady, this, &MainWindow::CpuResultReady);
    GoToOptions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("TicTacToe"), tr("TicTacToe About us..."));
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit();
}

void MainWindow::on_actionNew_Game_triggered()
{
    GoToOptions();
}

void MainWindow::SetStatus(Status status)
{
    switch (status)
    {
    case SetOptions: ui->statusbar->showMessage(tr("Set options for next game")); break;
    case UserMove: ui->statusbar->showMessage(tr("Your move, click a tile")); break;
    case UserWon: ui->statusbar->showMessage(tr("You won! Congratulations!")); break;
    case CpuMove: ui->statusbar->showMessage(tr("CPU thinking..")); break;
    case CpuWon: ui->statusbar->showMessage(tr("You lost! Better luck next time!")); break;
    case Tied: ui->statusbar->showMessage(tr("It is a draw!")); break;
    case None:
    default: ui->statusbar->clearMessage(); break;
    }
}

void MainWindow::GoToOptions()
{
    mWorker.requestInterruption();
    SetStatus(SetOptions);
    ui->stackedWidget->setCurrentIndex(OptionsIndex);
}

void MainWindow::GoToGame()
{
    int gridSize = ui->pbGridSize->text().toInt();
    mGame = Game(ui->cbEasyMode->isChecked(), ui->cbCpuFirst->isChecked(), gridSize);

    if (mButtons.size() != (gridSize * gridSize))
    {
        for (auto& button : mButtons)
            delete button;

        mButtons.clear();
        mButtons.reserve(gridSize * gridSize);

        for (int i = 0; i != mGame.GetGridSize(); i++)
        {
            for (int j = 0; j != mGame.GetGridSize(); j++)
            {
                QPushButton* pb = new QPushButton(this);

                QFont f = pb->font();
                f.setPointSize(20);

                pb->setFont(f);
                pb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                pb->setProperty(sXPos, i);
                pb->setProperty(sYPos, j);
                pb->setText(" ");

                ui->glGame->addWidget(pb, i, j);
                mButtons.push_back(pb);
                connect(pb, &QPushButton::clicked, this, &MainWindow::PlayerClicked);
            }
        }
    }
    else
    {
        for (auto& button : mButtons)
            button->setText(" ");
    }

    ProcessButton(nullptr, Game::UiSign::None);

    ui->stackedWidget->setCurrentIndex(GameIndex);
}

void MainWindow::on_psStart_clicked()
{
    GoToGame();
}

void MainWindow::ProcessButton(QPushButton* pb, Game::UiSign sign)
{
    if (pb)
    {
        switch (sign)
        {
        case Game::UiSign::None: pb->setText(" "); break;
        case Game::UiSign::X: pb->setText("X"); break;
        case Game::UiSign::O: pb->setText("O"); break;
        }
    }

    switch (mGame.GetPlayerAtMove())
    {
    case Game::PlayerEntity::None:
    {
        switch (mGame.GetWinner())
        {
        case Game::PlayerEntity::None: SetStatus(Tied); break;
        case Game::PlayerEntity::User: SetStatus(UserWon); break;
        case Game::PlayerEntity::Cpu: SetStatus(CpuWon); break;
        }
        break;
    }
    case Game::PlayerEntity::User: SetStatus(UserMove); break;
    case Game::PlayerEntity::Cpu:
        SetStatus(CpuMove);
        QTimer::singleShot(1, this, &MainWindow::ExecuteCpuMove); // 1ms delay to process status message
        break;
    }
}

void MainWindow::PlayerClicked()
{
    if (!mWorker.isRunning())
    {
        QPushButton* pb = qobject_cast<QPushButton*>(QObject::sender());
        Game::Position p{pb->property(sXPos).toInt(), pb->property(sYPos).toInt()};
        if (mGame.UserCanMove(p))
        {
            mGame.SetMove(p);
            ProcessButton(pb, mGame.GetUiSign(p));
        }
        else if (mGame.GetPlayerAtMove() == Game::PlayerEntity::None)
        {
            GoToOptions();
        }
    }
}

void MainWindow::ExecuteCpuMove()
{
    mWorker.start();
}

void MainWindow::CpuResultReady(int posx, int posY, int uiSign)
{
    ProcessButton(mButtons[posx * mGame.GetGridSize() + posY], static_cast<Game::UiSign>(uiSign));
}

