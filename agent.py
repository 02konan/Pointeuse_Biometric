import os
import time
import subprocess

# Liste statique des IP des pointeuses
POINTEUSES = ["192.168.1.45", "192.168.1.46"]

def ping(ip):
    try:
        param = "-n" if os.name == "nt" else "-c"
        subprocess.check_output(["ping", param, "1", ip], stderr=subprocess.STDOUT)
        return True
    except subprocess.CalledProcessError:
        return False

def run_agent():
    while True:
        for ip in POINTEUSES:
            if ping(ip):
                print(f"[OK] Pointeuse {ip} accessible")
            else:
                print(f"[X] Pointeuse {ip} inaccessible")
        time.sleep(10)

if __name__ == "__main__":
    print("=== Agent Local - Ping statique démarré ===")
    run_agent()