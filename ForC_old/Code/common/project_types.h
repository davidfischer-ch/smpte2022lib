/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Project data types
    Main Developer : David Fischer (david.fischer.ch@gmail.com)
    Copyright      : Copyright (c) 2008-2013 smpte2022lib Team. All rights reserved.
    Sponsoring     : Developed for a HES-SO CTI Ra&D project called GaVi
                     Haute école du paysage, d'ingénierie et d'architecture @ Genève
                     Telecommunications Laboratory
\**************************************************************************************************/
/*
  This file is part of smpte2022lib Project.

  This project is free software: you can redistribute it and/or modify it under the terms of the
  EUPL v. 1.1 as provided by the European Commission. This project is distributed in the hope that
  it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.

  See the European Union Public License for more details.

  You should have received a copy of the EUPL General Public License along with this project.
  If not, see he EUPL licence v1.1 is available in 22 languages:
      22-07-2013, <https://joinup.ec.europa.eu/software/page/eupl/licence-eupl>

  Retrieved from https://github.com/davidfischer-ch/smpte2022lib.git
*/

#ifndef __PROJECT_TYPES__
#define __PROJECT_TYPES__

// Types de données créés durant le projet =====================================

#include "../data_structs/sChampBits.h"
#include "../data_structs/sLinkedList.h"
#include "../data_structs/sRbTree.h"

#include "../utilities/sTewfiq.h"

#include "../algo_structs/sSeqNx.h"
#include "../algo_structs/sCrossFec.h"
#include "../algo_structs/sPaquetFec.h"
#include "../algo_structs/sWaitFec.h"
#include "../algo_structs/sPaquetMedia.h"
#include "../algo_structs/sBufferFec.h"
#include "../algo_structs/sBufferMedia.h"

#include "../algorithmes/sBruteSmpte.h"
#include "../algorithmes/sDavidSmpte.h"

#endif
