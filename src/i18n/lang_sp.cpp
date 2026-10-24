// lang_sp.cpp -- Mewgenics Multi : les textes du mod et du lanceur en espagnol (sp).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : le tutoiement espagnol, comme le tutoiement du francais du mod.
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_sp[];
const TrPair kTable_sp[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"No se ha podido leer mgmp.json: no se han guardado los ajustes."},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"mgmp.json contiene comentarios (// o /* */) que se borrarían al guardar: quítalos o cambia el ajuste a mano en el archivo."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.json no es un JSON válido: corrígelo (o vuelve a poner el del zip) antes de cambiar los ajustes."},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"No se puede escribir en la carpeta del mod: no se han guardado los ajustes."},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"No se ha podido reemplazar mgmp.json (¿está abierto en otro programa?): no se han guardado los ajustes."},
    {u8"Steam est ouvert.", u8"Steam está abierto."},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Sin él, el juego se cierra antes de que se cargue el mod. Abre Steam y espera a que se conecte."},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"Falta mgmp.json junto a Mewgenics\u00A0Multi.exe: guarda los ajustes para volver a crearlo."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.json no es un JSON válido: corrígelo o vuelve a poner el del zip."},
    {u8"Réglages lus dans mgmp.json.", u8"Ajustes leídos de mgmp.json."},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"No se encuentra Mewgenics.exe (%s): en Ajustes, haz clic en Examinar… y elige Mewgenics.exe."},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"Falta resources.gpak junto a Mewgenics.exe: verifica los archivos del juego en Steam."},
    {u8"Jeu trouvé : %s.", u8"Juego encontrado: %s."},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.",
     u8"steam_appid.txt: se comprobará cuando se encuentre el juego."},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Falta junto a Mewgenics.exe: sin él, Steam vuelve a lanzar el juego sin el mod."},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"El que está junto a Mewgenics.exe no contiene 686060: Steam volvería a lanzar el juego sin el mod."},
    {u8"steam_appid.txt est en place à côté du jeu.",
     u8"steam_appid.txt está en su sitio junto al juego."},
    {u8"Fichiers du mod présents.", u8"Archivos del mod presentes."},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Faltan mgmp.dll y mgmp_loader.exe junto a Mewgenics\u00A0Multi.exe: vuelve a extraer toda la carpeta del zip."},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Falta %s junto a Mewgenics\u00A0Multi.exe: vuelve a extraer toda la carpeta del zip."},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi, build %s, protocolo %u: todos deben tener el mismo."},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"No se encuentra el juego: elige primero Mewgenics.exe en Ajustes."},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"No se ha podido escribir steam_appid.txt junto al juego: cópialo a mano (LISEZ-MOI, paso 2)."},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"No le has dado permiso a Windows: steam_appid.txt no se ha copiado. Puedes volver a intentarlo o copiarlo a mano (LISEZ-MOI, paso 2)."},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics Multi no ha podido reiniciarse como administrador: copia steam_appid.txt a mano (LISEZ-MOI, paso 2)."},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"La copia como administrador no ha funcionado: copia steam_appid.txt a mano (LISEZ-MOI, paso 2)."},
    {u8"Le mod n'a pas pu être chargé : %s", u8"No se ha podido cargar el mod: %s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exe se ha detenido sin cargar el mod. Vuelve a lanzarlo; si se repite, envía loader.txt."},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exe se ha detenido (código %s) sin cargar el mod. Causa más probable: Steam cerrado o desconectado. Abre Steam, espera a que se conecte y vuelve a lanzarlo."},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"El juego se ha cerrado al arrancar, antes de cargar el mod. Causa más frecuente: Steam cerrado o desconectado. Abre Steam, espera a que se conecte y vuelve a lanzarlo."},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"Steam vuelve a lanzar el juego (puede tardar un minuto o dos)…"},
    {u8"Ouverture du jeu…", u8"Abriendo el juego…"},
    {u8"Le jeu démarre…", u8"El juego arranca…"},
    {u8"Injection du mod dans le jeu…", u8"Inyectando el mod en el juego…"},
    {u8"Le mod est en place, le jeu reprend…", u8"El mod está listo, el juego continúa…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Esperando aún a que Steam relance el juego… Si Steam acaba de abrirse, quizá te esté haciendo una pregunta."},
    {u8"Jeu relancé trouvé : injection du mod…", u8"Juego relanzado encontrado: inyectando el mod…"},
    {u8"Le jeu est lancé avec le mod.", u8"El juego se ha iniciado con el mod."},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.",
     u8"El mod ha tardado en activarse; el juego continúa de todos modos."},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.",
     u8"El juego relanzado tarda en arrancar; se inyecta de todos modos."},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"No se encuentra mgmp.dll junto a Mewgenics\u00A0Multi.exe: vuelve a extraer la carpeta del mod desde el zip."},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"No se encuentra Mewgenics.exe: en Ajustes, haz clic en Examinar… y elige Mewgenics.exe."},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"No se puede leer mgmp.json: corrígelo o vuelve a poner el del zip."},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steam ha relanzado el juego sin el mod: comprueba steam_appid.txt junto a Mewgenics.exe (botón Reparar) y vuelve a lanzarlo."},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"Este juego no es la versión para la que está hecho el mod (1.1.21239): ¿se ha actualizado?"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"Un programa (overlay, antivirus, antitrampas) ha impedido que se cargue el mod."},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"El juego ha tardado demasiado en arrancar: el mod no se ha cargado. Vuelve a lanzarlo."},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"La inyección del mod ha fallado y el juego se ha cerrado. Vuelve a lanzarlo; si se repite, envía loader.txt."},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"mgmp.json no indica dónde está el juego: en Ajustes, haz clic en Examinar… y elige Mewgenics.exe."},
    {u8"build %s · protocole %u", u8"build %s · protocolo %u"},
    {u8"Steam ouvert", u8"Steam abierto"},
    {u8"Steam fermé", u8"Steam cerrado"},
    {u8"Réglages (mgmp.json)", u8"Ajustes (mgmp.json)"},
    {u8"Jeu trouvé", u8"Juego encontrado"},
    {u8"Jeu incomplet", u8"Juego incompleto"},
    {u8"Jeu introuvable", u8"Juego no encontrado"},
    {u8"Fichiers du mod", u8"Archivos del mod"},
    {u8"Réparer", u8"Reparar"},
    {u8"Jouer", u8"Jugar"},
    {u8"Règle d'abord les croix rouges.", u8"Resuelve primero las cruces rojas."},
    {u8"Réglages", u8"Ajustes"},
    {u8"Journaux", u8"Registros"},
    {u8"Langue", u8"Idioma"},
    {u8"Le jeu (Mewgenics.exe)", u8"El juego (Mewgenics.exe)"},
    {u8"Parcourir…", u8"Examinar…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe y resources.gpak están aquí."},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"Falta resources.gpak al lado: no es la carpeta correcta."},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.",
     u8"No hay ningún Mewgenics.exe aquí: elígelo con Examinar."},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Jugar juntos por Steam (lista de amigos)"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Desmárcalo si Steam da problemas: solo quedará la dirección, en el menú Multijugador del juego."},
    {u8"Enregistrer", u8"Guardar"},
    {u8"Retour", u8"Atrás"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"Aún no hay registros: el mod escribe uno en cada partida."},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"1 registro del mod, junto a Mewgenics\u00A0Multi.exe."},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"%d registros del mod, junto a Mewgenics\u00A0Multi.exe."},
    {u8"Le dernier : %s", u8"El último: %s"},
    {u8"écrit le %s", u8"escrito el %s"},
    {u8"Ouvrir le dossier", u8"Abrir carpeta"},
    {u8"Copier le dernier journal", u8"Copiar último registro"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"Para pedir ayuda: copia el último registro y pégalo en Discord con Ctrl+V."},
    {u8"Lancement", u8"Lanzamiento"},
    {u8"Le jeu se lance avec le mod…", u8"El juego se inicia con el mod…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.",
     u8"¡Buena partida! Esta ventana se cierra sola."},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"Los detalles están en loader.txt, junto a Mewgenics\u00A0Multi.exe."},
    {u8"Réessayer", u8"Reintentar"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"Si Steam relanza el juego, puede tardar un minuto o dos."},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.",
     u8"Registro copiado: pégalo en Discord con Ctrl+V."},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"No se ha podido copiar al portapapeles: vuelve a intentarlo o abre la carpeta."},
    {u8"Où est Mewgenics.exe ?", u8"¿Dónde está Mewgenics.exe?"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"Direct2D no está disponible en este PC: Mewgenics Multi no puede mostrarse."},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"No se ha podido preparar la tarjeta gráfica: Mewgenics Multi no puede mostrarse."},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"Cada uno con su casa y sus gatos,\n"
     u8"¡la aventura, juntos!"},
    {u8"(illustrations du jeu introuvables)", u8"(ilustraciones del juego no encontradas)"},
    {u8"Comme le jeu (%s)", u8"Como el juego (%s)"},
    {u8"Comme Windows (%s)", u8"Como Windows (%s)"},
    {u8"Automatique (%s)", u8"Automático (%s)"},
    {u8"Lancement annulé.", u8"Lanzamiento cancelado."},
    {u8"Lancement du mod…", u8"Lanzando el mod…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windows no ha podido preparar el lanzamiento. Vuelve a abrir Mewgenics Multi."},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exe no ha podido arrancar: vuelve a extraer la carpeta del mod desde el zip (o tu antivirus lo ha puesto en cuarentena)."},
    {u8"la connexion Steam avec l'hôte est coupée",
     u8"se ha cortado la conexión de Steam con el anfitrión"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"el anfitrión no responde por Steam (¿sala cerrada o no tiene el mod?)"},
    {u8"Steam refuse la connexion", u8"Steam rechaza la conexión"},
    {u8"À %s de choisir", u8"Le toca elegir a %s"},
    {u8"À toi de choisir", u8"Te toca elegir"},
    {u8"À %s de choisir la capacité à remplacer.", u8"Le toca a %s elegir la habilidad que sustituir."},
    {u8"Ce chat est à %s", u8"Este gato es de %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"Selección de collares\n"
     u8"\n"
     u8"La elección compartida se ha detenido (error del juego):\n"
     u8"elige los collares para todo el grupo."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"Selección de collares\n"
     u8"\n"
     u8"¡Listo! %s sabe que has terminado:\n"
     u8"confirmará la salida para todo el grupo."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"Selección de collares\n"
     u8"\n"
     u8"%s es quien confirma la salida:\n"
     u8"avísale cuando tus gatos tengan sus collares."},
    {u8"%s et %s", u8"%s y %s"},
    {u8"%s, %s", u8"%s, %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Selección de collares\n"
     u8"\n"
     u8"%s aún están eligiendo los collares de sus gatos.\n"
     u8"Haz clic otra vez en «Asignar» para partir\n"
     u8"de todos modos."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Selección de collares\n"
     u8"\n"
     u8"%s aún está eligiendo los collares de sus gatos.\n"
     u8"Haz clic otra vez en «Asignar» para partir\n"
     u8"de todos modos."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"La salida de %s ya no se muestra aquí.\n"
     u8"Termina tú estas pantallas: te unirás a él\n"
     u8"en el mapa."},
    {u8"  %s : parti", u8"  %s: se ha ido"},
    {u8"  %s : prêt !", u8"  %s: ¡listo!"},
    {u8"  %s : charge ta partie...", u8"  %s: cargando tu partida..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Tu grupo está cargando tu partida para salir contigo:\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"Selección de objetos\n"
     u8"\n"
     u8"No se ha podido cargar tu almacén aquí:\n"
     u8"tus gatos parten con los objetos que llevan."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Selección de objetos\n"
     u8"\n"
     u8"%s aún están equipando a sus gatos.\n"
     u8"Haz clic otra vez en «Asignar» para partir\n"
     u8"de todos modos."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Selección de objetos\n"
     u8"\n"
     u8"%s aún está equipando a sus gatos.\n"
     u8"Haz clic otra vez en «Asignar» para partir\n"
     u8"de todos modos."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"Selección de objetos\n"
     u8"\n"
     u8"¡Listo! Tus gatos parten con estos objetos:\n"
     u8"%s confirmará la salida para todo el grupo."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"Selección de objetos\n"
     u8"\n"
     u8"Los objetos elegidos para tus gatos no se han\n"
     u8"podido llevar a la aventura de %s: tus gatos\n"
     u8"parten con los que tenían y tu almacén\n"
     u8"no cambia."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"Selección de objetos\n"
     u8"\n"
     u8"%s ya ha confirmado la salida: tus gatos parten\n"
     u8"con los objetos elegidos hasta ahora; lo demás\n"
     u8"ya no cuenta."},
    {u8"Ta bourse : %s.", u8"Tu bolsa: %s."},
    {u8"%s n'est plus dans la partie.", u8"%s ya no está en la partida."},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"No tienes bolsa en esta aventura (no has traído ningún gato)."},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"Los intercambios funcionan en tu casa, o en una aventura multijugador con tu bolsa. Por ahora: %s."},
    {u8"pas de partie à plusieurs en cours", u8"no hay partida multijugador en curso"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés",
     u8"falta una función del juego: no hay intercambios en casa"},
    {u8"tu es dans la partie de l'hôte", u8"estás en la partida del anfitrión"},
    {u8"tu n'es pas dans ta maison", u8"no estás en tu casa"},
    {u8"ta save n'est pas chargée", u8"tu partida guardada no está cargada"},
    {u8"un départ en aventure est en cours", u8"se está preparando una aventura"},
    {u8"ta maison est attaquée", u8"tu casa está bajo ataque"},
    {u8"ta maison est illisible", u8"no se puede leer tu casa"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"tu almacén espera lo que tus gatos traigan de la aventura del anfitrión (si ya han vuelto, recarga tu partida guardada: Esc, «Guardar y salir al menú», luego «Jugar» y tu partida)"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"algunos objetos de tu almacén aún esperan a su nuevo dueño (botín, compras, PNJ)"},
    {u8"Pas d'échange maintenant : %s.", u8"No puedes intercambiar ahora: %s."},
    {u8"Ton don précédent attend encore sa réponse.", u8"Tu regalo anterior aún espera respuesta."},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"El mod aún no conoce tu partida guardada: recárgala desde el menú."},
    {u8"Choisis un joueur de la partie.", u8"Elige a un jugador de la partida."},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.",
     u8"No puedes intercambiar ahora: %s no está en su casa."},
    {u8"Ce montant n'est pas possible.", u8"Esa cantidad no es válida."},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.",
     u8"Elige monedas, comida u objetos para dar."},
    {u8"16 objets au plus par don.", u8"16 objetos como máximo por regalo."},
    {u8"Pas d'objets maintenant : %s.", u8"Ahora no puedes dar objetos: %s."},
    {u8"Ta maison est illisible : pas de don.", u8"Error al leer tu casa: no hay regalo."},
    {u8"Tu n'as pas autant de pièces.", u8"No tienes tantas monedas."},
    {u8"Tu n'as pas autant de nourriture.", u8"No tienes tanta comida."},
    {u8"Ton rangement est illisible : pas de don.", u8"Error al leer tu almacén: no hay regalo."},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"Un objeto elegido ya está reservado para un regalo sin terminar."},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"Un objeto elegido ya no está en tu almacén: vuelve a elegir."},
    {u8"Un objet choisi ne peut pas se donner.", u8"Un objeto elegido no se puede regalar."},
    {u8"Le don n'a pas pu partir (connexion).", u8"El regalo no se ha enviado (conexión)."},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…",
     u8"Regalo enviado a %s (%s): esperando su respuesta…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…",
     u8"Regalo solicitado al anfitrión: esperando su respuesta…"},
    {u8"Pas de don : %s.", u8"No puedes regalar: %s."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%s ha recibido tu regalo: tus objetos saldrán de tu almacén en cuanto se cierre la pantalla de objetos."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%s ha recibido tu regalo: tus objetos saldrán de tu almacén en cuanto sea posible (tras la salida, la defensa de tu casa o el regreso de tus gatos)."},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%s ha recibido tu regalo: lo que has dado saldrá de tu casa cuando vuelvas a esta partida guardada."},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"%s no ha respondido a tu regalo: no ha salido nada de tu casa."},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"%s no ha respondido a tu regalo: no ha salido nada de tu casa."},
    {u8"Les échanges marchent pendant une partie à plusieurs.",
     u8"Los intercambios funcionan durante una partida multijugador."},
    {u8"Dans ta maison : %s.", u8"En tu casa: %s."},
    {u8"%s n'est pas dans sa maison.", u8"%s no está en su casa."},
    {u8"%lld pièces", u8"%lld monedas"},
    {u8"%lld pièce", u8"%lld moneda"},
    {u8"%lld nourritures", u8"%lld de comida"},
    {u8"%lld nourriture", u8"%lld de comida"},
    {u8"rien", u8"nada"},
    {u8"%s n'est pas dans sa maison", u8"%s no está en su casa"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s",
     u8"%s tiene abierta una ventana o la pantalla de objetos"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"el juego de %s no pudo guardarlos"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"el juego de %s no pudo guardarlos"},
    {u8"les dons ne marchent pas chez %s", u8"los regalos no funcionan con %s"},
    {u8"%s n'est plus dans la partie", u8"%s ya no está en la partida"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%s aún tiene gatos en la aventura del anfitrión (su almacén espera su regreso)"},
    {u8"raison inconnue", u8"motivo desconocido"},
    {u8"Cadeau d'%s", u8"Regalo de %s"},
    {u8"Cadeau de %s", u8"Regalo de %s"},
    {u8"%s t'a donné %s.", u8"%s te ha dado %s."},
    {u8"C'est dans ta maison, et ta save est à jour.",
     u8"Ya está en tu casa y tu partida guardada está al día."},
    {u8"C'est dans ta maison.", u8"Ya está en tu casa."},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"Al final, %s ha recibido %s: ya ha salido de tu casa."},
    {u8"%s a bien reçu %s : c'est parti de ta maison.", u8"%s ha recibido %s: ya ha salido de tu casa."},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"(1 objeto no ha podido salir de tu almacén: también sigue en tu casa)."},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"(%u objetos no han podido salir de tu almacén: también siguen en tu casa)."},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"El regalo a %s no se ha completado: %s. No ha salido nada de tu casa."},
    {u8"Rien à donner dans ton rangement.", u8"No hay nada que dar en tu almacén."},
    {u8"Tape des lettres pour chercher un objet.", u8"Escribe para buscar un objeto."},
    {u8"Recherche : %s_", u8"Búsqueda: %s_"},
    {u8"Aucun objet ne correspond.", u8"Ningún objeto coincide."},
    {u8"(page %u sur %u)", u8"(página %u de %u)"},
    {u8"Butin : %s.", u8"Botín: %s."},
    {u8"Besoin", u8"Necesidad"},
    {u8"Cupidité", u8"Codicia"},
    {u8"Passer", u8"Pasar"},
    {u8"%u s", u8"%u s"},
    {u8"toi", u8"tú"},
    {u8"Joueur %u", u8"Jugador %u"},
    {u8"Un objet", u8"Un objeto"},
    {u8"Partage du butin : objet %u sur %u", u8"Reparto del botín: objeto %u de %u"},
    {u8"Besoin : %s", u8"Necesidad: %s"},
    {u8"Cupidité : %s", u8"Codicia: %s"},
    {u8"Passer : %s", u8"Pasar: %s"},
    {u8"Partage du butin", u8"Reparto del botín"},
    {u8"%u objets du sac à partager entre %s.", u8"Se reparten %u objetos entre %s."},
    {u8"%u objet du sac à partager entre %s.", u8"Se reparte %u objeto entre %s."},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"Para cada objeto: Necesidad (te sirve), Codicia (si\n"
     u8"nadie lo necesita) o Pasar. Un dado de 100 caras desempata."},
    {u8"En attente de %s...", u8"Esperando a %s..."},
    {u8"En attente de %s... (%s)", u8"Esperando a %s... (%s)"},
    {u8"Tout le monde est là : ça commence !", u8"Ya está todo el mundo: ¡empezamos!"},
    {u8"Commencer", u8"Empezar"},
    {u8"D'accord", u8"Vale"},
    {u8"Le veux-tu ? (%s)", u8"¿Lo quieres? (%s)"},
    {u8"Je le veux", u8"Lo quiero"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"Necesidad: te sirve, tienes prioridad sobre los demás.\n"
     u8"Codicia: te lo llevas si nadie lo necesita.\n"
     u8"(%s)"},
    {u8"Ton choix : %s.", u8"Tu elección: %s."},
    {u8"Ton choix est fait.", u8"Ya has elegido."},
    {u8"En attente de %s (%s).", u8"Esperando a %s (%s)."},
    {u8"Le résultat arrive...", u8"Llega el resultado..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s no están: pierden su turno."},
    {u8"%s n'est pas là : son tour passe.", u8"%s no está: pierde su turno."},
    {u8"Personne n'en veut : il reste chez toi.", u8"Nadie lo quiere: se queda contigo."},
    {u8"Personne n'en veut : il reste chez %s.", u8"Nadie lo quiere: se queda con %s."},
    {u8"Tu l'emportes (%s) !", u8"¡Te lo llevas (%s)!"},
    {u8"%s l'emporte (%s).", u8"%s se lo lleva (%s)."},
    {u8"Dés : %s (égalité départagée).", u8"Dados: %s (empate resuelto)."},
    {u8"Dés : %s.", u8"Dados: %s."},
    {u8"Partage du butin terminé", u8"Reparto del botín terminado"},
    {u8"Tu remportes : %s.", u8"Te llevas: %s."},
    {u8"%s remporte : %s.", u8"%s se lleva: %s."},
    {u8"Remporté, mais resté chez %s : %s.", u8"Ganado, pero sigue con %s: %s."},
    {u8"Personne n'en a voulu (restent chez toi) : %s.",
     u8"Nadie los ha querido (se quedan contigo): %s."},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"Nadie los ha querido (se quedan con %s): %s."},
    {u8"Aucun objet n'a été partagé.", u8"No se ha repartido ningún objeto."},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"Tu partida guardada ya está abierta: la siguiente\n"
     u8"ventana te dice cómo recargarla con tu botín."},
    {u8"Ton butin t'attend dans ton rangement, chez toi.",
     u8"Tu botín te espera en tu almacén, en tu casa."},
    {u8"Ce que les autres remportent quitte ton rangement.",
     u8"Lo que se llevan los demás sale de tu almacén."},
    {u8"Dr Beanies", u8"Dr. Beanies"},
    {u8"Butch", u8"Butch"},
    {u8"Tink", u8"Tink"},
    {u8"Frank", u8"Frank"},
    {u8"Bébé Jack", u8"Bebé Jack"},
    {u8"Tracy", u8"Tracy"},
    {u8"Broyeur d'organe", u8"Pelagatos"},
    {u8"Steven", u8"Steven"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"Recompensa lista\n"
     u8"\n"
     u8"Gracias al gato de %s, %s tiene\n"
     u8"el medidor lleno. La recompensa se\n"
     u8"recogerá sola cuando estés en casa:\n"
     u8"monedas, comida y objetos, para %s;\n"
     u8"desbloqueos, para todos."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"%s: recompensa\n"
     u8"\n"
     u8"La recompensa del gato de %s no se\n"
     u8"ha podido enviar: se queda contigo."},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"Recompensa para %s\n"
     u8"\n"
     u8"%s ha recompensado al gato que %s\n"
     u8"le ha dado y le manda a casa: %s.\n"
     u8"Los desbloqueos son para todos."},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Recompensa para todos\n"
     u8"\n"
     u8"%s ha recompensado al gato que %s\n"
     u8"le ha dado: sus desbloqueos son para\n"
     u8"todos."},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s te agradece tu gato: %s\n"
     u8"(sus desbloqueos son para todos)."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"%s: recompensa\n"
     u8"\n"
     u8"%s te agradece tu gato: %s.\n"
     u8"Lo recibirás en casa la próxima vez\n"
     u8"que cargues tu partida (al volver de\n"
     u8"la aventura). Sus desbloqueos son\n"
     u8"para todos."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"%s: recompensa\n"
     u8"\n"
     u8"Con tu gato, %s tiene el medidor\n"
     u8"lleno. Su recompensa (desbloqueos)\n"
     u8"es para todos y llegará a tu partida\n"
     u8"la próxima vez que la cargues."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"%s: recompensa\n"
     u8"\n"
     u8"%s ya no está: %s te da a ti la\n"
     u8"recompensa que se había ganado."},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"Recompensa lista\n"
     u8"\n"
     u8"¡%s tiene el medidor lleno por ti!\n"
     u8"%s recogerá tu recompensa\n"
     u8"en cuanto esté en su casa."},
    {u8"les bourses ne marchent pas dans cette partie", u8"las bolsas no funcionan en esta partida"},
    {u8"rien à donner", u8"nada que dar"},
    {u8"c'est trop d'un coup", u8"es demasiado de una vez"},
    {u8"choisis un autre joueur", u8"elige a otro jugador"},
    {u8"ton don précédent attend encore la réponse de l'hôte",
     u8"tu regalo anterior aún espera la respuesta del anfitrión"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"no tienes bolsa en esta aventura"},
    {u8"envoi impossible (connexion)", u8"no se puede enviar (conexión)"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"Fin de la aventura\n"
     u8"\n"
     u8"Cada uno se lleva su bolsa a su casa:"},
    {u8"%s n'est plus là : sa bourse reste chez toi.", u8"%s ya no está: su bolsa se queda contigo."},
    {u8"%s : rien.", u8"%s: nada."},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.",
     u8"%s no ha podido recibir su bolsa: se queda contigo."},
    {u8"%s : %s.", u8"%s: %s."},
    {u8"Toi : %s.", u8"Tú: %s."},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"El anfitrión aún no ha respondido a tu regalo: mira tu bolsa en la ventana de elección del camino."},
    {u8"Ton don n'est pas passé : %s.", u8"Tu regalo no se ha completado: %s."},
    {u8"(et d'autres trouvailles)", u8"(y otros hallazgos)"},
    {u8"pas de bourse pour ce joueur dans cette aventure",
     u8"este jugador no tiene bolsa en esta aventura"},
    {u8"ta bourse n'en a pas autant", u8"tu bolsa no tiene tanto"},
    {u8"l'aventure n'est plus suivie", u8"ya no se sigue la aventura"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s te ha dado %s: va a tu bolsa."},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"Has dado %s a %s: va a su bolsa."},
    {u8"%s a donné %s à %s.", u8"%s ha dado %s a %s."},
    {u8"Un chat", u8"Un gato"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%s ha encontrado %s y perdido %s: cuenta para tu bolsa."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s ha encontrado %s y perdido %s: cuenta para la bolsa de %s."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s ha encontrado %s y perdido %s: cuenta para la bolsa de %s."},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s ha encontrado %s: va a tu bolsa."},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s ha encontrado %s: va a la bolsa de %s."},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s ha encontrado %s: va a la bolsa de %s."},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s ha gastado %s: sale de tu bolsa."},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.", u8"%s ha gastado %s: sale de la bolsa de %s."},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.", u8"%s ha gastado %s: sale de la bolsa de %s."},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s ha perdido %s: sale de tu bolsa."},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%s ha perdido %s: sale de la bolsa de %s."},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%s ha perdido %s: sale de la bolsa de %s."},
    {u8"%s n'a rien trouvé.", u8"%s no ha encontrado nada."},
    {u8"un objet", u8"un objeto"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.",
     u8"Has comprado %s por %s: sale de tu bolsa."},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.",
     u8"%s ha comprado %s por %s: sale de su bolsa."},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"Has comprado %s: %s más para tu bolsa."},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%s ha comprado %s: %s más para su bolsa."},
    {u8"Tu as acheté %s (gratuit).", u8"Has comprado %s (gratis)."},
    {u8"%s a acheté %s (gratuit).", u8"%s ha comprado %s (gratis)."},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"En combate, tus gatos han ganado %s y perdido %s: cuenta para tu bolsa."},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"En combate, tus gatos han encontrado %s: va a tu bolsa."},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"En combate, tus gatos han perdido %s: sale de tu bolsa."},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"Combate: tus gatos no ganaron nada."},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"En combate, los gatos de %s han encontrado %s y perdido %s: cuenta para su bolsa."},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"En combate, los gatos de %s han encontrado %s y perdido %s: cuenta para su bolsa."},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"En combate, los gatos de %s han encontrado %s: va a su bolsa."},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"En combate, los gatos de %s han encontrado %s: va a su bolsa."},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"En combate, los gatos de %s han perdido %s: sale de su bolsa."},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"En combate, los gatos de %s han perdido %s: sale de su bolsa."},
    {u8"En combat, les chats d'%s n'ont rien trouvé.",
     u8"En combate, los gatos de %s no han encontrado nada."},
    {u8"En combat, les chats de %s n'ont rien trouvé.",
     u8"En combate, los gatos de %s no han encontrado nada."},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"Tu mochila está abierta\n"
     u8"\n"
     u8"El anfitrión ha entrado en una casilla:\n"
     u8"cierra tu mochila para seguirlo."},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"Objetos\n"
     u8"\n"
     u8"Este gato es de %s: solo su dueño\n"
     u8"puede cambiar sus objetos."},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"Objetos\n"
     u8"\n"
     u8"En una casilla (tienda, subida de nivel...),\n"
     u8"solo el anfitrión cambia los objetos.\n"
     u8"Equipa a tus gatos en el mapa."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"Objetos\n"
     u8"\n"
     u8"El anfitrión aún no ha vuelto al mapa:\n"
     u8"espéralo, tu mochila se desbloqueará sola."},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"Objetos\n"
     u8"\n"
     u8"Tu mochila se actualiza con la del anfitrión:\n"
     u8"ciérrala y vuelve a abrirla en un momento."},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"Objetos\n"
     u8"\n"
     u8"Otro jugador ha cogido el objeto justo\n"
     u8"antes que tú: tu cambio no cuenta.\n"
     u8"Tu mochila se actualizará al cerrarla."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"Objetos\n"
     u8"\n"
     u8"El anfitrión ha entrado en una casilla antes de que\n"
     u8"llegara tu último cambio: se ha anulado.\n"
     u8"Vuelve a hacerlo en el mapa."},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"Objetos\n"
     u8"\n"
     u8"Este objeto ya no está: otro jugador\n"
     u8"lo ha cogido justo antes que tú."},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).",
     u8"%s ya no está: sus compras se quedan contigo (%s)."},
    {u8"%s rapporte ses achats : %s.", u8"%s se lleva sus compras: %s."},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Las compras de %s no se le han podido enviar: se quedan contigo."},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Las compras de %s no se le han podido enviar: se quedan contigo."},
    {u8"Achats de l'aventure", u8"Compras de la aventura"},
    {u8"Boutique", u8"Tienda"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s no han terminado en esta\n"
     u8"tienda: espéralos (una vez fuera, ya no\n"
     u8"podrías comprar por ellos).\n"
     u8"Haz clic otra vez para salir de todos modos."},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s no ha terminado en esta\n"
     u8"tienda: espéralo (una vez fuera, ya no\n"
     u8"podrías comprar por él).\n"
     u8"Haz clic otra vez para salir de todos modos."},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"No se ha podido pedir tu compra al anfitrión:\n"
     u8"no se ha comprado nada. Inténtalo de nuevo en un momento."},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"El anfitrión ya no está en esta tienda:\n"
     u8"la compra no se ha hecho."},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"El anfitrión ya no tiene este objeto a la venta:\n"
     u8"la compra no se ha hecho."},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Este objeto se acaba de vender:\n"
     u8"la compra no se ha hecho."},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"Cuesta %s; tu bolsa tiene %lld."},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"Tu bolsa no alcanza para este objeto."},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"(Cada uno paga con su bolsa, tú también)."},
    {u8"(Chacun paie avec sa bourse.)", u8"(Cada uno paga con su bolsa)."},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"Por ahora, solo el anfitrión compra los\n"
     u8"caramelos y los familiares (con su bolsa)."},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.",
     u8"El juego del anfitrión no ha podido hacer la compra."},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"No hay bolsas en esta aventura:\n"
     u8"solo compra el anfitrión."},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"No has traído gatos a esta\n"
     u8"aventura: no tienes bolsa para\n"
     u8"comprar."},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"No tienes bolsa en esta aventura\n"
     u8"(no has traído gatos): no puedes comprar."},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"El anfitrión no puede comprar por ti en este\n"
     u8"momento (tiene una ventana abierta):\n"
     u8"no se ha comprado nada. Inténtalo de nuevo en un momento."},
    {u8"un bonbon", u8"un caramelo"},
    {u8"un familier", u8"un familiar"},
    {u8"un article", u8"un artículo"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"Fin de la aventura\n"
     u8"\n"
     u8"%s vuelven a casa de %s."},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"Fin de la aventura\n"
     u8"\n"
     u8"%s vuelve a casa de %s."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Los gatos de los demás jugadores no han podido partir\n"
     u8"y tu caja de aventura está vacía: esta vez no hay\n"
     u8"aventura. (El registro del mod, mgmp.log, explica por qué)."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Tu caja de aventura está vacía y\n"
     u8"nadie trae gatos: esta vez no hay aventura."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Grupo mixto\n"
     u8"\n"
     u8"Los gatos de los demás jugadores no han podido\n"
     u8"unirse al grupo: partes con los tuyos.\n"
     u8"(El registro del mod, mgmp.log, explica por qué)."},
    {u8"Retour de l'aventure", u8"Regreso de la aventura"},
    {u8"%s est de retour à la maison.", u8"%s ha vuelto a casa."},
    {u8"%s sont de retour à la maison.", u8"%s han vuelto a casa."},
    {u8"%s a péri pendant l'aventure.", u8"%s ha muerto durante la aventura."},
    {u8"%s ont péri pendant l'aventure.", u8"%s han muerto durante la aventura."},
    {u8"%s ne revient pas de l'aventure.", u8"%s no vuelve de la aventura."},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s no vuelven de la aventura."},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s ha cambiado desde la salida:\n"
     u8"no se ha copiado su regreso de la aventura."},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s han cambiado desde la salida:\n"
     u8"no se ha copiado su regreso de la aventura."},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"Tu almacén ha cambiado desde la salida:\n"
     u8"no se han quitado de él los objetos equipados al salir."},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"Tu almacén no se ha modificado: los objetos\n"
     u8"equipados al salir siguen ahí."},
    {u8"Tu rapportes %s dans ta maison.", u8"Te llevas %s a tu casa."},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.",
     u8"Los PNJ te agradecen tus gatos: %s en tu casa."},
    {u8"Cadeaux des PNJ", u8"Regalos de los PNJ"},
    {u8"Ton butin est dans ton rangement : %s.", u8"Tu botín está en tu almacén: %s."},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.",
     u8"Los regalos de los PNJ están en tu almacén: %s."},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.",
     u8"%s, reclutados durante la aventura, llegan a tu casa."},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.",
     u8"%s, reclutado durante la aventura, llega a tu casa."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"%s no han podido unirse al grupo:\n"
     u8"en esta aventura, diriges a una parte\n"
     u8"de los gatos del grupo."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"%s no ha podido unirse al grupo:\n"
     u8"en esta aventura, diriges a una parte\n"
     u8"de los gatos del grupo."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"%s no han podido unirse al grupo:\n"
     u8"sigues la aventura sin dirigir gatos."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"%s no ha podido unirse al grupo:\n"
     u8"sigues la aventura sin dirigir gatos."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"%s no han podido partir y se quedan en casa."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"%s no ha podido partir y se queda en casa."},
    {u8"Tes chats sont rentrés de l'aventure.", u8"Tus gatos han vuelto de la aventura."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"Nuevo gato en el grupo\n"
     u8"\n"
     u8"%s se une al grupo de %s: ¡es tuyo!\n"
     u8"Mientras no estés, la IA del juego lo\n"
     u8"controla en combate y %s elige por él;\n"
     u8"al final de la aventura, volverá a tu casa."},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"Nuevos gatos: %s llegan a tu casa."},
    {u8"Nouveaux chats : %s arrive chez toi.", u8"Nuevos gatos: %s llega a tu casa."},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"La 4.ª plaza\n"
     u8"\n"
     u8"Con 3 jugadores, hay una plaza libre:\n"
     u8"es tuya y llevas 2 gatos."},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"La 4.ª plaza\n"
     u8"\n"
     u8"%s ya ha ocupado la plaza libre:\n"
     u8"con 3 jugadores, llevas 1 gato.\n"
     u8"Quita un gato de tu caja de aventura."},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"La 4.ª plaza\n"
     u8"\n"
     u8"Hay un cuarto jugador: cada uno lleva\n"
     u8"1 gato. Quita un gato de tu caja de aventura."},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"La 4.ª plaza es para el primero que ponga un 2.º gato en su caja."},
    {u8"La 4e place est à %s.", u8"La 4.ª plaza es de %s."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"Con %d jugadores, llevas %d gatos como máximo:\n"
     u8"quita %d gatos de tu caja de aventura.%s\n"
     u8"Los demás completarán el grupo."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"Con %d jugadores, llevas %d gatos como máximo:\n"
     u8"quita %d gato de tu caja de aventura.%s\n"
     u8"Los demás completarán el grupo."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"Con %d jugadores, llevas %d gato como máximo:\n"
     u8"quita %d gatos de tu caja de aventura.%s\n"
     u8"Los demás completarán el grupo."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Grupo mixto\n"
     u8"\n"
     u8"Con %d jugadores, llevas %d gato como máximo:\n"
     u8"quita %d gato de tu caja de aventura.%s\n"
     u8"Los demás completarán el grupo."},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"No estás en tu casa:\n"
     u8"jugarás con los gatos de los demás."},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Tu caja de aventura aún no está desbloqueada:\n"
     u8"jugarás con los gatos de los demás."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Tu caja de aventura está vacía: jugarás con los gatos de los demás.\n"
     u8"Para llevar los tuyos: responde «Todavía no», pon 1 o 2 en tu caja\n"
     u8"(en la guarida de Butch) y haz clic en tu cartel «Partir»."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Tu caja de aventura está vacía: jugarás con los gatos de los demás.\n"
     u8"Para llevar los tuyos: responde «Todavía no», pon uno en tu caja\n"
     u8"(en la guarida de Butch) y haz clic en tu cartel «Partir»."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"Tu caja de aventura tiene %d gatos, %d como máximo:\n"
     u8"responde «Todavía no», quita %d\n"
     u8"y haz clic en tu cartel «Partir».%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).",
     u8"Tus gatos parten de tu caja de aventura (en la guarida de Butch)."},
    {u8"Tu emmènes %s.", u8"Llevas a %s."},
    {u8"Départ en aventure", u8"Rumbo a la aventura"},
    {u8"La session n'est pas prête : réessaie dans un instant.",
     u8"La sesión no está lista: inténtalo de nuevo en un momento."},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"Se te está asignando la 4.ª plaza:\n"
     u8"la pregunta volverá en un momento."},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"No se ha podido preparar a tus gatos\n"
     u8"para la aventura (ver mgmp.log)."},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s vuelven de una aventura, pero su regreso\n"
     u8"aún no se ha copiado a tu partida guardada: recárgala\n"
     u8"antes de que vuelvan a partir (se te ofrecerá «Recargar»\n"
     u8"tras esta salida; si no, Esc, «Guardar y salir al menú»,\n"
     u8"luego «Jugar» y tu partida guardada)."},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s vuelve de una aventura, pero su regreso\n"
     u8"aún no se ha copiado a tu partida guardada: recárgala\n"
     u8"antes de que vuelva a partir (se te ofrecerá «Recargar»\n"
     u8"tras esta salida; si no, Esc, «Guardar y salir al menú»,\n"
     u8"luego «Jugar» y tu partida guardada)."},
    {u8"Le retour de %s attend d'être recopié.", u8"Falta copiar el regreso de %s."},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"No se ha podido enviar tu respuesta al anfitrión:\n"
     u8"reinténtalo."},
    {u8"Ta bourse de l'aventure : %s.", u8"Tu bolsa de la aventura: %s."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"Tu caja de aventura tiene %d gatos, %d como máximo:\n"
     u8"quita %d y haz clic en tu cartel «Partir»."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"Nuevo gato en el grupo\n"
     u8"\n"
     u8"%s se une al grupo: ¡este gato es tuyo!\n"
     u8"Lo diriges tú y volverá a tu casa\n"
     u8"al final de la aventura."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"Nuevo gato en el grupo\n"
     u8"\n"
     u8"%s se une al grupo: este gato es tuyo."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"Nuevo gato en el grupo\n"
     u8"\n"
     u8"%s se une al grupo: este gato es de %s."},
    {u8"Un nouveau chat", u8"Un gato nuevo"},
    {u8"en bas", u8"abajo"},
    {u8"en haut", u8"arriba"},
    {u8"à droite", u8"a la derecha"},
    {u8"à gauche", u8"a la izquierda"},
    {u8"au milieu", u8"en medio"},
    {u8"n°%d", u8"n.º %d"},
    {u8"%s (%s)", u8"%s (%s)"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]Tu voto no se ha aceptado: %s.[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"Elección del camino\n"
     u8"\n"},
    {u8"Même chemin", u8"Mismo camino"},
    {u8"Choisir", u8"Elegir"},
    {u8"Changer", u8"Cambiar"},
    {u8"Sortie", u8"Salida"},
    {u8"Retour à la maison", u8"Vuelta a casa"},
    {u8"Combat", u8"Combate"},
    {u8"Combat difficile", u8"Combate difícil"},
    {u8"Mini-boss", u8"Minijefe"},
    {u8"Boss", u8"Jefe"},
    {u8"Événement", u8"Evento"},
    {u8"Événement spécial", u8"Evento especial"},
    {u8"Trésor", u8"Tesoro"},
    {u8"Meuble", u8"Mueble"},
    {u8"Nourriture", u8"Comida"},
    {u8"Bonus", u8"Bonus"},
    {u8"Case vide", u8"Casilla vacía"},
    {u8"Case", u8"Casilla"},
    {u8"l'hôte n'est pas sur la carte", u8"el anfitrión no está en el mapa"},
    {u8"vos cartes ne sont pas les mêmes", u8"los mapas no coinciden"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"el anfitrión no puede ir a esa casilla"},
    {u8"le chemin est déjà choisi", u8"el camino ya está elegido"},
    {u8"ce tour est fini", u8"esta ronda ha terminado"},
    {u8"l'hôte ne l'a pas pris", u8"el anfitrión no lo ha aceptado"},
    {u8"pas de vote", u8"sin voto"},
    {u8"absent, ne vote pas", u8"ausente, no vota"},
    {u8"pas encore sur la carte", u8"aún no está en el mapa"},
    {u8"pas encore voté", u8"aún no ha votado"},
    {u8"%s (toi) : %s", u8"%s (tú): %s"},
    {u8"%s : %s", u8"%s: %s"},
    {u8"En pause : on attend tout le monde sur la carte.", u8"En pausa: esperando a todos en el mapa."},
    {u8"À toi de choisir : encore %u s.", u8"Te toca elegir: quedan %u s."},
    {u8"En attente des autres : encore %u s.", u8"Esperando a los demás: quedan %u s."},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"Empate entre %s:\n"
     u8"la suerte elige %s."},
    {u8"Tout le monde a choisi %s.", u8"Todos han elegido %s."},
    {u8"%s l'emporte.", u8"Gana %s."},
    {u8"%s : prêt", u8"%s: listo"},
    {u8"%s : défend sa maison", u8"%s: defiende su casa"},
    {u8"%s : pas encore", u8"%s: todavía no"},
    {u8"Tout le monde est prêt !", u8"¡Todos están listos!"},
    {u8"En attente d'un joueur.", u8"Esperando a un jugador."},
    {u8"En attente de %d joueurs.", u8"Esperando a %d jugadores."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"Fin del día\n"
     u8"\n"
     u8"Estás listo para pasar la noche.\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"Tu cartel sirve para salir de aventura:\n"
     u8"quita tus gatos del grupo para terminar el día."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Fin del día\n"
     u8"\n"
     u8"1 jugador espera para pasar la noche.\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Fin del día\n"
     u8"\n"
     u8"%d jugadores esperan para pasar la noche.\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1 jugador espera"},
    {u8"%d joueurs attendent", u8"%d jugadores esperan"},
    {u8"Passer la nuit", u8"Pasar la noche"},
    {u8"Plus tard", u8"Más tarde"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"Para continuar, pulsa Esc\n"
     u8"y luego «Guardar y salir al menú»."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"Lo que te espera\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Tu partida guardada ya está abierta:\n"
     u8"nada de esto aparecerá hasta que la cargues.\n"
     u8"«Recargar»: el mod pasa por el menú y la reabre."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"Lo que te espera\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Tu partida guardada ya está abierta: todo esto\n"
     u8"no aparecerá hasta que la cargues. Recárgala:\n"
     u8"Esc, «Guardar y salir al menú»,\n"
     u8"luego «Jugar» y tu partida guardada."},
    {u8"%s : j'arrive !", u8"%s: ¡me apunto!"},
    {u8"%s : réfléchit...", u8"%s: lo está pensando..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Quieres irte de aventura.\n"
     u8"Tu grupo responde:\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s: aún no ha llegado"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"Esperando a tu grupo\n"
     u8"\n"
     u8"%s\n"
     u8"Si avanzas sin esperarlo, la IA del juego\n"
     u8"controla sus gatos en los combates hasta\n"
     u8"que llegue."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"¡%s se va de aventura!\n"
     u8"Cada uno lleva sus gatos y dirige los suyos.\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"Aventura en curso\n"
     u8"\n"
     u8"¡%s está de aventura!\n"
     u8"Tus gatos no van en este viaje: seguirás\n"
     u8"la aventura sin dirigir ningún gato."},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"Aventura en curso\n"
     u8"\n"
     u8"¡%s está de aventura con tus gatos!\n"
     u8"Mientras no estés, la IA del juego los controla\n"
     u8"en combate y %s elige por ellos.\n"
     u8"¿Te unes a su grupo?"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"Aventura en curso\n"
     u8"\n"
     u8"¡%s está de aventura!\n"
     u8"¿Te unes a su grupo?"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"Dejar la aventura\n"
     u8"\n"
     u8"La aventura de %s ha terminado:\n"
     u8"vuelves a tu casa."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"Dejar la aventura\n"
     u8"\n"
     u8"Es la aventura de %s: solo él puede\n"
     u8"abandonarla. Seguirá sin ti.\n"
     u8"\n"
     u8"¿Vuelves a tu casa? Para regresar,\n"
     u8"haz clic en un cartel de tu casa."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi, et tes\n"
     u8"chats restent dans son équipe : l'IA du jeu\n"
     u8"les joue en combat, puis ils rentrent chez\n"
     u8"toi à la fin.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"Dejar la aventura\n"
     u8"\n"
     u8"Es la aventura de %s: solo él puede\n"
     u8"abandonarla. Seguirá sin ti, y tus\n"
     u8"gatos se quedan en su grupo: la IA del juego\n"
     u8"los controla en combate y al final vuelven\n"
     u8"a tu casa.\n"
     u8"\n"
     u8"¿Vuelves a tu casa? Para regresar,\n"
     u8"haz clic en un cartel de tu casa."},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"Unirse a la aventura\n"
     u8"\n"
     u8"El juego de %s te envía la aventura en curso.\n"
     u8"Si acaba de elegir algo en una casilla\n"
     u8"(evento, subida de nivel), llegarás\n"
     u8"cuando vuelva al mapa."},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"De camino a la aventura\n"
     u8"\n"
     u8"La partida de %s se va a cargar:\n"
     u8"partirás con él."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Vuelta a casa\n"
     u8"\n"
     u8"Se ha cortado la conexión con %s.\n"
     u8"Vuelves a tu casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"Vuelta a casa\n"
     u8"\n"
     u8"La salida con %s no se ha producido.\n"
     u8"Vuelves a tu casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Vuelta a casa\n"
     u8"\n"
     u8"La aventura de %s ha terminado.\n"
     u8"Vuelves a tu casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Vuelta a casa\n"
     u8"\n"
     u8"El combate de %s había terminado, pero tu copia\n"
     u8"seguía: se había desincronizado y ya nadie\n"
     u8"jugaba ese combate. Has vuelto a casa;\n"
     u8"tus gatos se quedan en su grupo.\n"
     u8"\n"
     u8"Para volver a la aventura, haz clic en\n"
     u8"un cartel de tu casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ton combat s'est fini, mais celui de %s\n"
     u8"continuait : ta copie s'était\n"
     u8"désynchronisée. Tu es rentré ; tes chats\n"
     u8"restent dans son équipe, l'IA du jeu les\n"
     u8"joue jusqu'à la fin du combat.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Vuelta a casa\n"
     u8"\n"
     u8"Tu combate ha acabado, pero el de %s\n"
     u8"seguía: tu copia se había\n"
     u8"desincronizado. Has vuelto a casa;\n"
     u8"tus gatos siguen en su grupo y la IA\n"
     u8"los controla hasta el fin del combate.\n"
     u8"\n"
     u8"Para volver a la aventura, haz clic en\n"
     u8"un cartel de tu casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"%s est entré dans une case que ton jeu\n"
     u8"n'a pas pu suivre : ta copie de l'aventure\n"
     u8"n'était plus la même. Tu es rentré ; tes\n"
     u8"chats restent dans son équipe, l'IA du jeu\n"
     u8"les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Vuelta a casa\n"
     u8"\n"
     u8"%s ha entrado en una casilla que tu juego\n"
     u8"no ha podido seguir: tu copia de la aventura\n"
     u8"ya no era la misma. Has vuelto a casa; tus\n"
     u8"gatos se quedan en su grupo y la IA del juego\n"
     u8"los controla.\n"
     u8"\n"
     u8"Para volver a la aventura, haz clic en\n"
     u8"un cartel de tu casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Vuelta a casa\n"
     u8"\n"
     u8"Tu copia del combate se ha desincronizado de\n"
     u8"la de %s: el combate seguía sin ti,\n"
     u8"así que has vuelto a casa. Tus gatos se quedan\n"
     u8"en su grupo y la IA del juego los controla.\n"
     u8"\n"
     u8"Para volver a la aventura, haz clic en\n"
     u8"un cartel de tu casa."},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"Tu partida está en plena aventura.\n"
     u8"En multijugador, las aventuras se juegan con el anfitrión:\n"
     u8"vuelve al menú y elige otra partida guardada,\n"
     u8"o termina esta en solitario."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Tu juego carga la aventura de %s."},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%s ha recargado la aventura:\n"
     u8"tú también la recargas."},
    {u8"Rentrer", u8"Volver a casa"},
    {u8"Rester", u8"Quedarme"},
    {u8"Ne pas attendre", u8"No esperar"},
    {u8"Recharger", u8"Recargar"},
    {u8"J'arrive", u8"¡Me apunto!"},
    {u8"Pas encore", u8"Todavía no"},
    {u8"Échanger", u8"Intercambiar"},
    {u8"Multijoueur", u8"Multijugador"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"Esta tubería es la de la casa de %s:\n"
     u8"envía tus gatos a los PNJ desde\n"
     u8"tu propia casa."},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"Los PNJ siguen la partida de %s:\n"
     u8"esta versión del juego no permite\n"
     u8"enviar gatos por la tubería."},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"Es una derrota para todo el grupo:\n"
     u8"los gatos de %s tampoco volverán."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"Unirse a %s\n"
     u8"\n"
     u8"Ya estás en una sala multijugador:\n"
     u8"sal primero (Multijugador y luego «Salir»),\n"
     u8"y vuelve a hacer clic en «Unirse a la partida»."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"Unirse a %s\n"
     u8"\n"
     u8"Tu juego volverá al menú principal (tu partida\n"
     u8"se guarda, como con «Guardar y salir al menú»),\n"
     u8"y se unirá a la sala de %s.\n"
     u8"\n"
     u8"Más tarde: se unirá cuando vayas\n"
     u8"al menú principal."},
    {u8"Y aller", u8"Ir ahora"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"Unirse a %s\n"
     u8"\n"
     u8"Tu juego se unirá a la sala de %s\n"
     u8"en cuanto estés en el menú principal\n"
     u8"(Esc y luego «Guardar y salir al menú»)."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"Unirse a %s\n"
     u8"\n"
     u8"Ahora mismo no es posible conectar por Steam.\n"
     u8"Inténtalo de nuevo en un momento o únete por dirección\n"
     u8"(Multijugador y luego «Unirse»)."},
    {u8"%s a annulé le départ.", u8"%s ha cancelado la salida."},
    {u8"réseau local", u8"red local"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Tus gatos ya han partido con el grupo de %s:\n"
     u8"mientras no estés, la IA del juego los controla\n"
     u8"en combate y %s elige por ellos; volverán\n"
     u8"a tu casa al final de la aventura."},
    {u8"Internet", u8"Internet"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s defiende su casa: un combate que juega\n"
     u8"solo, con sus gatos. Luego volverá."},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s defienden sus casas: cada uno juega\n"
     u8"su combate solo, con sus gatos. Luego volverán."},
    {u8"%s a fini de défendre sa maison.", u8"%s ha terminado de defender su casa."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s ha terminado de defender su casa.\n"
     u8"%s aún defiende la suya."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s ha terminado de defender su casa.\n"
     u8"%s aún defienden las suyas."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Has cancelado tras la votación: %s parte\n"
     u8"sin ti ni tus gatos. Podrás unirte a\n"
     u8"su aventura cuando esté en el mapa."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"Rumbo a la aventura\n"
     u8"\n"
     u8"Esta vez no puede partir ningún gato:\n"
     u8"%s no se va de aventura."},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"El reparto del botín no ha terminado:\n"
     u8"espera a que pasen todos los objetos."},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"Vas a unirte a la aventura de %s:\n"
     u8"defenderás tu casa a tu regreso."},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"%s es quien inicia la aventura:\n"
     u8"pídele que haga clic en «Partir»."},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"Multijugador\n"
     u8"\n"
     u8"Crea una sala para tus amigos,\n"
     u8"o únete a la partida de un amigo."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"Sala multijugador\n"
     u8"\n"
     u8"[c:red]No se puede abrir el puerto %u.[/c]\n"
     u8"Quizá otro programa lo esté usando.\n"
     u8"Cierra la sala y vuelve a intentarlo."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"Sala multijugador\n"
     u8"\n"
     u8"Abriendo la sala..."},
    {u8"%s (hôte)", u8"%s (anfitrión)"},
    {u8"En attente d'un ami...", u8"Esperando a un amigo..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Tus amigos de Steam se unen con un clic: en su\n"
     u8"lista de amigos de Steam, clic en tu nombre y «Unirse a la partida».\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"O por dirección:\n"
     u8"%s   (%s, puerto %u)\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Dale esta dirección a tu amigo:\n"
     u8"%s   (%s, puerto %u)\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"Ya está copiada: pégasela con Ctrl+V.\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"Dirección no encontrada en este PC (puerto %u).\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"Sala multijugador\n"
     u8"\n"
     u8"%s\n"
     u8"Jugadores (%d/4):\n"
     u8"%s\n"
     u8"Cuando estén todos, elige tu partida guardada."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"Unirse a una partida\n"
     u8"\n"
     u8"Dirección del anfitrión:\n"
     u8"%s_\n"
     u8"\n"
     u8"Escribe la dirección de tu amigo\n"
     u8"o pégala con Ctrl+V. Intro para confirmar.%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Por Steam es más fácil: en tu lista\n"
     u8"de amigos, clic en tu amigo y «Unirse a la partida»."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"Unirse a una partida\n"
     u8"\n"
     u8"[c:red]Dirección no válida: %s[/c]\n"
     u8"\n"
     u8"Ejemplo: 26.123.45.67\n"
     u8"o, con otro puerto: 26.123.45.67:27600"},
    {u8"(vide)", u8"(vacía)"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.",
     u8"Comprobad que tenéis la misma versión del juego y del mod."},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).",
     u8"La partida del anfitrión está completa (4 jugadores como máximo)."},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"No tenéis la misma versión del mod:\n"
     u8"instalad los dos la última."},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"No tenéis la misma versión del juego:\n"
     u8"actualizad Mewgenics los dos (Steam)."},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"Los archivos del juego no son iguales:\n"
     u8"verificadlos en Steam (Propiedades, Archivos instalados)."},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"El ajuste net.control de mgmp.json da el mismo gato\n"
     u8"a los dos jugadores: volved a ponerlo en \"auto\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"Sala multijugador\n"
     u8"\n"
     u8"[c:red]Se ha rechazado la conexión.[/c]\n"
     u8"%s\n"
     u8"(%s)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"Sala multijugador\n"
     u8"\n"
     u8"[c:red]No te puedes unir a %s:[/c]\n"
     u8"%s.\n"
     u8"Sal de la sala y vuelve a intentarlo (o únete por dirección)."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"Sala multijugador\n"
     u8"\n"
     u8"Conectando con %s..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"Sala multijugador\n"
     u8"\n"
     u8"[c:red]No se puede contactar con %s.[/c]\n"
     u8"Revisa la dirección y que tu amigo\n"
     u8"ha hecho clic en «Crear sala»."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"Sala multijugador\n"
     u8"\n"
     u8"Desconectado."},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"La pantalla de partidas guardadas se abrirá sola."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"Sala multijugador\n"
     u8"\n"
     u8"Conectando con %s...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"Sala multijugador\n"
     u8"\n"
     u8"[c:red]Se ha cortado la conexión con el anfitrión.[/c]\n"
     u8"Sal de la sala y vuelve a unirte."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"Sala multijugador\n"
     u8"\n"
     u8"Conectando con %s por Steam...\n"
     u8"(unos segundos)%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"Sala multijugador\n"
     u8"\n"
     u8"Conectado, saludando al anfitrión...%s"},
    {u8"%s (toi)", u8"%s (tú)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"Sala multijugador\n"
     u8"\n"
     u8"¡Te has unido a la partida de %s!\n"
     u8"\n"
     u8"Jugadores (%d/4):\n"
     u8"%s\n"
     u8"Esperando a la partida de %s:\n"
     u8"en cuanto elija su partida guardada,\n"
     u8"la pantalla de partidas guardadas se abrirá sola."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"Sala multijugador\n"
     u8"\n"
     u8"¡Te has unido a la partida de %s!\n"
     u8"\n"
     u8"Jugadores (%d/4):\n"
     u8"%s\n"
     u8"Esperando a la partida de %s:\n"
     u8"en cuanto elija su partida guardada,\n"
     u8"podrás elegir la tuya."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"Sala multijugador\n"
     u8"\n"
     u8"¡Te has unido a la partida de %s!\n"
     u8"\n"
     u8"Jugadores (%d/4):\n"
     u8"%s\n"
     u8"Elige tu partida guardada: tu casa, tus gatos.\n"
     u8"Los PNJ y la historia serán los de %s,\n"
     u8"y para ir de aventura, te unirás a su grupo."},
    {u8"Héberger", u8"Crear sala"},
    {u8"Rejoindre", u8"Unirse"},
    {u8"Choisir la sauvegarde", u8"Cargar partida"},
    {u8"Arrêter", u8"Cerrar sala"},
    {u8"Choisir ta sauvegarde", u8"Cargar tu partida"},
    {u8"Quitter", u8"Salir"},
    {u8"Annuler", u8"Cancelar"},
    {u8"Échanger avec %s", u8"Intercambiar con %s"},
    {u8"Donner à qui ?", u8"¿A quién se lo das?"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"Elige con Arriba/Abajo y pulsa Intro."},
    {u8"Pièces à donner : %lld_", u8"Monedas a dar: %lld_"},
    {u8"Nourriture à donner : %lld_", u8"Comida a dar: %lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"Pon un número (T: todo) y pulsa Intro."},
    {u8"Échanger avec %s : les objets de ton rangement",
     u8"Intercambiar con %s: los objetos de tu almacén"},
    {u8"16 au plus par don : %u choisis.", u8"Máximo 16 por regalo: %u elegidos."},
    {u8"16 au plus par don : %u choisi.", u8"Máximo 16 por regalo: %u elegido."},
    {u8"Lecture du rangement...", u8"Leyendo el almacén..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"Arriba/Abajo: elegir, Espacio: marcar, Izquierda/Derecha: cuántos.\n"
     u8"Letras para buscar y luego Intro."},
    {u8"Tu n'as rien choisi à donner.", u8"No has elegido nada para dar."},
    {u8"Tu n'as rien à donner pour l'instant.", u8"Ahora mismo no tienes nada que dar."},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"¿Dar %s de tu bolsa\n"
     u8"a %s?"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"¿Dar %s\n"
     u8"a %s?"},
    {u8"Envoi du don...", u8"Enviando el regalo..."},
    {u8"Personne ne peut recevoir maintenant.", u8"Ahora nadie puede recibir."},
    {u8"Personne d'autre n'est dans la partie.", u8"No hay nadie más en la partida."},
    {u8"Confirmer", u8"Confirmar"},
    {u8"Donner", u8"Dar"},
    {u8"Recommencer", u8"Volver a empezar"},
    {u8"Fermer", u8"Cerrar"},
    {u8"Pas d'échange possible ici.", u8"Aquí no se puede intercambiar."},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"La ventana se ha cerrado: el anfitrión ha entrado en una casilla.\n"
     u8"Lo que habías elegido se conserva (E para retomarlo)."},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"Intercambiar en aventura\n"
     u8"\n"
     u8"En el mapa, pulsa E para dar monedas\n"
     u8"o comida de tu bolsa a otro jugador."},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
