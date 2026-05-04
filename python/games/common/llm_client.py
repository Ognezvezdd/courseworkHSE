from dataclasses import dataclass
import logging
import os
from typing import Optional

import requests


logger = logging.getLogger(__name__)


@dataclass
class LLMResult:
    text: str
    provider: str
    model: str
    used_fallback: bool = False
    fallback_reason: str = ""
    error: Optional[str] = None

class LLMClient:
    def __init__(self, provider="ollama", model="gemma3", api_key=None, api_url=None):
        self.provider = provider
        self.model = model
        self.api_key = api_key or os.getenv("OPENAI_API_KEY")
        self.api_url = api_url

    def call(self, prompt, system_prompt=None):
        return self.call_result(prompt, system_prompt).text

    def call_result(self, prompt, system_prompt=None) -> LLMResult:
        if self.provider == "ollama":
            return self._call_ollama(prompt, system_prompt)
        elif self.provider == "openai":
            return self._call_openai(prompt, system_prompt)
        else:
            raise ValueError(f"Unknown provider: {self.provider}")

    def _call_ollama(self, prompt, system_prompt) -> LLMResult:
        primary_url = self.api_url or os.getenv("OLLAMA_API_URL", "http://host.docker.internal:11434/api/generate")
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
        urls = [primary_url]
        if "host.docker.internal" in primary_url:
            urls.append("http://localhost:11434/api/generate")

        first_error = None
        for index, url in enumerate(urls):
            try:
                r = requests.post(url, json=payload, timeout=60)
                r.raise_for_status()
                text = r.json().get("response", "")
                return LLMResult(
                    text=text,
                    provider=self.provider,
                    model=self.model,
                    used_fallback=index > 0,
                    fallback_reason="primary_ollama_endpoint_unavailable" if index > 0 else "",
                    error=first_error,
                )
            except Exception as exc:
                if first_error is None:
                    first_error = str(exc)
                logger.warning("Ollama request failed for %s: %s", url, exc)

        return LLMResult(
            text="",
            provider=self.provider,
            model=self.model,
            used_fallback=True,
            fallback_reason="ollama_unavailable",
            error=first_error,
        )

    def _call_openai(self, prompt, system_prompt) -> LLMResult:
        if not self.api_key:
            return LLMResult(
                text="",
                provider=self.provider,
                model=self.model,
                used_fallback=True,
                fallback_reason="missing_openai_api_key",
                error="OpenAI API key is not configured",
            )

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
            return LLMResult(
                text=r.json()["choices"][0]["message"]["content"],
                provider=self.provider,
                model=payload["model"],
            )
        except Exception as exc:
            logger.warning("OpenAI request failed: %s", exc)
            return LLMResult(
                text="",
                provider=self.provider,
                model=payload["model"],
                used_fallback=True,
                fallback_reason="openai_unavailable",
                error=str(exc),
            )
