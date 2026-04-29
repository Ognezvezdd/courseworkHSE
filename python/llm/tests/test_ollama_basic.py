import pytest

ollama = pytest.importorskip("ollama")


def test_ollama_basic():
    try:
        response = ollama.chat(
            model="gemma3",
            messages=[{"role": "user", "content": "Why is the sky blue?"}],
        )
    except Exception as exc:
        pytest.skip(f"Ollama is not available: {exc}")

    assert response["message"]["content"]
