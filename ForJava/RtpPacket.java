/**********************************************************************************************************************\
                    OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description   : RTP packet
   Main Developer : David Fischer (david.fischer.ch@gmail.com)
   Copyright      : Copyright (c) 2008-2013 smpte2022lib Team. All rights reserved.
   Sponsoring     : Developed for a HES-SO CTI Ra&D project called GaVi
                    Haute école du paysage, d'ingénierie et d'architecture @ Genève
                    Telecommunications Laboratory

\**********************************************************************************************************************/
/*
  This file is part of smpte2022lib.

  This project is free software: you can redistribute it and/or modify it under the terms of the EUPL v. 1.1 as provided
  by the European Commission. This project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  See the European Union Public License for more details.

  You should have received a copy of the EUPL General Public License along with this project.
  If not, see he EUPL licence v1.1 is available in 22 languages:
      22-07-2013, <https://joinup.ec.europa.eu/software/page/eupl/licence-eupl>

  Retrieved from https://github.com/davidfischer-ch/smpte2022lib.git
*/

package smpte2022lib;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

// *********************************************************************************************************************
// TODO
public class RtpPacket implements Comparable<RtpPacket>
{
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constants >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static String ER_VERSION          = "RTP Header : Version must be set to 2";
	public static String ER_PADDING_LENGTH   = "RTP Header : Bad padding length";
	public static String ER_EXTENSION_LENGTH = "RTP Header : Bad extension length";
	public static String ER_PAYLOAD          = "RTP packet must have a payload";

	public static final int HEADER_LENGTH = 12;
	public static final byte V_MASK   = (byte)0xc0;
	public static final byte P_MASK   = (byte)0x20;
	public static final byte X_MASK   = (byte)0x10;
	public static final byte CC_MASK  = (byte)0x0f;
	public static final byte M_MASK   = (byte)0x80;
	public static final byte PT_MASK  = (byte)0x7f;
	public static final byte MP2T_PT  =    33; // MPEG2 TS payload type
	public static final int  MP2T_CLK = 90000; // MPEG2 TS clock rate [Hz]
	public static final int  S_MASK   = (int)0x0000FFFF;

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Fields >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public byte    version     = 0;
	public boolean padding     = false;
	public boolean extension   = false;
	public boolean marker      = false;
	public byte    payloadType = 0;
	public int     sequence    = 0;
	public long    timestamp   = 0;
	public long    ssrc        = 0;
	public long[]  csrc        = null;
	public byte[]  payload     = null;
	protected String ctorError = null;

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Properties >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public boolean isValid()
	{
		return getErrors().isEmpty();
	}

	public boolean isValidMP2T()
	{
		return isValid() && payloadType == MP2T_PT;
	}

	public List<String> getErrors()
	{
		List<String> errors = new ArrayList<String>();
		if (ctorError != null)     errors.add(ctorError);
		if (version   != 2)        errors.add(ER_VERSION);
		if (getPayloadSize() == 0) errors.add(ER_PAYLOAD);
		return errors;
	}

	public int getClockRate()
	{
		return payloadType == MP2T_PT ? MP2T_CLK : 1;
	}

	public int getCSRCCount()
	{
		return csrc == null ? 0 : csrc.length;
	}

	public int getPayloadSize()
	{
		return payload == null ? 0 : payload.length;
	}

