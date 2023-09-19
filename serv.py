#!/usr/bin/python
#
# serv.py
# A simple script to act as a relay between smart-panels
# connected via USB (usually to a RBPi zero) and a
# network client on a desktop computer.
#
# NOTE: Install PySerial via following command;
#       python -m pip install pyserial
import re
import time
import socket
import select
import traceback
from collections.abc import Callable

import serial
from serial.tools import list_ports


# Delay after sending command to panel to wait for a response
DEVICE_RESPONSE_DELAY_INIT = 0.1
DEVICE_RESPONSE_DELAY = 1.0
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


class PanelTimeoutException(Exception):
    "Raised when a panel timeout's on command"
    pass


class Panel:
    """
    Represents a digital panel that can send and receive messages/commands
    over a serial connection on the designated port.

    Optionally manages a collection of panels for pseudo-global state. Example:
    > Panel.panels
    {}
    > Panel.add_panel("panel one", Panel(999))
    > Panel.panels
    {'panel 1': <__main__.Panel object...>, 'panel 2': <__main__.Panel object...>}
    > Panel.delete_panel("panel 1")
    > Panel.panels
    {'panel 2': <__main__.Panel object...>}
    """
    port = None
    connection = None
    last_contact_ts = None
    ident = None

    _panels = None

    @classmethod
    @property
    def panels(cls):
        """Getter for _panels
        """
        return cls._panels or {}

    @classmethod
    def add_panel(cls, panel) -> None:
        """
        Don't allow manual access to _panels; add values here

        :param name: Label for the panel
        :param panel: Panel object
        """
        if not cls._panels:
            cls._panels = {}

        cls._panels[panel.ident] = panel

    @classmethod
    def delete_panel(cls, name) -> None:
        """
        Delete a panel from _panels by name

        :param name: Dict key in _panels
        """
        if not cls._panels:
            raise RuntimeError(
                "delete_panel when _panels hasn't been initialized")

        del cls._panels[name]

    def __init__(self, sport) -> None:
        """
        Create a new Panel. Initialize with its port value.

        :param sport: Port for this panel to open a connection on
        """
        self.port = sport

    def mark(self) -> None:
        """Update last-contact TS
        """
        self.last_contact_ts = time.time()

    def get_event(self) -> str:
        """
        Process received event.

        :return: Single line constituting the event
        """
        if not self.connection:
            raise RuntimeError(
                f"Connection isn't established for {self.ident}")
        self.mark()
        return self.connection.readline().decode().strip()

    def cmd(self, cmd) -> str:
        """
        Send a command and return its (single-line) result.

        :param cmd: Command to execute
        :return: Single-line response
        """
        if not self.connection:
            raise RuntimeError(
                f"Connection isn't established for {self.ident}")
        self.connection.write(f"{cmd}\n".encode())
        time.sleep(DEVICE_RESPONSE_DELAY_INIT)

        resp = self.connection.readline().decode().strip()
        if not resp:
            time.sleep(DEVICE_RESPONSE_DELAY)
            resp = self.connection.readline().decode().strip()

        if not resp:
            raise PanelTimeoutException(
                f"TIMEOUT\twhen calling '{cmd}' for {self.ident}")

        self.mark()
        return resp

    def cmds(self, cmd) -> list[str]:
        """
        Send a command and return its (multi-line) result.

        :param cmd: Command to execute
        :return: List of response lines
        """
        if not self.connection:
            raise RuntimeError(
                f"Connection isn't established for {self.ident}")
        self.connection.write(f"{cmd}\n".encode())
        time.sleep(DEVICE_RESPONSE_DELAY_INIT)

        resp = self.connection.readlines()
        if not resp:
            time.sleep(DEVICE_RESPONSE_DELAY)
            resp = self.connection.readlines()

        if not resp or resp.pop().decode().strip() != "ACK":
            raise PanelTimeoutException(
                f"TIMEOUT\tmissing ACK when calling '{cmd}' for {self.ident}")

        self.mark()
        return list(map(lambda line: line.decode().strip(), resp))

    def is_healthcheck_needed(self) -> bool:
        """
        Check whether we've gone too long without a health check
        :return: True if we need a new healthcheck, False if not
        """
        if not self.last_contact_ts:
            raise RuntimeError(
                f"last_contact_ts isn't established for {self.ident} yet")
        if time.time() - self.last_contact_ts > HEALTHCHECK_TIME:
            return True
        return False

    def open(self) -> None:
        """Open a connection on the defined port and set our device identity
        """
        self.connection = serial.Serial(self.port, 115200, timeout=0)

        # Clear the serial buffer of any debug information
        time.sleep(1)  # Wait to fill buffer
        _ = self.connection.readlines()  # Capture debug data and ignore
        time.sleep(DEVICE_RESPONSE_DELAY)  # Wait to fill buffer

        # Grab IDENT
        self.ident = self.cmd("IDENT")

        if not re.search("PANEL$", self.ident):
            self.connection.close()
            raise ValueError(
                f"{self.port} identifies as '{self.ident}' and not PANEL")

        self.mark()

    def close(self) -> None:
        """Close our connection
        """
        if not self.connection:
            raise RuntimeError(
                f"Connection isn't established for {self.ident}")
        self.connection.close()

    def fileno(self):
        """
        Get our connection's fileno
        :return: fileno for select()
        """
        if not self.connection:
            raise RuntimeError(
                f"Connection isn't established for {self.ident}")
        return self.connection.fileno()

    def ping(self) -> None:
        """Validate our device is responsive
        """
        resp = self.cmd("PING")

        if resp != "PONG":
            # TODO: Isn't this a failure state? Restart connection?
            log("PING", f"DEBUG\t{time.ctime()}\tPing failed for {self.ident}")


