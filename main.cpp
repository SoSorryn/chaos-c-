#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <cmath>

using namespace std;
using namespace cv;

// --- МЕТОД: GAUSS MAP (Отображение Гаусса) ---
// Формула: x_{n+1} = exp(-alpha * x_n^2) + beta
double gaussMap(double x, double alpha = 6.2, double beta = -0.5) {
    return exp(-alpha * x * x) + beta;
}

// Превращаем пароль в начальное значение x0
double passwordToKey(const string& pass) {
    double key = 0.123456789;
    for (char c : pass) {
        // Смешиваем символы пароля, используя дробную часть
        key = fmod(fabs(key * 43.0 + (unsigned char)c * 0.111), 1.0);
    }
    // Для метода Гаусса x может быть в диапазоне от -1 до 1
    // Приводим результат к интервалу [-0.5, 0.5] для стабильного хаоса
    return key - 0.5;
}

int main() {
    string inputPath, outputPath, password, mode;

    cout << "=== Шифрование изображения(.png) ===\n";
    cout << "Режим (e - шифрование, d - расшифрование): ";
    cin >> mode;
    cin.ignore();

    cout << "Введите пароль: ";
    getline(cin, password);

    cout << "Путь к изображению: ";
    getline(cin, inputPath);

    Mat img = imread(inputPath, IMREAD_COLOR);
    if (img.empty()) {
        cerr << "Ошибка: Файл не найден!\n";
        return 1;
    }

    // 1. Инициализация ключа (x0)
    double x = passwordToKey(password);
    
    // 2. Фаза "разогрева"
    for (int i = 0; i < 1000; i++) {
        x = gaussMap(x);
    }

    Mat result = img.clone();

    // 3. Процесс поточного шифрования
    for (int y = 0; y < img.rows; y++) {
        for (int x_pos = 0; x_pos < img.cols; x_pos++) {
            Vec3b& pixel = result.at<Vec3b>(y, x_pos);
            
            // Шифруем каждый канал (B, G, R) отдельным значением хаоса
            for (int c = 0; c < 3; c++) {
                x = gaussMap(x);
                
                // Преобразуем дробное x в байт (0-255)
                // Используем fmod, так как Gauss Map может выдавать отрицательные числа
                unsigned char chaosByte = (unsigned char)(fmod(fabs(x) * 1000000.0, 256.0));
                
                pixel[c] ^= chaosByte;
            }
        }
    }

    // Определение пути сохранения
    outputPath = (mode == "e") ? "encrypted.png" : "decrypted.png";
    
    if (imwrite(outputPath, result)) {
        cout << "\n[OK] Результат сохранен в: " << outputPath << endl;
    } else {
        cerr << "\n[Ошибка] Не удалось сохранить файл!" << endl;
    }

    // Вывод окон для сравнения
    double scale = 800.0 / max(img.cols, img.rows);
    if (scale > 1.0) scale = 1.0;

    Mat showOrig, showRes;
    resize(img, showOrig, Size(), scale, scale);
    resize(result, showRes, Size(), scale, scale);

    imshow("Original", showOrig);
    imshow(mode == "e" ? "Encrypted (Gauss Chaos)" : "Decrypted", showRes);
    
    cout << "Нажмите любую клавишу для выхода..." << endl;
    waitKey(0);

    return 0;
}
