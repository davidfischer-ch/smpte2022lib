/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Messages (values)
    Authors     : David Fischer
    Contact     : david.fischer.ch@gmail.com / david.fischer@hesge.ch
    Copyright   : 2008-2013 smpte2022lib Team. All rights reserved.
    Sponsoring  : Developed for a HES-SO CTI Ra&D project called GaVi
                  Haute école du paysage, d'ingénierie et d'architecture @ Genève
                  Telecommunications Laboratory
\**************************************************************************************************/
/*
  This file is part of smpte2022lib.

  This project is free software: you can redistribute it and/or modify it under the terms of the
  GNU General Public License as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this project.
  If not, see <http://www.gnu.org/licenses/>

  Retrieved from:
    git clone git://github.com/davidfischer-ch/smpte2022lib.git
*/

#include "../smpte.h"

// Constantes messages utilisés ================================================

const char* cTheGuyTitle =
  "Developed in 2008-2009 to contribute to VLC project (and others), by :\n";

const char* cTheGuyLabel =
  "  David Fischer\n"
  "  david.fischer@hesge.ch\n"
  "  david.fischer.ch@gmail.com\n";

const char* cMsgLoose = "[*** %u] ";
const char* cMsgMed   = "[med %u] ";
const char* cMsgCol   = "[col %u] ";
const char* cMsgRow   = "[row %u] ";

const char* cMsgMedOk  = "[ ]";
const char* cMsgMedPok = "[*]";
const char* cMsgColOk  = "[c]";
const char* cMsgColPok = "[*]";
const char* cMsgRowOk  = "[r]";
const char* cMsgRowPok = "[*]";

const char* cMsgEnded = "\n\nWork finished !\n";

const char* cMsgSmpteStandard =
  "Warning, SMPTE STANDARD : \n"
  "As a minimum, senders and receivers shall support all combinations\n"
  "of values of L and D that comply with the limits below:\n"
  "L * D <= 100\n"
  "1 <= L <= 20\n"
  "4 <= D <= 20\n\n"
  "But in our case limits are fixed by sChampBit, so I can handle:\n"
  "L * D <= 65'536\n"
  "1 <= L <= 256\n"
  "1 <= D <= 256\n";

// Constantes messages algorithmes =============================================

const char* cMsgOverwriteMediaYes = "accepted";
const char* cMsgOverwriteMediaNo  = "asserted";

const char* cMsgDavidArPaFecPresent =
  "David ArriveePaquetFec : media packet %u is present\n";

const char* cMsgDavidArPaFecMissing =
  "David ArriveePaquetFec : media packet %u missing\n";

const char* cMsgDavidRePaMediaRecover =
  "David RecupPaquetMedia : recover a media packet mediaNo=%u !\n";

const char* cMsgBruteApFecPresent =
  "Brute AppliqueFec : media packet %u is present\n";

const char* cMsgBruteApFecMissing =
  "Brute AppliqueFec : media packet %u missing\n";

const char* cMsgBruteApFecRecover =
  "Brute AppliqueFec : recover a media packet mediaNo=%u !\n";

const char* cMsgPrintDavidMedia =
  "\n\n"
  "DAVID FEC ALGORITHM DETAILS\n\n"
  "1 of 4) Content of the media buffer\n"
  "***********************************\n";

const char* cMsgPrintDavidCross =
  "\n"
  "2 of 4) Content of the fec cross buffer\n"
  "***************************************\n";

const char* cMsgPrintDavidWaitCol =
  "\n"
  "3 of 4) Content of the fec wait[COL] buffer\n"
  "*******************************************\n";

const char* cMsgPrintDavidWaitRow =
  "\n"
  "4 of 4) Content of the fec wait[ROW] buffer\n"
  "*******************************************\n";

const char* cMsgPrintDavid =
  "\n"
  "overwrite media        = %s\n"
  "overwrite count        = %u media packets\n"
  "recovered              = %u media packets\n"
  "unrecovered on reading = %u media packets\n"
  "reading no             = %u mediaNo\n"
  "arrival no             = %u mediaNo\n"
  "chrono total           = %lu ms\n"
  "chrono media           = %lu ms\n"
  "chrono FEC             = %lu ms\n\n"
  "nb ArriveePaquetMedia  = %u calls\n"
  "nb ArriveePaquetFec    = %u calls\n"
  "nb LecturePaquetMedia  = %u calls\n"
  "nb PerduPaquetMedia    = %u calls\n"
  "nb RecupPaquetMedia    = %u calls\n"
  "maximum buffered cross = %u nodes\n"
  "maximum buffered wait  = %u nodes\n\n";

