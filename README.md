# Marauder's Map Admin Scanner

## Description
Le Marauder's Map Admin Scanner est un outil conçu pour scanner les chemins admin courants sur un site web donné. Inspiré par le thème de Harry Potter, cet outil utilise des threads pour effectuer des requêtes HTTP en parallèle, augmentant ainsi la vitesse de l'analyse.

## Auteur
Créé par 0liv3vanders.

## Avertissement
**DISCLAIMER**: Utilisez cet outil de manière responsable et légale. L'auteur décline toute responsabilité en cas d'utilisation abusive.

## Utilisation
Pour exécuter le programme, compilez le fichier `admin_scanner.c` avec `gcc` et exécutez-le en fournissant une URL cible et un fichier de chemins.

```bash
gcc -o admin_scanner admin_scanner.c -lcurl
./admin_scanner <url> <path_file>
```

## Dépendances
- **libcurl**: Utilisé pour effectuer des requêtes HTTP.
- **pthread**: Utilisé pour l'exécution parallèle des requêtes.

## Remarques
Assurez-vous d'avoir les permissions nécessaires pour scanner l'URL cible afin d'éviter tout problème légal.
