import serial
import sqlite3
import time

PORT = 'COM4'
BAUD_RATE = 9600
DB_PATH = 'badges.db'

conn = sqlite3.connect(DB_PATH)
cursor = conn.cursor()

cursor.execute('''
CREATE TABLE IF NOT EXISTS logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uid TEXT NOT NULL,
    timestamp TEXT NOT NULL,
    statut TEXT NOT NULL
)
''')
conn.commit()

autorises = ["C3AD2113", "C31C6D93","00E4DA11"]

ser = serial.Serial(PORT, BAUD_RATE)
print(f"[INFO] Connexion sur {PORT} ")

try:
    while True:
        if ser.in_waiting:
            ligne = ser.readline().decode('utf-8').strip()
            if ligne.startswith("UID:"):
                uid_lu = ligne.replace("UID:", "").replace(" ", "").replace("-", "").strip().upper()
                print(f"Badge detecte: {uid_lu}")

                if uid_lu in autorises:
                    statut = "Acces autorise"
                else:
                    statut = "Acces refuse"


                cursor.execute("INSERT INTO logs (uid, timestamp, statut) VALUES (?, ?, ?)",
                        (uid_lu, time.strftime('%Y-%m-%d %H:%M:%S'), statut))
                conn.commit()
                print(f"[INFO] {statut} badges.db")

except KeyboardInterrupt:
    print("[ARRET] Fermeture propre.")
    ser.close()
    conn.close()
