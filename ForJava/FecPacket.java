/**********************************************************************************************************************\
                    OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description   : FEC packet
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
import java.util.Collections;
import java.util.List;

//FIXME error message is set in the constructor but it is not updated if packet become valid !

//**********************************************************************************************************************
//TODO
public class FecPacket
{
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constants >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static String ER_PAYLOAD_TYPE = "RTP Header : Payload type must be set to 96";
	public static String ER_EXTENDED     = "SMPTE 2022-1 Header : Extended must be set to one";
	public static String ER_MASK         = "SMPTE 2022 Header : Mask must be set to zero";
	public static String ER_N            = "SMPTE 2022-1 Header : N must be set to zero";
	public static String ER_ALGORITHM    = "SMPTE 2022 Header : Algorithm must be set to zero";
	public static String ER_INDEX        = "SMPTE 2022 Header : Index must be set to zero";
	public static String ER_LD = "SMPTE 2022 Header : The following limitation failed : L*D ≤ 256";
	public static String ER_L  = "SMPTE 2022 Header : The following limitation failed : 1 ≤ L ≤ 50";
	public static String ER_D  = "SMPTE 2022 Header : The following limitation failed : 4 ≤ D ≤ 50";
	public static String ER_PAYLOAD = "FEC packet must have a payload";

	public static String EX_ALGORITHM = "SMPTE 2022 Header : Only XOR FEC algorithm is handled";
	public static String EX_VALID_MP2T =
		"One of the packets of pPackets is an invalid RTP packet (+expected MPEG2-TS payload)";
	public static String EX_OFFSET   = "(pPackets) Computed offset is out of range [1..255]";
	public static String EX_SEQUENCE =
		"One of the packets of pPackets doesn't verify : sequence = snbase+i*offset, 0<i<na";

	public static final int HEADER_LENGTH = 16;
	public static final byte E_MASK  = (byte)0x80;
	public static final byte PT_MASK = (byte)0x7F;
	public static final byte N_MASK  = (byte)0x80;
	public static final byte D_MASK  = (byte)0x40;
	public static final byte T_MASK  = (byte)0x38;
	public static final byte I_MASK  = (byte)0x07;
	public static final byte DYNAMIC_PT = 96; // Dynamic payload type

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Data Types >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public enum Direction { COL /*0*/, ROW /*1*/ };
	public enum Algorithm { XOR /*0*/, Hamming /*1*/, Reed_Solomon /*2*/ };

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Fields >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// Status
	protected List<String> ctorErrors;

	// Forward error correction
	public int       sequence;
	public Algorithm algorithm;           //  3 bits
	public Direction direction;           //  1 bit
	public int       snbase;              // 24 bits
	public byte      offset;              //  8 bits
	public byte      na;                  //  8 bits
	public byte      payloadTypeRecovery; //  7 bits
	public long      timestampRecovery;   // 32 bits
	public int       lengthRecovery;      // 16 bits
	public byte[]    payloadRecovery;

	// Unused has defined in SMPTE 2022-1
	public byte    index;    //  3 bits
	public int     mask;     // 24 bits
	public boolean extended; //  1 bit
	public boolean n;        //  1 bit

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constructors >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	protected FecPacket() {}

	// Parse a byte sequence to a FEC packet
	public FecPacket(byte[] pBytes, int pLength)
	{
		this (new RtpPacket(pBytes, pLength));
	}

	// Convert a RTP packet to a FEC packet
	public FecPacket(RtpPacket pPacket)
	{
		// FEC header format (RFC 2733)
		/*
			 0                   1                   2                   3
			 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|       SNBase low bits         |        Length recovery        |
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|E| PT recovery |                    Mask                       |
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|                          TS recovery                          |
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			|N|D|type |index|    Offset     |      NA       |SNBase ext bits|
			+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 */
		sequence   = pPacket.sequence;
		ctorErrors = pPacket.getErrors();
		if (!ctorErrors.isEmpty()) return;
		if (pPacket.payloadType != DYNAMIC_PT)
		{
			ctorErrors.add(ER_PAYLOAD_TYPE);
			return;
		}

		snbase         = Utils.get16bits(pPacket.payload, 0);
		lengthRecovery = Utils.get16bits(pPacket.payload, 2);

		extended = (pPacket.payload[4] & E_MASK) != 0;
		//if (!extended) return;

		payloadTypeRecovery = (byte)(pPacket.payload[4] & PT_MASK);

		mask = Utils.get24bits(pPacket.payload, 5);
		//if (mask != 0) return;

		timestampRecovery = Utils.get32bits(pPacket.payload, 8);

		n = (pPacket.payload[12] & N_MASK) != 0;
		//if (n) return;

		direction = ((pPacket.payload[12] & D_MASK) != 0) ? Direction.ROW : Direction.COL;
		switch (pPacket.payload[12] & T_MASK)
		{
		case 0: algorithm = Algorithm.XOR;          break;
		case 1: algorithm = Algorithm.Hamming;      break;
		case 2: algorithm = Algorithm.Reed_Solomon; break;
		}
		//if (algorithm != Algorithm.XOR) return;

		index = (byte)(pPacket.payload[12] & I_MASK);
		//if (index != 0) return;

		offset  = pPacket.payload[13];
		na      = pPacket.payload[14];
		snbase += pPacket.payload[15] << 16;

		payloadRecovery = // And finally ... The payload !
			Arrays.copyOfRange(pPacket.payload, HEADER_LENGTH, pPacket.payload.length);
	}

	// Create a new FEC packet based on RTP packets payload (xor'ed)
	public FecPacket
		(Algorithm pAlgorithm, Direction pDirection, byte pL, byte pD, List<RtpPacket> pPackets)
	{
		if (pPackets == null) throw new IllegalArgumentException("pPackets is null");

		extended  = true;
		algorithm = pAlgorithm;
		direction = pDirection;
		switch (direction)
		{
		case COL: na = pD; offset = pL; break;
		case ROW: na = pL; offset =  1; break;
		default:
		}

		if (algorithm != Algorithm.XOR) throw new UnsupportedOperationException(EX_ALGORITHM);

		if (pPackets.size() != na)
			throw new IllegalArgumentException("pPackets must contain exactly "+na+" packets");

		Collections.sort(pPackets);
		snbase = pPackets.get(0).sequence;

		// Detect maximum length of packets payload and check packets validity
		int size = 0;
		for (int i = 0; i < pPackets.size(); i++)
		{
			RtpPacket packet = pPackets.get(i);
			if (!packet.isValidMP2T())            throw new IllegalArgumentException(EX_VALID_MP2T);
			if (packet.sequence!=snbase+i*offset) throw new IllegalArgumentException(EX_SEQUENCE);
			size = Math.max(size, packet.getPayloadSize());
		}
        // Create payload recovery field according to size/length
		payloadRecovery = new byte[size];
        // Compute fec packet's fields based on input packets
		for (int i = 0; i < pPackets.size(); i++)
		{
			// Update (...) recovery fields by xor'ing corresponding fields of all packets
			RtpPacket packet = pPackets.get(i);
			payloadTypeRecovery ^= packet.payloadType;
			timestampRecovery   ^= packet.timestamp;
			lengthRecovery      ^= packet.getPayloadSize();

			// Update payload recovery by xor'ing all packets payload
			for (int no = 0; no < Math.min(size, packet.payload.length); no++)
			{
				payloadRecovery[no] ^= packet.payload[no];
			}
		}
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Properties >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public boolean isValid()
	{
		return getErrors().isEmpty();
	}

	public List<String> getErrors()
	{
		List<String> errors = new ArrayList<String>();
		if (ctorErrors != null)         errors.addAll(ctorErrors);
		if (!extended)                  errors.add(ER_EXTENDED);
		if (mask != 0)                  errors.add(ER_MASK);
		if (n)                          errors.add(ER_N);
		if (algorithm != Algorithm.XOR) errors.add(ER_ALGORITHM);
		if (index     != 0)             errors.add(ER_INDEX);
		if (getPayloadSize() == 0)      errors.add(ER_PAYLOAD);
		if (getL() < 1 || getL() > 50)  errors.add(ER_L);
		if (direction == Direction.COL &&  getL() * getD() > 256)      errors.add(ER_LD);
		if (direction == Direction.COL && (getD() < 4 || getD() > 50)) errors.add(ER_D);
		return errors;
	}

	public int getD() { return direction == Direction.COL ? na     : 0;  }
	public int getL() { return direction == Direction.COL ? offset : na; }

	public int getPayloadSize()
	{
		return payloadRecovery == null ? 0 : payloadRecovery.length;
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public String toString()
	{
		return
			"errors                = "+getErrors()+"\n"+
			"sequence              = "+sequence+"\n"+
			"algorithm             = "+algorithm+"\n"+
			"direction             = "+direction+"\n"+
			"snbase                = "+snbase+"\n"+
			"offset                = "+offset+"\n"+
			"na                    = "+na+"\n"+
			"L x D                 = "+getL()+" x "+getD()+"\n"+
			"payload type recovery = "+payloadTypeRecovery+"\n"+
			"timestamp recovery    = "+timestampRecovery+"\n"+
			"length recovery       = "+lengthRecovery+"\n"+
			"payload recovery size = "+getPayloadSize()+"\n";
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Static >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static int UnitTest() // FIXME put this in a real unit test
	{
		FecPacket fec;
		byte[] header, bytes;

		if (!RtpPacket.UnitTest()) return 1; // RtPacket unit test must be successful

		// Testing header fields value (based on packet 3 of capture DCM_FEC_2D_6_10.pcap) -----------------------------
		header = new byte[]
		{
			// RTP header
			(byte)0x80, (byte)0x60, (byte)0x93, (byte)0xA6, // sequence = 37 798
			(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
			(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
			// FEC header
			(byte)0xC4, (byte)0x70, (byte)0x00, (byte)0x00, // SN = 50 288
			(byte)0x80, (byte)0x00, (byte)0x00, (byte)0x00, // PT recovery = 0
			(byte)0x00, (byte)0x00, (byte)0x1E, (byte)0xAA, // TS recovery = 7850
			(byte)0x00, (byte)0x06, (byte)0x0A, (byte)0x00
		};

		bytes = new byte[1344];
		System.arraycopy(header, 0, bytes, 0, header.length);

		fec = new FecPacket(bytes, bytes.length);
		if (!fec.isValid()) return 2; // Is valid
		if (fec.sequence            != 37798         || fec.algorithm         != Algorithm.XOR ||
			fec.direction           != Direction.COL || fec.snbase            != 50288         ||
			fec.offset              != 6             || fec.na                !=    10         ||
			fec.getL()              != fec.offset    || fec.getD()            != fec.na        ||
			fec.payloadTypeRecovery != 0             || fec.timestampRecovery !=  7850         ||
			fec.lengthRecovery      != 0 || fec.payloadRecovery.length != 7*188) return 3;

		// Testing header fields value (based on packet 5 of capture DCM_FEC_2D_6_10.pcap) -----------------------------
		header = new byte[]
		{
			// RTP header
			(byte)0x80, (byte)0x60, (byte)0xF6, (byte)0x1C, // sequence = 63 004
			(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
			(byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
			// FEC header
			(byte)0xC4, (byte)0xA8, (byte)0x00, (byte)0x00, // SN = 50 344
			(byte)0x80, (byte)0x00, (byte)0x00, (byte)0x00, // PT recovery = 0
			(byte)0x00, (byte)0x00, (byte)0x03, (byte)0x6E, // TS recovery = 878
			(byte)0x40, (byte)0x01, (byte)0x06, (byte)0x00
		};

		bytes = new byte[1344];
		System.arraycopy(header, 0, bytes, 0, header.length);

		fec = new FecPacket(bytes, bytes.length);
		if (!fec.isValid()) return 4; // Is valid
		if (fec.sequence            != 63004         || fec.algorithm         != Algorithm.XOR ||
			fec.direction           != Direction.ROW || fec.snbase            != 50344         ||
			fec.offset              != 1             || fec.na                !=     6         ||
			fec.getL()              != fec.na        || // D cannot be known in a ROW FEC packet
			fec.payloadTypeRecovery != 0             || fec.timestampRecovery !=   878         ||
			fec.lengthRecovery      != 0 || fec.payloadRecovery.length != 7*188) return 5;

		// Testing fec packet constructor (based on source RTP packets) ------------------------------------------------

		byte L = 4, D = 5, OFF = 2;
		RtpPacket[][] matrix = Utils.createRandomRtpPacketsMatrix(L, D, 50, 100);

		// Retrieve the OFF'th column of the matrix
		byte   expectedPayloadTypeRecovery = 0;
		long   expectedTimestampRecovery   = 0;
		int    expectedLenghtRecovery      = 0;
		byte[] expectedPayloadRecovery     = new byte[100];
		List<RtpPacket> packets = new ArrayList<RtpPacket>();
		for (int i = 0; i < D; i++)
		{
			RtpPacket packet = matrix[i][OFF];
			packets.add(packet);
			// Compute expected recovery fields values
			expectedPayloadTypeRecovery ^= packet.payloadType;
			expectedTimestampRecovery   ^= packet.timestamp;
			expectedLenghtRecovery      ^= packet.getPayloadSize();
			for (int j = 0; j < packet.getPayloadSize(); j++)
			{
				expectedPayloadRecovery[j] ^= packet.payload[j];
			}
		}
		fec = new FecPacket(Algorithm.XOR, Direction.COL, L, D, packets);
		if (!fec.isValid()) return 6;
		if (fec.snbase != matrix[0][OFF].sequence || fec.na != D || fec.offset != L) return 7;
		if (fec.payloadTypeRecovery != expectedPayloadTypeRecovery) return 8;
		if (fec.timestampRecovery   != expectedTimestampRecovery)   return 9;
		if (fec.lengthRecovery      != expectedLenghtRecovery)      return 10;
		for (int i = 0; i < fec.getPayloadSize(); i++)
		{
			if (fec.payloadRecovery[i] != expectedPayloadRecovery[i]) return 11;
		}

		return 0;
	}
}
