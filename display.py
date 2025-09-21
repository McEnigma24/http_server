#!/usr/bin/env python3

import socket
import threading
import tkinter as tk
import argparse
import time
import pyphen

HOST = '0.0.0.0'
PORT = 12345
BUFFER_SIZE = 1024

# Inicjuj hyphenator dla języka polskiego
# print(pyphen.LANGUAGES.keys())
hyph = pyphen.Pyphen(lang='pl')

def hyphenate_long_word(word, max_len):
    """
    Jeśli słowo jest dłuższe niż max_len, spróbuj je podzielić z użyciem pyphen,
    zwracając tuple (first_part+'-', rest) jeśli się da, w przeciwnym wypadku całe słowo.
    """
    if len(word) <= max_len:
        return word, None
    # pyphen.inserted zwraca string z '-' w miejscach podziału sylab
    # np. "przykład" -> "przyk-ład"
    inserted = hyph.inserted(word)
    parts = inserted.split('-')
    # Próbujemy znaleźć podział który pozwoli, żeby first part + '-' się zmieściło
    cum = ''
    for i in range(len(parts)-1, 0, -1):  # spróbuj od największej części
        first = ''.join(parts[:i])
        rest = ''.join(parts[i:])
        if len(first) + 1 <= max_len:  # +1 bo '-' dołączy
            return first + '-', rest
    # jeśli żaden podział się nie udał, po prostu zwróć całe słowo
    return word, None

def hyphenate_text(text, max_chars_per_line):
    """
    Dzieli text na linie, tak by żadna linia nie przekraczała max_chars_per_line znaków,
    hyphenizuje pojedyncze słowa jeśli są za długie.
    """
    words = text.split()
    lines = []
    current = ""
    for w in words:
        # jeśli dodanie słowa do current przekroczy limit
        if current:
            # +1 dla spacji
            prospective = current + ' ' + w
        else:
            prospective = w
        if len(prospective) <= max_chars_per_line:
            current = prospective
        else:
            # prospective jest za długie
            # sprawdź czy samo w jest za długie
            if len(w) > max_chars_per_line:
                # spróbuj hyphenować
                first, rest = hyphenate_long_word(w, max_chars_per_line - (len(current) + 1) if current else max_chars_per_line)
                if rest:
                    # jeśli current nie jest pusty, zakończ linię na current
                    if current:
                        lines.append(current + ' ' + first if current else first)
                    else:
                        lines.append(first)
                    # rozpocznij nową linię od rest
                    current = rest
                else:
                    # nie udało się hyphenować, cały w do nowej linii
                    if current:
                        lines.append(current)
                    current = w
            else:
                # całe słowo w przejdzie do nowej linii
                if current:
                    lines.append(current)
                current = w
    if current:
        lines.append(current)
    return "\n".join(lines)

class SocketListener(threading.Thread):
    def __init__(self, host, port, on_message):
        super().__init__(daemon=True)
        self.host = host
        self.port = port
        self.on_message = on_message
        self.running = True

    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((self.host, self.port))
            s.listen(1)
            while self.running:
                try:
                    conn, addr = s.accept()
                    with conn:
                        while self.running:
                            data = conn.recv(BUFFER_SIZE)
                            if not data:
                                break
                            msg = data.decode('utf-8', errors='ignore').strip()
                            self.on_message(msg)
                except Exception as e:
                    print("SocketListener error:", e)
                    time.sleep(1)
        print("SocketListener zakończony")

    def stop(self):
        self.running = False

def hyphenate_text_preserve_newlines(text, max_chars_per_line):
    """
    Dzieli text na linie, zachowując istniejące znaki '\n'.
    Każda linia jest ograniczona do max_chars_per_line,
    hyphenizuje słowa dłuższe niż limit.
    """
    final_lines = []
    # Rozdziel tekst na linie według istniejących \n
    for line in text.split('\n'):
        words = line.split()
        current = ""
        for w in words:
            # jeśli dodanie słowa do current przekroczy limit
            prospective = current + (' ' if current else '') + w
            if len(prospective) <= max_chars_per_line:
                current = prospective
            else:
                # prospective jest za długie
                if len(w) > max_chars_per_line:
                    first, rest = hyphenate_long_word(
                        w, max_chars_per_line - (len(current) + 1) if current else max_chars_per_line
                    )
                    if current:
                        final_lines.append(current + ' ' + first if current else first)
                    else:
                        final_lines.append(first)
                    current = rest
                else:
                    if current:
                        final_lines.append(current)
                    current = w
        if current:
            final_lines.append(current)
    return "\n".join(final_lines)

def main():
    parser = argparse.ArgumentParser(description="GUI display z flagą fullscreen")
    parser.add_argument('--fullscreen', action='store_true',
                        help='Uruchom w trybie pełnoekranowym')
    args = parser.parse_args()

    root = tk.Tk()

    root.update_idletasks()
    screen_w = root.winfo_screenwidth()
    screen_h = root.winfo_screenheight()
    print(f"[INFO] Wymiary ekranu: {screen_w}×{screen_h}")

    if args.fullscreen:
        root.geometry(f"{screen_w}x{screen_h}+0+0")
        root.overrideredirect(True)
        root.attributes('-fullscreen', True)
        root.attributes('-topmost', True)
    else:
        w = 800
        h = 480
        root.geometry(f"{w}x{h}+0+0")

    root.configure(bg='black')

    # label = tk.Label(root, text="Czekam na dane...", font=('Arial', 40),
    #                  fg='white', bg='black', justify='left')
    
    label = tk.Label(root, text="Czekam na dane...", font=('Arial', 40),
                 fg='white', bg='black', justify='left', wraplength=screen_w * 0.8)

    label.pack(expand=True, fill='both')

    label.config(text="Linia 1\nLinia 2\nLinia 3")

    # Określ maksymalną liczbę znaków na linię – możesz to obliczyć dynamicznie:
    # Na przykład zakładając że czcionka Arial 40, jedna litera zajmuje ~X pikseli... Można użyć font.measure itd.
    # Teraz daję przykładowo 30 znaków:
    MAX_CHARS = 30

    def on_message(msg):
        # new_text = msg
        new_text = hyphenate_text_preserve_newlines(msg, MAX_CHARS)
        root.after(0, label.config, {'text': new_text})

    listener = SocketListener(HOST, PORT, on_message)
    listener.start()

    def on_key(event):
        if event.keysym == 'Escape':
            listener.stop()
            root.destroy()

    root.bind('<Escape>', on_key)

    root.mainloop()

if __name__ == '__main__':
    main()