const char* cMsgPrintBruteMedia =
  "\n\n"
  "BRUTE FEC ALGORITHM DETAILS\n\n"
  "1 of 2) Content of the media buffer\n"
  "***********************************\n";

const char* cMsgPrintBruteFec =
  "\n"
  "2 of 2) Content of the fec buffer\n"
  "*********************************\n";

const char* cMsgPrintBrute =
  "\n"
  "overwrite media        = %s\n"
  "overwrite count        = %u media packets\n"
  "recovered              = %u media packets\n"
  "unrecovered on reading = %u media packets\n"
  "reading no             = %u mediaNo\n"
  "arrival no             = %u mediaNo\n"
  "chrono total           = %lu ms\n"
  "chrono media           = %lu ms\n"
  "chrono FEC             = %lu ms\n\n"
  "nb ArriveePaquetMedia  = %u calls\n"
  "nb ArriveePaquetFec    = %u calls\n"
  "nb LecturePaquetMedia  = %u calls\n"
  "nb AppliqueFec         = %u calls\n"
  "maximum buffered FEC   = %u FEC packets\n\n";

// Constantes labels de paramètres =============================================

const char* cLabelVerbose1    = "vv";
const char* cLabelVerbose2    = "vvv";
const char* cLabelAutoKey     = "auto";
const char* cLabelAbout       = "about";
const char* cLabelHelp        = "help";
const char* cLabelSource      = "source";
const char* cLabelDest        = "dest";
const char* cLabelDestRaw     = "destRaw";
const char* cLabelDestDavid   = "destDavid";
const char* cLabelDestBrute   = "destBrute";
const char* cLabelLrecov      = "lrecov";
const char* cLabel2DMatrix    = "matrix";
const char* cLabelMedia0      = "media0";
const char* cLabelCol0        = "col0";
const char* cLabelRow0        = "row0";
const char* cLabelL           = "l";
const char* cLabelD           = "d";
const char* cLabelGap         = "gap";
const char* cLabelTewfiqP     = "p";
const char* cLabelTewfiqQ     = "q";
const char* cLabelReorderVar  = "var";
const char* cLabelReorderProb = "prob";
const char* cLabelWindow      = "window";
const char* cLabelFBrute      = "fbrute";

// Constantes messages modules =================================================

const char* cMsgAboutTGoal =
  "Goal of this module :\n";

const char* cMsgAboutLGoal =
  "  Proove that my smpte2022  decoder algorithms work's fine\n\n";

const char* cMsgAboutTFunction =
  "Function of this module :\n";

#define GENFEC "FecGenerator"
#define GENERR "ErrorsGenerator"
#define DECFEC "FecDecoder"

// Constantes messages GenerateurFec ===========================================

const char* cFecGeneratorLogFile     = GENFEC ".log";
const char* cFecGeneratorDestSuffixe = "_rtp_et_fec";

const char* cFecGeneratorMsgTitle =
  "\nDemo " GENFEC " by David Fischer!\n\n";

const char* cFecGeneratorMsgSyntax =
  "Please, call this program with those arguments (3 variants):\n\n";

const char* cFecGeneratorMsgAboutLFunction =
  "  Generate a file who represent media and FEC RTP packets protected\n"
  "  by the SMPTE 2022-1  norm from a (simple) source file.\n"
  "  The result can be readed by "GENERR" or "DECFEC" module.\n\n";

const char* cFecGeneratorMsgHelp =
  GENFEC ".exe (vv(v) auto) about : about text and exit\n"
  GENFEC ".exe (vv(v) auto) help  : this text and exit\n"
  GENFEC ".exe (vv(v) auto) source=\"...\" ... lrecov=(value) L=(value) ... *\n"
  "* missing options (excepted source) are setted to default\n\n"
  "vv:     verbose level 1 if present\n"
  "vvv:    verbose level 2 if present\n"
  "auto:   console don't wait for a key press if this option is present\n"
  "source: name of the source file\n"
  "dest:   name of the destination file\n\n"
  "lrecov  [1316] Length_recovery parameter of the FEC packets\n"
  "matrix  [2]    type of FEC to apply, 1D or 2D ? 1=1D 2=2D\n"
  "media0  [0]    RTP sequency number of the first media packet\n"
  "col0    [0]    RTP sequency number of the first col FEC packet\n"
  "row0    [0]    RTP sequency number of the first row FEC packet\n"
  "L       [5]    L parameter of the FEC matrix\n"
  "D       [5]    D parameter of the FEC matrix\n"
  "gap     [0]    gap between the packets of the FEC matrix *\n\n"
  "* You can know what is gap with the help option (press enter)\n";

