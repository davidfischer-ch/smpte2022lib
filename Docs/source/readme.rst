.. include:: common.rst

README
******

ForPython
=========

I developped this version to generate SMPTE 2022-1 FEC streams from sniffed RTP media stream.

.. note::

    * :file:`FecGenerator.py` is the SMPTE 2022-1 FEC streams generator from incoming RTP stream.
    * :file:`SocketFecGenerator.py` is a network-capable utility using the *FEC generator* class.
    * :file:`TwistedFecGenerator.py` is another network-capable utility using the *FEC generator* class.
    * :file:`FecReceiver.py` is the RTP / SMPTE 2022-1 FEC receiver.

ForJava
=======

This is the Java re-implementation of the old C/C++ SMPTE 2022-1 FEC library.

I developped this version to receive protected RTP multicast streams from an Android device.

Of course, you can also use it for another purpose (e.g. a Java powered PC application).

.. note::

    * :file:`FecReceiver.java` is the RTP / SMPTE 2022-1 FEC receiver.

ForC_old
========

This is the first (abandonware) implementation of the library.

I do not use it anymore and I really think that this implementation needs some heavy refactoring.

You can read these files if you understand french :

* Project's goals : |goal|_.
* Project's doxygen : |doxy|_.

Don't hesitate to contact me to get further explanations.

GNU/Linux (e.g. Ubuntu 64)
--------------------------

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
----------------------------

Compiling step by step
^^^^^^^^^^^^^^^^^^^^^^

Steps are equivalent to GNU/Linux steps with some variations (CodeBlocks_ running under a MinGW_ environment).

Testing step by step
^^^^^^^^^^^^^^^^^^^^

Steps are equivalent to GNU/Linux steps with some variations (scripts names and files extensions).

ForVlc_old
==========

This is the work done by Jérémie Mathieu Rossier to integrate the old C/C++ SMPTE 2022-1 FEC library into VLC_.

Contact
=======

David Fischer

david.fischer.ch at gmail.com
