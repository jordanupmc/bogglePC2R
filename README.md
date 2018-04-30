# PC2R 2017

Projet PC2R - Jeu de Lettres

## Pour commencer

Ces instructions vont vous permettre de lancer correctement le projet, que ce soit le client, ou les différents serveur.

### Prérequis

* **GCC**
* **Python 2.3**
* **Java 8**

### Installation

Pour compiler le serveur de jeu, placez dans le répertoire serveur lancez la commande.

```
make
```

Pour lancer le serveur il faut d'abord lancer le serveur de mot
```
java DicoServer x
```

Enfin vous pouvez lancer le serveur de jeu
```
./bin/server -port|-tours|-immediat|-grilles 
```

* *-port numport : precisant le numero de port du serveur*  

* *-tours n : indiquant qu’il y aura n tours par session*

* *-grilles grille1 ... grillen : indiquant les n grilles de tests, chacune sous forme d’une chaîne de
caractères de 16 lettres.*

* *-immediat: le serveur vérifie qu’il est valable et l’indique au client. Si un autre joueur propose le même mot, celui est refusé
en indiquant une raison qui commence par PRI*

Vous pouvez lancer le serveur Web python, pour visualiser sur une page html les résultats de sessions.

Placez dans le répertoire journal lancer la commande

```
./server.py 
```

## Lancer les tests

Pour lancer les tests contenu dans le dossiers tests/
```

```

### Style des tests

## Build

Makefile 

## Auteur

* **Jeudy Jordan**
* **Ta Michael** 

