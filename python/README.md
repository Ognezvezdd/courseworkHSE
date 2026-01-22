# AI Agents Platform

Платформа симуляции конкурентного взаимодействия AI-агентов

---

## Описание

Python часть проекта реализует:

- **Игровую среду** 5×5 крестики-нолики (победа при 4 символах подряд)
- **Систему AI-агентов** с единым интерфейсом
- **Функцию `run_game()`** для запуска партий с пошаговой историей
- **Воспроизводимость** результатов через механизм seed

---

## Архитектура

```
.
├── agents/                 # AI-агенты
│   ├── base_agent.py       # Абстрактный базовый класс
│   ├── random_agent.py     # Случайный агент
│   ├── heuristic_agent.py  # Эвристический агент
│   └── qlearning_agent.py  # Q-learning агент
├── game/                   # Игровая среда
│   ├── board.py            # Игровое поле
│   ├── rules.py            # Правила игры
│   └── engine.py           # Игровой движок
├── visualization/          # Визуализация
│   ├── board_visualizer.py # Отрисовка поля
│   └── renderer.py         # Рендеринг игр
├── tests/                  # Тесты
├── run.py                  # Демонстрационный скрипт
├── visualize.py            # CLI визуализации
├── requirements.txt        # Зависимости
├── Dockerfile              # Docker-образ
└── README.md
```

---

## Быстрый старт

### Локальный запуск

```bash
# Установка зависимостей
pip install -r requirements.txt

# Запуск демонстрации
python3 run.py --agent-x random --agent-o heuristic --seed 42

# Запуск тестов
python3 -m pytest tests/ -v
```

### Docker (in progress)

```bash
# Сборка образа
docker build -t ai-agents-platform .

# Запуск демонстрации
docker run --rm ai-agents-platform

# Запуск с параметрами
docker run --rm ai-agents-platform python run.py --agent-x qlearning --agent-o random --train 1000 --seed 42

# Запуск тестов
docker run --rm ai-agents-platform pytest tests/ -v
```

---

## Использование

### Демонстрационный скрипт

```bash
# Базовый запуск
python3 run.py

# С выбором агентов
python3 run.py --agent-x heuristic --agent-o random

# С фиксированным seed
python3 run.py --seed 42

# Вывод в JSON
python3 run.py --json

# Множественные игры (статистика)
python3 run.py --games 100

# Обучение Q-learning агента
python3 run.py --agent-x qlearning --train 1000 --seed 42
```

### Программное использование

```python
from game import run_game
from agents import RandomAgent, HeuristicAgent

# Создание агентов
agent_x = HeuristicAgent()
agent_o = RandomAgent()

# Запуск игры
slides = run_game(agent_x, agent_o, seed=42)

# Анализ результатов
for slide in slides:
    print(f"Ход {slide['step']}: {slide['current_player']} -> {slide['action']}")

print(f"Победитель: {slides[-1]['winner']}")
```

---

## AI-агенты

| Агент            | Описание                                                              |
|------------------|-----------------------------------------------------------------------|
| `RandomAgent`    | Выбирает случайный допустимый ход                                     |
| `HeuristicAgent` | Использует правила: выигрыш => блокировка => центр => соседние клетки |
| `QLearningAgent` | Табличный Q-learning с жадной стратегией                              |

### Создание нового агента

```python
from agents.base_agent import BaseAgent


class MyAgent(BaseAgent):
    def select_action(self, board: list[list[str]], player_symbol: str) -> tuple[int, int]:
        valid_moves = self._get_valid_moves(board)
        # Ваша логика выбора хода
        return valid_moves[0]
```

---

## 📊 Формат slides

Функция `run_game()` возвращает список шагов:

```python
var = {
    "step": 3,
    "current_player": "X",
    "action": (1, 2),
    "board": [
        ["X", ".", ".", ".", "."],
        [".", "O", "X", ".", "."],
        [".", ".", ".", ".", "."],
        [".", ".", ".", ".", "."],
        [".", ".", ".", ".", "."]
    ],
    "is_terminal": False,
    "winner": None
}
```

---

## 🖼 Визуализация

Модуль `visualization/` генерирует изображения игровых партий из JSON.

### CLI-скрипт

```bash
# Сохранить игру в JSON
python3 run.py --agent-x heuristic --agent-o random --seed 40 --json > game.json

# Сводное изображение игры
python3 visualize.py game.json --summary

# Все шаги по отдельности
python3 visualize.py game.json --all --output output/

# Только финальное состояние
python3 visualize.py game.json --final
```

### Программное использование

```python
from visualization import GameRenderer

renderer = GameRenderer()

# Из JSON-файла
renderer.render_summary("game.json", "summary.png")

# Из списка slides
from game import run_game
from agents import RandomAgent

slides = run_game(RandomAgent(), RandomAgent(), seed=42)
renderer.render_final(slides, "final.png")
```

---

## Тестирование

```bash
# Все тесты
python3 -m pytest tests/ -v

# Конкретный модуль
python3 -m pytest tests/test_board.py -v

# С покрытием
python3 -m pytest tests/ --cov=. --cov-report=term-missing
```

---

## Расширяемость

### Добавление новой игры

1. Создать класс `Board` с методами:
    - `make_move(row, col, player)`
    - `get_valid_moves()`
    - `to_list()`

2. Создать класс `Rules` с методами:
    - `check_winner(board)`
    - `is_terminal(board)`

3. Адаптировать `engine.py` или создать отдельный движок

### Интеграция с платформой

Функция `run_game()` возвращает данные в формате, готовом для:

- REST API
- WebSocket трансляции
- Сохранения в БД
- Визуализации на frontend

---
