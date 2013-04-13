
/******************************************************************************\
              ALGORITHME DE FEC SMPTE 2022-1 DAVID (OPTIMAL SELON MOI)

  Auteur    : David Fischer
  Intégration VLC : Rossier Jérémie (2011)
  Contact   : david.fischer.ch@gmail.com / david.fischer@hesge.ch
              jeremie.rossier@gmail.com

  Projet     : Implémentation SMPTE 2022 de VLC
  Date début : 02.05.2008
  Employeur  : Ecole d'Ingénieurs de Genève
               Laboratoire de Télécommunications
\******************************************************************************/

/* Copyright (c) 2009 David Fischer (david.fischer.ch@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from:
  https://sourceforge.net/projects/smpte2022lib/
*/

#ifndef __SDAVIDSMPTE__
#define __SDAVIDSMPTE__

// Types de données ============================================================

// Structure stockant ce qu'il faut pour l'algorithme de SMPTE 2022-1 optimisé ---
typedef struct sDavidSmpte
{
  sBufferMedia_t *media; //. Stockage des paquets de média
  sBufferFec_t   *fec;   //. Stockage des paquets de FEC et FEC <-> média

  bool resized_matrix; //savoir si la matrice a été redimentionnée
  mtime_t resized_matrix_check;
  unsigned recovered;            //. Nombre de paquets média récupérés
  unsigned unrecoveredOnReading; //. Nb paq. média manquants lors de la lecture !

  unsigned nbArPaMedia; //. Nombre d'appels à ArriveePaquetMedia
  unsigned nbArPaFec;   //. Nombre d'appels à ArriveePaquetFec
  unsigned nbLePaMedia; //. Nombre d'appels à LecturePaquetMedia
  unsigned nbPePaMedia; //. Nombre d'appels à PerduPaquetMedia
  unsigned nbRePaMedia; //. Nombre d'appels à RecupPaquetMedia

  unsigned maxC; //. Nombre max d'éléments stockés dans Cross
  unsigned maxW; //. Nombre max d'éléments stockés dans Wait

}sDavidSmpte_t;

// Déclaration des fonctions ===================================================

sDavidSmpte_t *sDavidSmpte_New ();
void sDavidSmpte_Release (sDavidSmpte_t *);

bool sDavidSmpte_ArriveePaquetMedia (demux_t *demux, sDavidSmpte_t*, sPaquetMedia*);
bool sDavidSmpte_ArriveePaquetMedia_Convert(demux_t *demux, sDavidSmpte_t*, block_t*);
void sDavidSmpte_ArriveePaquetFec   (demux_t *demux, sDavidSmpte_t*, sPaquetFec*);
void sDavidSmpte_ArriveePaquetFec_Convert(demux_t *demux, sDavidSmpte_t*, block_t*);
bool sDavidSmpte_LecturePaquetMedia (demux_t *demux, sDavidSmpte_t*, sMediaNo);

sCrossFec *sDavidSmpte_PerduPaquetMedia (demux_t *demux,sDavidSmpte_t*,sMediaNo,sWaitFec*);
void sDavidSmpte_RecupPaquetMedia       (demux_t *demux,sDavidSmpte_t*,sMediaNo,sCrossFec*,sWaitFec*);

#endif
