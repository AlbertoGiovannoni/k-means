//
// Created by Alberto Giovannoni on 29/02/24.
//
#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
#include <SFML/Graphics.hpp>

struct point {
    double x;
    double y;
};

void generateDataset(int numPointsPerCluster, int numClusters, int centerDist, int stdDev) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::ofstream outFile("../dataset/dataset.txt");
    if (!outFile) {
        std::cerr << "Errore nell'apertura del file" << std::endl;
        return;
    }

    for (int c = 0; c < numClusters; c++) {
        // Genero cluster con distribuzione normale
        std::normal_distribution<double> disX(centerDist * c, stdDev); // Intervallo per le coordinate x
        std::normal_distribution<double> disY(0, stdDev); // Intervallo per le coordinate y

        for (int i = 0; i < numPointsPerCluster; ++i) {
            double x = disX(gen);
            double y = disY(gen);
            outFile << x << " " << y << std::endl;
        }
    }

    outFile.close();
    std::cout << "Dataset generato con successo" << std::endl;
}

std::vector<point> extractDataset() {
    std::vector<point> points;
    std::ifstream inFile("../dataset/dataset.txt");

    if (!inFile) {
        std::cerr << "Errore nell'apertura del file" << std::endl;
        return points;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        point p{};
        iss >> p.x >> p.y; // assegno al punto p le coordinate x e y
        points.push_back(p);
    }

    inFile.close();
    return points;
}

void drawPoints(sf::RenderWindow& window, std::vector<point>& points, std::vector<point>& centroids) {
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
            std::cerr << "Impossibile caricare il font Arial." << std::endl;
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
        sf::CircleShape pointShape(2); // Imposta la forma del punto
        pointShape.setFillColor(sf::Color::Red); // Imposta il colore del punto

        for (auto& point : points) {
            pointShape.setPosition(point.x + window.getSize().x / 2, window.getSize().y / 2 - point.y);
            window.draw(pointShape);
        }

        // Disegna i centroidi
        sf::CircleShape centroidShape(2); // Imposta la forma del centroide
        centroidShape.setFillColor(sf::Color::Black);

        for (auto& centroid : centroids) {
            centroidShape.setPosition(centroid.x + window.getSize().x / 2, window.getSize().y / 2 - centroid.y);
            window.draw(centroidShape);
        }

        window.display();
    }

}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    int numPointsPerCluster = 100;
    int numClusters = 3;
    int centerDist = 50;
    //int radius = 10;
    int stdDev = 10;

    generateDataset(numPointsPerCluster, numClusters, centerDist, stdDev); // da usare per ricreare il dataset
    std::vector<point> points = extractDataset();

//    for (auto& point : points) {
//        std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
//    }


    std::vector<point> centroids;
    std::uniform_real_distribution<double> disX(- stdDev*2, ((numClusters - 1) * centerDist + stdDev*2));
    std::uniform_real_distribution<double> disY(- stdDev*2, stdDev*2);

    point centroid{};
    for (int i = 0; i < numClusters; i++) {
        centroid.x = disX(gen);
        centroid.y = disY(gen);
        centroids.push_back(centroid);
    }

    sf::RenderWindow window(sf::VideoMode(800, 600), "Clusters");
    drawPoints(window, points, centroids);


    return 0;
}

