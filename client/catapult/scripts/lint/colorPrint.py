import sys

from colorama import Fore, Style


def color_print(color, *args):
	print(color + Style.BRIGHT, *args)
	sys.stdout.write(Style.RESET_ALL)


def warning(*args):
	color_print(Fore.RED, *args)
