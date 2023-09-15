#!/usr/bin/python
#
# serv.py
# A simple script to act as a relay between smart-panels
# connected via USB (usually to a RBPi zero) and a 
# network client on a desktop computer. 
# 
# NOTE: Install PySerial via following command;
#       python -m pip install pyserial 
import socket
import select
import time
import re
import traceback
import serial
import serial.tools.list_ports


# 
# CONSTANTS
# 
DEVICE_RESPONSE_DELAY=1.0 # Delay after sending command to panel to wait for a response
HEALTHCHECK_TIME=120 # Seconds between health checks


# 
# Manage some global state
# 
panels = {}


# 
# Define Panel
# 
class Panel:
    # Define member variables here
    #
    # sport is the Serial Port Ex. /dev/tty.USB0
    # ser is the serial port connection
    # ts is a timestamp of the last contact with the device
    # ident is the device identifier


    def __init__(self, sport) -> None:
        self.sport = sport
        self.ser = None
        self.ts = time.time()
        self.ident = None

   
    # An EVENT was detected, return the single line that constitutes the EVENT
    def get_event(self) -> str:
        if self.ser is None:
            raise RuntimeError("Serial connection is not open")

        self.mark()
        return self.ser.readline().decode().rstrip('\n\r')


    # Send cmd and get single line response
    def cmd(self, cmd) -> str:
        if self.ser is None:
            raise RuntimeError("Serial connection is not open")

        self.ser.write(f"{cmd}\n".encode())
        time.sleep(DEVICE_RESPONSE_DELAY)
        self.mark()
        return self.ser.readline().decode().rstrip('\n\r')


    # Send cmd and get back list of lines as response
    # Note: final ACK removed
    def cmds(self, cmd) -> list[str]:
        if self.ser is None:
            raise RuntimeError("Serial connection is not open")

        self.ser.write(f"{cmd}\n".encode())
        time.sleep(DEVICE_RESPONSE_DELAY)
        self.mark()

        resp = self.ser.readlines()

        # Validate we got ACK
        if resp.pop().decode().rstrip('\n\r') != "ACK":
            raise RuntimeError(f"{self.ident} failed to ACK for {cmd}")

        # rval = []
        # for line in resp:
        #     rval.append( line.decode().rstrip('\n\r') )

        return list(map(lambda line: line.decode().rstrip('\n\r'), resp))



    # Update ts
    def mark(self) -> None:
        self.ts = time.time()


    # Return true if healthy, false otherwise
    def is_healthcheck_needed(self) -> bool:
        if time.time() - self.ts > HEALTHCHECK_TIME:
            return True
        return False


    # Open serial port and get device IDENT
    def open(self) -> None:

        # Try and open the serial connection
        self.ser = serial.Serial(self.sport, 115200, timeout=0)
        
        # Let's clear the serial buffer of any debug information
        time.sleep(1) # Wait to fill buffer
        self.ser.readlines() # Capture debug data and ignore
        time.sleep(DEVICE_RESPONSE_DELAY) # Wait to fill buffer

        # Grab IDENT
        self.ident = self.cmd("IDENT")

        if not re.search("PANEL$", self.ident):
            self.ser.close()
            raise ValueError(f"{self.sport} identifies as {self.ident} and not PANEL") 


    # Close the serial port
    def close(self) -> None:
        if self.ser is None:
            raise RuntimeError("Serial connection is not open")

        self.ser.close()


    # Return fileno for select()
    def fileno(self):
        if self.ser is None:
            raise RuntimeError("Serial connection is not open")

        return self.ser.fileno()


    # Do ping/pong with device
    def ping(self) -> None:
        resp = self.cmd("PING")

        if resp != "PONG":
            print(f"DEBUG\t{time.ctime()}\tPing failed for {self.ident}")

        


# Return a closure over a connection which sends a line of text
def get_send_fn(conn):
    return lambda line: conn.send(f"{line}\n".encode())

# Handle a command for a specific panel
def handle_panel_command(send, panel, command):
    resp = panel.cmds(command)
    for line in resp:
        send(line)
    

