#pragma once
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class Jogador {
private:
    Mat img;
    int xPos, yPos;
    int largura, altura;

public:
    Jogador(string path, int w, int h, int frameCenterX) {
        img = imread(path, IMREAD_UNCHANGED);
        if (img.empty())
            cout << "ERRO: não foi possível carregar: " << path << endl;
        largura = w;
        altura  = h;
        xPos = frameCenterX - largura / 2;  // fixo no centro horizontal
        yPos = 0;
    }

    Rect getRect() {
        int mx = largura * 0.3;
        int my = altura  * 0.3;
        return Rect(xPos + mx, yPos + my, largura - mx * 2, altura - my * 2);
    }

    void update(Mat& frame, CascadeClassifier& cascade) {
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
        equalizeHist(grayFrame, grayFrame);

        vector<Rect> faces;
        cascade.detectMultiScale(grayFrame, faces,
            1.3, 2, CASCADE_SCALE_IMAGE, Size(40, 40));

        if (!faces.empty()) {
            Rect rosto = faces[0];
            // xPos permanece fixo — só atualiza Y
            yPos = rosto.y + rosto.height / 2 - altura / 2;
        }

        drawImage(frame);
    }

    void drawImage(Mat& frame) {
        if (img.empty()) return;

        Mat imgEscalada;
        resize(img, imgEscalada, Size(largura, altura));

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
};