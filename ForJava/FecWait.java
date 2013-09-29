/**********************************************************************************************************************\
                    OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description   : FEC packet stored for future usage
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
public class FecWait extends FecPacket
{
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constants >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static String ER_INDEX = "Unable to get missing media packet index";
	public static String ER_J =
		"Unable to find a suitable j € N that satisfy : pMediaSequence = snbase + j*offset";

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Fields >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	protected int      missingCount; //. Amount of missing (protected) media packets
	protected BitField missingValue; //. Every bit = flag (missing ?) of any protected media packet

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constructors >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public FecWait(byte[] pBytes, int pLength)
	{
		super(pBytes, pLength);
		missingValue = new BitField();
	}

	public FecWait(RtpPacket pPacket)
	{
		super(pPacket);
		missingValue = new BitField();
	}

	public FecWait(FecPacket pPacket)
	{
		ctorErrors          = pPacket.ctorErrors;
		sequence            = pPacket.sequence;
		algorithm           = pPacket.algorithm;
		direction           = pPacket.direction;
		snbase              = pPacket.snbase;
		offset              = pPacket.offset;
		na                  = pPacket.na;
		payloadTypeRecovery = pPacket.payloadTypeRecovery;
		timestampRecovery   = pPacket.timestampRecovery;
		lengthRecovery      = pPacket.lengthRecovery;
		payloadRecovery     = pPacket.payloadRecovery.clone();
		index               = pPacket.index;
		mask                = pPacket.mask;
		extended            = pPacket.extended;
		n                   = pPacket.n;
		missingValue        = new BitField();
	}

	// Constructor for unit testing purpose
	protected FecWait()
	{
		missingValue = new BitField();
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Properties >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public int getMissingSequence(int pIndex)
	{
		if (pIndex <= 0 || pIndex > missingCount) throw new IllegalArgumentException
			("pIndex not in range [1.."+missingCount+"] : "+pIndex);

		int j = missingValue.getOne (pIndex, false); // false = LSB_FIRST
		if (j < 0) throw new IllegalStateException(ER_INDEX);

		return (snbase + j* offset) & RtpPacket.S_MASK;
	}

	public int setMissing(int pMediaSequence)
	{
		int j = computeJ(pMediaSequence);
		if (j == -1) throw new IllegalArgumentException(ER_J);
		boolean value = missingValue.getBit(j);
		if (!value)
		{
			missingValue.setBit(j, true);
			missingCount++;
		}
		return j;
	}

	public int setRecovered(int pMediaSequence)
	{
		int j = computeJ(pMediaSequence);
		if (j == -1) throw new IllegalArgumentException(ER_J);
		boolean value = missingValue.getBit(j);
		if (value)
		{
			missingValue.setBit(j, false);
			missingCount--;
		}
		return j;
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	protected int computeJ(int pMediaSequence)
	{
		int delta = pMediaSequence - snbase;
		if (delta < 0) delta = RtpPacket.S_MASK+1 + delta;
		if (delta % offset != 0) return -1;

		return delta / offset;
	}

	public String toString()
	{
		String values = "";
		for (int i = 0; i < missingCount; i++)
		{
			values += " "+getMissingSequence(i+1);
		}
		return
			super.toString()+"\n"+
			"missing count       = "+missingCount+"\n"+
			"missing value       = "+values+"\n";
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Static >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static int UnitTest() // FIXME put this in a real unit test
	{
		int result = FecPacket.UnitTest();
		if (result != 0) return 1;

		FecWait w;
		boolean thrown;

		w = new FecWait();
		w.snbase = 65530;
		w.offset = 3;

		// Testing set/get of a unique missing value
		if (w.setMissing(3) != 3) return 2;
		if (w.getMissingSequence(1) != 3 || w.missingCount != 1) return 3;

		// Testing simple recovery of a unique value
		if (w.setRecovered(3) != 3 || w.missingCount != 0) return 4;

		// Testing set/get of multiple missing values (including re-setting of a value)
		if (w.setMissing(6) != 4 || w.setMissing(w.snbase) != 0 || w.setMissing(w.snbase+w.offset) != 1 ||
			w.setMissing(w.snbase+w.offset) != 1) return 5;
		if (w.getMissingSequence(1) != w.snbase || w.getMissingSequence(2) != w.snbase+w.offset ||
			w.getMissingSequence(3) != 6 || w.missingCount != 3) return 6;

		// Testing re-recovery of a value
		if (w.setRecovered(6) != 4 || w.setRecovered(6) != 4 || w.missingCount != 2) return 7;

		// Testing that bad input values effectively throws an exception
		thrown = false;
		try
		{
			// Remark :
			// If offset=1, we can't find a number x € N+ > 1 that satisfy lcm(offset,x)=offset*x
			w.setMissing(w.snbase + w.offset - 1);
		}
		catch (IllegalArgumentException e) { thrown = true; }
		if (!thrown) return 8;

		thrown = false;
		try
		{
			w.setRecovered(-1);
		}
		catch (IllegalArgumentException e) { thrown = true; }
		if (!thrown) return 9;

		return 0;
	}
}
