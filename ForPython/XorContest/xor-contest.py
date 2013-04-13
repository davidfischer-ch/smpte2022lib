#! /usr/bin/env python
# -*- coding: utf-8 -*-

#**************************************************************************************************#
#       OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH
#
#   Description : SMPTE 2022-1 FEC Library
#   Authors     : David Fischer
#   Contact     : david.fischer.ch@gmail.com / david.fischer@hesge.ch
#   Copyright   : 2012 smpte2022lib Team. All rights reserved.
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

import numpy as numpy
from fastxor import fast_xor_inplace
#from xorcpp import xorcpp_inplace


def xor_inplace_loop(a, b):
    for i in xrange(len(b)):
        a[i] ^= b[i]


def xor_list_comprehension(a, b):
    return [a[i] ^ b[i] for i in xrange(len(b))]
    #c = [x ^ y for x, y in zip(a, b)]


def numpy_xor(a, b):
    u"""
    https://gist.github.com/zed/353005
    """
    return numpy.bitwise_xor(a, b)

if __name__ == '__main__':

    print('Xor benchmark')

    import os
    from time import time

    a1 = bytearray(os.urandom(1024*1024*2))
    a2 = a1[:]
    a3 = a1[:]
    a4 = a1[:]
    a5 = a1[:]
    b = bytearray(os.urandom(len(a1)))

    t0 = time()
    xor_inplace_loop(a1, b)
    t1 = time()
    a2 = xor_list_comprehension(a2, b)
    t2 = time()
    a3 = numpy_xor(a3, b)
    t3 = time()
    fast_xor_inplace(a4, b)
    t4 = time()

    for i in xrange(len(a1)):
        if a1[i] != a2[i] or a1[i] != a3[i] or a1[i] != a4[i] or a1[i] != a5[i]:
            print(i)

    print('function xor_inplace_loop takes %f' % (t1-t0))
    print('function xor_list_comprehension takes %f' % (t2-t1))
    print('function numpy.bitwise_xor takes %f' % (t3-t2))
    print('function fastxor.fast_xor_inplace takes %f' % (t4-t3))

