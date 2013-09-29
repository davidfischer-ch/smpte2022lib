/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Messages (values)
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

#ifndef __MESSAGES__
#define __MESSAGES__

// Constantes messages utilisés ================================================

extern const char* cTheGuyTitle; //. Le mec derrière tout ça ;-)
extern const char* cTheGuyLabel; //. Le mec derrière tout ça ;-)

extern const char* cMsgLoose;
extern const char* cMsgMed;
extern const char* cMsgCol;
extern const char* cMsgRow;

extern const char* cMsgMedOk;
extern const char* cMsgMedPok;
extern const char* cMsgColOk;
extern const char* cMsgColPok;
extern const char* cMsgRowOk;
extern const char* cMsgRowPok;

extern const char* cMsgEnded;
extern const char* cMsgSmpteStandard;

extern const char* cMsgOverwriteMediaYes;
extern const char* cMsgOverwriteMediaNo;

extern const char* cMsgDavidArPaFecPresent;
extern const char* cMsgDavidArPaFecMissing;
extern const char* cMsgDavidRePaMediaRecover;

extern const char* cMsgBruteApFecPresent;
extern const char* cMsgBruteApFecMissing;
extern const char* cMsgBruteApFecRecover;

extern const char* cMsgPrintDavid;
extern const char* cMsgPrintDavidMedia;
extern const char* cMsgPrintDavidCross;
extern const char* cMsgPrintDavidWaitCol;
extern const char* cMsgPrintDavidWaitRow;

extern const char* cMsgPrintBrute;
extern const char* cMsgPrintBruteMedia;
extern const char* cMsgPrintBruteFec;

extern const char* cLabelVerbose1;
extern const char* cLabelVerbose2;
extern const char* cLabelAutoKey;
extern const char* cLabelAbout;
extern const char* cLabelHelp;
extern const char* cLabelSource;
extern const char* cLabelDest;
extern const char* cLabelDestRaw;
extern const char* cLabelDestDavid;
extern const char* cLabelDestBrute;
extern const char* cLabelLrecov;
extern const char* cLabel2DMatrix;
extern const char* cLabelMedia0;
extern const char* cLabelCol0;
extern const char* cLabelRow0;
extern const char* cLabelL;
extern const char* cLabelD;
extern const char* cLabelGap;
extern const char* cLabelTewfiqP;
extern const char* cLabelTewfiqQ;
extern const char* cLabelReorderVar;
extern const char* cLabelReorderProb;
extern const char* cLabelWindow;
extern const char* cLabelFBrute;

extern const char* cMsgAboutTGoal;
extern const char* cMsgAboutLGoal;
extern const char* cMsgAboutTFunction;

extern const char* cFecGeneratorLogFile;
extern const char* cFecGeneratorDestSuffixe;
extern const char* cFecGeneratorMsgTitle;
extern const char* cFecGeneratorMsgSyntax;
extern const char* cFecGeneratorMsgAboutLFunction;
extern const char* cFecGeneratorMsgHelp;
extern const char* cFecGeneratorMsgHelpGap0;
extern const char* cFecGeneratorMsgHelpGap1;
extern const char* cFecGeneratorMsg1of1;

extern const char* cErrorsGeneratorLogFile;
extern const char* cErrorsGeneratorDestSuffixe;
extern const char* cErrorsGeneratorMsgTitle;
extern const char* cErrorsGeneratorMsgSyntax;
extern const char* cErrorsGeneratorMsgAboutLFunction;
extern const char* cErrorsGeneratorMsgHelp;
extern const char* cErrorsGeneratorMsg1of3;
extern const char* cErrorsGeneratorMsg2of3;
extern const char* cErrorsGeneratorMsg3of3;

extern const char* cFecDecoderLogFile;
extern const char* cFecDecoderDestSuffixeRaw;
extern const char* cFecDecoderDestSuffixeDavid;
extern const char* cFecDecoderDestSuffixeBrute;
extern const char* cFecDecoderMsgTitle;
extern const char* cFecDecoderMsgSyntax;
extern const char* cFecDecoderMsgAboutLFunction;
extern const char* cFecDecoderMsgHelp;
extern const char* cFecDecoderMsg1of3;
extern const char* cFecDecoderMsg2of3;
extern const char* cFecDecoderMsg3of3;

extern const char* cExByeBye;
extern const char* cExUndefined;
extern const char* cExNullPtr;
extern const char* cExNullFunc;
extern const char* cExAllocateMemory;
extern const char* cExFreopen;

extern const char* cExAlgorithm;
extern const char* cExAlgorithmCaller;

extern const char* cExChampBitNo;
extern const char* cExRbTreeFirst;
extern const char* cExRbTreeValue;
extern const char* cExLinkedListValue;
extern const char* cExLectureNxValue;

extern const char* cExMediaForge;
extern const char* cExMediaAdd;
extern const char* cExMediaDelete;
extern const char* cExMediaToFile;

extern const char* cExFecForge;
extern const char* cExFecDelete;
extern const char* cExFecToFile;
extern const char* cExFecFindWait;
extern const char* cExFecFindCross;

extern const char* cExCrossNew;
extern const char* cExCrossAdd;
extern const char* cExCrossDelete;

extern const char* cExWaitAdd;
extern const char* cExWaitDelete;
extern const char* cExWaitLimit;
extern const char* cExWaitComputeJ;
extern const char* cExWaitComputeNo;
extern const char* cExWaitSetManque;

extern const char* cExSourceFile;
extern const char* cExDestFile;

extern const char* cExMatrixDim;
extern const char* cExMatrixLD;
extern const char* cExMatrixMin;
extern const char* cExMatrixMax;
extern const char* cExMatrixGap;
#endif
