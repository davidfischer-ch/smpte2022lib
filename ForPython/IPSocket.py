#! /usr/bin/env python
# -*- coding: utf-8 -*-

#**************************************************************************************************#
#       OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH
#
#   Description : SMPTE 2022-1 FEC Library
#   Authors     : David Fischer
#   Contact     : david.fischer.ch@gmail.com / david.fischer@hesge.ch
#   Copyright   : 2008-2013 smpte2022lib Team. All rights reserved.
#   Sponsoring  : Developed for a HES-SO CTI Ra&D project called GaVi
#                 Haute école du paysage, d'ingénierie et d'architecture @ Genève
#                 Telecommunications Laboratory
#**************************************************************************************************#
#
#  This file is part of smpte2022lib.
#
#  This project is free software: you can redistribute it and/or modify it under the terms of the
#  GNU General Public License as published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
#  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along with this project.
#  If not, see <http://www.gnu.org/licenses/>
#
#  Retrieved from:
#    git clone git://github.com/davidfischer-ch/smpte2022lib.git
#

from ipaddr import IPAddress


def IPSocket(string):
    u"""
    This helper create a dictionary containing address and port from a parsed IP address string.
    Throws ValueError in case of failure (e.g. string is not a valid IP address).

    **Example usage**

    >>> IPSocket('gaga:gogo')
    Traceback (most recent call last):
        ...
    ValueError: 'gaga:gogo' is not a valid IP socket.

    >>> IPSocket('239.232.0.222:5004')
    {'ip': '239.232.0.222', 'port': 5004}

    .. warning::

        TODO IPv6 ready : >>> IPSocket('[2001:0db8:0000:0000:0000:ff00:0042]:8329')
    """
    try:
        (ip, port) = string.rsplit(':', 1)
        #ip = ip.translate(None, '[]')
        IPAddress(ip)  # Seem not IPv6 ready
        port = int(port)
    except Exception:
        raise ValueError("%r is not a valid IP socket." % string)
    return {'ip': ip, 'port': port}

if __name__ == '__main__':
    print('Testing IPSocket with doctest')
    import doctest
    doctest.testmod(verbose=False)
    print ('OK')
