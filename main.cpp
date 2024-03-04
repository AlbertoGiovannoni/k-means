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
        totalX += x;
    }

    void addTotalY(double y) {
        totalY += y;
    }

    void countPoints() {
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

void generateDataset(int numPointsPerCluster, int numClusters, int centerDist, int stdDev, int rangeX, int rangeY) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::ofstream outFile("../dataset/dataset.txt");
    if (!outFile) {
        std::cerr << "Errore nell'apertura del file" << std::endl;
        return;
    }

    std::vector<point> centers;

    // Genera posizioni casuali per i centroidi
    std::uniform_real_distribution<double> cenX(-rangeX, rangeX); // Intervallo per le coordinate x
    std::uniform_real_distribution<double> cenY(-rangeY, rangeY); // Intervallo per le coordinate y

    for (int i = 0; i < numClusters; ++i) {
        point center;
        bool validPos = false;

        // Trova una posizione valida per il centroide
        while (!validPos) {
            double x = cenX(gen);
            double y = cenY(gen);
            center.x = x;
            center.y = y;

            validPos = true;

            // Controlla se il nuovo centroide si sovrappone con quelli già generati
            for (const auto &existingCenter: centers) {
                if (std::sqrt(std::pow(existingCenter.x - x, 2) + std::pow(existingCenter.y - y, 2)) < centerDist) {
                    validPos = false;
                    break;
                }
            }
        }

        centers.push_back(center); // Aggiungi il nuovo centroide alla lista dei centroidi validi

        // Genera punti intorno al centroide con distribuzione normale
        std::normal_distribution<double> disX(center.x, stdDev);
        std::normal_distribution<double> disY(center.y, stdDev);

        for (int j = 0; j < numPointsPerCluster; ++j) {
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
    point p{};
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        iss >> p.x >> p.y; // Assegno al punto p le coordinate x e y
        points.push_back(p);
    }

    inFile.close();
    return points;
}

void createClusters(int numCluster, int rangeX, int rangeY, int centerDist) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::ofstream outFile("../dataset/centroids.txt");
    if (!outFile) {
        std::cerr << "Errore nell'apertura del file dei dati" << std::endl;
        return;
    }

    // Creazione centroidi
    std::uniform_real_distribution<double> disX(-rangeX, rangeX);
    std::uniform_real_distribution<double> disY(-rangeY, rangeY);

    point c{};
    std::vector<point> centroids;

    for (int i = 0; i < numCluster; i++) {
        bool validPosition = false;
        while (!validPosition) {
            c.x = disX(gen);
            c.y = disY(gen);

            // Verifica che la nuova posizione non sia troppo vicina a nessun altro centroide
            validPosition = true;
            for (auto &centroid: centroids) {
                double distance = std::sqrt(std::pow(centroid.x - c.x, 2) +
                                            std::pow(centroid.y - c.y, 2));
                if (distance < centerDist) {
                    validPosition = false;
                    break;
                }
            }
        }

        outFile << c.x << " " << c.y << std::endl;
        centroids.push_back(c);
    }
    outFile.close();
}

std::vector<cluster> extractClusters(){
    std::vector<cluster> clusters;
    std::ifstream inFile("../dataset/centroids.txt");

    if (!inFile) {
        std::cerr << "Errore nell'apertura del file dei centroidi" << std::endl;
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

std::vector<cluster> kmean(std::vector<cluster> &clusters, std::vector<point> &points) {
    bool centerUpdated;
    std::vector<point> centroids;
    int minIndex;
    double minDist;
    double dist;
    int i = 0;

    do {
        centerUpdated = false;
        if (i % 10 == 0)
            std::cout << "Numero iterazioni k-means: " << i << std::endl;
        i++;

        centroids.clear(); // Elimino i centroidi precedenti e li ricarico
        for (auto &cluster: clusters) {
            centroids.push_back(cluster.getCentroid());

            // reset valori in ogni cluster
            cluster.resetTotalX();
            cluster.resetTotalY();
            cluster.resetCountPoints();
        }

        for (auto &point: points) {
            minDist = INFINITY;
            for (int i = 0; i < centroids.size(); i++) {
                dist = std::sqrt(std::pow(centroids[i].x - point.x, 2) + std::pow(centroids[i].y - point.y, 2));
                if (dist < minDist) {
                    minDist = dist;
                    minIndex = i;
                }
            }
            if (point.clusterID != minIndex) {
                centerUpdated = true; // Se nessun punto cambia cluster allora termino
            }
            point.clusterID = minIndex;
            clusters[minIndex].addTotalX(point.x);
            clusters[minIndex].addTotalY(point.y);
            clusters[minIndex].countPoints();
        }

        for (auto &cluster: clusters) {
            cluster.updateCentroid();
        }
    } while (centerUpdated);

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

    int numPointsPerCluster = 10000;
    int numCluster = 10;
    int centerDist = 200;
    int stdDev = 20;
    int rangeX = 700;
    int rangeY = 350;

    generateDataset(numPointsPerCluster, numCluster, centerDist, stdDev, rangeX,
                    rangeY); // da usare per ricreare il dataset
    std::vector<point> points = extractDataset();

    // creazione cluster
    createClusters(numCluster, rangeX, rangeY, centerDist);
    std::vector<cluster> clusters = extractClusters();

    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Clusters");
    std::vector<point> centroids;
    centroids.reserve(clusters.size());

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    clusters = kmean(clusters, points);
    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    std::cout << "Tempo impiegato da k-means: " << elapsed_seconds.count() << " secondi" << std::endl;

    for (auto &cluster: clusters) {
        centroids.push_back(cluster.getCentroid());
    }

    drawPoints(window, points, centroids);

    return 0;
}

