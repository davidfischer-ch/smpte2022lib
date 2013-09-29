/**********************************************************************************************************************\
                    OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description   : Some utilities
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

import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.InetSocketAddress;
import java.net.URI;
import java.security.*;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;
import java.util.zip.Adler32;
import java.util.zip.Checksum;

// *********************************************************************************************************************
// TODO
public class Utils
{
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constants >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static final int UDP_MAX_LEN  = 1024*1024;   // [bytes]
	public static final int UDP_TIMEOUT  = 500;         // [ms]
	public static final int ADLER32_SIZE = Long.SIZE/8; // [characters]
	public static final int MD5_SIZE     = 32;          // [characters]

	protected static final String HEXES     = "0123456789ABCDEF";
	protected static final char[] HEXES_TAB =
	{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// Returns the stack trace of an exception as a string ===
	public static String getException(Exception e)
	{
		// FIXME method's parameters validation (not null, ...)
		StringWriter w = new StringWriter();
		e.printStackTrace (new PrintWriter(w));
		return w.toString();
	}

	public static String getHex(long raw)
	{
		return String.format("%08x", raw);
	}

	public static String getHex(byte[] raw)
	{
		if (raw == null) return null;
		final StringBuilder hex = new StringBuilder(2 * raw.length);
		for (final byte b : raw)
		{
			hex.append(HEXES.charAt((b & 0xF0) >> 4)).append(HEXES.charAt((b & 0x0F)));
		}
		return hex.toString();
	}

	public static int get16bits(byte[] raw, int offset)
	{
		return (int)((raw[offset] & 0xff) << 8) + (int)(raw[offset+1] & 0xff);
	}

	public static int get24bits(byte[] raw, int offset)
	{
		return (int)((raw[offset]   & 0xff) << 24) + (int)((raw[offset+1] & 0xff) << 16) +
		       (int)((raw[offset+2] & 0xff) <<  8);
	}

	public static long get32bits(byte[] raw, int offset)
	{
		return (long)((raw[offset]   & 0xffL) << 24) + (long)((raw[offset+1] & 0xffL) << 16) +
		       (long)((raw[offset+2] & 0xffL) <<  8) + (long) (raw[offset+3] & 0xffL);
	}

	public static String adler32sum(byte[] input, int length)
	{
		Checksum adler32 = new Adler32();
		adler32.update(input, 0, length);
		return String.format("%08x", adler32.getValue());
	}

	public static String md5sum(byte[] input, int length)
	{
		try
		{
			MessageDigest md5 = MessageDigest.getInstance("MD5");
			md5.update(input, 0, length);
			return getHex(md5.digest());
		}
		catch (NoSuchAlgorithmException e) { return null; }
	}

	public static String sha1sum(byte[] input, int length)
	{
		try
		{
			MessageDigest sha1 = MessageDigest.getInstance("SHA1");
			sha1.update(input, 0, length);
			return getHex(sha1.digest());
		}
		catch (NoSuchAlgorithmException e) { return null; }
	}

	public static InetSocketAddress str2inet(String string)
	{
		try
		{
			URI    uri  = new URI(string.contains("://") ? string : "my://" + string);
			String host = uri.getHost();
			int    port = uri.getPort();
			// URI must have host and port parts
			return (host == null || port == -1) ? null : new InetSocketAddress(host, port);
		}
		// FIXME Which exception to catch ?
		catch (Exception e) { return null; }
	}

	public static <T> Set<T> array2set(T[] array)
	{
		return new HashSet<T>(Arrays.asList(array));
	}

	public static RtpPacket[][] createRandomRtpPacketsMatrix
		(byte pL, byte pD, int pMinPayloadSize, int pMaxPayloadSize)
	{
		Random random = new Random();
		RtpPacket[][] matrix = new RtpPacket[pD][pL];
		for (int d = 0; d < pD; d++)
		{
			for (int l = 0; l < pL; l++)
			{
				int    size      = pMinPayloadSize+random.nextInt(pMaxPayloadSize-pMinPayloadSize);
				byte[] payload   = new byte[size]; random.nextBytes(payload);
				int    sequence  = pL * d + l;
				long   timestamp = sequence*100+random.nextInt(50);
				matrix[d][l] = RtpPacket.createMP2T(sequence, timestamp, payload);
			}
		}
		return matrix;
	}
}

