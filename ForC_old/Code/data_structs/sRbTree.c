/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Red-black tree
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

  Based on: http://en.literateprograms.org/
            Red-black_tree_(C)?action=history&offset=20080731190038
*/

#include "../smpte.h"

#include "sRbTree_helpers.h"

// Constantes privées ==========================================================

const sRbTree INIT_RB_TREE = //. Valeur initiale d'un arbre rouge-noire
  {0, 0, 0, 0, 0, 0, 0, 0, false};

// Fonctions publiques =========================================================

// Initialise un nouvel arbre rouge-noire --------------------------------------
//> Nouvel arbre rouge-noire
sRbTree sRbTree_New
  (sRbReleaseFunc _releaseFunc, //: Fonction de suppression d'un noeud
   sRbPrintNdFunc _printNdFunc) //: Fonction d'affichage d'un noeud
{
  sRbTree t     = INIT_RB_TREE;
  t.releaseFunc = _releaseFunc;
  t.printNdFunc = _printNdFunc;
  VerifyProperties (&t);
  return t;
}

// Libère la mémoire allouée par un arbre rouge-noire --------------------------
void sRbTree_Release
  (sRbTree* pTree) //: Arbre à vider
{
  ASSERTpc (pTree,, cExNullPtr)

  sRbTree_ReleaseHelper (pTree, pTree->root);

  *pTree = INIT_RB_TREE;
}

// Affiche le contenu d'un arbre rouge-noire -----------------------------------
void sRbTree_Print
  (const sRbTree* pTree,       //: Arbre à afficher
         unsigned pIndentStep, //: Pas d'indentation
         bool     pBuffers)    //: Faut-il afficher le contenu des buffers ?
{
  ASSERTpc (pTree,, cExNullPtr)

  sRbTree_PrintHelper (pTree, pTree->root, pIndentStep, pBuffers);
  PRINT1              ("\n")
}

// Retourne un pointeur sur le premier noeud de l'arbre rouge-noire. -----------
// L'arbre étant trié par key croissant                              -----------
//> Pointeur sur le premier noeud de l'arbre ou 0 si vide
sRbNode* sRbTree_First
  (const sRbTree* pTree) //: Arbre à traiter
{
  ASSERTpc (pTree, 0, cExNullPtr)

  sRbNode* _node = pTree->root;
  IFNOT   (_node, 0) // Pointeur null ?

  while (_node->left)
  {
    _node = _node->left;
  }

  return _node;
}

// Retourne un pointeur sur le dernier noeud de l'arbre rouge-noire. -----------
// L'arbre étant trié par key croissant                              -----------
//> Pointeur sur le dernier noeud de l'arbre ou 0 si vide
sRbNode* sRbTree_Last
  (const sRbTree* pTree) //: Arbre à traiter
{
  ASSERTpc (pTree, 0, cExNullPtr)

  sRbNode* _node = pTree->root;
  IFNOT   (_node, 0) // Pointeur null ?

  while (_node->right)
  {
    _node = _node->right;
  }

  return _node;
}

// Retourne un pointeur sur le prochain noeud (partant de pNode) de l'arbre ----
// rouge-noire. L'arbre étant trié par key croissant                        ----
//> Pointeur sur le prochain noeud de l'arbre ou 0 si pas de prochain
sRbNode* sRbNode_Next
  (const sRbNode* pNode) //: Noeud de départ
{
  ASSERTpc (pNode, 0, cExNullPtr)

  // Si le noeud courant a un enfant de droite, alors il faut retourner celui-ci
  // car c'est le prochain dans l'ordre croissant (dû au tri de l'arbre) !
  if (pNode->right)
  {
    sRbNode* _node = pNode->right;
    while   (_node->left) _node = _node->left;
    return   _node;
  }

  /* Sinon, il ne faut pas retourner l'enfant de gauche car celui-ci est plus
   * petit (dû au tri de l'arbre), mais alors, lesquel retourner ?
   *
   * L'arbre suivant illustre le résultat que nous devons atteindre.
   * Les noeuds minuscules représentent plusieurs points de départ et leur homo-
   * logue majuscule leurs successeurs respectifs :
   *
   *            D                * Généalogie relative à <noeud>
   *           / \               *
   *          /   \              *            grand-parent
   *         /     \             *             /
   *        /       \            *         parent
   *       /         \           *          /
   *      B           F          *       noeud (enfant de gauche du parent)
   *     / \         / \         *      /     \
   *    /   \       /   \        *  enfant   enfant
   *   A     C     E     G       *  gauche   droite
   *  / \   / \   / \   / \
   * a   b c   d e   f g   h   (H n'existe pas)
   *
   * Le successeur se trouvant parmi les ancêtres (parent, grand-parent, ...).
   * L'ancêtre le moins lointain dont l'enfant le liant au noeud en question est
   * un enfant de gauche. Par exemple, le successeur de d est D, car B, l'enfant
   * de D qui le lie à d, est sont enfant de gauche.
   * S'il faut remonter plus loin que l'ancêtre commun (racine), cela veut dire
   * qu'il n'y a plus de successeur.
   */

  sRbNode* x = (sRbNode*)pNode;         // X représente l'ancêtre enfant
  sRbNode* y = (sRbNode*)pNode->parent; // Y représente l'ancêtre parent

  /* Remonter dans l'abre tant que l'on est dans la configuration ci-dessous :
   *
   *   y
   *  / \
   * ?   x
   */

  while (y && (x == y->right)) // S'il y a un père / si c'est l'enfant de droite
  {
    // Remonte dans l'arbre généalogique
    x = y;
    y = y->parent;
  }

  /* Le successeur Y est le père, et X, la liaison, est un enfant de gauche :
   *
   *     y     <------ successeur
   *    / \
   *   x   ?
   *  / \
   * ?   ...   <------ chemin menant au noeud dont on cherche le successeur
   */
  return y;
}

