# embeddebSys_RPI3
These codes provide all necessary, for get temperature , humidity and pressure whith the VMA335 sensor

# --------------But-----------------------
Fabriquer une station météo

# --------------Matériel------------------
- une carte Raspberry PI3 (RPI3 OU RPI) 
- une LED rouge
- une LED bleu
- un capteur VMA335 ou BME280
- une carte SD
- une breadboard

# --------------Préalable------------------

# Le docker pour la cross compilation

Télécharger l'image docker disponible ici : 

$ docker rmi pblottiere/embsys-rpi3-buildroot
$ docker pull pblottiere/embsys-rpi3-buildroot 

ensuite l'exécuter:
$ docker run -it pblottiere/embsys-rpi3-buildroot /bin/bash

décompresser le tarball :
$ tar zxvf buildroot-precompiled-2017.08.tar.gz
$ cd buildroot-precompiled-2017.08

et lancer 
$ make embsys_defconfig

c'est sur cette distribution docker que nous allons cross compiler notre code sachant que la cible est la RPI3

# Flashage de la RPI3

recupérer l'image de la carte SD se trouvant sur le conteneur :
$ docker cp <container_id>:/root/buildroot-precompiled-2017.08/output/images/sdcard.img .

Ensuite flasher une carte SD à votre disposition :
$ sudo dd if=sdcard.img of=/dev/sdX (remplacer X par le path de votre carte. dmesg peut vous aider)

Après celà, votre carte contient deux partitions (une de 210MB: c'est rtfs et une de 34 MB pour le kernel).


# -----------------montage électronique---------------

L'électronique est constitué d'une RPI3 sur laquelle nous avons connecté un capteur de température (VMA335 ou BME280),une LED rouge et une LED bleu
voir figure : bme280_pins.png pour connecter le capteur

# -------------Le code--------------------------------

Pour le code, nous sommes partis de l'API du capteur VMA335 ou BME280 et nous avons ajouté quelques fonctionnalités. 
Le code établie la liaison i2c et recupére les valeurs des registres de données qui contiennent la température, la pression et l'humidité.
Ces valeurs sont imprimées dans un terminal si la RPI est conecté à un terminal (gtkterm par exemple)

Dans le cas où la carte n'est pas connectée, nous avons établi un code de couleur.
Le montage comprend deux LED (rouge et bleu):
Losrque la LED rouge s'allume, celà signifie que le code a pu démarré sans aucun problème. Par contre si elle se met à clignoter, ceci veut dire qu'une exception a était levée.
Quant à la LED bleu elle clignote après que la LED rouge ait confirmée que tout a pu bien démarrer. chaque chaque clignotement correspond à une nouvelle mesure faite par le capteur de température, donc à de nouvelles données.

pour finir, notre code se lance  de façon autonome au branchement de la carte RPI3 (pas besoin de l'intervention de l'homme).
Ceci vient du fichier bash "S99zzzbme280_driver" qu'il faudrat insérer dans le dossier init.d du repertoire /etc/ de la carte RPI3.
En effet, le fichier "rcS" du même repertoire permet de lancer tous les fichiers "S??*" du repertoire /etc/init.d au démarrage de la RPI3. Ainsi en nommant notre bash "S99zzzbme280_driver", celui ci serra lancé au démarage de la carte. Il est le dernier fichier bash lancé au démarrage de la carte.

NB :  nous avons commencé à réfléchir à comment recupérer nos informations (température, pression, humidité) sur un serveur(domoticz ou page internet), mais le temps à notre disposition était court. Néanmoins nous avons réussi à écrire un serveur (dossier ./src/socket) qui permet de recupérer les valeurs de la température, l'humidité et la pression dans un navigateur web, via un ordinateur connecté sur le même réseau que la carte. 
Cependant nous avons pas eu le temps de lier ce code au reste du projet. 

# ------------------Un test-------------------------

Pour faire un test il vous faut préparer votre carte RPI3 (voir préalable ci dessus) et brancher l'électronique tel que montré: LED rouge GPIO24 et LED bleu GPIO17 (voir montage électronique ci dessous). Une fois fait :

- démarrer le docker téléchargé dans la partie préalable: $ sudo docker exec -it <container_id> bash
- Dans un autre terminal copiez le repertoire du projet à la racine du docker : $ sudo docker cp embeddebSys_RPI3/ <container_id>:/root/
- Revener dans le terminal où tourne votre docker et faite $ cd /root/ 
- Vous allez voir un nouveau dossier embeddebSys_RPI3.
- faite les commandes suivantes pour cross compiler:
$ cd /root/buildroot-precompiled-2017.08
$ ./output/host/usr/bin/arm-linux-gcc ../embeddebSys_RPI3/src/BME280_driver.c ../embeddebSys_RPI3/src/bme280.c -o BME_driver

- Une fois la cross compilation OK: ouvrer un terminal et copier l'exécutable BME_driver sur votre machine
- Ensuite l'excécutable doit être copié sur la carte SD de la RPI3 sur la partition de 210MB à l'emplacement /home/user/
- et le fichier bash "S99zzzbme280_driver" doit être copié dans sur la carte SD aussi mais cette fois à l'emplacement /etc/init.d/ de la partition de 210MB.
- Vous pouvez éjecter la carte SD de la RPI3 et l'introduire dans la RPI3

# ---------------------Résultats-------------------

- Après avoir inséré la carte SD dans la RPI3, allumez la carte. 
- Après 5 à 10secondes, la LED rouge s'allume signe que le code a pu être lancé sans erreur
- La LED bleu se met à clignoter chaque seconde , signe que les mesures se font parfaitement.
- pour vérifier les mésures faites, vous pouvez établir une connection série avec la RPI et ouvrir un terminal avec gtkterm par example.
- Les données s'afficheront alors dans le terminal.

Le lien vidéo de presentation du résultat final : https://youtu.be/u70xgFuJd9I
