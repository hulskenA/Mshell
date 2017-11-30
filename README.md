#   Gestionnaire de travaux

Ce dépôt correspond au TP de PDS
« [mshell](http://www.fil.univ-lille1.fr/~hym/e/pds/tp/tdjobs.html) ».

Pour compiler le mshell tapez la commande `make` puis ensuite pour l'executer `./mshell`.

Quelques particularitées: La manière dont nous avons implementé notre `do_pipe` fait que par exemple
la commande `cat pipe.c | grep #include | wc -l` sera considerée comme **un seul processus** par le mshell!
De ce fait si on consulte la liste des jobs (avec la commande `jobs`). On voit s'afficher un **pid** pour ce
processus. Si on stop ou kill ce processus, tous les processus *fils* le seront également.
Pour illuster cela on peut taper la sequence de code suivante:

```bash
mshell> xeyes | xeyes &
mshell> jobs
[1] (**[PID]**) Running xeyes | xeyes
mshell> stop **[PID]**
mshell> jobs
[1] (**[PID]**) Stopped xeyes | xeyes
mshell> fg %1
^Z
mshell> jobs
[1] (**[PID]**) Stopped xeyes | xeyes
mshell> kill **[PID]**
mshell> jobs
mshell> exit
```

On remarque bien qu'en utilisant les commandes *stop* et *kill*, c'est à chaque fois les deux xeyes qui sont stoppé ou tué.