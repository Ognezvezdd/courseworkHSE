import os
import datetime

class InteractionLogger:
    def __init__(self, game_name="bunker"):
        self.game_name = game_name
        self.log_dir = os.path.join(os.path.dirname(__file__), "../../../output/protocols")
        os.makedirs(self.log_dir, exist_ok=True)
        self.current_session_file = None

    def start_session(self, session_id=None):
        if not session_id:
            session_id = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        self.current_session_file = os.path.join(self.log_dir, f"{self.game_name}_{session_id}.md")
        with open(self.current_session_file, "w", encoding="utf-8") as f:
            f.write(f"# Протокол взаимодействия: {self.game_name.capitalize()}\n")
            f.write(f"Дата: {datetime.datetime.now().isoformat()}\n\n")

    def log_interaction(self, agent_name, prompt, response, thinking=""):
        if not self.current_session_file:
            self.start_session()
        
        with open(self.current_session_file, "a", encoding="utf-8") as f:
            f.write(f"## Агент: {agent_name}\n")
            if thinking:
                f.write(f"### Рассуждение (CoT):\n> {thinking}\n\n")
            f.write(f"### Ответ:\n```json\n{response}\n```\n\n")
            f.write("---\n\n")

# Global instance for easy access
bunker_logger = InteractionLogger("bunker")
