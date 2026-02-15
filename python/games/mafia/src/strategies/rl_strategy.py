
from typing import Dict, Any, List
from mafia.models import GameState, AgentAction
from mafia.strategies.base_strategy import RoleStrategy
import random

class LearningStrategy(RoleStrategy):
    """
    Базовая заготовка для RL (Reinforcement Learning) стратегии.
    
    Идея:
    1. Агент имеет 'policy' (политику), которая выбирает действие.
    2. Агент запоминает (state, action, reward).
    3. После игры (эпизода) происходит обучение.

    Здесь пока заглушка, но с правильными интерфейсами.
    """
    def __init__(self, role_name: str, epsilon: float = 0.1):
        self.role_name = role_name
        self.epsilon = epsilon
        # Пример Q-таблицы: state_hash -> {action_hash -> q_value}
        self.q_table: Dict[str, Dict[str, float]] = {} 
        self.last_state = None
        self.last_action = None

    def _get_state_representation(self, game_state: GameState) -> str:
        """
        Превращает сложное состояние игры в упрощенный вектор/хэш для RL.
        Например: "Day:1,Alive:5,MyRole:MAFIA"
        """
        # Упрощенная заглушка
        alive_count = len([p for p in game_state.players if p.is_alive])
        return f"{game_state.phase}:{game_state.day}:{alive_count}:{self.role_name}"

    def _get_possible_actions(self, game_state: GameState) -> List[AgentAction]:
        """Возвращает список всех валидных действий в текущем состоянии."""
        actions = []
        alive_players = [p.id for p in game_state.players if p.is_alive]
        
        # Пример для голосования
        if "VOTE" in game_state.phase:
            for pid in alive_players:
                actions.append(AgentAction(action_type="VOTE_KILL", target_id=pid))
        
        # Пример для ночных действий
        elif "NIGHT" in game_state.phase:
             if self.role_name == "MAFIA":
                 for pid in alive_players:
                    actions.append(AgentAction(action_type="MAFIA_KILL", target_id=pid))
             # ... и так далее для других ролей
        
        if not actions:
            actions.append(AgentAction(action_type="PASS"))
        
        return actions

    def decide_action(self, game_state: GameState) -> AgentAction:
        state_key = self._get_state_representation(game_state)
        possible_actions = self._get_possible_actions(game_state)

        # Epsilon-greedy: иногда выбираем случайное
        if random.random() < self.epsilon:
            chosen_action = random.choice(possible_actions)
        else:
            # Выбираем лучшее действие из Q-таблицы (или рандом если нет данных)
            if state_key in self.q_table:
                # Тут логика выбора action с max Q-value
                # (Для простоты пока рандом, т.к. Q-таблица пуста)
                chosen_action = random.choice(possible_actions)
            else:
                 chosen_action = random.choice(possible_actions)

        self.last_state = state_key
        self.last_action = chosen_action
        return chosen_action

    def update(self, reward: float, next_state: GameState):
        """Здесь должна быть логика обновления Q-table (Bellman equation)."""
        pass
