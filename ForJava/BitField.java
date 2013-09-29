/**********************************************************************************************************************\
                    OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description   : Bit-field : A big unisgned number
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

// *********************************************************************************************************************
// TODO
public class BitField
{
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constants >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static final int BITS_COUNT  = 64;
	public static final int UNIT_SIZE   = Long.SIZE;
	public static final int UNITS_COUNT = BITS_COUNT / UNIT_SIZE;

	protected static final String ZERO_PADDING =
		"0000000000000000000000000000000000000000000000000000000000000000";

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Data Types >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public enum Direction { LSB_FIRST, MSB_FIRST };

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Fields >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	protected long buffer[] = new long[UNITS_COUNT];

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Properties >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public boolean getBit(int pPosition)
	{
		// FIXME Implement shifts if JIT is not able to optimize / by a 2^N to r-shift by N.
		// http://stackoverflow.com/questions/9511836/size-of-java-primitives-on-32-64-bit-jvm
		if (pPosition < 0 || pPosition >= BITS_COUNT) throw new IllegalArgumentException
			("pPosition not in range [0.."+BITS_COUNT+"] : "+pPosition);
		int  unit = pPosition / UNIT_SIZE;
		long bit  = pPosition - unit * UNIT_SIZE;
		return (buffer[unit] & (1L << bit)) != 0;
	}

	public void setBit(int pPosition, boolean pValue)
	{
		// FIXME Implement shifts if JIT is not able to optimize / by a 2^N to r-shift by N.
		// http://stackoverflow.com/questions/9511836/size-of-java-primitives-on-32-64-bit-jvm
		if (pPosition < 0 || pPosition >= BITS_COUNT) throw new IllegalArgumentException
			("pPosition not in range [0.."+BITS_COUNT+"] : "+pPosition);
		int  unit = pPosition / UNIT_SIZE;
		long bit  = pPosition - unit * UNIT_SIZE;
		long val  = buffer[unit];
		buffer[unit] = pValue ? val | (1L << bit) : val & ~(1L << bit);
	}

	int getOne(int pIndex, boolean pMsbFirst)
	{
		if (pIndex < 1) throw new IllegalArgumentException("pIndex not in range [1..] : "+pIndex);

		int n = 0;

		if (pMsbFirst)
		{
			for (int no = UNITS_COUNT-1; no >= 0; no--)
			{
				if (buffer[no] == 0) continue;

				for (int nb = UNIT_SIZE-1; nb >= 0; nb--)
				{
					if ((buffer[no] & (1L << nb)) != 0)
					{
						if (++n == pIndex) return no * UNIT_SIZE + nb;
					}
				}
			}
		}
		else
		{
			for (int no = 0; no < UNITS_COUNT; no++)
			{
				if (buffer[no] == 0) continue;

				for (int nb = 0; nb < UNIT_SIZE; nb++)
				{
					if ((buffer[no] & (1L << nb)) != 0)
					{
						if (++n == pIndex) return no * UNIT_SIZE + nb;
					}
				}
			}
		}

		return -1;
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public String toString()
	{
		String string = "[";
		for (long unit : buffer)
		{
			String result = ZERO_PADDING + Long.toBinaryString(unit);
			result = result.substring(result.length() - 64, result.length());
			string += result+"|";
		}
		return string+"]";
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Static >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static boolean UnitTest() // FIXME put this in a real unit test
	{
		BitField sequential = new BitField();
		for (int i = 0; i < BitField.BITS_COUNT; i++)
		{
			if (sequential.getBit(i) != false) return false;
			sequential.setBit(i,true);
			for (int j = 0; j < BitField.BITS_COUNT; j++)
			{
				if (sequential.getBit(j) == j>i) return false;
			}
		}
		for (int i = 0; i < BitField.BITS_COUNT; i++)
		{
			if (sequential.getOne(i+1, false) != i) return false;
			if (sequential.getOne(i+1, true)  != BitField.BITS_COUNT-(i+1)) return false;
		}

		for (int i = 0; i < BitField.BITS_COUNT; i++)
		{
			if (sequential.getBit(i) != true) return false;
			sequential.setBit(i,false);
			for (int j = 0; j < BitField.BITS_COUNT; j++)
			{
				if (sequential.getBit(j) != j>i) return false;
			}
		}
		for (int i = 0; i < BitField.BITS_COUNT; i++)
		{
			BitField onlyOne = new BitField();
			if (onlyOne.getOne(1, false) != -1) return false;
			if (onlyOne.getBit(i) != false) return false;
			onlyOne.setBit(i,true);
			if (onlyOne.getOne(1, false) != onlyOne.getOne(1, true)) return false;
			if (onlyOne.getOne(1, false) != i) return false;
			for (int j = 0; j < BitField.BITS_COUNT; j++)
			{
				if (onlyOne.getBit(j) != (j==i)) return false;
			}
		}
		return true;
	}
}