// Retourne un pointeur sur le précédent noeud (partant de pNode) de l'arbre ---
// rouge-noire. L'arbre étant trié par key croissant                         ---
//> Pointeur sur le précédent noeud de l'arbre ou 0 si pas de précédent
sRbNode* sRbNode_Prev
  (const sRbNode* pNode) //: Noeud de départ
{
  ASSERTpc (pNode, 0, cExNullPtr)

  // Si le noeud courant a un enfant de gauche, alors il faut retourner celui-ci
  // car c'est le prochain dans l'ordre décroissant (dû au tri de l'arbre) !
  if (pNode->left)
  {
    sRbNode* _node = pNode->left;
    while   (_node->right) _node = _node->right;
    return   _node;
  }

  /* Sinon, il ne faut pas retourner l'enfant de droite car celui-ci est plus
   * grand (dû au tri de l'arbre), mais alors, lesquel retourner ?
   *
   * L'arbre suivant illustre le résultat que nous devons atteindre.
   * Les noeuds minuscules représentent plusieurs points de départ et leur homo-
   * logue majuscule leurs successeurs respectifs :
   *
   *            D                * Généalogie relative à <noeud>
   *           / \               *
   *          /   \              * grand-parent
   *         /     \             *          \
   *        /       \            *         parent
   *       /         \           *            \
   *      F           B          *           noeud (enfant de droite du parent)
   *     / \         / \         *          /     \
   *    /   \       /   \        *      enfant   enfant
   *   G     E     C     A       *      gauche   droite
   *  / \   / \   / \   / \
   * h   g f   e d   c b   a   (H n'existe pas)
   *
   * Le successeur se trouvant parmi les ancêtres (parent, grand-parent, ...).
   * L'ancêtre le moins lointain dont l'enfant le liant au noeud en question est
   * un enfant de droite. Par exemple, le successeur de d est D, car B, l'enfant
   * de D qui le lie à d, est sont enfant de droite.
   * S'il faut remonter plus loin que l'ancêtre commun (racine), cela veut dire
   * qu'il n'y a plus de successeur.
   */

  sRbNode* x = (sRbNode*)pNode;         // X représente l'ancêtre enfant
  sRbNode* y = (sRbNode*)pNode->parent; // Y représente l'ancêtre parent

  /* Remonter dans l'abre tant que l'on est dans la configuration ci-dessous :
   *
   *   y
   *  / \
   * x   ?
   */

  while (y && (x == y->left)) // s'il y a un pere / si c'est le fils gauche
  {
    // Remonte dans l'arbre généalogique
    x = y;
    y = y->parent;
  }

  /* Le successeur Y est le père, et X, la liaison, est un enfant de droite :
   *
   *     y     <------ successeur
   *    / \
   *   ?   x
   *      / \
   *   ...   ? <------ chemin menant au noeud dont on cherche le successeur
   */
  return y;
}

// Retourne value du noeud portant un certaine clé -----------------------------
//> Value du noeud lié à key=pKey ou 0 si aucun de trouvé
void* sRbTree_Lookup
  (const sRbTree* pTree, //: Arbre à traiter
   uint32_t       pKey)  //: Paramètre de recherche
{
  ASSERTpc (pTree, 0, cExNullPtr)

  sRbNode* _node = LookupNode (pTree, pKey);
  return   _node != 0 ? _node->value : 0;
}

