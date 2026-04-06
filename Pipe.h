#pragma once
#include <opencv2/opencv.hpp>
#include "Jogo.h"
using namespace cv;
using namespace std;

class Pipe : public Jogo {
private:
    Mat img;
    int xPos, yPos;
    int velocidade;
    int alturaFixa;

public:
    Pipe(string path, int startX, int startY, int altura) {
        img = imread(path, IMREAD_UNCHANGED);
        if (img.empty())
            cout << "ERRO: não foi possível carregar: " << path << endl;
        xPos       = startX;
        yPos       = startY;
        velocidade = 5;
        alturaFixa = altura;
    }

    Rect getRect() {
        return Rect(xPos, yPos, 52, alturaFixa);
    }

    void setPos(int novoY, int novaAltura) {
        yPos       = novoY;
        alturaFixa = novaAltura;
    }

    void drawImage(Mat& frame) {
        if (img.empty()) return;
        if (alturaFixa <= 0) return;

        Mat imgEscalada;
        resize(img, imgEscalada, Size(52, alturaFixa));

        int xStart = max(xPos, 0);
        int yStart = max(yPos, 0);
        int xEnd   = min(xPos + imgEscalada.cols, frame.cols);
        int yEnd   = min(yPos + imgEscalada.rows, frame.rows);

        if (xStart >= xEnd || yStart >= yEnd) return;

        int imgX = xStart - xPos;
        int imgY = yStart - yPos;

        Mat imgROI   = imgEscalada(Rect(imgX, imgY, xEnd - xStart, yEnd - yStart));
        Mat frameROI = frame(Rect(xStart, yStart, xEnd - xStart, yEnd - yStart));

        vector<Mat> layers;
        split(imgROI, layers);

        if (layers.size() == 4) {
            Mat rgb[3] = { layers[0], layers[1], layers[2] };
            Mat alpha  = layers[3];
            Mat imgRGB;
            merge(rgb, 3, imgRGB);
            imgRGB.copyTo(frameROI, alpha);
        } else {
            imgROI.copyTo(frameROI);
        }
    }

    bool update(Mat& smallFrame) {
        xPos -= velocidade;
        bool resetou = false;
        if (xPos < -52) {
            xPos    = smallFrame.cols;
            resetou = true;
        }
        drawImage(smallFrame);
        return resetou;
    }

    void aumentarDificuldade() {
        double DELTAv =
        pontuacao / (12.0 + (pontuacao*log(pontuacao)) / 5.0);
        double DELTAv_lateGame =
        0.8 * (pontuacao / (20.0 + pontuacao))
            + (pontuacao / (12.0 + (pontuacao*log(pontuacao)) / 5.0));

        if(pontuacao <= 8){
            velocidade+=1;
        }
        else if (pontuacao < 32)
            velocidade += DELTAv;
        else
            velocidade += DELTAv_lateGame;
    }
};