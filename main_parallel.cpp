//
// Created by Alberto Giovannoni on 06/03/24.
//

#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <omp.h>
#include <chrono>

struct point {
    double x;
    double y;
    int clusterID;
};

class cluster {

private:
    point centroid{};
    std::vector<point> points;
    double totalX = 0;
    double totalY = 0;
    int count = 0;

public:

    void addTotalX(double x) {
#pragma omp atomic
        totalX += x;
    }

    void addTotalY(double y) {
#pragma omp atomic
        totalY += y;
    }

    void countPoints() {
#pragma omp atomic
        count++;
    }

    void resetCountPoints() {
        count = 0;
    }

    void resetTotalX() {
        totalX = 0;
    }

    void resetTotalY() {
        totalY = 0;
    }


    void createCentroid(point c) {
        centroid = c;
    }

    void updateCentroid() {
        if (count > 0) {
            centroid.x = totalX / count;
            centroid.y = totalY / count;
        } else {
            // Se non ci sono punti assegnati al cluster, mantieni il centroide invariato
        }
    }

    point getCentroid() {
        return centroid;
    }
};

std::vector<point> extractDataset() {
    std::vector<point> points;
    std::ifstream inFile("../dataset/dataset.txt");

    if (!inFile) {
        std::cerr << "[Par] Errore nell'apertura del file" << std::endl;
        return points;
    }

    std::string line;
    point p{};
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        iss >> p.x >> p.y; // Assegno al punto p le coordinate x e y
        p.clusterID = -1;
        points.push_back(p);
    }

    inFile.close();
    return points;
}

std::vector<cluster> extractClusters() {
    std::vector<cluster> clusters;
    std::ifstream inFile("../dataset/centroids.txt");

    if (!inFile) {
        std::cerr << "[Par] Errore nell'apertura del file dei centroidi" << std::endl;
        return clusters;
    }

    std::string line;
    cluster c{};
    point centroid{};
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        iss >> centroid.x >> centroid.y;
        c.createCentroid(centroid);
        clusters.push_back(c);
    }

    inFile.close();
    return clusters;
}

std::vector<cluster> kmean(std::vector<cluster> &clusters, std::vector<point> &points, int maxIter) {
    bool centerUpdated;
    int i = 0;

    do {
        centerUpdated = false;
        if (i % 10 == 0)
            std::cout << "[Par] Numero iterazioni k-means: " << i << std::endl;
        i++;

        // Reset dei valori nei cluster
#pragma omp parallel for
        for (int c = 0; c < clusters.size(); c++) {
            clusters[c].resetTotalX();
            clusters[c].resetTotalY();
            clusters[c].resetCountPoints();
        }


        int minIndex;
        double minDist;
        double dist;
        // Assegnazione dei punti ai cluster più vicini
#pragma omp parallel for private(minIndex, minDist, dist)
        for (int p = 0; p < points.size(); p++) {
            minIndex = -1;
            minDist = INFINITY;

            for (int c = 0; c < clusters.size(); c++) {
                dist = std::sqrt(std::pow(clusters[c].getCentroid().x - points[p].x, 2) +
                                 std::pow(clusters[c].getCentroid().y - points[p].y, 2));
                if (dist < minDist) {
                    minDist = dist;
                    minIndex = c;
                }
            }

            if (points[p].clusterID != minIndex) {
                centerUpdated = true;
            }
            points[p].clusterID = minIndex;
            clusters[minIndex].addTotalX(points[p].x); // uso di atomic
            clusters[minIndex].addTotalY(points[p].y); // uso di atomic
            clusters[minIndex].countPoints(); // uso di atomic
        }




        // Aggiornamento dei centroidi
#pragma omp parallel for
        for (int c = 0; c < clusters.size(); c++) {

            clusters[c].updateCentroid();
        }

    } while (centerUpdated && i <= maxIter);

    return clusters;
}