// Créé et ajoute (en dernier) un nouvel élément à la liste chaînée ------------
//> Pointeur sur le nouvel élément ou 0 si problème
sRbNode* sRbTree_AddByReference
  (sRbTree* pTree,  //: Arbre à modifier
   uint32_t pKey,   //: Clé du nouveau noeud
   void*    pValue, //: Ce que nous voulons dans value du noeud
   bool     pReplaceNode) //: Ecraser le noeud si noeud key=pKey déjà présent ?
{
  ASSERTpc (pTree, 0, cExNullPtr)

  sRbNode* _insertedNode = NewNode (pKey, pValue, RED, NULL, NULL);
  IFNOT   (_insertedNode, 0) // Allocation ratée ?

  if (pTree->root == NULL)
  {
    pTree->root = _insertedNode;
  }
  else
  {
    sRbNode* _node = pTree->root;

    while (1)
    {
      if (pKey == _node->key)
      {
        if (pReplaceNode)
        {
          pTree->overCount++;

          if (pTree->releaseFunc)
          { // freeze memory
            pTree->releaseFunc (_node->key, _node->value);
          }
          _node->value = pValue;

          return _node;
        }

        return 0;
      }
      else if (pKey < _node->key)
      {
        if (_node->left == NULL)
        {
          _node->left = _insertedNode;
          break;
        }
        else _node = _node->left;
      }
      else
      {
        ASSERTc (pKey > _node->key, 0, cExUndefined)

        if (_node->right == NULL)
        {
          _node->right = _insertedNode;
          break;
        }
        else _node = _node->right;
      }
    }

    _insertedNode->parent = _node;
  }

  InsertCase1      (pTree, _insertedNode);
  VerifyProperties (pTree);

  pTree->count++;

  return _insertedNode;
}

// Supprime un élément de l'arbre rouge-noire ----------------------------------
//> Status de l'opération / suppression réussie ?
bool sRbTree_Delete
  (sRbTree* pTree, //: Arbre à modifier
  uint32_t  pKey)  //: Clé du noeud à supprimer
{
  ASSERTpc (pTree, false, cExNullPtr)

  sRbNode* _child;
  sRbNode* _node = LookupNode (pTree, pKey);
  IFNOT   (_node, false) // Clé introuvable ?

  if (pTree->releaseFunc)
  {
    pTree->releaseFunc (_node->key, _node->value);
  }

  if (_node->left != NULL && _node->right != NULL)
  {
    // Copy key/value from predecessor and then delete it instead
    sRbNode* pred = MaximumNode (_node->left);
    _node->key    = pred->key;
    _node->value  = pred->value;
    _node = pred;
  }

  ASSERTc (_node->left==NULL || _node->right==NULL, false, cExUndefined)

  _child = _node->right == NULL ? _node->left : _node->right;

  if (NodeColor (_node) == BLACK)
  {
    _node->color = NodeColor (_child);
    DeleteCase1 (pTree, _node);
  }

  ReplaceNode (pTree, _node, _child);

  if (_node->parent == NULL && _child != NULL)
  {
    _child->color = BLACK;
  }

  free             (_node);
  VerifyProperties (pTree);

  pTree->count--;

  return true;
}

// Initalise la boucle foreach like sur l'arbre rouge-noire --------------------
//> Status de l'opération / Est-ce que foreachNode est un noeud valide ?
bool sRbTree_InitForeach
  (sRbTree* pTree,    //: Arbre à traiter
   bool     pReverse) //: Faut-il effectuer la boucle à " l'envers " ?
{
  ASSERTpc (pTree, false, cExNullPtr)

  pTree->foreachReverse = pReverse;
  pTree->foreachNode    = pReverse ? sRbTree_Last  (pTree) :
                                     sRbTree_First (pTree);

  pTree->foreachCount   = 0;
  pTree->foreachDeleted = false;

  return pTree->foreachNode != 0;
}

// Continue la boucle foreach like sur l'arbre rouge-noire ---------------------
//> Status de l'opération / Est-ce que foreachNode est un noeud valide ?
bool sRbTree_NextForeach
  (sRbTree* pTree) //: Arbre à traiter
{
  ASSERTpc (pTree, false, cExNullPtr)

  if (pTree->foreachCount++ > 0)
  {
    // Un noeud vient d'être supprimé pendant la boucle : déjà avancé ...
    if (pTree->foreachDeleted)
    {
      pTree->foreachDeleted = false;
    }
    else
    {
      if (pTree->foreachNode != 0)
      {
        pTree->foreachNode = pTree->foreachReverse ?
                             sRbNode_Prev (pTree->foreachNode) :
                             sRbNode_Next (pTree->foreachNode);
      }
    }
  }

  return pTree->foreachNode != 0;
}

// Retourne key du noeud pointé par la boucle foreach like ---------------------
//> Key du noeud pointé par la boucle foreach like ou 0 si inactif
uint32_t sRbTree_ForeachKey
  (const sRbTree* pTree) //: Arbre à traiter
{
  ASSERTpc (pTree, false, cExNullPtr)

  if (pTree->foreachNode == 0) return 0;

  return pTree->foreachNode->key;
}

// Retourne value du noeud pointé par la boucle foreach like -------------------
//> Value du noeud pointé par la boucle foreach like ou 0 si inactif
void* sRbTree_ForeachValue
  (const sRbTree* pTree) //: Arbre à traiter
{
  ASSERTpc (pTree, false, cExNullPtr)

  if (pTree->foreachNode == 0) return 0;

  return pTree->foreachNode->value;
}

void* sRbNode_GetValue
  (const sRbNode* pNode) //: Noeud à traiter
{
  if (pNode == 0) return 0;

  return pNode->value;
}
