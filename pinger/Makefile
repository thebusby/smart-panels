all: pinger
pinger: pinger.c
	gcc -Wall pinger.c -o pinger
	sudo setcap cap_net_raw+ep pinger
