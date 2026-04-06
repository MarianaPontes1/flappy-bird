#pragma once
#include <opencv2/opencv.hpp>
#include "bits/stdc++.h"
using namespace cv;
using namespace std;

class Jogo {
protected:
    int pontuacao;
public:
    bool started = false;
    bool gameOver = false;
    int x = 0;
    int direcao = 5;

    Jogo() {
        pontuacao = 0;
    }

    void reset() {
        started = false;
        gameOver = false;
        x = 0;
    }

    void Pontuar() {
        pontuacao += 1;
    }
};