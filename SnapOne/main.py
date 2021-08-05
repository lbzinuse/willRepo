#!/usr/bin/env python
#
# https://github.com/fgimian/paramiko-expect/blob/master/examples/paramiko_expect-demo.py
#
from __future__ import print_function
import traceback

import paramiko
from paramiko_expect import SSHClientInteraction

# def change_audio():
#     interact.send("amixer -c1 sget OutputMode")


def main():
    # Set login credentials and the server prompt
    HOSTNAME = '192.168.1.177'
    USERNAME = 'root'
    PASSWORD = 't0talc0ntr0l4!'
    PROMPT = '~#'

    # Use SSH client to login
    try:
        # Create a new SSH client object
        client = paramiko.SSHClient()

        # Set SSH key parameters to auto accept unknown hosts
        client.load_system_host_keys()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        # Connect to the host
        client.connect(hostname=HOSTNAME, username=USERNAME, password=PASSWORD)


        # Create a client interaction class which will interact with the host
        with SSHClientInteraction(client, timeout=10, display=True) as interact:
            interact.expect(PROMPT)

            # Run the first command and capture the cleaned output, if you want
            # the output without cleaning, simply grab current_output instead.
            interact.send('uname -a')
            interact.expect(PROMPT)
            cmd_output_uname = interact.current_output_clean

            interact.send("amixer -c1 sget OutputMode")
            interact.expect("Item0:")
            interact.send("amixer -c1 sset OutputMode EQ")
            interact.expect("EQ:")

            # Send the exit command and expect EOF (a closed session)
            interact.send('exit')
            interact.expect()

        # Print the output of each command
        print('-' * 79)
        print('Cleaned Command Output')
        print('-' * 79)
        print('uname -a output:')
        print(cmd_output_uname)

    except Exception:
        traceback.print_exc()
    finally:
        try:
            client.close()
        except Exception:
            pass


if __name__ == '__main__':
    main()