def get_send_fn(conn) -> Callable[[str], None]:
    """
    Build a 'send' function using the provided connection

    :param conn: An open serial connection
    :return: A closure over a connection which sends a line of text
    """
    return lambda line: conn.send(f"{line}\n".encode())


def handle_panel_command(send, panel, command) -> None:
    """
    Handle a command for a specified panel

    :param send: 'send' function with connection built in
    :param panel: Panel object
    :param command: Command to process
    """
    resp = panel.cmds(command)
    for line in resp:
        send(line)
    send("ACK")


def handle_event(send,  panel) -> None:
    """
    Handle an EVENT incoming from a panel

    :param send: 'send' function with connection built in
    :param panel: Panel object
    """
    event_str = panel.get_event()
    if event_str:
        send(f"{panel.ident}\t{event_str}")
    else:
        log("EVENT", f"NOTE\t{panel.ident}.get_event() returned None\n")
        # raise RuntimeError(f"{panel.ident}.get_event() returned None\n")


def pop_token(line: str):
    """
    Pull the first token off a string, and return the remainder
    """
    tokens = line.split(" ", 1)
    token = tokens.pop(0)
    rest = tokens[0] if tokens else None
    return (token, rest)


def handle_network_command(conn) -> bool:
    """
    Handle a command from the network

    :param panels: Dict of name: Panel pairs
    :param conn: Some kind of connection/file descriptor
    :return: True for success, False otherwise?
    """
    try:
        data = conn.recv(1024)
    except OSError as err:
        log("ERR", f"DISCONNECT\tOSError on recv\t{type(err)=}\t{err=}")
        return False

    # The case the connection is closed
    if not data:
        return False

    # Figure out who this command is addressed to
    (addr, cline) = pop_token(data.decode().strip())

    # Produce send() to standardize below
    send = get_send_fn(conn)

    # Validate input
    if cline is None:
        send("ERR\tMalformed command")
        log("ERR", f"Malformed command: {data}\n")
        return True

    try:
        if addr == "SERV":
            (cmd, params) = pop_token(cline)

            if cmd == "LIST":
                for (device, desc, _) in list_ports.grep("^/dev/ttyUSB.*"):
                    send(f"{device}\t{desc}")

            elif cmd == "OPEN":
                if params is None:
                    send("ERR\tMust provide port to open")
                    return True

                panel = Panel(params)

                try:
                    panel.open()

                except Exception as err:
                    log("ERR", f"ERROR\twhile doing\t'{data}'")
                    log("ERR", f"EXCEPTION\t{err=}\t{type(err)}")
                    log("ERR", traceback.format_exc())
                    send(f"ERR\t{err=}")
                    return True

                if panel.ident is not None:
                    Panel.add_panel(panel)
                    log("LOG", f"OPEN\t{panel.ident}\t{panel.port}")
                    send(panel.ident)

            elif cmd == "OPENALL":
                for (device, desc, _) in list_ports.grep("^/dev/ttyUSB.*"):
                    try:
                        panel = Panel(device)
                        panel.open()

                        if panel.ident is not None:
                            Panel.add_panel(panel)
                            log("LOG", f"OPENALL\t{panel.ident}\t{panel.port}")
                            send(f"{panel.ident}\t{device}")
                    except Exception as err:
                        log("ERR",
                            f"OPENALL\tERR\t{device}\t{type(err)=}\t{err=}")

            elif cmd == "CLOSE":
                if params is None:
                    send("ERR\tMust provide panel IDENT to close")
                    return True

                panel = Panel.panels.get(params)
                if panel is None:
                    send(f"ERR\tPanel '{params}' is not found")

                else:
                    panel.close()
                    log("LOG", f"CLOSE\t{panel.ident}\t{panel.port}")
                    Panel.delete_panel(params)

            elif cmd == "AVAIL":
                for panel in Panel.panels.values():
                    send(f"{panel.ident}\t{panel.port}")

            elif cmd == "DEBUG":
                if params is None:
                    send("ERR\tMust provide provide panel ident and command")
                    return True

                (panel_name, op) = pop_token(params)
                if op is None:
                    send("ERR\tDEBUG, panel command not found")
                    return True

                panel = Panel.panels.get(panel_name)
                if panel is None:
                    send(f"ERR\tPanel '{panel_name}' is not found")
                else:
                    send(panel.cmd(op))

            elif cmd == "PING":
                send("PONG")

            send("ACK")
        else:
            panel = Panel.panels.get(addr)

            if panel is None:
                send(f"ERR\tPanel '{addr}' is not found\n")
            else:
                handle_panel_command(send, panel, cline)

    except PanelTimeoutException as err:
        log("ERR", f"Panel Timeout Exception\t{err=}")
        send(f"ERR\tTIMEOUT\t{err=}")

    return True