	public double getTime()
	{
		return (double)timestamp / (double)getClockRate();
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constructors >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public RtpPacket() {}

	public RtpPacket(byte[] pBytes, int pLength)
	{
		// http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xml
		// RFC 3550 page 13
		/*
			 0                   1                   2                   3
			 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|V=2|P|X|  CC   |M|     PT      |       sequence number         |
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|                           timestamp                           |
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|           synchronization source (SSRC) identifier            |
			+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
			|            contributing source (CSRC) identifiers             |
			|                             ....                              |
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    */
		int offset = HEADER_LENGTH;
		int length = pLength;
		if (length < offset) return;

		version = (byte)((pBytes[0] & V_MASK & 0xff) >> 6);
		if (version != 2) return;

		padding = (pBytes[0] & P_MASK) == P_MASK;
		if (padding) // Remove padding if present
		{
			byte paddingLength = pBytes[length-1];
			if (paddingLength == 0 || length < (offset + paddingLength))
			{
				ctorError = ER_PADDING_LENGTH;
				return;
			}
			length -= paddingLength;
		}

		extension   = (pBytes[0] & X_MASK) == X_MASK;
		int cc      =  pBytes[0] & CC_MASK;
		csrc        = cc > 0 ? new long[cc] : null;
		marker      = (pBytes[1] & M_MASK) == M_MASK;
		payloadType = (byte)(pBytes[1] & PT_MASK);
		sequence    = Utils.get16bits(pBytes, 2);
		timestamp   = Utils.get32bits(pBytes, 4);
		ssrc        = Utils.get32bits(pBytes, 8);

		for (int i = 0; i < cc; i++)
		{
			csrc[i] = Utils.get32bits(pBytes, offset);
			offset += 4;
			// FIXME In session.c of VLC they store per-source statistics in a rtp_source_t struct
		}

		if (extension) // Extension header (ignored for now)
		{
			// RFC 3550 page 19
			/*
				 0                   1                   2                   3
				 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
				+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				|      defined by profile       |           length              |
				+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				|                        header extension                       |
				|                             ....                              |
			*/
			int extensionLength = Utils.get16bits(pBytes, offset + 2);
			offset += 4 + extensionLength;
			if (length < offset)
			{
				ctorError = ER_EXTENSION_LENGTH;
				return;
			}
		}

		// And finally ... The payload !
		payload = Arrays.copyOfRange(pBytes, offset, length);
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public int compareTo(RtpPacket pPacket)
	{
	    final int BEFORE = -1;
	    final int EQUAL   = 0;
	    final int AFTER   = 1;

		// This optimization is usually worthwhile, and can always be added
		if (this == pPacket) return EQUAL;
		if (this.sequence < pPacket.sequence) return BEFORE;
		if (this.sequence > pPacket.sequence) return AFTER;
		return EQUAL;
	}

	public static RtpPacket createMP2T(int pSequence, long pTimestamp, byte[] pPayload)
	{
		RtpPacket rtp = new RtpPacket();
		rtp.version     = 2;
		rtp.padding     = false;
		rtp.extension   = false;
		rtp.marker      = false;
		rtp.payloadType = MP2T_PT;
		rtp.sequence    = pSequence;
		rtp.timestamp   = pTimestamp;
		rtp.ssrc        = 0;
		rtp.csrc        = null;
		rtp.payload     = pPayload;
		assert rtp.isValidMP2T();
		return rtp;
	}

	public String toString()
	{
		return
			"version      = "+version+"\n"+
			"errors       = "+getErrors()+"\n"+
			"padding      = "+padding+"\n"+
			"extension    = "+extension+"\n"+
			"marker       = "+marker+"\n"+
			"payload type = "+payloadType+"\n"+
			"sequence     = "+sequence+"\n"+
			"timestamp    = "+timestamp+"\n"+
			"clock rate   = "+getClockRate()+"\n"+
			"time         = "+String.format("%.3f", getTime())+"\n"+
			"ssrc         = "+ssrc+"\n"+
			"csrc count   = "+getCSRCCount()+"\n"+
			"payload size = "+getPayloadSize()+"\n";
	}

	public static boolean UnitTest() // FIXME put this in a real unit test
	{
		RtpPacket rtp;
		byte[] bytes;

		// Testing invalid headers -------------------------------------------------------------------------------------
		rtp = new RtpPacket(new byte[1], HEADER_LENGTH-1);
		if (rtp.isValid()) return false; // Bad length

		rtp = new RtpPacket(new byte[HEADER_LENGTH], HEADER_LENGTH);
		if (rtp.isValid()) return false; // Bad version

		bytes = new byte[HEADER_LENGTH];
		bytes[0] = (byte)0xA0;
		rtp = new RtpPacket(bytes, HEADER_LENGTH);
		if (rtp.isValid()) return false; // Padding enabled but not present

		// Testing header fields value ---------------------------------------------------------------------------------
		bytes = new byte[]
		{
			(byte)0x80, (byte)0xA1, (byte)0xA4, (byte)0x25,
			(byte)0xCA, (byte)0xFE, (byte)0xB5, (byte)0x04, // = 3 405 690 116
			(byte)0xB0, (byte)0x60, (byte)0x5E, (byte)0xBB, // = 2 959 105 723
			(byte)0x12, (byte)0x34
		};
		rtp = new RtpPacket(bytes, bytes.length);
		if (!rtp.isValid()) return false; // Is valid
		if (rtp.version     !=          2  || rtp.padding || rtp.extension || !rtp.marker ||
			rtp.payloadType !=         33  || rtp.sequence       != 0xA425 ||
			rtp.timestamp   != 3405690116L || rtp.getClockRate() !=  90000 ||
			rtp.ssrc        != 2959105723L || rtp.payload.length !=      2 || rtp.csrc != null ||
			rtp.payload[0]  !=       0x12  || rtp.payload[1]     !=   0x34) return false;

		// Testing header fields value (with padding, extension and ccrc) ----------------------------------------------
		bytes = new byte[]
		{
			(byte)0xB5, (byte)0xA1, (byte)0xA4, (byte)0x01,
			(byte)0xCA, (byte)0xFE, (byte)0xA4, (byte)0x21, // = 3 405 685 793
			(byte)0xB0, (byte)0x60, (byte)0x5E, (byte)0xBB, // = 2 959 105 723
			(byte)0x11, (byte)0x11, (byte)0x11, (byte)0x11, // =   286 331 153
			(byte)0x22, (byte)0x22, (byte)0x22, (byte)0x22, // =   572 662 306
			(byte)0x33, (byte)0x33, (byte)0x33, (byte)0x33, // =   858 993 459
			(byte)0x44, (byte)0x44, (byte)0x44, (byte)0x44, // = 1 145 324 612
			(byte)0x55, (byte)0x55, (byte)0x55, (byte)0x55, // = 1 431 655 765
			(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x04, // Extension lenght = 4
			(byte)0x87, (byte)0x65, (byte)0x43, (byte)0x21, // Extension data
			(byte)0x12, (byte)0x34, (byte)0x00, (byte)0x02 // Data + padding 1x 0x00
		};
		rtp = new RtpPacket(bytes, bytes.length);
		if (!rtp.isValid()) return false; // Is valid
		if (rtp.version     !=          2  || !rtp.padding || !rtp.extension || !rtp.marker ||
			rtp.payloadType !=         33  || rtp.sequence       !=    0xA401   ||
			rtp.timestamp   != 3405685793L || rtp.getClockRate() !=     90000   ||
			rtp.ssrc        != 2959105723L || rtp.csrc.length    !=         5   ||
			rtp.csrc[0]     !=  286331153L || rtp.csrc[1]        !=  572662306L ||
			rtp.csrc[2]     !=  858993459L || rtp.csrc[3]        != 1145324612L ||
			rtp.csrc[4]     != 1431655765L || rtp.payload.length !=          2  ||
			rtp.payload[0]  !=       0x12  || rtp.payload[1]     !=       0x34) return false;

		return true;
	}
}
