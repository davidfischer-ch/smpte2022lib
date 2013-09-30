.. |goal| replace:: Resume.pdf
.. _goal: ForC_old/Resume.pdf

.. |doxy| replace:: DoxyCode/html/index.html
.. _doxy: ForC_old/DoxyCode/html/index.html

.. _fastxor_so: http://stackoverflow.com/questions/15459684/transmission-bytearray-from-python-to-c-and-return-it
.. |fastxor_so| replace:: Transmission bytearray from Python to C and return it

.. _CodeBlocks: http://www.codeblocks.org/
.. _MinGW: http://www.mingw.org/
.. _Python: http://www.python.org/
.. _VLC: http://www.videolan.org/vlc/index.html

smpte2022lib
============

Optimized and cross platform SMPTE 2022-1 FEC library in C, Python, Java.

Brief description
-----------------

SMPTE 2022-1 is a forward error correction standard for real-time video/audio (RTP) transport over IP networks.

Initial goal of the project was to help VLC and others by implementing a C/C++ RTP / SMPTE 2022-1 FEC streams receiver.

Nowadays I focus on using the newer Java and Python implementations for some Ra&D projects using RTP streams.

ForPython
---------

I developed this version to generate SMPTE 2022-1 FEC streams from sniffed RTP media stream.
This implementation is now part of `pyutils <https://github.com/davidfischer-ch/pyutils/>`_

ForJava
-------

This is the Java re-implementation of the old C/C++ SMPTE 2022-1 FEC library.

I developed this version to receive protected RTP multicast streams from an Android device.

Of course, you can also use it for another purpose (e.g. a Java powered PC application).

.. note::

    * :file:`FecReceiver.java` is the RTP / SMPTE 2022-1 FEC receiver.

ForC_old
--------

This is the first (abandon-ware) implementation of the library.

I do not use it anymore and I really think that this implementation needs some heavy refactoring.

You can read these files if you understand French :

* Project's goals : |goal|_.
* Project's doxygen : |doxy|_.

Don't hesitate to contact me to get further explanations.

GNU/Linux (e.g. Ubuntu 64)
++++++++++++++++++++++++++

Compiling step by step
^^^^^^^^^^^^^^^^^^^^^^

* install CodeBlocks_ and g++ with ``sudo apt-get install codeblocks g++`` ;
* open file :file:`ForC_old/CodeBlocks/VLC-SMPTE.workspace` with CodeBlocks_ ;
* double click on project **Smpte-2022-** in CodeBlocks_ IDE and Ctrl+F11 to compile ;
* double click on project **FecGenerator**    in CodeBlocks_ IDE and Ctrl+F11 to compile ;
* double click on project **ErrorsGenerator** in CodeBlocks_ IDE and Ctrl+F11 to compile ;
* double click on project **FecDecoder**      in CodeBlocks_ IDE and Ctrl+F11 to compile ;

Testing step by step
^^^^^^^^^^^^^^^^^^^^

* open a terminal in path :file:`ForC_old/Release-linux64` ;
* execute ``sh script_example.sh "source_file_name"`` ;
* read logs of each module ;
* compare ``*.raw`` output file (without any error recovery) and :file:`*.david` output file (recovered by SMPTE 2022-1 library) ;
* execute each module with different options :
   - like this : ``source_file -> FecGenerator -> FecDecoder -> results_files``
   - or like this : ``source_file -> FecGenerator -> ErrorsGenerator -> FecDecoder -> results_files``

Windows (e.g. Windows XP 32)
++++++++++++++++++++++++++++

Compiling step by step
^^^^^^^^^^^^^^^^^^^^^^

Steps are equivalent to GNU/Linux steps with some variations (CodeBlocks_ running under a MinGW_ environment).

Testing step by step
^^^^^^^^^^^^^^^^^^^^

Steps are equivalent to GNU/Linux steps with some variations (scripts names and files extensions).

ForVlc_old
----------

This is the work done by Jérémie Mathieu Rossier to integrate the old C/C++ SMPTE 2022-1 FEC library into VLC_.

2013 - David Fischer