const char* cFecGeneratorMsgHelpGap0 =
  "There, colNo's are replaced by letters a,b ...\n\n"
  "3x3 gap 0 : can be viewed as a continuous set of 3x3 matrix's\n\n"
  "    <- L ->\n"
  " |  a0 b0 c0 > 3 media pk protected by FEC row 0 & col a b c\n"
  " D  a1 b1 c1 > 3 media pk protected by FEC row 1 & col a b c\n"
  " |  a2 b2 c2 > 3 media pk protected by FEC row 2 & col a b c\n\n"
  "    d3 e3 f3 > 3 media pk protected by FEC row 3 & col d e f\n"
  "    d4 e4 f4 > 3 media pk protected by FEC row 4 & col d e f\n"
  "    d5 e5 f5 > 3 media pk protected by FEC row 5 & col d e f\n";

const char* cFecGeneratorMsgHelpGap1 =
  "3x3 gap 1 : can't be viewed as a continuous set of 3x3 matrix's\n\n"
  "    <- L packets ->\n"
  " |  a0 .0 .0 > 3 media pk protected by FEC row 0 & col a . .\n"
  " D  a1 b1 .1 > 3 media pk protected by FEC row 1 & col a b .\n"
  " |  a2 b2 c2 > 3 media pk protected by FEC row 2 & col a b c\n"
  "    d3 b3 c3 > 3 media pk protected by FEC row 3 & col d b c\n"
  "    d4 e4 c4 > 3 media pk protected by FEC row 4 & col d e c\n"
  "    d5 e5 f5 > 3 media pk protected by FEC row 5 & col d e f\n"
  "    g6 e6 f6 > 3 media pk protected by FEC row 6 & col g e f\n";

const char* cFecGeneratorMsg1of1  = "[1 of 1] Work in progress... ";

// Constantes messages GenerateurErreurs =======================================

const char* cErrorsGeneratorLogFile     = GENERR ".log";
const char* cErrorsGeneratorDestSuffixe = "_et_erreurs";

const char* cErrorsGeneratorMsgTitle =
  "\nDemo " GENERR " by David Fischer!\n\n";

const char* cErrorsGeneratorMsgSyntax =
  "Please, call this program with those arguments (3 variants):\n\n";

const char* cErrorsGeneratorMsgAboutLFunction =
  "  Handle a file who represent media and FEC RTP packets protected\n"
  "  by the SMPTE 2022-1  norm and generate a destination who is \n"
  "  the source with some errors (lossing & reordering of RTP packets).\n"
  "  The result can be readed by "DECFEC" module.\n\n";

const char* cErrorsGeneratorMsgHelp =
  GENERR ".exe (vv(v) auto) about : about text and exit\n"
  GENERR ".exe (vv(v) auto) help  : this text and exit\n"
  GENERR ".exe (vv(v) auto) source=\"...\" ... var=(value) ... *\n"
  "* missing options (excepted source) are setted to default\n\n"
  "vv:     verbose level 1 if present\n"
  "vvv:    verbose level 2 if present\n"
  "auto:   console don't wait for a key press if this option is present\n"
  "source: name of the source file (must contain RTP+FEC packets)\n"
  "dest:   name of the destination file\n\n"
  "p    [0.001] probability (maximum is 1) to have a RTP loss burst\n"
  "q    [2]     mean length (number of packets) of a RTP loss burst\n"
  "var  [4]     length (number of packets) of a RTP reordering burst\n"
  "prob [0.01]  probability (maximum is 1) to have a RTP reordering burst\n";

const char* cErrorsGeneratorMsg1of3=  "[1 of 3] Reading&loosing in progress ";
const char* cErrorsGeneratorMsg2of3="\n[2 of 3] Reordering      in progress ";
const char* cErrorsGeneratorMsg3of3="\n[3 of 3] Writing         in progress ";

// Constantes messages DecodeurFec =============================================

const char* cFecDecoderLogFile          = DECFEC ".log";
const char* cFecDecoderDestSuffixeRaw   = "_resultat_sans_fec";
const char* cFecDecoderDestSuffixeDavid = "_resultat_avec_david";
const char* cFecDecoderDestSuffixeBrute = "_resultat_avec_brute";

const char* cFecDecoderMsgTitle =
  "\nDemo " DECFEC " by David Fischer!\n\n";

const char* cFecDecoderMsgSyntax =
  "Please, call this program with those arguments (3 variants):\n\n";

