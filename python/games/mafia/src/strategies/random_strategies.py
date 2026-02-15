
import random
from typing import List, Optional
from ..models import GameState, AgentAction, PlayerState
from .base_strategy import RoleStrategy

class RandomMafiaStrategy(RoleStrategy):
    """Случайная стратегия для Мафии: убивает и голосует наугад."""
    def decide_action(self, game_state: GameState, my_role: str = "MAFIA") -> AgentAction:
        phase = game_state.phase
        alive_players = [p.id for p in game_state.players if p.is_alive]
        
        if phase == "NIGHT_MAFIA":
             # Пытается убить случайного живого игрока
             if not alive_players: return AgentAction(action_type="PASS")
             target = random.choice(alive_players)
             return AgentAction(action_type="MAFIA_KILL", target_id=target)
        
        elif phase == "DAY_VOTING":
             if not alive_players: return AgentAction(action_type="PASS")
             target = random.choice(alive_players)
             return AgentAction(action_type="VOTE_KILL", target_id=target)
             
        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Я мирный житель, честно!")

        return AgentAction(action_type="PASS")

class RandomCitizenStrategy(RoleStrategy):
    """Случайная стратегия для Мирного жителя: голосует наугад."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        alive_players = [p.id for p in game_state.players if p.is_alive]

        if phase == "DAY_VOTING":
             if not alive_players: return AgentAction(action_type="PASS")
             target = random.choice(alive_players)
             return AgentAction(action_type="VOTE_KILL", target_id=target)

        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Кто мафия?")

        return AgentAction(action_type="PASS")

class RandomSheriffStrategy(RoleStrategy):
    """Случайная стратегия для Шерифа: проверяет и голосует наугад."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        alive_players = [p.id for p in game_state.players if p.is_alive]

        if phase == "NIGHT_SHERIFF":
             if not alive_players: return AgentAction(action_type="PASS")
             target = random.choice(alive_players)
             return AgentAction(action_type="SHERIFF_CHECK", target_id=target)
        
        elif phase == "DAY_VOTING":
             if not alive_players: return AgentAction(action_type="PASS")
             target = random.choice(alive_players)
             return AgentAction(action_type="VOTE_KILL", target_id=target)

        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Я проверю кого-нибудь.")

        return AgentAction(action_type="PASS")

class RandomDoctorStrategy(RoleStrategy):
    """Случайная стратегия для Доктора: лечит и голосует наугад."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        alive_players = [p.id for p in game_state.players if p.is_alive]

        if phase == "NIGHT_DOCTOR":
            if not alive_players: return AgentAction(action_type="PASS")
            target = random.choice(alive_players)
            return AgentAction(action_type="DOCTOR_HEAL", target_id=target)
             
        elif phase == "DAY_VOTING":
             if not alive_players: return AgentAction(action_type="PASS")
             target = random.choice(alive_players)
             return AgentAction(action_type="VOTE_KILL", target_id=target)
             
        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Надо кого-то лечить.")

        return AgentAction(action_type="PASS")