def server_program() -> None:
    """Start, run, and attempt to gracefully shut down our server
    """
    port = 5000  # initiate port no above 1024

    # socket.socket can work as a context manager, which will automatically
    # close itself when it goes out of scope.
    with socket.socket() as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # look closely. The bind() function takes tuple as argument
        server_socket.bind(('', port))  # bind host address and port together

        # configure how many client the server can listen simultaneously
        server_socket.listen(3)

        rlist = [server_socket]

        # Loop over connections
        while True:
            try:

                rl, _, xl = select.select(
                    rlist + list(Panel.panels.values()),
                    [],
                    rlist + list(Panel.panels.values()),
                    HEALTHCHECK_TIME,
                )

                # Iterate over readable events
                for fd in rl:
                    # Case we have a new network connection
                    if fd is server_socket:
                        client_socket, address = server_socket.accept()
                        rlist.append(client_socket)
                        log("LOG", f"CONN\t{str(address[0])}")

                    # Case we have an EVENT from a panel
                    elif fd in list(Panel.panels.values()):
                        # Gotta send event to each network connection
                        event_str = fd.get_event()
                        if event_str:

                            # Don't send the event to the server socket...
                            for conn in filter(
                                lambda fd: fd is not server_socket,
                                rlist,
                            ):
                                send = get_send_fn(conn)
                                send(f"{fd.ident}\t{event_str}")
                        else:
                            raise RuntimeError(
                                f"{fd.ident}.get_event() returned None\n")

                    # Case we have a network command
                    else:
                        try:
                            if not handle_network_command(fd):
                                # Handle the case connection is closed
                                rlist.remove(fd)

                        # Just disconnect if user does ^C or something similar
                        except UnicodeDecodeError:
                            fd.close()
                            rlist.remove(fd)

                # Iterate through panels to perform healthchecks
                for panel in Panel.panels.values():
                    if panel.is_healthcheck_needed():
                        try:
                            panel.ping()

                        # Report failed healthchecks to any client connected
                        except PanelTimeoutException:
                            for conn in filter(
                                lambda fd: fd is not server_socket,
                                rlist,
                            ):
                                panel.mark()  # Don't complain about this again until the next scheduled healthcheck
                                conn.send(
                                    f"{panel.ident}\tEVENT\tSERV_PING\tPING\tNOPONG\n"
                                    .encode())

                for x in xl:
                    log("DEBUG", f"select().xl is {x}")

            except Exception as err:
                # Log exception
                log("ERR", f"TOPEXCEPT\t{err=}\t{type(err)=}")
                log("ERR", traceback.format_exc())

                # Send info to clients
                for conn in filter(
                    lambda fd: fd is not server_socket,
                    rlist,
                ):
                    conn.send(f"ERR\tTOPEX\t{type(err)=}\t{err=}\n".encode())

                if SHUTDOWN_ON_EXCEPTION:
                    panels = list(Panel.panels.values())
                    for panel in panels:
                        log("LOG", f"Closing panel {panel.ident}")
                        Panel.delete_panel(panel.ident)
                        panel.close()


if __name__ == '__main__':
    server_program()
