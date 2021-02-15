import signal
import sys
import time


def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')
    sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)
print('Press Ctrl+C')
#signal.pause()

x = 1
while True:
    print(x)
    time.sleep(0.1)
    x = x + 1