# Handle an EVENT incoming from a panel
def handle_event(send,  panel):
    send(panel.get_event())


# Handle a command from the network
def handle_network_command(conn) -> bool:
    global panels
    
    # receive data stream. it won't accept data packet greater than 1024 bytes
    try:
        data = conn.recv(1024)
    except OSError as err:
        print(f"DISCONNECT\tException on recv\t{type(err)=}\t{err=}")
        return False
    
    if not data:
        print(f"DISCONNECT\tNo data recv()")
        return False
    
    # Figure out who this command is addressed to
    try:
        (addr, cline) = data.decode().rstrip('\r\n').split(" ", 1)
    except Exception as err:
        print(f"DISCONNECT\tException on input\t{type(err)=}\t{err=}")
        return False;

    # Let's standardize on using this
    send = get_send_fn(conn)

    # Handle commands against serv.py
    if addr == "SERV":
        (cmd, params) = cline.split(" ", 1)
    
        # List all valid Serial devices
        if cmd == "LIST":
            for (device, desc, _) in serial.tools.list_ports.grep("^/dev/ttyUSB.*"):
                send(f"{device}\t{desc}")

        # Open a serial device for panel protocol
        if cmd == "OPEN":
            panel = Panel(params)

            try:
                panel.open()

            except Exception as err:
                print(f"ERROR\twhile doing\t'{data}'")
                print(f"EXCEPTION\t{err=}\t{type(err)=}")
                print(traceback.format_exc())
                send(f"ERR\t{err=}")
            
            send(panel.ident)

        if cmd == "CLOSE":
            panel = panels.get(params)
            if panel is None:
                conn.send(f"ERR\tPanel '{params}' is not found")

            else:
                panel.close()
                panels.pop(params)

        # List available devices by their IDENT
        if cmd == "AVAIL":
            for panel in panels.keys():
                send(panel)

        if cmd == "PING":
            send("PONG")
    
        # ACK response
        send("ACK")


    # Handle commands against a panel
    else:
        panel = panels.get(addr)
        if panel is None:
            conn.send(f"ERR\tPanel '{addr}' is not found")

        else:
            handle_panel_command(send, panel, cline)

    return True


def server_program():
    global panels

    # get the hostname
    # host = socket.gethostname()
    port = 5000  # initiate port no above 1024

    server_socket = socket.socket()  # get instance
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # look closely. The bind() function takes tuple as argument
    server_socket.bind(('', port))  # bind host address and port together

    # configure how many client the server can listen simultaneously
    server_socket.listen(3)

    rlist = [ server_socket ]

    # Loop over connections
    while True:
        try: 
            rl, _, xl = select.select( rlist + list(panels.values()), [], rlist + list(panels.values()), HEALTHCHECK_TIME)

            # Iterate over readable events
            for fd in rl:

                # Case we have a new network connection
                if fd is server_socket:
                    client_socket, address = server_socket.accept()
                    rlist.append(client_socket)
                    print("CONN\t" + str(address[0]))

                # Case we have an EVENT from a panel
                elif fd in list(panels.values()):
                    # Gotta send event to each network connection
                    for conn in list(filter(lambda fd: fd is not server_socket, rlist)):
                        handle_event(get_send_fn(conn), fd)

                # Case we have a network command
                else:
                    if not handle_network_command(fd):
                        # Handle the case connection is closed
                        rlist.remove(fd)

            # Manage health checks
            for panel in panels:
                if panel.is_healthcheck_needed():
                    panel.ping()

            # Report on any exceptions
            for x in xl:
                print(f"DEBUG\tselect().xl is {x}")


        # Let's at least try and shutdown cleanly...
        except Exception as err:
            # conn.close()  # close the connection
            # print(exc_type, fname, exc_tb.tb_lineno)
            print(f"TOPEXCEPT\t{err=}\t{type(err)=}")
            print(traceback.format_exc())
            # server_socket.shutdown(socket.SHUT_RDWR)
            # server_socket.close()
            # raise


if __name__ == '__main__':
    server_program()