const char* cFecDecoderMsgAboutLFunction =
  "  Handle a file who represent media and FEC RTP packets protected\n"
  "  by the SMPTE 2022-1  norm (with / out errors) and generate a\n"
  "  destination who is the source recovered by the FEC algorithms.\n"
  "  In parallel another file is created from the same source, this\n"
  "  one is generated without FEC recovery to proove that SMPTE-FEC\n"
  "  is not useless!\n\n";

const char* cFecDecoderMsgHelp =
  DECFEC ".exe (vv(v) auto) about : about text and exit\n"
  DECFEC ".exe (vv(v) auto) help  : this text and exit\n"
  DECFEC ".exe (vv(v) auto) source=\"...\" ... window=(value) ... *\n"
  "* missing options (excepted source) are setted to default\n\n"
  "vv:        verbose level 1 if present\n"
  "vvv:       verbose level 2 if present\n"
  "auto:      console don't wait for a key press if this option is present\n"
  "source:    name of the source file (must contain RTP+FEC packets)\n"
  "destRaw:   name of the destination file generated without FEC recovery\n"
  "destDavid: name of the destination file generated by david's algorithm\n"
  "destBrute: name of the destination file generated by brute's algorithm\n\n"
  "window [200] number (max) of media packets to have in buffer (0=infinite)\n"
  "fbrute [0]   periodicity of the 'brute' treatment (0=don't use this algo)\n"
  "             ex. 6 mean: do the 'brute' treatment each 6 packets received\n";

const char* cFecDecoderMsg1of3  =   "[1 of 3] Work             in progress ";
const char* cFecDecoderMsg2of3  = "\n[2 of 3] Writing to david in progress ";
const char* cFecDecoderMsg3of3  = "\n[3 of 3] Writing to brute in progress ";

// Constantes messages d'exception =============================================

const char* cExByeBye =
  "\nSorry but this program is not able to continue : bye bye\n\n";

const char* cExUndefined      = "Undefined message";
const char* cExNullPtr        = "This ptr can't be null";
const char* cExNullFunc       = "This must be linked to a function";
const char* cExAllocateMemory = "Unable to allocate memory";
const char* cExFreopen        = "Unable to reopen standard (console) output";

const char* cExAlgorithm       = "Bug of the algorithm";
const char* cExAlgorithmCaller = "Bug of the caller / algorithm";

const char* cExChampBitNo = "No must be greater than 0";

const char* cExRbTreeFirst = "Unable to find first node of the rbtree";
const char* cExRbTreeValue = "Unable to get node value";

const char* cExLinkedListValue = "Unable to get element value";
const char* cExLectureNxValue  = "Unable to get lectureNx value";

const char* cExMediaForge  = "Unable to <forge> a new media packet";
const char* cExMediaAdd    = "Unable to add media packet %u to the buffer";
const char* cExMediaDelete = "Unable to delete media packet %u from the buffer";
const char* cExMediaToFile = "Unable to save the media packet %u to the file";

const char* cExFecForge    = "Unable to <forge> a new FEC packet";
const char* cExFecDelete   = "Unable to delete FEC packet from the buffer";
const char* cExFecToFile   = "Unable to save the FEC packet %u to the file";
const char* cExFecFindWait =
  "Unable to find the wait FEC linked to the cross FEC parameters";
const char* cExFecFindCross =
  "Unable to find the cross FEC linked to the media packets %u";

const char* cExCrossNew    = "Unable to create a new cross FEC";
const char* cExCrossAdd    = "Unable to add cross FEC %u to the buffer";
const char* cExCrossDelete = "Unable to delete cross FEC from the buffer";

const char* cExWaitAdd       = "Unable to add wait FEC %u to the buffer";
const char* cExWaitDelete    = "Unable to delete wait FEC from the buffer";
const char* cExWaitLimit     = "Is out of limits %u";
const char* cExWaitComputeJ  = "Unable to compute j from wait FEC parameters";
const char* cExWaitComputeNo =
  "Unable to compute mediaNo form wait FEC parameters";
const char* cExWaitSetManque = "Unable to set a bit of wait FEC";

const char* cExSourceFile = "Unable to open source file";
const char* cExDestFile   = "Unable to create destination file";

const char* cExMatrixDim = "Must be 1 (1D) or 2 (2D)";
const char* cExMatrixLD  = "LD must be equal to L*D";
const char* cExMatrixMin = "Must be greater or equal to 1, that's logic";
const char* cExMatrixMax = "Must be smaller, that's sChampBit fault";
const char* cExMatrixGap = "Gap must be smaller than D";
