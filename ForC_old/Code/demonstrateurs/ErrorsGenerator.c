/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Errors generator of the test bench
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

// Types de données ============================================================

// Structure représentant un paquet soit média/FEC pour générateur erreurs -----
typedef struct
{
  bool  media;  //. Est-ce un paquet média (ou de FEC) ?
  void* paquet; //. Pointeur vers le paquet
} sPaquets;

// Variables Globales ==========================================================

static bool   optionAutoKey   = false;   //. Automatiquement valider les msgs ?
static char*  optionSource      = NULL;  //. Fichier source
static char*  optionDest        = NULL;  //. Fichier destination
static double optionTewfiqP     = 0.001; //. Prob de passer de ok à perte max 1
static double optionTewfiqQ     = 0.500; //. Prob de passer de perte à ok max 1
static double optionReorderVar  = 4.00;  //. Variance du reordering max ?
static double optionReorderProb = 0.1;   //. Proba. de passer à reorder max 1

static uint8_t detectedL = 0; //. Taille (colonne) de la matrice de FEC
static uint8_t detectedD = 0; //. Taille (ligne)   de la matrice de FEC

#define MAXP 1024*1024
static sPaquets bufP[MAXP]; //. Tampon de paquets média et de FEC
static unsigned numP = 0;   //. Nombre de paquets contenu par le buffer bufP

// Fonctions publiques =========================================================

// Affiche (et enregistre dans un fichier) le contenu des deux algorithmes -----
void AssertPrintError()
{
}