void drawPoints(sf::RenderWindow &window, std::vector<point> &points, std::vector<point> &centroids) {
    // Imposta l'origine della vista al centro della finestra
    sf::View view = window.getDefaultView();
    view.setCenter(window.getSize().x / 2, window.getSize().y / 2);
    window.setView(view);
    while (window.isOpen()) {

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::White);

        // Disegna l'asse delle x
        sf::VertexArray xAxis(sf::Lines, 2);
        xAxis[0].position = sf::Vector2f(0, window.getSize().y / 2);
        xAxis[1].position = sf::Vector2f(window.getSize().x, window.getSize().y / 2);
        xAxis[0].color = sf::Color::Black;
        xAxis[1].color = sf::Color::Black;

        // Disegna l'asse delle y
        sf::VertexArray yAxis(sf::Lines, 2);
        yAxis[0].position = sf::Vector2f(window.getSize().x / 2, 0);
        yAxis[1].position = sf::Vector2f(window.getSize().x / 2, window.getSize().y);
        yAxis[0].color = sf::Color::Black;
        yAxis[1].color = sf::Color::Black;

        // Importa il font
        sf::Font font;
        if (!font.loadFromFile("../font/arial.ttf")) {
            std::cerr << "[Par] Impossibile caricare il font Arial." << std::endl;
            return;
        }

        // Etichetta sull'asse x
        sf::Text xAxisLabel("x", font, 16);
        xAxisLabel.setFillColor(sf::Color::Black);
        xAxisLabel.setPosition(window.getSize().x - 20, window.getSize().y / 2 + 10);

        // Etichetta sull'asse y
        sf::Text yAxisLabel("y", font, 16);
        yAxisLabel.setFillColor(sf::Color::Black);
        yAxisLabel.setPosition(window.getSize().x / 2 + 10, 10);

        window.draw(xAxis);
        window.draw(yAxis);
        window.draw(xAxisLabel);
        window.draw(yAxisLabel);

        // Disegna i punti
        sf::CircleShape pointShape(1); // Imposta la forma del punto

        for (auto &point: points) { // Ogni punto ha un colore associato al cluster
            switch (point.clusterID) {
                case 0:
                    pointShape.setFillColor(sf::Color::Red);
                    break;
                case 1:
                    pointShape.setFillColor(sf::Color::Blue);
                    break;
                case 2:
                    pointShape.setFillColor(sf::Color::Green);
                    break;
                case 3:
                    pointShape.setFillColor(sf::Color::Yellow);
                    break;
                case 4:
                    pointShape.setFillColor(sf::Color::Magenta);
                    break;
                case 5:
                    pointShape.setFillColor(sf::Color::Cyan);
                    break;
                case 6:
                    pointShape.setFillColor(sf::Color(255, 182, 193)); // rosa
                    break;
                case 7:
                    pointShape.setFillColor(sf::Color(165, 42, 42)); // marrone
                    break;
                case 8:
                    pointShape.setFillColor(sf::Color(128, 128, 128)); // grigio
                    break;
                case 9:
                    pointShape.setFillColor(sf::Color(128, 0, 128)); //viola
                    break;

            }

            pointShape.setPosition(point.x + window.getSize().x / 2, window.getSize().y / 2 - point.y);
            window.draw(pointShape);
        }

        // Disegna i centroidi
        sf::CircleShape centroidShape(3);
        centroidShape.setFillColor(sf::Color::Black);

        for (auto &centroid: centroids) {
            centroidShape.setPosition(centroid.x + window.getSize().x / 2, window.getSize().y / 2 - centroid.y);
            window.draw(centroidShape);
        }

        window.display();
    }
}

int main() {
    std::cout << "[Par] Versione parallela kmeans\n" << std::endl;

    // Controllo del funzionamento di openMP
#ifdef _OPENMP
    std::cout << "[Par] _OPENMP defined" << std::endl;
    std::cout << "[Par] Num processors (Phys+HT): " << omp_get_num_procs() << "\n" << std::endl;
#endif

    int threadNum = 8;
    omp_set_num_threads(threadNum);
    std::cout << "[Par] Thread in uso: " << threadNum << std::endl;

    int numCluster = 10;
    int maxIter = 150;


    std::vector<point> points = extractDataset();

    std::vector<cluster> clusters = extractClusters();

    std::vector<point> centroids;
    centroids.reserve(clusters.size());

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    clusters = kmean(clusters, points, maxIter);

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    std::cout << "[Par] Tempo impiegato da k-means: " << elapsed_seconds.count() << " secondi" << std::endl;

    for (auto &cluster: clusters) {
        centroids.push_back(cluster.getCentroid());
    }

    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Parallel clusters");
    drawPoints(window, points, centroids);

    return 0;
}