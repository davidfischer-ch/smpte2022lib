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

import logging
import socket
import struct
from FecGenerator import FecGenerator
from RtpPacket import RtpPacket

log = logging.getLogger('smpte2022lib')


# FIXME send to multicast address fail (?)
class SocketFecGenerator(object):
    u"""
    A SMPTE 2022-1 FEC streams generator with network skills based on :mod:`socket`.
    This generator listen to incoming RTP media stream, compute and output corresponding FEC streams.

    **Example usage**

    >>> from IPSocket import IPSocket
    >>> media = IPSocket(SocketFecGenerator.DEFAULT_MEDIA)
    >>> col = IPSocket(SocketFecGenerator.DEFAULT_COL)
    >>> row = IPSocket(SocketFecGenerator.DEFAULT_ROW)
    >>> generator = SocketFecGenerator(media, col, row, 5, 6)
    >>> print generator._generator
    Matrix size L x D            = 5 x 6
    Total invalid media packets  = 0
    Total media packets received = 0
    Column sequence number       = 1
    Row    sequence number       = 1
    Media  sequence number       = None
    Medias buffer (seq. numbers) = []
    """

    # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Properties >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    DEFAULT_MEDIA = '239.232.0.222:5004'
    DEFAULT_COL = '232.232.0.222:5006'
    DEFAULT_ROW = '232.232.0.222:5008'

    # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constructor >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    def __init__(self, media_socket, col_socket, row_socket, L, D):
        u"""
        Construct a SocketFecGenerator.

        :param media_socket: Socket of incoming RTP media stream
        :type media_socket: IPSocket
        :param col_socket: Socket of output FEC stream (column)
        :type col_socket: IPSocket
        :param row_socket: Socket of output FEC stream (row)
        :type row_socket: IPSocket
        :param L: Horizontal size of the FEC matrix (columns)
        :type L: int
        :param D: Vertical size of the FEC matrix (rows)
        :type D: int
        """
        self.media_socket = media_socket
        self.col_socket = col_socket
        self.row_socket = row_socket
        self._generator = FecGenerator(L, D)
        self._generator.onNewCol = self.onNewCol
        self._generator.onNewRow = self.onNewRow
        self._generator.onReset = self.onReset

    # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    def run(self):
        log.info('SMPTE 2022-1 FEC Generator by David Fischer')
        log.info('started Listening %s' % self.media_socket)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)
        sock.bind((self.media_socket['ip'], self.media_socket['port']))
        # Tell the operating system to add the socket to the multicast group on all interfaces
        group = socket.inet_aton(self.media_socket['ip'])
        mreq = struct.pack('4sL', group, socket.INADDR_ANY)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        # Receive loop
        while True:
            datagram, address = sock.recvfrom(1024)
            media = RtpPacket(bytearray(datagram), len(datagram))
            log.debug('Incoming media packet seq=%s ts=%s psize=%s ssrc=%s address=%s' %
                      (media.sequence, media.timestamp, media.payload_size, media.ssrc, address))
            self._generator.putMedia(media)

    def onNewCol(self, col, generator):
        u"""
        Called by self=FecGenerator when a new column FEC packet is generated and available for output.

        Send the encapsulated column FEC packet.

        :param col: Generated column FEC packet
        :type col: FecPacket
        :param generator: The generator that fired this method / event
        :type generator: FecGenerator
        """
        col_rtp = RtpPacket.create(col.sequence, 0, RtpPacket.DYNAMIC_PT, col.bytes)
        log.debug('Send COL FEC packet seq=%s snbase=%s LxD=%sx%s trec=%s socket=%s' %
                 (col.sequence, col.snbase, col.L, col.D, col.timestamp_recovery, self.col_socket))
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
        sock.sendto(col_rtp.bytes, (self.col_socket['ip'], self.col_socket['port']))

    def onNewRow(self, row, generator):
        u"""
        Called by self=FecGenerator when a new row FEC packet is generated and available for output.

        Send the encapsulated row FEC packet.

        :param row: Generated row FEC packet
        :type row: FecPacket
        :param generator: The generator that fired this method / event
        :type generator: FecGenerator
        """
        row_rtp = RtpPacket.create(row.sequence, 0, RtpPacket.DYNAMIC_PT, row.bytes)
        log.debug('Send ROW FEC packet seq=%s snbase=%s LxD=%sx%s trec=%s socket=%s' %
                  (row.sequence, row.snbase, row.L, row.D, row.timestamp_recovery, self.row_socket))
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
        sock.sendto(row_rtp.bytes, (self.row_socket['ip'], self.row_socket['port']))

    def onReset(self, media, generator):
        u"""
        Called by self=FecGenerator when the algorithm is resetted (an incoming media is out of sequence).

        Log a warning message.

        :param media: Out of sequence media packet
        :type row: RtpPacket
        :param generator: The generator that fired this method / event
        :type generator: FecGenerator
        """
        log.warning('Media seq=%s is out of sequence (expected %s) : FEC algorithm resetted !' %
                    (media.sequence, generator._media_sequence))

    @staticmethod
    def main():
        u"""
        This is a working example utility using this class, this method will :

        * Parse arguments from command line
        * Register handlers to SIGTERM and SIGINT
        * Instantiate a :mod:`SocketFecGenerator` and start it
        """
        import signal
        from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
        from IPSocket import IPSocket

        HELP_MEDIA = 'Socket of input stream'
        HELP_COL = 'Socket of generated FEC column stream'
        HELP_ROW = 'Socket of generated FEC row stream'

        dmedia = SocketFecGenerator.DEFAULT_MEDIA
        dcol = SocketFecGenerator.DEFAULT_COL
        drow = SocketFecGenerator.DEFAULT_ROW

        parser = ArgumentParser(
            formatter_class=ArgumentDefaultsHelpFormatter,
            epilog='''This utility create SMPTE 2022-1 FEC streams from a sniffed source stream.
                      SMPTE 2022-1 help streaming systems to improve QoE of real-time RTP transmissions.''')
        parser.add_argument('-m', '--media', type=IPSocket, help=HELP_MEDIA, default=dmedia)
        parser.add_argument('-c', '--col', type=IPSocket, help=HELP_COL, default=dcol)
        parser.add_argument('-r', '--row', type=IPSocket, help=HELP_ROW, default=drow)
        args = parser.parse_args()

        def handle_stop_signal(SIGNAL, stack):
            log.info('\nGenerator stopped\n')

        signal.signal(signal.SIGTERM, handle_stop_signal)
        signal.signal(signal.SIGINT, handle_stop_signal)
        generator = SocketFecGenerator(args.media, args.col, args.row, 5, 6)
        generator.run()

if __name__ == '__main__':
    import doctest
    from Utils import setup_logging
    setup_logging(name='smpte2022lib', filename=None, console=True, level=logging.DEBUG)
    log.info('Testing SocketFecGenerator with doctest')
    doctest.testmod(verbose=False)
    log.info('OK')
    SocketFecGenerator.main()
