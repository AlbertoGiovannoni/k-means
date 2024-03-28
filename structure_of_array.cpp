//
// Created by Alberto Giovannoni on 05/03/24.
//

#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <chrono>


std::pair<std::vector<double>, std::vector<double>> extractDataset() {

    std::vector<double> x_vec;
    std::vector<double> y_vec;

    std::ifstream inFile("../dataset/dataset.txt");

    if (!inFile) {
        std::cerr << "[SoA] Errore nell'apertura del file" << std::endl;
        return {x_vec, y_vec};
    }

    std::string line;
    double x, y;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        iss >> x >> y; // Assegno a x e y i valori da file
        x_vec.push_back(x);
        y_vec.push_back(y);
    }

    inFile.close();
    return {x_vec, y_vec};
}

std::pair<std::vector<double>, std::vector<double>> extractCentroids() {
    std::vector<double> x_vec;
    std::vector<double> y_vec;

    std::ifstream inFile("../dataset/centroids.txt");

    if (!inFile) {
        std::cerr << "[SoA] Errore nell'apertura del file dei centroidi" << std::endl;
        return {x_vec, y_vec};
    }

    std::string line;
    double x, y;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        iss >> x >> y; // Assegno a x e y i valori da file
        x_vec.push_back(x);
        y_vec.push_back(y);
    }

    inFile.close();
    return {x_vec, y_vec};
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<int>> kmean(std::vector<double> &x_centroids,
                                                                             std::vector<double> &y_centroids,
                                                                             std::vector<double> &x_values,
                                                                             std::vector<double> &y_values,
                                                                             int numCluster, int maxIter) {
    std::vector<int> points_id(x_values.size(), -1);
    std::vector<double> totalX(numCluster, 0);
    std::vector<double> totalY(numCluster, 0);
    std::vector<int> countPoints(numCluster, 0);

    bool centerUpdated;
    int minIndex;
    double minDist;
    double dist;
    int i = 0;

    do {
        if (i % 10 == 0)
            std::cout << "[SoA] Numero iterazioni k-means: " << i << std::endl;
        i++;

        centerUpdated = false;

        // Reset dei metadati del passo precedente
        totalX.assign(numCluster, 0);
        totalY.assign(numCluster, 0);
        countPoints.assign(numCluster, 0);

        for (int j = 0; j < x_values.size(); j++) {
            minDist = INFINITY;
            for (int k = 0; k < x_centroids.size(); k++) {
                dist = std::sqrt(std::pow(x_centroids[k] - x_values[j], 2) + std::pow(y_centroids[k] - y_values[j], 2));
                if (dist < minDist) {
                    minDist = dist;
                    minIndex = k;
                }
            }
            if (points_id[j] != minIndex) {
                centerUpdated = true; // Se nessun punto cambia cluster allora termino
            }
            points_id[j] = minIndex;
            totalX[minIndex] += x_values[j];
            totalY[minIndex] += y_values[j];
            countPoints[minIndex]++;
        }

        if (centerUpdated) {
            for (int w = 0; w < numCluster; ++w) {
                if (countPoints[w] > 0) {
                    x_centroids[w] = totalX[w] / countPoints[w];
                    y_centroids[w] = totalY[w] / countPoints[w];
                } else {
                    // Se non ci sono punti assegnati al cluster, mantieni il centroide invariato
                }
            }
        }
    } while (centerUpdated && i <= maxIter);

    return std::make_tuple(x_centroids, y_centroids, points_id);
}

void drawPoints(sf::RenderWindow &window, std::vector<double> &x_centroids,
                std::vector<double> &y_centroids,
                std::vector<double> &x_values,
                std::vector<double> &y_values, std::vector<int> &points_id) {
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
            std::cerr << "[SoA] Impossibile caricare il font Arial." << std::endl;
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

        for (int i = 0; i < points_id.size(); ++i) {
            // Ogni punto ha un colore associato al cluster
            switch (points_id[i]) {
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

            pointShape.setPosition(x_values[i] + window.getSize().x / 2, window.getSize().y / 2 - y_values[i]);
            window.draw(pointShape);
        }

        // Disegna i centroidi
        sf::CircleShape centroidShape(3);
        centroidShape.setFillColor(sf::Color::Black);

        for (int j = 0; j < x_centroids.size(); ++j) {
            centroidShape.setPosition(x_centroids[j] + window.getSize().x / 2, window.getSize().y / 2 - y_centroids[j]);
            window.draw(centroidShape);
        }

        window.display();
    }
}


int main() {
    int numCluster = 10;
    int maxIter = 150;

    std::cout << "[SoA] Versione SoA kmeans\n" << std::endl;

    auto [x_values, y_values] = extractDataset();

    auto [x_centroids, y_centroids] = extractCentroids();

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    auto [x_centroids_new, y_centroids_new, points_id] = kmean(x_centroids, y_centroids, x_values, y_values,
                                                               numCluster, maxIter);

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    std::cout << "[SoA] Tempo impiegato da k-means: " << elapsed_seconds.count() << " secondi" << std::endl;

    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Parallel clusters");
    drawPoints(window, x_centroids, y_centroids, x_values, y_values, points_id);
    return 0;
}
