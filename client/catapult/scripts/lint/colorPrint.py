import sys
from colorama import Fore, Style

def colorPrint(color, *args):
    print(color + Style.BRIGHT, *args)
    sys.stdout.write(Style.RESET_ALL)

def warning(*args):
    colorPrint(Fore.RED, *args)
