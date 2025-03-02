#include <SDL.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdlib>

struct GameObject {
    SDL_Rect rect; // Прямоугольник объекта
    float velocityX = 0.0f; // Скорость по X
    float velocityY = 0.0f; // Скорость по Y
};

// Функция проверки столкновения двух прямоугольников
bool CheckCollision(const SDL_Rect& rectA, const SDL_Rect& rectB) {
    return (rectA.x < rectB.x + rectB.w &&
        rectA.x + rectA.w > rectB.x &&
        rectA.y < rectB.y + rectB.h &&
        rectA.y + rectA.h > rectB.y);
}

// Функция разрешения столкновения
void ResolveCollision(GameObject& objA, GameObject& objB) {
    int overlapX = 0;
    int overlapY = 0;

    overlapX = std::min(objA.rect.x + objA.rect.w - objB.rect.x, objB.rect.x + objB.rect.w - objA.rect.x);
    overlapY = std::min(objA.rect.y + objA.rect.h - objB.rect.y, objB.rect.y + objB.rect.h - objA.rect.y);

    if (overlapX < overlapY) {
        if (objA.rect.x < objB.rect.x) {
            objA.rect.x -= overlapX; // Коллизия слева
        }
        else {
            objA.rect.x += overlapX; // Коллизия справа
        }
    }
    else if (overlapX > overlapY) {
        if (objA.rect.y < objB.rect.y) {
            objA.rect.y -= overlapY; // Коллизия сверху
        }
        else {
            objA.rect.y += overlapY; // Коллизия снизу
        }
    }

    objB.velocityX = -objB.velocityX;
    objB.velocityY = -objB.velocityY;
}

// Генерация сетки для оптимизации коллизий
std::unordered_map<int, std::vector<GameObject*>> GenerateGrid(std::vector<GameObject>& objects, int cellSize, int width, int height) {
    std::unordered_map<int, std::vector<GameObject*>> grid;

    for (size_t i = 0; i < objects.size(); ++i) {
        auto& obj = objects[i]; // Берем ссылку на объект
        int minX = obj.rect.x / cellSize;
        int maxX = (obj.rect.x + obj.rect.w - 1) / cellSize;
        int minY = obj.rect.y / cellSize;
        int maxY = (obj.rect.y + obj.rect.h - 1) / cellSize;

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                int cellIndex = y * (width / cellSize) + x;
                grid[cellIndex].push_back(&objects[i]); // Добавляем указатель на объект
            }
        }
    }

    return grid;
}

// Проверка коллизий с использованием сетки
void CheckCollisionsWithGrid(std::vector<GameObject>& objects, int cellSize, int width, int height) {
    auto grid = GenerateGrid(objects, cellSize, width, height);
    int collisionChecks = 0; // Счетчик проверок

    for (const auto& [cellIndex, cellObjects] : grid) {
        for (size_t i = 0; i < cellObjects.size(); ++i) {
            for (size_t j = i + 1; j < cellObjects.size(); ++j) {
                collisionChecks++;
                if (CheckCollision(cellObjects[i]->rect, cellObjects[j]->rect)) {
                    ResolveCollision(*cellObjects[i], *cellObjects[j]);
                }
            }
        }
    }

    std::cout << "Number of collision checks: " << collisionChecks << std::endl;
}

void DrawGrid(SDL_Renderer* renderer, int cellSize, int width, int height) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Зеленый цвет для сетки

    for (int x = 0; x < width; x += cellSize) {
        SDL_RenderDrawLine(renderer, x, 0, x, height);
    }

    for (int y = 0; y < height; y += cellSize) {
        SDL_RenderDrawLine(renderer, 0, y, width, y);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Collision Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::vector<GameObject> objects;

    // Добавляем объекты
    objects.push_back(GameObject{ {100, 100, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {16, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {32, 100, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {16, 200, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {55, 75, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {16, 80, 16, 16}, 0.0f, 0.0f });

    /*
    objects.push_back(GameObject{ {16, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {32, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {48, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {64, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {80, 16, 16, 16}, 0.0f, 0.0f });
    */

    bool running = true;
    SDL_Event event;

    const Uint8* keyboardState = SDL_GetKeyboardState(NULL); // Получаем состояние клавиш

    int screenWidth = 800;
    int screenHeight = 600;
    int cellSize = 32; // Размер ячейки сетки

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Управление первым объектом через WASD
        float speed = 2.0f; // Скорость перемещения
        if (keyboardState[SDL_SCANCODE_W]) {
            objects[0].rect.y -= speed;
        }
        if (keyboardState[SDL_SCANCODE_S]) {
            objects[0].rect.y += speed;
        }
        if (keyboardState[SDL_SCANCODE_A]) {
            objects[0].rect.x -= speed;
        }
        if (keyboardState[SDL_SCANCODE_D]) {
            objects[0].rect.x += speed;
        }

        // Обновление положения объектов
        for (auto& obj : objects) {
            obj.rect.x += obj.velocityX;
            obj.rect.y += obj.velocityY;
        }

        // Проверка столкновений с использованием сетки
        CheckCollisionsWithGrid(objects, cellSize, screenWidth, screenHeight);

        // Очистка экрана
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Рисование объектов
        for (const auto& obj : objects) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет
            SDL_RenderFillRect(renderer, &obj.rect);
        }

        // Рисование сетки
        DrawGrid(renderer, cellSize, screenWidth, screenHeight);

        // Отображение результата
        SDL_RenderPresent(renderer);

        // Задержка для стабильного FPS
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}