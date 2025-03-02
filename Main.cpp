#include <SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>

struct GameObject {
    SDL_Rect rect; // Прямоугольник объекта
    float velocityX = 0.0f; // Скорость по X
    float velocityY = 0.0f; // Скорость по Y
};

bool CheckCollision(const SDL_Rect& rectA, const SDL_Rect& rectB) {
    return (rectA.x < rectB.x + rectB.w &&
        rectA.x + rectA.w > rectB.x &&
        rectA.y < rectB.y + rectB.h &&
        rectA.y + rectA.h > rectB.y);
}

void ResolveCollision(GameObject& objA, GameObject& objB) {
    int overlapX = 0;
    int overlapY = 0;

    overlapX = std::min(objA.rect.x + objA.rect.w - objB.rect.x, objB.rect.x + objB.rect.w - objA.rect.x);
    overlapY = std::min(objA.rect.y + objA.rect.h - objB.rect.y, objB.rect.y + objB.rect.h - objA.rect.y);

    if (overlapX < overlapY) {
        if (objA.rect.x < objB.rect.x) {
            objA.rect.x -= overlapX; // Коллизия слева
        }
        else if (objA.rect.x > objB.rect.x) {
            objA.rect.x += overlapX; // Коллизия справа
        }
    }
    else if (overlapX > overlapY) {
        if (objA.rect.y < objB.rect.y) {
            objA.rect.y -= overlapY; // Коллизия сверху
        }
        else if (objA.rect.y > objB.rect.y) {
            objA.rect.y += overlapY; // Коллизия снизу
        }
    }

    objB.velocityX = -objB.velocityX;
    objB.velocityY = -objB.velocityY;
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
    objects.push_back(GameObject{ {16, 32, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {16, 48, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {16, 64, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {16, 80, 16, 16}, 0.0f, 0.0f });

    objects.push_back(GameObject{ {16, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {32, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {48, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {64, 16, 16, 16}, 0.0f, 0.0f });
    objects.push_back(GameObject{ {80, 16, 16, 16}, 0.0f, 0.0f });

    bool running = true;
    SDL_Event event;

    const Uint8* keyboardState = SDL_GetKeyboardState(NULL); // Получаем состояние клавиш

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

        // Проверка столкновений
        for (size_t i = 0; i < objects.size(); ++i) {
            for (size_t j = i + 1; j < objects.size(); ++j) {
                if (CheckCollision(objects[i].rect, objects[j].rect)) {
                    ResolveCollision(objects[i], objects[j]);
                }
            }
        }

        // Очистка экрана
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Рисование объектов
        for (const auto& obj : objects) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет
            SDL_RenderFillRect(renderer, &obj.rect);
        }

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