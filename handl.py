#!/usr/bin/python
#
# handl.py
# A script to handle on/off button for Redrum
#
# import re
import os
import time
import socket
import select
import traceback
# from collections.abc import Callable


# Seconds between health checks
HEALTHCHECK_TIME = 120
# Whether we should shutdown of an top level exception or not
SHUTDOWN_ON_EXCEPTION = False


def log(tag, logline) -> None:
    """
    Record log record to STDOUT

    :param logline: str to write
    """
    print(f"{time.ctime()}\t{tag}\t{logline}")


def handl_program() -> None:
    """Connect to server, and check for button events
    """
    port = 5000  # initiate port no above 1024

    # socket.socket can work as a context manager, which will automatically
    # close itself when it goes out of scope.
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as conn:
        try:
            conn.connect(("127.0.0.1", port))
            conn.settimeout(60)  # Set timeout at 60 Seconds

            while True:
                # Try and grab an event
                data = conn.recv(1024)

                if not data:
                    conn.sendall("SERV PING\n".encode())
                    data = conn.recv(1024)

                    if not data:
                        break

                else:
                    ndata = data.decode().strip()
                    # log("EVENT", ndata)

                    if ndata == "TRAY_PANEL\tEVENT\tTOG_PC\tTOG\tOFF":
                        # Send etherwake command
                        # sudo etherwake 00:d8:61:50:60:51
                        os.system('etherwake 00:d8:61:50:60:51 -i wlan0')
                        # log("TEST", "OFF")

                    if ndata == "TRAY_PANEL\tEVENT\tTOG_PC\tTOG\tONN":
                        # Send hibernate command!
                        os.system("ssh -t redrum 'sudo systemctl suspend'")
                        # log("TEST", "ONN")

            conn.close()

            # TODO: need to re-open the connection if possible!

        except Exception as err:
            # Log exception
            log("ERR", f"TOPEXCEPT\t{err=}\t{type(err)=}")
            log("ERR", traceback.format_exc())


if __name__ == '__main__':
    handl_program()
