from flask import Flask, render_template
import sqlite3

app = Flask(__name__)

def get_logs():
    conn = sqlite3.connect('badges.db')
    cursor = conn.cursor()
    cursor.execute("SELECT uid, timestamp, statut FROM logs ORDER BY id DESC")
    logs = cursor.fetchall()
    conn.close()
    return logs

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/login', methods=['POST'])
def login():
    username = request.form['username']
    password = request.form['password']

    # Exemple simple d'authentification
    if username == 'admin' and password == 'admin123':
        return redirect(url_for('dashboard'))  # ✅ redirection vers /dashboard
    else:
        return 'Identifiants incorrects'

@app.route('/dashboard')
def dashboard():
    return render_template('dashboard.html')  # ✅ fichier dashboard.html dans /templates
