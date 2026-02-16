
import random
from typing import List, Optional
from ..models import GameState, AgentAction, PlayerState
from .base_strategy import RoleStrategy

class RandomMafiaStrategy(RoleStrategy):
    """Случайная стратегия для Мафии: убивает и голосует наугад, не трогает своих."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        # Список живых НЕ мафиози
        enemies = [p.id for p in game_state.players if p.is_alive and p.role.lower() not in ["mafia", "don"]]
        all_alive = [p.id for p in game_state.players if p.is_alive]
        
        if phase == "NIGHT_MAFIA":
             if not enemies: return AgentAction(action_type="PASS")
             # Все мафиози выберут одну и ту же цель на основе внешнего сида (например, дня игры)
             # Или просто самого первого из списка выживших врагов (простейшая координация)
             target = enemies[0] 
             return AgentAction(action_type="MAFIA_KILL", target_id=target)
        
        elif phase == "NIGHT_DON":
             # Дон ищет шерифа среди тех, кто не мафия
             if not enemies: return AgentAction(action_type="PASS")
             target = random.choice(enemies)
             return AgentAction(action_type="DON_CHECK", target_id=target)

        elif phase == "DAY_VOTING":
             if not enemies: return AgentAction(action_type="PASS")
             target = random.choice(enemies)
             return AgentAction(action_type="VOTE_KILL", target_id=target)
             
        elif phase == "DAY_DISCUSSION":
            phrases = [
                "Я обычный мирный, не тратьте на меня время.",
                "Давайте присмотримся к тем, кто молчит.",
                "Сложная ночь... Но мы победим!",
                "Я думаю, мафия среди нас, но это не я."
            ]
            return AgentAction(action_type="CHAT_MESSAGE", text_message=random.choice(phrases))

        return AgentAction(action_type="PASS")

class RandomCitizenStrategy(RoleStrategy):
    """Случайная стратегия для Мирного жителя: голосует наугад, не за себя."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        # Все живые, кроме себя
        others = [p.id for p in game_state.players if p.is_alive and p.id != game_state.my_id]

        if phase == "DAY_VOTING":
             if not others: return AgentAction(action_type="PASS")
             target = random.choice(others)
             return AgentAction(action_type="VOTE_KILL", target_id=target)

        elif phase == "DAY_DISCUSSION":
            phrases = [
                "Кто мафия? Есть подозрения?",
                "Я верю, что мы найдем убийц.",
                "Шериф, дай нам знак!",
                "Подозрительно всё это...",
                "Мне кажется, игрок рядом что-то скрывает."
            ]
            return AgentAction(action_type="CHAT_MESSAGE", text_message=random.choice(phrases))

        return AgentAction(action_type="PASS")

class RandomSheriffStrategy(RoleStrategy):
    """Случайная стратегия для Шерифа: проверяет и голосует наугад."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        others = [p.id for p in game_state.players if p.is_alive and p.id != game_state.my_id]

        if phase == "NIGHT_SHERIFF":
             if not others: return AgentAction(action_type="PASS")
             target = random.choice(others)
             return AgentAction(action_type="SHERIFF_CHECK", target_id=target)
        
        elif phase == "DAY_VOTING":
             if not others: return AgentAction(action_type="PASS")
             target = random.choice(others)
             return AgentAction(action_type="VOTE_KILL", target_id=target)

        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Я анализирую всех. Скоро мы их поймаем.")

        return AgentAction(action_type="PASS")

class RandomDoctorStrategy(RoleStrategy):
    """Случайная стратегия для Доктора: лечит и голосует наугад."""
    def decide_action(self, game_state: GameState) -> AgentAction:
        phase = game_state.phase
        alive_players = [p.id for p in game_state.players if p.is_alive]
        others = [p.id for p in game_state.players if p.is_alive and p.id != game_state.my_id]

        if phase == "NIGHT_DOCTOR":
            if not alive_players: return AgentAction(action_type="PASS")
            # Доктор может лечить и себя
            target = random.choice(alive_players)
            return AgentAction(action_type="DOCTOR_HEAL", target_id=target)
             
        elif phase == "DAY_VOTING":
             if not others: return AgentAction(action_type="PASS")
             target = random.choice(others)
             return AgentAction(action_type="VOTE_KILL", target_id=target)
             
        elif phase == "DAY_DISCUSSION":
            return AgentAction(action_type="CHAT_MESSAGE", text_message="Надеюсь, сегодня ночью никто не пострадает.")

        return AgentAction(action_type="PASS")
