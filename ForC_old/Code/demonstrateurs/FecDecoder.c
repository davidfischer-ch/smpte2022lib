/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Decoder of the test bench
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

#include "../smpte.h"

// Variables Globales ==========================================================

static bool     optionAutoKey   = false; //. Automatiquement valider les msgs ?
static char*    optionSource    = NULL;  //. Fichier source
static char*    optionDestRaw   = NULL;  //. Fichier destination raw
static char*    optionDestDavid = NULL;  //. Fichier destination david
static char*    optionDestBrute = NULL;  //. Fichier destination brute
static sMediaNo optionWindow    = 200;   //. Nb de media stockés avant lecture
static unsigned optionFBrute    = 0;     //. Fréquence du traitement brute

static sDavidSmpte david; //. Notre variable d'utilisation de l'algo optimisé
static sBruteSmpte brute; //. Notre variable d'utilisation de l'algo force brute
static bool        init = false; //. Algorithmes initialisés ?

// Fonctions publiques =========================================================

// Affiche (et enregistre dans un fichier) le contenu des deux algorithmes -----
void AssertPrintError()
{
  if (!init) return;

  sDavidSmpte_Print (&david, true);

  if (optionFBrute > 0)
  {
    sBruteSmpte_Print (&brute, true);
  }
}

