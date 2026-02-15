
import random
from typing import List, Dict, Optional
from mafia.models import GameState, AgentAction, PlayerState

class BaseAgent:
    def __init__(self, name: str):
        self.name = name
    
    def decide_action(self, game_state: GameState) -> AgentAction:
        """Решает, какое действие выполнить (в зависимости от роли)"""
        raise NotImplementedError

    def _get_random_alive_player_id(self, game_state: GameState, exclude: List[int] = []) -> int:
        alive_players = [
            p.id for p in game_state.players 
            if p.is_alive and p.id not in exclude
        ]
        if not alive_players:
            return -1
        return random.choice(alive_players)

class MafiaAgent(BaseAgent):
    """Агент мафии. Убивает и голосует против всех."""
    def decide_action(self, game_state: GameState, my_role: str = "MAFIA") -> AgentAction:
        phase = game_state.phase
        if phase == "NIGHT_MAFIA":
             # Выбирает жертву из живых мирных (если такие есть)
             target = self._get_random_alive_player_id(game_state) # Упрощенно: рандом
             return AgentAction(action_type="MAFIA_KILL", target_id=target)
        
        elif phase == "DAY_VOTING":
             # Днем мафия голосует против кого-то 
             target = self._get_random_alive_player_id(game_state)
             return AgentAction(action_type="VOTE_KILL", target_id=target)
             
        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Я мирный житель, верьте мне!")

        return AgentAction(action_type="PASS")

class CitizenAgent(BaseAgent):
    """Мирный житель. Пытается найти мафию."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        if phase == "DAY_VOTING":
             # Голосует случайно (пока логики нет)
             target = self._get_random_alive_player_id(game_state)
             return AgentAction(action_type="VOTE_KILL", target_id=target)

        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Кто же мафия?")

        return AgentAction(action_type="PASS")

class SheriffAgent(BaseAgent):
    """Шериф. Проверяет игроков ночью."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        if phase == "NIGHT_SHERIFF":
             target = self._get_random_alive_player_id(game_state)
             return AgentAction(action_type="SHERIFF_CHECK", target_id=target)
        
        elif phase == "DAY_VOTING":
             target = self._get_random_alive_player_id(game_state)
             return AgentAction(action_type="VOTE_KILL", target_id=target)

        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Я проверю подозрительных.")

        return AgentAction(action_type="PASS")

class DoctorAgent(BaseAgent):
    """Доктор. Лечит игроков ночью."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        if phase == "NIGHT_DOCTOR":
             target = self._get_random_alive_player_id(game_state)
             return AgentAction(action_type="DOCTOR_HEAL", target_id=target)
             
        elif phase == "DAY_VOTING":
             target = self._get_random_alive_player_id(game_state)
             return AgentAction(action_type="VOTE_KILL", target_id=target)
             
        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Я вылечу кого-нибудь.")

        return AgentAction(action_type="PASS")
