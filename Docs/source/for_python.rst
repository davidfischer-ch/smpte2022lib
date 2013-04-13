.. include:: common.rst

Implementation in Python
************************

Profiling
=========

Xor Contest
-----------

.. code-block:: bash

    ~/smpte2022lib/ForPython/XorContest$ python xor-contest.py
    Xor benchmark
    function xor_inplace_loop takes 0.201619
    function xor_list_comprehension takes 0.131905
    function numpy.bitwise_xor takes 0.001775
    function fastxor.fast_xor_inplace takes 0.001401

xor-contest.py
^^^^^^^^^^^^^^

.. literalinclude:: ../../ForPython/XorContest/xor-contest.py
    :language: python
    :linenos:
    :lines: 33-


SMPTE 2022-1 Generator
------------------------

* Source movie :file:`project_london_720p_h264_22_480_800k_64k_22050.mp4` streamed by VLC.

.. code-block:: bash

    ~smpte2022lib/ForPython$ sh pycallgraph.sh

Results
^^^^^^^

With ``FecPacket.compute()`` using (...) to generate FEC packets :

1. A simple python Xor loop::

    ...
    for i in xrange(min(size, len(packet.payload))):
        fec.payload_recovery[i] ^= packet.payload[i]

2. Using module ``numpy``::

    ...
    payload = packet.payload
    if len(packet.payload) < size:
        payload = payload + bytearray(size - len(packet.payload))
    fec.payload_recovery = bytearray(numpy.bitwise_xor(fec.payload_recovery, payload))

3. Using module ``fastxor``::

    payload = packet.payload
    if len(packet.payload) < size:
        payload = payload + bytearray(size - len(packet.payload))
    fast_xor_inplace(fec.payload_recovery, payload)

Graphical output of pycallgraph:

1. :download:`images/graph-xor-loop.svg`
2. :download:`images/graph-numpy.svg`
3. :download:`images/graph-fastxor.svg`

Computed ``FecPacket.compute`` speed-up depending of Xor-loop implementation:

== ===================== ========== ===== =========== ========
No Xor implementation    Total time Calls Time / call Speed-up
== ===================== ========== ===== =========== ========
1. Python xor loop       4.826102   409    11.80 ms     1.0x
2. Python module (numpy) 7.774805   1761   4.415 ms     2.7x
3. C xor loop            0.933720   632    1.477 ms     8.0x
== ===================== ========== ===== =========== ========

The method can run even faster by optimizing (other) methods used by ``FecPacket.compute`` !

About fastxor module
--------------------

I packaged a modified version of `eryksun <http://stackoverflow.com/users/205580/eryksun>`_'s fast-xor implementation (link : |fastxor_so|_) in the form of a Python_ module. To install it, you only need to run :file:`ForPython/XorContest/fastxor/install.sh` and this module is then available to use from your own code. I asked original author for his approval in using this code GPLv3 licensed for the purposes of this library. As you can see, I already added modified :file:`fastxor.cpp` on project's repository ... I will remove it if necessary.

Code Documentation
==================

.. automodule:: IPSocket
    :members:
    :inherited-members:
    :private-members:
    :undoc-members:

----

.. automodule:: RtpPacket
    :members:
    :inherited-members:
    :private-members:
    :undoc-members:

----

.. automodule:: FecPacket
    :members:
    :inherited-members:
    :private-members:
    :undoc-members:

----

.. automodule:: FecReceiver
    :members:
    :inherited-members:
    :private-members:
    :undoc-members:

----

.. automodule:: FecGenerator
    :members:
    :inherited-members:
    :private-members:
    :undoc-members:

----

.. automodule:: SocketFecGenerator
    :members:
    :inherited-members:
    :private-members:
    :undoc-members:

----

.. automodule:: TwistedFecGenerator
    :members:
    :inherited-members:
    :private-members:
    :undoc-members:

.. seealso::

    * Documenting your project with Sphinx `(link) <http://pythonhosted.org/an_example_pypi_project/sphinx.html#full-code-example>`_
    * Include documentation from docstrings `(link) <http://sphinx-doc.org/ext/autodoc.html>`_
    * Epydoc fields `(link) <http://epydoc.sourceforge.net/fields.html>`_