// Point d'entrée du programme -------------------------------------------------
//> Code d'erreur renvoyé au système (0 = ok)
int main (int argc, char ** argv)
{
  PRINT_INIT_COLOR()

  // Affiche le titre du logiciel
  PRINT0_CONc (cConDefault, cFecDecoderMsgTitle)

  signed sno;
  for (sno = 1; sno < argc; sno++)
  {
    char* arg = argv[sno];

    if (strcmp (arg, cLabelVerbose1) == 0)
    {
      verbose = 1;
    }
    else if (strcmp (arg, cLabelVerbose2) == 0)
    {
      verbose = 2;
    }
    else if (strcmp (arg, cLabelAbout) == 0)
    {
      PRINT0_CON    (cConTitle,   "%s", cMsgAboutTGoal)
      PRINT0_CON    (cConDefault, "%s", cMsgAboutLGoal)
      PRINT0_CON    (cConTitle,   "%s", cMsgAboutTFunction)
      PRINT0_CON    (cConDefault, "%s", cFecDecoderMsgAboutLFunction)
      PRINT0_CON    (cConTitle,   "%s", cTheGuyTitle)
      PRINT0_CON    (cConDefault, "%s", cTheGuyLabel)
      KeyToContinue (optionAutoKey);

      return 0;
    }
    else if (strcmp (arg, cLabelHelp) == 0)
    {
      PRINT0_CON    (cConDefault, "%s", cFecDecoderMsgHelp)
      KeyToContinue (optionAutoKey);

      return 0;
    }
    else
    {
      char* value;

      if ((value = GetParameterValue (arg, cLabelSource, '=')) != 0)
      {
        optionSource = value;
      }
      else if ((value = GetParameterValue (arg, cLabelDestRaw, '=')) != 0)
      {
        optionDestRaw = value;
      }
      else if ((value = GetParameterValue (arg, cLabelDestDavid, '=')) != 0)
      {
        optionDestDavid = value;
      }
      else if ((value = GetParameterValue (arg, cLabelDestBrute, '=')) != 0)
      {
        optionDestBrute = value;
      }
      else if ((value = GetParameterValue (arg, cLabelWindow, '=')) != 0)
      {
        optionWindow = atoi (value);
      }
      else if ((value = GetParameterValue (arg, cLabelFBrute, '=')) != 0)
      {
        optionFBrute = atoi (value);
      }
      else // Un paramètre incorrect
      {
        goto __params_error;
      }
    }
  }

  if (optionSource == 0)
  {
  __params_error:
    PRINT0_CON    (cConError, "%s", cFecDecoderMsgSyntax)
    PRINT0_CON    (cConError, "%s", cFecDecoderMsgHelp)
    KeyToContinue (optionAutoKey);

    return 0;
  }

  PRINT0_FILE (cFecDecoderLogFile, "w",
              "window:%u, fbrute:%u\n\n", optionWindow, optionFBrute)

  // ===========================================================================

  // Ouverture de la source et création des destinations

  FILE*    source = fopen (optionSource, "rb");
  ASSERTc (source, -1, cExSourceFile)

  if (optionDestRaw == 0)
    optionDestRaw = UtilePourMoi (optionSource, cFecDecoderDestSuffixeRaw);

  FILE*    destRaw = fopen (optionDestRaw, "wb");
  ASSERTc (destRaw, -1, cExDestFile)

  if (optionDestDavid == 0)
    optionDestDavid = UtilePourMoi (optionSource, cFecDecoderDestSuffixeDavid);

  FILE*    destDavid = fopen (optionDestDavid, "wb");
  ASSERTc (destDavid, -1, cExDestFile)

  FILE* destBrute = 0;

  if (optionFBrute > 0)
  {
    if (optionDestBrute == 0)
      optionDestBrute= UtilePourMoi (optionSource, cFecDecoderDestSuffixeBrute);

    destBrute = fopen (optionDestBrute, "wb");
    ASSERTc (destBrute, -1, cExDestFile)
  }

  // Lecture de la taille du fichier source
  fseek (source, 0, SEEK_END);
  long sourceSize = ftell (source);
  long sourcePos  = 0;
  rewind (source);

  // Barre de pourcentage
  double oldPcent = 0, newPcent = 0;

  // INITIALISATION DE SESSION MEDIA / FEC =====================================

  david = sDavidSmpte_New (true);

  if (optionFBrute > 0)
  {
    brute = sBruteSmpte_New (true);
  }

  init = true;

  // DÉMARRAGE DE SESSION MÉDIA / FEC ==========================================

  // BOUCLE DE LECTURE DU FICHIER RTP + FEC -> ALGORITHME DE FEC ===============

  PRINT0_CONc    (cConDefault, cFecDecoderMsg1of3)
  PRINT_SET_FILE (cFecDecoderLogFile, "a")

  unsigned nbMedia = 0;

  bool eof = false;

  while (!eof)
  {
    sPaquetMedia *_mediaDavid, *_mediaBrute = 0;
    sPaquetFec   *_fecDavid,   *_fecBrute   = 0;

    // RÉCEPTION D'UN PAQUET MÉDIA =============================================

    if ((_mediaDavid = sPaquetMedia_FromFile (source)) != 0)
    {
      PRINT1 ("paquet media lu : ")
      sPaquetMedia_Print (_mediaDavid);
      PRINT1 ("\n")

      sPaquetMedia_ToFile (_mediaDavid, destRaw, false);

      if (optionFBrute > 0)
      {
        _mediaBrute = sPaquetMedia_Copy (_mediaDavid);
      }

      sDavidSmpte_ArriveePaquetMedia (&david, _mediaDavid);

      nbMedia++;

      if (optionFBrute > 0)
      {
        sBruteSmpte_ArriveePaquetMedia (&brute, _mediaBrute);
      }
    }

    // RÉCEPTION D'UN PAQUET FEC ===============================================

    else if ((_fecDavid = sPaquetFec_FromFile (source)) != 0)
    {
      PRINT1 ("paquet FEC lu : ")
      sPaquetFec_Print (_fecDavid);
      PRINT1 ("\n")

      if (optionFBrute > 0)
      {
        _fecBrute = sPaquetFec_Copy (_fecDavid);
      }

      sDavidSmpte_ArriveePaquetFec (&david, _fecDavid);

      if (optionFBrute > 0)
      {

        sBruteSmpte_ArriveePaquetFec (&brute, _fecBrute);
      }
    }
    else
    {
      eof = true;
    }

    // SIMULE LA LECTURE DE X PAQUETS MEDIA ====================================

    while (optionWindow > 0)
    {
      if (!sDavidSmpte_LecturePaquetMedia (&david, optionWindow, destDavid))
        break;
    }

    if (optionFBrute > 0)
    {
      while (optionWindow > 0)
      {
        if (!sBruteSmpte_LecturePaquetMedia (&brute, optionWindow, destBrute))
          break;
      }
    }

    // APPLIQUE PÉRIODIQUEMENT LA FORCE BRUTE ==================================

    if (optionFBrute > 0)
    {
      // Applique la FEC brute au bout d'un certain nb de paquets média reçus
      if (nbMedia % optionFBrute == 0)
      {
        sBruteSmpte_AppliqueFec (&brute);
      }
    }

    // Met à jour la barre de pourcentage
    sourcePos = ftell (source);
    PCENT ((double)sourcePos / (double)sourceSize, eof, cFecDecoderLogFile)
  }

  if (optionFBrute > 0)
  {
    // Applique la FEC brute une dernière fois (éviter différences! avec david)
    sBruteSmpte_AppliqueFec (&brute);
  }

  // SIMULE LA LECTURE COMPLÈTE DES BUFFERS MÉDIA ==============================

  PRINT0_CONc    (cConDefault, cFecDecoderMsg2of3)
  PRINT_SET_FILE (cFecDecoderLogFile, "a")

  oldPcent   = 0;
  sourcePos  = 0;
  sourceSize = david.media.rbtree.count > 0 ? david.media.rbtree.count : 1;

  eof = false;
  while (!eof)
  {
    eof = !sDavidSmpte_LecturePaquetMedia (&david, 0, destDavid);

    // Met à jour la barre de pourcentage
    PCENT ((double)sourcePos / (double)sourceSize, sourcePos == sourceSize,
           cFecDecoderLogFile)
    sourcePos++;
  }

  PRINT0_CONc    (cConDefault, cFecDecoderMsg3of3)
  PRINT_SET_FILE (cFecDecoderLogFile, "a")

  oldPcent   = 0;
  sourcePos  = 0;
  sourceSize = brute.media.rbtree.count > 0 ? brute.media.rbtree.count : 1;

  if (optionFBrute > 0)
  {
    eof = false;
    while (!eof)
    {
      eof = !sBruteSmpte_LecturePaquetMedia (&brute, 0, destBrute);

      // Met à jour la barre de pourcentage
      PCENT ((double)sourcePos / (double)sourceSize, sourcePos == sourceSize,
             cFecDecoderLogFile)
      sourcePos++;
    }
  }

  sDavidSmpte_Print (&david, true);

  if (optionFBrute > 0)
  {
    sBruteSmpte_Print (&brute, true);
  }

  // FIN DE SESSION MEDIA / FEC ================================================

  sDavidSmpte_Release (&david);

  if (optionFBrute > 0)
  {
    sBruteSmpte_Release (&brute);
    fclose (destBrute);
  }

  fclose (source);
  fclose (destRaw);
  fclose (destDavid);

  // ===========================================================================

  PRINT0_CONc   (cConDefault, cMsgEnded)
  KeyToContinue (optionAutoKey);

  return 0;
}