// Point d'entrée du programme -------------------------------------------------
//> Code d'erreur renvoyé au système (0 = ok)
int main (int argc, char ** argv)
{
  PRINT_INIT_COLOR()

  unsigned no;

  // Affiche le titre du logiciel
  PRINT0_CONc (cConDefault, cErrorsGeneratorMsgTitle)

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
    else if (strcmp (arg, cLabelAutoKey) == 0)
    {
      optionAutoKey = true;
    }
    else if (strcmp (arg, cLabelAbout) == 0)
    {
      PRINT0_CON    (cConTitle,   "%s", cMsgAboutTGoal)
      PRINT0_CON    (cConDefault, "%s", cMsgAboutLGoal)
      PRINT0_CON    (cConTitle,   "%s", cMsgAboutTFunction)
      PRINT0_CON    (cConDefault, "%s", cErrorsGeneratorMsgAboutLFunction)
      PRINT0_CON    (cConTitle,   "%s", cTheGuyTitle)
      PRINT0_CON    (cConDefault, "%s", cTheGuyLabel)
      KeyToContinue (optionAutoKey);

      return 0;
    }
    else if (strcmp (arg, cLabelHelp) == 0)
    {
      PRINT0_CON    (cConDefault, "%s", cErrorsGeneratorMsgHelp)
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
      else if ((value = GetParameterValue (arg, cLabelDest, '=')) != 0)
      {
        optionDest = value;
      }
      else if ((value = GetParameterValue (arg, cLabelTewfiqP, '=')) != 0)
      {
        double p = fabs (strtod (value, NULL));
        if (p > 1.0) p = 1.0;

        optionTewfiqP = p;
      }
      else if ((value = GetParameterValue (arg, cLabelTewfiqQ, '=')) != 0)
      {
        double q = fabs (strtod (value, NULL));
        if (q < 1.0) q = 1.0;

        optionTewfiqQ = 1.0 / q;
      }
      else if ((value = GetParameterValue (arg, cLabelReorderVar, '=')) != 0)
      {
        double v = fabs (strtod (value, NULL));
        if (v > 5.0) v = 5.0;

        optionReorderVar = v;
      }
      else if ((value = GetParameterValue (arg, cLabelReorderProb, '=')) != 0)
      {
        double r = fabs (strtod (value, NULL));
        if (r > 1.0) r = 1.0;

        optionReorderProb = r;
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
    PRINT0_CON    (cConError, "%s", cErrorsGeneratorMsgSyntax)
    PRINT0_CON    (cConError, "%s", cErrorsGeneratorMsgHelp)
    KeyToContinue (optionAutoKey);

    return 0;
  }

  PRINT0_FILE (cErrorsGeneratorLogFile, "w",
              "P:%g, Q:%g var:%g prob:%g\n\n",
              optionTewfiqP, optionTewfiqQ, optionReorderVar, optionReorderProb)

  // ===========================================================================

  if (optionDest == 0)
  {
    optionDest = UtilePourMoi (optionSource, cErrorsGeneratorDestSuffixe);
  }

  // Ouverture de la source
  FILE*    source = fopen (optionSource, "rb");
  ASSERTc (source, -1, cExSourceFile)
  // Création de la destination
  FILE*    dest = fopen (optionDest, "wb");
  ASSERTc (dest, -1, cExDestFile)

  // Lecture de la taille du fichier source
  fseek (source, 0, SEEK_END);
  long sourceSize = ftell (source);
  long sourcePos  = 0;
  rewind (source);

  // Barre de pourcentage
  double oldPcent = 0, newPcent = 0;

  // Initialise Tewfiq !
  sTewfiq _tewfiqPerte   = sTewfiq_New2 (optionTewfiqP, optionTewfiqQ);
  sTewfiq _tewfiqReorder = sTewfiq_New2 (optionReorderProb, 1);

  // BOUCLE DE CONVERSION DU FICHIER RTP+FEC -> FICHIER RTP+FEC+ERREURS ========

  PRINT0_CONc    (cConDefault, cErrorsGeneratorMsg1of3)
  PRINT_SET_FILE (cErrorsGeneratorLogFile, "a")

  bool     eof = false;
  unsigned mediaNb = 0;

  while (!eof)
  {
    ASSERT (numP < MAXP, -1, "oups...")
    IFNOT  (numP < MAXP, -1)

    // LECTURE D'UN PAQUET MÉDIA ===============================================

    bool keep = sTewfiq_IsOkayOrLost (&_tewfiqPerte);

    if ((bufP[numP].paquet = sPaquetMedia_FromFile (source)) != 0)
    {
      bufP[numP].media = true;

      //sPaquetMedia* _media = bufP[numP].paquet;

      if (detectedL != 0 && detectedD != 0)
      {
        if (mediaNb %  detectedL            == 0) PRINT1("\n")
        if (mediaNb % (detectedL*detectedD) == 0) PRINT1("\n")
      }
      mediaNb++;

      // Perte  ?
      if (!keep)
      {
        PRINT0c (cMsgMedPok)
        sPaquetMedia_Release (bufP[numP].paquet);
      }
      else
      {
        PRINT0c (cMsgMedOk)
        numP++;
      }
    }
    else if ((bufP[numP].paquet = sPaquetFec_FromFile (source)) != 0)
    {
      bufP[numP].media = false;

      sPaquetFec* _fec = bufP[numP].paquet;

      if (_fec->DWORD3.D == COL)
      {
        detectedL = _fec->DWORD3.Offset;
        detectedD = _fec->DWORD3.NA;
      }
      else
      {
        detectedL = _fec->DWORD3.NA;
      }

      // Perte ?
      if (!keep)
      {
        PRINT0c (_fec->DWORD3.D == COL ? cMsgColPok : cMsgRowPok)
        sPaquetFec_Release (bufP[numP].paquet);
      }
      else
      {
        PRINT0c (_fec->DWORD3.D == COL ? cMsgColOk : cMsgRowOk)
        numP++;
      }
    }
    else
    {
      eof = true;
    }

    // Met à jour la barre de pourcentage
    sourcePos = ftell (source);
    PCENT ((double)sourcePos / (double)sourceSize, eof, cErrorsGeneratorLogFile)
  }

  // PASSE DE RÉARRANGEMENT ====================================================

  PRINT0_CONc    (cConDefault, cErrorsGeneratorMsg2of3)
  PRINT_SET_FILE (cErrorsGeneratorLogFile, "a")

  if (optionReorderProb > 0)
  {
    oldPcent = 0;

    unsigned dist = 4;

    for (no = 0; no < numP; no++)
    {
      // Réarrangement ?
      if (!sTewfiq_IsOkayOrLost (&_tewfiqReorder))
      {
        if (no + 2 * dist - 1 < numP)
        {
          sPaquets tmp[4];
          memcpy (tmp,         &bufP[no],   4 * sizeof (sPaquets));
          memcpy (&bufP[no],   &bufP[no+4], 4 * sizeof (sPaquets));
          memcpy (&bufP[no+4], tmp,         4 * sizeof (sPaquets));
        }
      }

      // Met à jour la barre de pourcentage
      PCENT ((double)(no+1) / (double)numP, no == numP-1,
             cErrorsGeneratorLogFile)
    }
  }

  // PASSE D'ENREGISTREMENT DES PAQUETS ========================================

  PRINT0_CONc    (cConDefault, cErrorsGeneratorMsg3of3)
  PRINT_SET_FILE (cErrorsGeneratorLogFile, "a")

  oldPcent = 0;

  for (no = 0; no < numP; no++)
  {
    if (bufP[no].media)
    {
      sPaquetMedia* media = bufP[no].paquet;
      sPaquetMedia_ToFile  (media, dest, true);
      sPaquetMedia_Release (media);
    }
    else
    {
      sPaquetFec* fec = bufP[no].paquet;
      sPaquetFec_ToFile  (fec, dest);
      sPaquetFec_Release (fec);
    }

    // Met à jour la barre de pourcentage
    PCENT ((double)(no+1) / (double)numP, no == numP-1,
           cErrorsGeneratorLogFile)
  }

  fclose (source);
  fclose (dest);

  sTewfiq_Print (&_tewfiqPerte);
  sTewfiq_Print (&_tewfiqReorder);

  // ===========================================================================

  PRINT0_CONc   (cConDefault, cMsgEnded)
  KeyToContinue (optionAutoKey);

  return 0;
}
