#include <opencv2/opencv.hpp>
#include "Jogo.h"
#include "Pipe.h"
#include "Jogador.h"
#include <fstream>
#include <cstdlib>

using namespace std;
using namespace cv;

bool colidiu(Rect a, Rect b) {
    return (a & b).area() > 0;
}

int sortearGap(int frameH, int gapSize) {
    int margem = 80;
    int minGap = margem;
    int maxGap = frameH - gapSize - margem;
    return minGap + (rand() % (maxGap - minGap));
}

int carregarHighScore(const string& arquivo) {
    ifstream f(arquivo);
    int hs = 0;
    if (f.is_open()) f >> hs;
    return hs;
}

void salvarHighScore(const string& arquivo, int hs) {
    ofstream f(arquivo);
    if (f.is_open()) f << hs;
}

int main(int argc, const char** argv)
{
    srand(time(0));
    VideoCapture capture;
    Mat frame;
    CascadeClassifier cascade;
    char key = 0;
    bool tryflip = true;

    string cascadeName = "haarcascade_frontalface_default.xml";

    if (!cascade.load(cascadeName)) {
        cout << "ERROR: Could not load classifier cascade: " << cascadeName << endl;
        return -1;
    }

    if (!capture.open(0)) {
        cout << "Capture from camera #0 didn't work" << endl;
        return 1;
    }

    capture >> frame;
    if (frame.empty()) {
        cout << "Erro ao capturar frame inicial" << endl;
        return 1;
    }

    int frameH  = frame.rows;
    int frameW  = frame.cols;
    int gapSize = 160;

    Mat iuriOriginal = imread("iurilindo.png", IMREAD_UNCHANGED);
    int iuriW = 80;
    int iuriH = 80;
    if (!iuriOriginal.empty()) {
        double proporcao = (double)iuriOriginal.rows / iuriOriginal.cols;
        iuriW = 80;
        iuriH = (int)(iuriW * proporcao);
    }

    const string wName = "Flappy Bird";
    namedWindow(wName, WINDOW_KEEPRATIO);
    resizeWindow(wName, frameW, frameH);

    int gapStart = sortearGap(frameH, gapSize);
    Pipe* pipeD  = new Pipe("PipeD.png", 500, -10,                gapStart + 10);
    Pipe* pipeU  = new Pipe("PipeU.png", 500, gapStart + gapSize, frameH - gapStart - gapSize + 10);

    Jogador jogador("iurilindo.png", iuriW, iuriH, frameW / 2);

    bool gameOver  = false;
    int  pontuacao = 0;

    const string hsArquivo = "highscore.txt";
    int highScore = carregarHighScore(hsArquivo);

    auto reiniciar = [&]() {
        pontuacao = 0;
        gameOver  = false;
        gapStart  = sortearGap(frameH, gapSize);
        delete pipeD;
        delete pipeU;
        pipeD = new Pipe("PipeD.png", frameW, -10,                gapStart + 10);
        pipeU = new Pipe("PipeU.png", frameW, gapStart + gapSize, frameH - gapStart - gapSize + 10);
    };

    while (1)
    {
        capture >> frame;
        if (frame.empty()) break;

        Mat smallFrame;
        resize(frame, smallFrame, Size(), 1.0, 1.0, INTER_LINEAR_EXACT);
        if (tryflip)
            flip(smallFrame, smallFrame, 1);

        if (!gameOver)
        {
            bool resetou = pipeD->update(smallFrame);
            pipeU->update(smallFrame);

            if (resetou) {
                system("mplayer dragon-studio-correct-472358.mp3 &");
                cout<<"Tocando som"<<endl;
                pontuacao++;
                gapStart = sortearGap(frameH, gapSize);
                pipeD->setPos(-10,                gapStart + 10);
                pipeU->setPos(gapStart + gapSize, frameH - gapStart - gapSize + 10);
                pipeD->aumentarDificuldade();
                pipeU->aumentarDificuldade();
            }

            jogador.update(smallFrame, cascade);

            if (colidiu(jogador.getRect(), pipeD->getRect()) ||
                colidiu(jogador.getRect(), pipeU->getRect()))
                gameOver = true;

            putText(smallFrame, "Pontos: " + to_string(pontuacao),
                    Point(20, 40), FONT_HERSHEY_COMPLEX, 1.2, Scalar(0, 0, 0), 4);
            putText(smallFrame, "Pontos: " + to_string(pontuacao),
                    Point(20, 40), FONT_HERSHEY_COMPLEX, 1.2, Scalar(255, 255, 255), 2);

            putText(smallFrame, "Best: " + to_string(highScore),
                    Point(20, 75), FONT_HERSHEY_COMPLEX, 1.0, Scalar(0, 0, 0), 3);
            putText(smallFrame, "Best: " + to_string(highScore),
                    Point(20, 75), FONT_HERSHEY_COMPLEX, 1.0, Scalar(0, 215, 255), 1);
        }
        else
        {
            if (pontuacao > highScore) {
                highScore = pontuacao;
                salvarHighScore(hsArquivo, highScore);
            }

            Mat overlay = smallFrame.clone();
            rectangle(overlay, Rect(0, 0, smallFrame.cols, smallFrame.rows),
                      Scalar(0, 0, 0), -1);
            addWeighted(overlay, 0.5, smallFrame, 0.5, 0, smallFrame);

            int cx = smallFrame.cols / 2;
            int cy = smallFrame.rows / 2;

            putText(smallFrame, "GAME OVER",
                    Point(cx-40, cy ),
                    FONT_HERSHEY_COMPLEX, 2.0, Scalar(0, 0, 255), 4);

            putText(smallFrame, "Pontuacao: " + to_string(pontuacao),
                    Point(cx - 120, cy),
                    FONT_HERSHEY_COMPLEX, 1.2, Scalar(255, 255, 255), 2);

            putText(smallFrame, "High Score: " + to_string(highScore),
                    Point(cx - 120, cy + 35),
                    FONT_HERSHEY_COMPLEX, 1.0, Scalar(0, 215, 255), 2);

            putText(smallFrame, "Aperte R para reiniciar",
                    Point(cx - 170, cy + 80),
                    FONT_HERSHEY_PLAIN, 1.8, Scalar(200, 200, 200), 2);

            putText(smallFrame, "Aperte Q para sair",
                    Point(cx - 130, cy + 120),
                    FONT_HERSHEY_PLAIN, 1.8, Scalar(200, 200, 200), 2);

            if (key == 'r' || key == 'R')
                reiniciar();
        }

        imshow(wName, smallFrame);

        key = (char)waitKey(10);
        if (key == 27 || key == 'q' || key == 'Q')
            break;
    }

    delete pipeD;
    delete pipeU;
    return 0;
}