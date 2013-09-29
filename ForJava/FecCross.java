/**********************************************************************************************************************\
                    OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description   : Link between a media packet <-> FEC packets able to recover it
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
public class FecCross
{
	int colSequence = -1;
	int rowSequence = -1;

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public String toString()
	{
		return
			"colSequence = "+colSequence+"\n"+
			"rowSequence = "+rowSequence+"\n";
	}
}
