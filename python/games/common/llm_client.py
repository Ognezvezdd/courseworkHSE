import os
import requests
import json
import re

class LLMClient:
    def __init__(self, provider="ollama", model="gemma3", api_key=None, api_url=None):
        self.provider = provider
        self.model = model
        self.api_key = api_key or os.getenv("OPENAI_API_KEY")
        self.api_url = api_url

    def call(self, prompt, system_prompt=None):
        if self.provider == "ollama":
            return self._call_ollama(prompt, system_prompt)
        elif self.provider == "openai":
            return self._call_openai(prompt, system_prompt)
        else:
            raise ValueError(f"Unknown provider: {self.provider}")

    def _call_ollama(self, prompt, system_prompt):
        url = self.api_url or os.getenv("OLLAMA_API_URL", "http://host.docker.internal:11434/api/generate")
        payload = {
            "model": self.model,
            "prompt": prompt,
            "system": system_prompt,
            "stream": False,
            "options": {
                "temperature": 0.7,
                "num_predict": 512
            }
        }
        try:
            r = requests.post(url, json=payload, timeout=60)
            r.raise_for_status()
            return r.json().get("response", "")
        except Exception as e:
            print(f"Ollama error: {e}")
            return ""

    def _call_openai(self, prompt, system_prompt):
        url = self.api_url or "https://api.openai.com/v1/chat/completions"
        headers = {
            "Authorization": f"Bearer {self.api_key}",
            "Content-Type": "application/json"
        }
        messages = []
        if system_prompt:
            messages.append({"role": "system", "content": system_prompt})
        messages.append({"role": "user", "content": prompt})
        
        payload = {
            "model": self.model if self.model.startswith("gpt") else "gpt-4o-mini",
            "messages": messages,
            "temperature": 0.7
        }
        try:
            r = requests.post(url, json=payload, headers=headers, timeout=60)
            r.raise_for_status()
            return r.json()["choices"][0]["message"]["content"]
        except Exception as e:
            print(f"OpenAI error: {e}")
            return ""
