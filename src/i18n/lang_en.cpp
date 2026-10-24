// lang_en.cpp -- Mewgenics Multi : les textes du mod et du lanceur en anglais (en).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : « you », direct et simple, comme les textes du jeu (combined.csv, colonne en).
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_en[];
const TrPair kTable_en[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"mgmp.json could not be read: settings not saved."},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"mgmp.json contains comments (// or /* */) that saving would erase: remove them, or change the setting by hand in the file."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.json is not valid JSON: fix it (or put back the one from the zip) before changing the settings."},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"Can't write to the mod's folder: settings not saved."},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"mgmp.json could not be replaced (open in another program?): settings not saved."},
    {u8"Steam est ouvert.", u8"Steam is running."},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Without it, the game closes before the mod loads. Open Steam and wait until it's connected."},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"mgmp.json is missing next to Mewgenics\u00A0Multi.exe: save the settings to recreate it."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.json is not valid JSON: fix it, or put back the one from the zip."},
    {u8"Réglages lus dans mgmp.json.", u8"Settings read from mgmp.json."},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe not found (%s): in Settings, click Browse… and choose Mewgenics.exe."},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"resources.gpak is missing next to Mewgenics.exe: verify the game files in Steam."},
    {u8"Jeu trouvé : %s.", u8"Game found: %s."},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.",
     u8"steam_appid.txt: will be checked once the game is found."},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"It's missing next to Mewgenics.exe: without it, Steam restarts the game without the mod."},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"The one next to Mewgenics.exe doesn't contain 686060: Steam would restart the game without the mod."},
    {u8"steam_appid.txt est en place à côté du jeu.", u8"steam_appid.txt is in place next to the game."},
    {u8"Fichiers du mod présents.", u8"Mod files present."},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"mgmp.dll and mgmp_loader.exe are missing next to Mewgenics\u00A0Multi.exe: extract the whole folder from the zip again."},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"%s is missing next to Mewgenics\u00A0Multi.exe: extract the whole folder from the zip again."},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi, build %s, protocol %u: everyone must have the same one."},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"Game not found: first choose Mewgenics.exe in Settings."},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"steam_appid.txt could not be written next to the game: copy it by hand (LISEZ-MOI, step 2)."},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"You didn't give Windows permission: steam_appid.txt was not copied. You can try again, or copy it by hand (LISEZ-MOI, step 2)."},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics Multi could not restart as administrator: copy steam_appid.txt by hand (LISEZ-MOI, step 2)."},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Copying as administrator didn't work: copy steam_appid.txt by hand (LISEZ-MOI, step 2)."},
    {u8"%02u/%02u/%04u à %02u:%02u", u8"%02u/%02u/%04u at %02u:%02u"},
    {u8"Le mod n'a pas pu être chargé : %s", u8"The mod could not be loaded: %s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exe stopped without loading the mod. Try again; if it happens again, send loader.txt."},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exe stopped (code %s) without loading the mod. Most likely cause: Steam closed or disconnected. Open Steam, wait until it's connected, then try again."},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"The game closed while starting up, before the mod. Most common cause: Steam closed or disconnected. Open Steam, wait until it's connected, then try again."},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"Steam is restarting the game (this can take a minute or two)…"},
    {u8"Ouverture du jeu…", u8"Opening the game…"},
    {u8"Le jeu démarre…", u8"The game is starting…"},
    {u8"Injection du mod dans le jeu…", u8"Injecting the mod into the game…"},
    {u8"Le mod est en place, le jeu reprend…", u8"The mod is in place, the game resumes…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Still waiting for Steam to restart the game… If Steam just opened, it may be asking you something."},
    {u8"Jeu relancé trouvé : injection du mod…", u8"Restarted game found: injecting the mod…"},
    {u8"Le jeu est lancé avec le mod.", u8"The game is running with the mod."},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.",
     u8"The mod took a while to set up; the game resumes anyway."},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.",
     u8"The restarted game is slow to start; injecting anyway."},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"mgmp.dll not found next to Mewgenics\u00A0Multi.exe: extract the mod folder from the zip again."},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe not found: in Settings, click Browse… and choose Mewgenics.exe."},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"mgmp.json is unreadable: fix it, or put back the one from the zip."},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steam restarted the game without the mod: check steam_appid.txt next to Mewgenics.exe (Repair button), then try again."},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"This isn't the game version the mod was made for (1.1.21239): was it updated?"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"A program (overlay, antivirus, anti-cheat) stopped the mod from loading."},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"The game took too long to start: the mod is not loaded. Try again."},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"Injecting the mod failed and the game was closed. Try again; if it happens again, send loader.txt."},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"mgmp.json doesn't say where the game is: in Settings, click Browse… and choose Mewgenics.exe."},
    {u8"build %s · protocole %u", u8"build %s · protocol %u"},
    {u8"Steam ouvert", u8"Steam running"},
    {u8"Steam fermé", u8"Steam closed"},
    {u8"Réglages (mgmp.json)", u8"Settings (mgmp.json)"},
    {u8"Jeu trouvé", u8"Game found"},
    {u8"Jeu incomplet", u8"Game incomplete"},
    {u8"Jeu introuvable", u8"Game not found"},
    {u8"Fichiers du mod", u8"Mod files"},
    {u8"Réparer", u8"Repair"},
    {u8"Jouer", u8"Play"},
    {u8"Règle d'abord les croix rouges.", u8"Fix the red crosses first."},
    {u8"Réglages", u8"Settings"},
    {u8"Journaux", u8"Logs"},
    {u8"Langue", u8"Language"},
    {u8"Le jeu (Mewgenics.exe)", u8"The game (Mewgenics.exe)"},
    {u8"Parcourir…", u8"Browse…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe and resources.gpak are there."},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"resources.gpak is missing next to it: wrong folder."},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.",
     u8"No Mewgenics.exe here: choose it with Browse."},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Play together through Steam (friends list)"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Uncheck this if Steam causes problems: you'll connect by address only, from the game's Multiplayer menu."},
    {u8"Enregistrer", u8"Save"},
    {u8"Retour", u8"Back"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"No logs yet: the mod writes one every time you play."},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"1 mod log, next to Mewgenics\u00A0Multi.exe."},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"%d mod logs, next to Mewgenics\u00A0Multi.exe."},
    {u8"Le dernier : %s", u8"Latest: %s"},
    {u8"écrit le %s", u8"written on %s"},
    {u8"Ouvrir le dossier", u8"Open Folder"},
    {u8"Copier le dernier journal", u8"Copy Latest Log"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"To ask for help: copy the latest log, then paste it into Discord with Ctrl+V."},
    {u8"Lancement", u8"Launching"},
    {u8"Le jeu se lance avec le mod…", u8"The game is launching with the mod…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.",
     u8"Have fun! This window will close on its own."},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"Details are in loader.txt, next to Mewgenics\u00A0Multi.exe."},
    {u8"Réessayer", u8"Retry"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"If Steam restarts the game, it can take a minute or two."},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.",
     u8"Log copied: paste it into Discord with Ctrl+V."},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"Couldn't fill the clipboard: try again, or open the folder."},
    {u8"Où est Mewgenics.exe ?", u8"Where is Mewgenics.exe?"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"Direct2D is unavailable on this PC: Mewgenics Multi can't be displayed."},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"The graphics card could not be set up: Mewgenics Multi can't be displayed."},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"Your own house, your own cats,\n"
     u8"adventures together!"},
    {u8"(illustrations du jeu introuvables)", u8"(game artwork not found)"},
    {u8"Comme le jeu (%s)", u8"Same as the game (%s)"},
    {u8"Comme Windows (%s)", u8"Same as Windows (%s)"},
    {u8"Automatique (%s)", u8"Automatic (%s)"},
    {u8"Lancement annulé.", u8"Launch canceled."},
    {u8"Lancement du mod…", u8"Launching the mod…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windows could not prepare the launch. Restart Mewgenics Multi."},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exe could not start: extract the mod folder from the zip again (or your antivirus quarantined it)."},
    {u8"la connexion Steam avec l'hôte est coupée", u8"the Steam connection to the host was lost"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"the host isn't answering through Steam (lobby closed, or mod missing?)"},
    {u8"Steam refuse la connexion", u8"Steam refused the connection"},
    {u8"À %s de choisir", u8"%s's turn to choose"},
    {u8"À toi de choisir", u8"Your turn to choose"},
    {u8"À %s de choisir la capacité à remplacer.", u8"%s's turn to choose the ability to replace."},
    {u8"Ce chat est à %s", u8"This cat belongs to %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"Choosing Collars\n"
     u8"\n"
     u8"The shared choice stopped (game error):\n"
     u8"choose the collars for the whole team."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"Choosing Collars\n"
     u8"\n"
     u8"Ready! %s knows you're done:\n"
     u8"they'll confirm the departure for the whole team."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"Choosing Collars\n"
     u8"\n"
     u8"%s is the one confirming the departure:\n"
     u8"tell them when your cats have their collars."},
    {u8"%s et %s", u8"%s and %s"},
    {u8"%s, %s", u8"%s, %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Choosing Collars\n"
     u8"\n"
     u8"%s are still choosing collars for their cats.\n"
     u8"Click \"Lock In!\" again to leave\n"
     u8"anyway."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Choosing Collars\n"
     u8"\n"
     u8"%s is still choosing collars for their cats.\n"
     u8"Click \"Lock In!\" again to leave\n"
     u8"anyway."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"Departure\n"
     u8"\n"
     u8"%s's departure no longer shows here.\n"
     u8"Finish these screens yourself: you'll join them\n"
     u8"on the map."},
    {u8"  %s : parti", u8"  %s: gone"},
    {u8"  %s : prêt !", u8"  %s: ready!"},
    {u8"  %s : charge ta partie...", u8"  %s: loading your game..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"Departure\n"
     u8"\n"
     u8"Your team is loading your game to leave with you:\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"Choosing Items\n"
     u8"\n"
     u8"Your storage couldn't be loaded here:\n"
     u8"your cats leave with the items they're carrying."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Choosing Items\n"
     u8"\n"
     u8"%s are still equipping their cats.\n"
     u8"Click \"Lock In!\" again to leave\n"
     u8"anyway."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Choosing Items\n"
     u8"\n"
     u8"%s is still equipping their cats.\n"
     u8"Click \"Lock In!\" again to leave\n"
     u8"anyway."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"Choosing Items\n"
     u8"\n"
     u8"Ready! Your cats leave with these items:\n"
     u8"%s will confirm the departure for the whole team."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"Choosing Items\n"
     u8"\n"
     u8"The items chosen for your cats couldn't be\n"
     u8"carried into %s's adventure: your cats\n"
     u8"leave with the ones they had, your storage\n"
     u8"doesn't change."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"Choosing Items\n"
     u8"\n"
     u8"%s already confirmed the departure: your cats\n"
     u8"leave with the items chosen so far; further\n"
     u8"changes won't count."},
    {u8"Ta bourse : %s.", u8"Your purse: %s."},
    {u8"%s n'est plus dans la partie.", u8"%s is no longer in the game."},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"You have no purse on this adventure (you didn't bring any cats)."},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"Trading works in your house, or on a multiplayer adventure with your purse. Right now: %s."},
    {u8"pas de partie à plusieurs en cours", u8"no multiplayer game in progress"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés",
     u8"a game function is missing, trading at home is turned off"},
    {u8"tu es dans la partie de l'hôte", u8"you're in the host's game"},
    {u8"tu n'es pas dans ta maison", u8"you're not in your house"},
    {u8"ta save n'est pas chargée", u8"your save isn't loaded"},
    {u8"un départ en aventure est en cours", u8"a departure is in progress"},
    {u8"ta maison est attaquée", u8"your house is under attack"},
    {u8"ta maison est illisible", u8"your house can't be read"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"your storage is waiting for what your cats bring back from the host's adventure (if they're back, reload your save: Esc, \"Save and Quit to Menu\", then \"Play\" and your save)"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"items in your storage are still waiting for their new owner (loot, purchases, NPCs)"},
    {u8"Pas d'échange maintenant : %s.", u8"No trading right now: %s."},
    {u8"Ton don précédent attend encore sa réponse.",
     u8"Your previous gift is still waiting for an answer."},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"The mod doesn't know your save yet: reload it from the menu."},
    {u8"Choisis un joueur de la partie.", u8"Choose a player in the game."},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.",
     u8"No trading right now: %s isn't in their house."},
    {u8"Ce montant n'est pas possible.", u8"That amount isn't possible."},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.",
     u8"Choose coins, food or items to give."},
    {u8"16 objets au plus par don.", u8"Up to 16 items per gift."},
    {u8"Pas d'objets maintenant : %s.", u8"No items right now: %s."},
    {u8"Ta maison est illisible : pas de don.", u8"Your house can't be read: no gift."},
    {u8"Tu n'as pas autant de pièces.", u8"You don't have that many coins."},
    {u8"Tu n'as pas autant de nourriture.", u8"You don't have that much food."},
    {u8"Ton rangement est illisible : pas de don.", u8"Your storage can't be read: no gift."},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"A chosen item is already promised in an unfinished gift."},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"A chosen item is no longer in your storage: choose again."},
    {u8"Un objet choisi ne peut pas se donner.", u8"A chosen item can't be given."},
    {u8"Le don n'a pas pu partir (connexion).", u8"The gift couldn't be sent (connection)."},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…",
     u8"Gift sent to %s (%s): waiting for their answer…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…",
     u8"Gift request sent to the host: waiting for an answer…"},
    {u8"Pas de don : %s.", u8"No gift: %s."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%s received your gift: your items will leave your storage as soon as the item screen is closed."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%s received your gift: your items will leave your storage as soon as possible (after departing, defending your house, or when your cats come back)."},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%s received your gift: what you gave will leave your house when you return to this save."},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"No answer from %s about your gift: nothing left your house."},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"No answer from %s about your gift: nothing left your house."},
    {u8"Les échanges marchent pendant une partie à plusieurs.",
     u8"Trading works during a multiplayer game."},
    {u8"Dans ta maison : %s.", u8"In your house: %s."},
    {u8"%s n'est pas dans sa maison.", u8"%s isn't in their house."},
    {u8"%lld pièces", u8"%lld coins"},
    {u8"%lld pièce", u8"%lld coin"},
    {u8"%lld nourritures", u8"%lld food"},
    {u8"%lld nourriture", u8"%lld food"},
    {u8"rien", u8"nothing"},
    {u8"%s n'est pas dans sa maison", u8"%s isn't in their house"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s",
     u8"a window or the item screen was left open on %s's side"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"%s's game couldn't store them"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"%s's game couldn't store them"},
    {u8"les dons ne marchent pas chez %s", u8"gifts don't work for %s"},
    {u8"%s n'est plus dans la partie", u8"%s is no longer in the game"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%s still has cats on the host's adventure (their storage is waiting for them)"},
    {u8"raison inconnue", u8"unknown reason"},
    {u8"Cadeau d'%s", u8"Gift from %s"},
    {u8"Cadeau de %s", u8"Gift from %s"},
    {u8"%s t'a donné %s.", u8"%s gave you %s."},
    {u8"C'est dans ta maison, et ta save est à jour.",
     u8"It's in your house, and your save is up to date."},
    {u8"C'est dans ta maison.", u8"It's in your house."},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"In the end, %s did receive %s: it left your house."},
    {u8"%s a bien reçu %s : c'est parti de ta maison.", u8"%s received %s: it left your house."},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"(1 item couldn't leave your storage: you still have it too.)"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"(%u items couldn't leave your storage: you still have them too.)"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"The gift to %s didn't go through: %s. Nothing left your house."},
    {u8"Rien à donner dans ton rangement.", u8"Nothing to give in your storage."},
    {u8"Tape des lettres pour chercher un objet.", u8"Type letters to search for an item."},
    {u8"Recherche : %s_", u8"Search: %s_"},
    {u8"Aucun objet ne correspond.", u8"No matching items."},
    {u8"(page %u sur %u)", u8"(page %u of %u)"},
    {u8"Butin : %s.", u8"Loot: %s."},
    {u8"Besoin", u8"Need"},
    {u8"Cupidité", u8"Greed"},
    {u8"Passer", u8"Pass"},
    {u8"%u s", u8"%us"},
    {u8"toi", u8"you"},
    {u8"Joueur %u", u8"Player %u"},
    {u8"Un objet", u8"An item"},
    {u8"Partage du butin : objet %u sur %u", u8"Loot Split: item %u of %u"},
    {u8"Besoin : %s", u8"Need: %s"},
    {u8"Cupidité : %s", u8"Greed: %s"},
    {u8"Passer : %s", u8"Pass: %s"},
    {u8"Partage du butin", u8"Loot Split"},
    {u8"%u objets du sac à partager entre %s.", u8"%u items from the backpack to split between %s."},
    {u8"%u objet du sac à partager entre %s.", u8"%u item from the backpack to split between %s."},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"For each item: Need (you'll use it), Greed (if\n"
     u8"nobody needs it) or Pass. A 100-sided die breaks ties."},
    {u8"En attente de %s...", u8"Waiting for %s..."},
    {u8"En attente de %s... (%s)", u8"Waiting for %s... (%s)"},
    {u8"Tout le monde est là : ça commence !", u8"Everyone's here: let's start!"},
    {u8"Commencer", u8"Start"},
    {u8"D'accord", u8"OK"},
    {u8"Le veux-tu ? (%s)", u8"Do you want it? (%s)"},
    {u8"Je le veux", u8"I Want It"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"Need: you'll use it, you go before the others.\n"
     u8"Greed: you take it if nobody needs it.\n"
     u8"(%s)"},
    {u8"Ton choix : %s.", u8"Your choice: %s."},
    {u8"Ton choix est fait.", u8"Your choice is made."},
    {u8"En attente de %s (%s).", u8"Waiting for %s (%s)."},
    {u8"Le résultat arrive...", u8"Here comes the result..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s aren't here: their turn is skipped."},
    {u8"%s n'est pas là : son tour passe.", u8"%s isn't here: their turn is skipped."},
    {u8"Personne n'en veut : il reste chez toi.", u8"Nobody wants it: you keep it."},
    {u8"Personne n'en veut : il reste chez %s.", u8"Nobody wants it: %s keeps it."},
    {u8"Tu l'emportes (%s) !", u8"You win it (%s)!"},
    {u8"%s l'emporte (%s).", u8"%s wins it (%s)."},
    {u8"Dés : %s (égalité départagée).", u8"Rolls: %s (tie broken)."},
    {u8"Dés : %s.", u8"Rolls: %s."},
    {u8"Partage du butin terminé", u8"Loot Split Complete"},
    {u8"Tu remportes : %s.", u8"You win: %s."},
    {u8"%s remporte : %s.", u8"%s wins: %s."},
    {u8"Remporté, mais resté chez %s : %s.", u8"Won, but stayed with %s: %s."},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"Nobody wanted these (you keep them): %s."},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"Nobody wanted these (%s keeps them): %s."},
    {u8"Aucun objet n'a été partagé.", u8"No items were shared."},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"Your save is already open: the next window\n"
     u8"tells you how to reload it with your loot."},
    {u8"Ton butin t'attend dans ton rangement, chez toi.",
     u8"Your loot is waiting in your storage, at home."},
    {u8"Ce que les autres remportent quitte ton rangement.", u8"What the others win leaves your storage."},
    {u8"Beanies", u8"Beanies"},
    {u8"Butch", u8"Butch"},
    {u8"Tink", u8"Tink"},
    {u8"Frank", u8"Frank"},
    {u8"Jack", u8"Jack"},
    {u8"Tracy", u8"Tracy"},
    {u8"l'orgue de barbarie", u8"the Organ Grinder"},
    {u8"Steven", u8"Steven"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"Reward Ready\n"
     u8"\n"
     u8"The cat %s gave to %s filled\n"
     u8"the meter. The reward will be claimed for them\n"
     u8"as soon as you're in your house: the coins,\n"
     u8"food and items go to %s,\n"
     u8"the unlocks to everyone."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"Reward from %s\n"
     u8"\n"
     u8"The reward for %s's gift couldn't\n"
     u8"be sent to them: you keep it."},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"Reward for %s\n"
     u8"\n"
     u8"%s rewarded the cat %s\n"
     u8"gave them: %s go to that player's house.\n"
     u8"The unlocks are for everyone."},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Reward for Everyone\n"
     u8"\n"
     u8"%s rewarded the cat %s\n"
     u8"gave them: the unlocks are for\n"
     u8"everyone."},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s thanks you for your cat: %s\n"
     u8"(the unlocks are for everyone)."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Reward from %s\n"
     u8"\n"
     u8"%s thanks you for your cat: %s.\n"
     u8"They'll arrive in your house the next\n"
     u8"time your save loads (when you're back\n"
     u8"from the adventure). The unlocks are for\n"
     u8"everyone."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"Reward from %s\n"
     u8"\n"
     u8"Your cat filled %s's meter:\n"
     u8"the reward (unlocks) is for\n"
     u8"everyone. It'll reach your save\n"
     u8"the next time it loads."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"Reward from %s\n"
     u8"\n"
     u8"%s, who filled %s's meter,\n"
     u8"isn't here anymore: you keep the reward."},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"Reward Ready\n"
     u8"\n"
     u8"Your cat filled %s's meter!\n"
     u8"%s will claim the reward for you\n"
     u8"as soon as they're in their house."},
    {u8"les bourses ne marchent pas dans cette partie", u8"purses don't work in this game"},
    {u8"rien à donner", u8"nothing to give"},
    {u8"c'est trop d'un coup", u8"that's too much at once"},
    {u8"choisis un autre joueur", u8"choose another player"},
    {u8"ton don précédent attend encore la réponse de l'hôte",
     u8"your previous gift is still waiting for the host's answer"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"you have no purse on this adventure"},
    {u8"envoi impossible (connexion)", u8"couldn't send (connection)"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"Adventure Over\n"
     u8"\n"
     u8"Everyone brings their purse back home:"},
    {u8"%s n'est plus là : sa bourse reste chez toi.", u8"%s is gone: you keep their purse."},
    {u8"%s : rien.", u8"%s: nothing."},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.",
     u8"%s couldn't receive their purse: you keep it."},
    {u8"%s : %s.", u8"%s: %s."},
    {u8"Toi : %s.", u8"You: %s."},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"No answer from the host about your gift yet: check your purse in the path vote window."},
    {u8"Ton don n'est pas passé : %s.", u8"Your gift didn't go through: %s."},
    {u8"(et d'autres trouvailles)", u8"(and other finds)"},
    {u8"pas de bourse pour ce joueur dans cette aventure", u8"no purse for this player on this adventure"},
    {u8"ta bourse n'en a pas autant", u8"your purse doesn't have that much"},
    {u8"l'aventure n'est plus suivie", u8"the adventure is no longer tracked"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s gave you %s: it's in your purse."},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"You gave %s to %s: it's in their purse."},
    {u8"%s a donné %s à %s.", u8"%s gave %s to %s."},
    {u8"Un chat", u8"A cat"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%s found %s and lost %s: it counts toward your purse."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s found %s and lost %s: it counts toward %s's purse."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s found %s and lost %s: it counts toward %s's purse."},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s found %s: it goes to your purse."},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s found %s: it goes to %s's purse."},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s found %s: it goes to %s's purse."},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s spent %s: it comes out of your purse."},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.", u8"%s spent %s: it comes out of %s's purse."},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.", u8"%s spent %s: it comes out of %s's purse."},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s lost %s: it comes out of your purse."},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%s lost %s: it comes out of %s's purse."},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%s lost %s: it comes out of %s's purse."},
    {u8"%s n'a rien trouvé.", u8"%s found nothing."},
    {u8"un objet", u8"an item"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.",
     u8"You bought %s for %s: it comes out of your purse."},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.",
     u8"%s bought %s for %s: it comes out of their purse."},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"You bought %s: %s more for your purse."},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%s bought %s: %s more for their purse."},
    {u8"Tu as acheté %s (gratuit).", u8"You bought %s (free)."},
    {u8"%s a acheté %s (gratuit).", u8"%s bought %s (free)."},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"In battle, your cats found %s and lost %s: it counts toward your purse."},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"In battle, your cats found %s: it goes to your purse."},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"In battle, your cats lost %s: it comes out of your purse."},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"In battle, your cats found nothing."},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"In battle, %s's cats found %s and lost %s: it counts toward their purse."},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"In battle, %s's cats found %s and lost %s: it counts toward their purse."},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"In battle, %s's cats found %s: it goes to their purse."},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"In battle, %s's cats found %s: it goes to their purse."},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"In battle, %s's cats lost %s: it comes out of their purse."},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"In battle, %s's cats lost %s: it comes out of their purse."},
    {u8"En combat, les chats d'%s n'ont rien trouvé.", u8"In battle, %s's cats found nothing."},
    {u8"En combat, les chats de %s n'ont rien trouvé.", u8"In battle, %s's cats found nothing."},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"Your Backpack Is Open\n"
     u8"\n"
     u8"The host entered a room:\n"
     u8"close your backpack to follow them."},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"Items\n"
     u8"\n"
     u8"This cat belongs to %s: only its owner\n"
     u8"can change its items."},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"Items\n"
     u8"\n"
     u8"Inside a room (a shop, a level-up...),\n"
     u8"only the host can change items.\n"
     u8"Equip your cats on the map."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"Items\n"
     u8"\n"
     u8"The host isn't back on the map yet:\n"
     u8"wait for them, your backpack will unlock by itself."},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"Items\n"
     u8"\n"
     u8"Your backpack is syncing with the host's:\n"
     u8"close it, then reopen it in a moment."},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"Items\n"
     u8"\n"
     u8"Another player took this item just before you:\n"
     u8"your change didn't go through. Your backpack will\n"
     u8"match the host's again when you close it."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"Items\n"
     u8"\n"
     u8"The host entered a room before your\n"
     u8"last change arrived: it was canceled.\n"
     u8"Redo it on the map."},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"Items\n"
     u8"\n"
     u8"This item is gone: another player\n"
     u8"took it just before you."},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).",
     u8"%s is gone: you keep their purchases (%s)."},
    {u8"%s rapporte ses achats : %s.", u8"%s brings their purchases home: %s."},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%s's purchases couldn't be sent to them: you keep them."},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%s's purchases couldn't be sent to them: you keep them."},
    {u8"Achats de l'aventure", u8"Adventure Purchases"},
    {u8"Boutique", u8"Shop"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s aren't done in this\n"
     u8"shop: wait for them (once you leave, you\n"
     u8"can't buy for them anymore).\n"
     u8"Click again to leave anyway."},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s isn't done in this\n"
     u8"shop: wait for them (once you leave, you\n"
     u8"can't buy for them anymore).\n"
     u8"Click again to leave anyway."},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"Your purchase couldn't be requested from the host:\n"
     u8"nothing was bought. Try again in a moment."},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"The host is no longer in this shop:\n"
     u8"the purchase didn't happen."},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"This item is no longer for sale in the host's shop:\n"
     u8"the purchase didn't happen."},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"This item was just sold:\n"
     u8"the purchase didn't happen."},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"It costs %s; your purse has %lld."},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"Your purse can't cover this item."},
    {u8"(Chacun paie avec sa bourse, toi aussi.)",
     u8"(Everyone pays from their own purse, including you.)"},
    {u8"(Chacun paie avec sa bourse.)", u8"(Everyone pays from their own purse.)"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"For now, only the host buys candy\n"
     u8"and familiars (with their purse)."},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.",
     u8"The game couldn't make the purchase on the host's side."},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"No purses on this adventure:\n"
     u8"only the host buys."},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"You didn't bring any cats on this\n"
     u8"adventure: you have no purse to buy\n"
     u8"with."},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"You have no purse on this adventure\n"
     u8"(you didn't bring any cats): no purchase."},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"The host can't buy for you right\n"
     u8"now (a window is open on their side):\n"
     u8"nothing was bought. Try again in a moment."},
    {u8"un bonbon", u8"a candy"},
    {u8"un familier", u8"a familiar"},
    {u8"un article", u8"something"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"Adventure Over\n"
     u8"\n"
     u8"%s go home to %s."},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"Adventure Over\n"
     u8"\n"
     u8"%s goes home to %s."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Departure\n"
     u8"\n"
     u8"The other players' cats couldn't leave\n"
     u8"and your departure box is empty: no adventure\n"
     u8"this time. (The mod's log, mgmp.log, says why.)"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"Departure\n"
     u8"\n"
     u8"Your departure box is empty and nobody is bringing\n"
     u8"a cat: no adventure this time."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Mixed Team\n"
     u8"\n"
     u8"The other players' cats couldn't\n"
     u8"join the team: you leave with your own.\n"
     u8"(The mod's log, mgmp.log, says why.)"},
    {u8"Retour de l'aventure", u8"Back from the Adventure"},
    {u8"%s est de retour à la maison.", u8"%s is back home."},
    {u8"%s sont de retour à la maison.", u8"%s are back home."},
    {u8"%s a péri pendant l'aventure.", u8"%s perished on the adventure."},
    {u8"%s ont péri pendant l'aventure.", u8"%s perished on the adventure."},
    {u8"%s ne revient pas de l'aventure.", u8"%s isn't coming back from the adventure."},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s aren't coming back from the adventure."},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s changed since departing:\n"
     u8"their adventure results weren't copied over."},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s changed since departing:\n"
     u8"their adventure results weren't copied over."},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"Your storage changed since the departure:\n"
     u8"the items equipped at departure weren't removed from it."},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"Your storage wasn't changed: the items\n"
     u8"equipped at departure are still in it."},
    {u8"Tu rapportes %s dans ta maison.", u8"You bring %s back to your house."},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.",
     u8"The NPCs thank you for your cats: %s in your house."},
    {u8"Cadeaux des PNJ", u8"NPC Gifts"},
    {u8"Ton butin est dans ton rangement : %s.", u8"Your loot is in your storage: %s."},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.", u8"The NPC gifts are in your storage: %s."},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.",
     u8"%s, recruited during the adventure, arrive at your house."},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.",
     u8"%s, recruited during the adventure, arrives at your house."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Mixed Team\n"
     u8"\n"
     u8"%s couldn't join the team:\n"
     u8"for this adventure, you control some\n"
     u8"of the team's cats."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Mixed Team\n"
     u8"\n"
     u8"%s couldn't join the team:\n"
     u8"for this adventure, you control some\n"
     u8"of the team's cats."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Mixed Team\n"
     u8"\n"
     u8"%s couldn't join the team:\n"
     u8"you follow the adventure without controlling a cat."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Mixed Team\n"
     u8"\n"
     u8"%s couldn't join the team:\n"
     u8"you follow the adventure without controlling a cat."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"Mixed Team\n"
     u8"\n"
     u8"%s couldn't leave and are staying home."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"Mixed Team\n"
     u8"\n"
     u8"%s couldn't leave and is staying home."},
    {u8"Tes chats sont rentrés de l'aventure.", u8"Your cats are back from the adventure."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"New Cat on the Team\n"
     u8"\n"
     u8"%s joins %s's team: this cat is\n"
     u8"yours! Until you join, the game's AI\n"
     u8"plays it in battle and %s chooses for it,\n"
     u8"and it will go home to you when the adventure ends."},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"New cats: %s are coming to your house."},
    {u8"Nouveaux chats : %s arrive chez toi.", u8"New cat: %s is coming to your house."},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"The 4th Slot\n"
     u8"\n"
     u8"With 3 players, the team has a free slot:\n"
     u8"it's yours, you bring 2 cats."},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"The 4th Slot\n"
     u8"\n"
     u8"The free slot is already taken by %s:\n"
     u8"with 3 players, you bring 1 cat.\n"
     u8"Remove a cat from your departure box."},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"The 4th Slot\n"
     u8"\n"
     u8"A fourth player is here: everyone brings\n"
     u8"1 cat. Remove a cat from your departure box."},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"The 4th slot goes to the first player to put a 2nd cat in their box."},
    {u8"La 4e place est à %s.", u8"%s has the 4th slot."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Mixed Team\n"
     u8"\n"
     u8"With %d players, you bring %d cats at most:\n"
     u8"remove %d cats from your departure box.%s\n"
     u8"The other players will fill the team with theirs."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Mixed Team\n"
     u8"\n"
     u8"With %d players, you bring %d cats at most:\n"
     u8"remove %d cat from your departure box.%s\n"
     u8"The other players will fill the team with theirs."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Mixed Team\n"
     u8"\n"
     u8"With %d players, you bring %d cat at most:\n"
     u8"remove %d cats from your departure box.%s\n"
     u8"The other players will fill the team with theirs."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Mixed Team\n"
     u8"\n"
     u8"With %d players, you bring %d cat at most:\n"
     u8"remove %d cat from your departure box.%s\n"
     u8"The other players will fill the team with theirs."},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"You're not in your house:\n"
     u8"you'll play with the others' cats."},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Your departure box isn't unlocked yet:\n"
     u8"you'll play with the others' cats."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Your departure box is empty: you'll play with the others' cats.\n"
     u8"To bring yours: answer \"Not Yet\", put 1 or 2 in your box\n"
     u8"(at Butch's), then click your \"Depart!\" sign."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Your departure box is empty: you'll play with the others' cats.\n"
     u8"To bring yours: answer \"Not Yet\", put one in your box\n"
     u8"(at Butch's), then click your \"Depart!\" sign."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"Your departure box has %d cats, %d at most:\n"
     u8"answer \"Not Yet\", remove %d,\n"
     u8"then click your \"Depart!\" sign.%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).",
     u8"Your cats leave from your departure box (at Butch's)."},
    {u8"Tu emmènes %s.", u8"You're bringing %s."},
    {u8"Départ en aventure", u8"Departure"},
    {u8"La session n'est pas prête : réessaie dans un instant.",
     u8"The session isn't ready: try again in a moment."},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"The 4th slot is being given to you:\n"
     u8"you'll be asked again in a moment."},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"Your cats couldn't be prepared\n"
     u8"for the adventure (see mgmp.log)."},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s are back from an adventure, but their results\n"
     u8"aren't in your save yet: reload it\n"
     u8"before sending them out again (\"Reload\" will be\n"
     u8"offered after this departure; otherwise Esc, \"Save and\n"
     u8"Quit to Menu\", then \"Play\" and your save)."},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s is back from an adventure, but their results\n"
     u8"aren't in your save yet: reload it\n"
     u8"before sending them out again (\"Reload\" will be\n"
     u8"offered after this departure; otherwise Esc, \"Save and\n"
     u8"Quit to Menu\", then \"Play\" and your save)."},
    {u8"Le retour de %s attend d'être recopié.", u8"%s's results are waiting to be copied."},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"Sending your answer to the host failed:\n"
     u8"try again."},
    {u8"Ta bourse de l'aventure : %s.", u8"Your adventure purse: %s."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"Your departure box has %d cats, %d at most:\n"
     u8"remove %d, then click your \"Depart!\" sign."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"New Cat on the Team\n"
     u8"\n"
     u8"%s joins the team: this cat is yours!\n"
     u8"You control it, and it will go home with you\n"
     u8"when the adventure ends."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"New Cat on the Team\n"
     u8"\n"
     u8"%s joins the team: this cat is yours."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"New Cat on the Team\n"
     u8"\n"
     u8"%s joins the team: this cat belongs to %s."},
    {u8"Un nouveau chat", u8"A new cat"},
    {u8"en bas", u8"bottom"},
    {u8"en haut", u8"top"},
    {u8"à droite", u8"right"},
    {u8"à gauche", u8"left"},
    {u8"au milieu", u8"middle"},
    {u8"n°%d", u8"#%d"},
    {u8"%s (%s)", u8"%s (%s)"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]Your vote wasn't counted: %s.[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"Choose a Path\n"
     u8"\n"},
    {u8"Même chemin", u8"Same Path"},
    {u8"Choisir", u8"Choose"},
    {u8"Changer", u8"Change"},
    {u8"Sortie", u8"Exit"},
    {u8"Retour à la maison", u8"Return Home"},
    {u8"Combat", u8"Fight"},
    {u8"Combat difficile", u8"Hard Fight"},
    {u8"Mini-boss", u8"Miniboss"},
    {u8"Boss", u8"Boss"},
    {u8"Événement", u8"Event"},
    {u8"Événement spécial", u8"Special Event"},
    {u8"Trésor", u8"Treasure"},
    {u8"Meuble", u8"Furniture"},
    {u8"Nourriture", u8"Food"},
    {u8"Bonus", u8"Bonus"},
    {u8"Case vide", u8"Empty Room"},
    {u8"Case", u8"Room"},
    {u8"l'hôte n'est pas sur la carte", u8"the host isn't on the map"},
    {u8"vos cartes ne sont pas les mêmes", u8"your maps don't match"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"the host can't go to that room"},
    {u8"le chemin est déjà choisi", u8"the path is already chosen"},
    {u8"ce tour est fini", u8"this round is over"},
    {u8"l'hôte ne l'a pas pris", u8"the host didn't accept it"},
    {u8"pas de vote", u8"no vote"},
    {u8"absent, ne vote pas", u8"away, not voting"},
    {u8"pas encore sur la carte", u8"not on the map yet"},
    {u8"pas encore voté", u8"hasn't voted yet"},
    {u8"%s (toi) : %s", u8"%s (you): %s"},
    {u8"%s : %s", u8"%s: %s"},
    {u8"En pause : on attend tout le monde sur la carte.",
     u8"Paused: waiting for everyone to reach the map."},
    {u8"À toi de choisir : encore %u s.", u8"Your turn to choose: %us left."},
    {u8"En attente des autres : encore %u s.", u8"Waiting for the others: %us left."},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"Tie between %s:\n"
     u8"fate picks %s."},
    {u8"Tout le monde a choisi %s.", u8"Everyone chose %s."},
    {u8"%s l'emporte.", u8"%s wins."},
    {u8"%s : prêt", u8"%s: ready"},
    {u8"%s : défend sa maison", u8"%s: defending their house"},
    {u8"%s : pas encore", u8"%s: not yet"},
    {u8"Tout le monde est prêt !", u8"Everyone is ready!"},
    {u8"En attente d'un joueur.", u8"Waiting for one player."},
    {u8"En attente de %d joueurs.", u8"Waiting for %d players."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"End of Day\n"
     u8"\n"
     u8"You're ready to end the day.\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"Your sign is set to go on an adventure:\n"
     u8"remove your cats from the team to end the day."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"End of Day\n"
     u8"\n"
     u8"1 player is waiting to end the day.\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"End of Day\n"
     u8"\n"
     u8"%d players are waiting to end the day.\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1 player waiting"},
    {u8"%d joueurs attendent", u8"%d players waiting"},
    {u8"Passer la nuit", u8"End Day"},
    {u8"Plus tard", u8"Later"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"To continue, press Esc,\n"
     u8"then \"Save and Quit to Menu\"."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"What's Waiting for You\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Your save is already open: none of this will\n"
     u8"be in it until it loads again. \"Reload\":\n"
     u8"the mod goes through the menu and reopens it."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"What's Waiting for You\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Your save is already open: none of this will\n"
     u8"be in it until it loads again. Reload it:\n"
     u8"Esc, \"Save and Quit to Menu\",\n"
     u8"then \"Play\" and your save."},
    {u8"%s : j'arrive !", u8"%s: on my way!"},
    {u8"%s : réfléchit...", u8"%s: thinking..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"Departure\n"
     u8"\n"
     u8"You want to go on an adventure.\n"
     u8"Your team answers:\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s: not here yet"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"Waiting for Your Team\n"
     u8"\n"
     u8"%s\n"
     u8"If you move on without waiting, the game's AI\n"
     u8"plays their cats in battle until\n"
     u8"they arrive."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"Departure\n"
     u8"\n"
     u8"%s is going on an adventure!\n"
     u8"Everyone brings their own cats and controls them.\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"Adventure in Progress\n"
     u8"\n"
     u8"%s is on an adventure!\n"
     u8"Your cats aren't on this trip: you'll follow\n"
     u8"the adventure without controlling a cat."},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"Adventure in Progress\n"
     u8"\n"
     u8"%s is on an adventure with your cats!\n"
     u8"Until you join, the game's AI plays them\n"
     u8"in battle and %s chooses for them.\n"
     u8"Join their team?"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"Adventure in Progress\n"
     u8"\n"
     u8"%s is on an adventure!\n"
     u8"Join their team?"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"Leave the Adventure\n"
     u8"\n"
     u8"%s's adventure is over:\n"
     u8"you're going back to your house."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"Leave the Adventure\n"
     u8"\n"
     u8"This is %s's adventure: only they can\n"
     u8"abandon it. It goes on without you.\n"
     u8"\n"
     u8"Go back to your house? To come back,\n"
     u8"click a sign in your house."},
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
     u8"Leave the Adventure\n"
     u8"\n"
     u8"This is %s's adventure: only they can\n"
     u8"abandon it. It goes on without you, and your\n"
     u8"cats stay on their team: the game's AI\n"
     u8"plays them in battle, then they come home\n"
     u8"to you at the end.\n"
     u8"\n"
     u8"Go back to your house? To come back,\n"
     u8"click a sign in your house."},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"Joining the Adventure\n"
     u8"\n"
     u8"%s's game is sending you the current adventure.\n"
     u8"If they just made a choice in a room\n"
     u8"(event, level-up), you'll arrive\n"
     u8"when they're back on the map."},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"Off on an Adventure\n"
     u8"\n"
     u8"%s's game is about to load:\n"
     u8"you'll leave with them."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Going Home\n"
     u8"\n"
     u8"The connection with %s was lost.\n"
     u8"You're going back to your house."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"Going Home\n"
     u8"\n"
     u8"The departure with %s didn't happen.\n"
     u8"You're going back to your house."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Going Home\n"
     u8"\n"
     u8"%s's adventure is over.\n"
     u8"You're going back to your house."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Going Home\n"
     u8"\n"
     u8"%s's fight was over, but your copy\n"
     u8"kept going: it had desynced, nobody\n"
     u8"was playing that fight anymore. You went\n"
     u8"home; your cats stay on their team.\n"
     u8"\n"
     u8"To get back into the adventure, click\n"
     u8"a sign in your house."},
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
     u8"Going Home\n"
     u8"\n"
     u8"Your fight ended, but %s's\n"
     u8"kept going: your copy had\n"
     u8"desynced. You went home; your cats\n"
     u8"stay on their team, the game's AI\n"
     u8"plays them until the fight ends.\n"
     u8"\n"
     u8"To get back into the adventure, click\n"
     u8"a sign in your house."},
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
     u8"Going Home\n"
     u8"\n"
     u8"%s entered a room your game\n"
     u8"couldn't follow: your copy of the adventure\n"
     u8"was no longer the same. You went home; your\n"
     u8"cats stay on their team, the game's AI\n"
     u8"plays them.\n"
     u8"\n"
     u8"To get back into the adventure, click\n"
     u8"a sign in your house."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Going Home\n"
     u8"\n"
     u8"Your copy of the fight desynced from\n"
     u8"%s's: the fight went on without you,\n"
     u8"so you went home. Your cats stay on\n"
     u8"their team, the game's AI plays them.\n"
     u8"\n"
     u8"To get back into the adventure, click\n"
     u8"a sign in your house."},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"Your save is mid-adventure.\n"
     u8"In multiplayer, adventures are played with the host:\n"
     u8"go back to the menu and choose another save,\n"
     u8"or finish this one solo."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"Departure\n"
     u8"\n"
     u8"Your game is loading %s's adventure."},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%s reloaded the adventure:\n"
     u8"you're reloading it too."},
    {u8"Rentrer", u8"Go Home"},
    {u8"Rester", u8"Stay"},
    {u8"Ne pas attendre", u8"Don't Wait"},
    {u8"Recharger", u8"Reload"},
    {u8"J'arrive", u8"On My Way!"},
    {u8"Pas encore", u8"Not Yet"},
    {u8"Échanger", u8"Trade"},
    {u8"Multijoueur", u8"Multiplayer"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"This pipe belongs to %s's house:\n"
     u8"send your cats to the NPCs from\n"
     u8"your own house."},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"The NPCs follow %s's game:\n"
     u8"sending your cats down the pipe isn't\n"
     u8"possible with this version of the game."},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"It's a defeat for the whole team:\n"
     u8"%s's cats won't come home either."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"Join %s\n"
     u8"\n"
     u8"You're already in a multiplayer lobby:\n"
     u8"leave it first (Multiplayer, then \"Leave\"),\n"
     u8"then click \"Join Game\" again."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"Join %s\n"
     u8"\n"
     u8"Your game will return to the main menu (your progress\n"
     u8"is saved, as with \"Save and Quit to Menu\"),\n"
     u8"then join %s's lobby.\n"
     u8"\n"
     u8"Later: it will join when you go to the\n"
     u8"main menu."},
    {u8"Y aller", u8"Go"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"Join %s\n"
     u8"\n"
     u8"Your game will join %s's lobby\n"
     u8"as soon as you're at the main menu\n"
     u8"(Esc, then \"Save and Quit to Menu\")."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"Join %s\n"
     u8"\n"
     u8"Connecting through Steam isn't possible right now.\n"
     u8"Try again in a moment, or join by address\n"
     u8"(Multiplayer, then \"Join\")."},
    {u8"%s a annulé le départ.", u8"%s canceled the departure."},
    {u8"réseau local", u8"local network"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"Departure\n"
     u8"\n"
     u8"Your cats already left with %s's team:\n"
     u8"until you join, the game's AI plays them\n"
     u8"in battle and %s chooses for them; they'll come\n"
     u8"home to you at the end of the adventure."},
    {u8"Internet", u8"Internet"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s is defending their house: a fight they play\n"
     u8"alone, with their cats. They'll be back afterward."},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s are defending their houses: each plays\n"
     u8"their fight alone, with their cats. They'll be back afterward."},
    {u8"%s a fini de défendre sa maison.", u8"%s finished defending their house."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s finished defending their house.\n"
     u8"%s is still defending theirs."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s finished defending their house.\n"
     u8"%s are still defending theirs."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"Departure\n"
     u8"\n"
     u8"You canceled after the vote: %s leaves\n"
     u8"without you or your cats. You can join\n"
     u8"their adventure once they're on the map."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"Departure\n"
     u8"\n"
     u8"No cat can leave this time:\n"
     u8"%s isn't going on an adventure."},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"The loot split isn't over:\n"
     u8"wait until all the items have been shown."},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"You're leaving to join %s's adventure:\n"
     u8"you'll defend your house when you return."},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"%s is the one starting the adventure:\n"
     u8"ask them to click \"Depart!\"."},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"Multiplayer\n"
     u8"\n"
     u8"Host a game so your friends can join you,\n"
     u8"or join a friend's game."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"[c:red]Can't open port %u.[/c]\n"
     u8"Another program may be using it.\n"
     u8"Stop the lobby, then try again."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"Opening the lobby..."},
    {u8"%s (hôte)", u8"%s (host)"},
    {u8"En attente d'un ami...", u8"Waiting for a friend..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Your Steam friends can join in one click: in their\n"
     u8"Steam friends list, they pick \"Join Game\" on your name.\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Or by address:\n"
     u8"%s   (%s, port %u)\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Give this address to your friend:\n"
     u8"%s   (%s, port %u)\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"It's already copied: paste it to them with Ctrl+V.\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"No address found on this PC (port %u).\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"%s\n"
     u8"Players (%d/4):\n"
     u8"%s\n"
     u8"When everyone's here, choose your save."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"Join a Game\n"
     u8"\n"
     u8"Host address:\n"
     u8"%s_\n"
     u8"\n"
     u8"Type the address your friend gave you,\n"
     u8"or paste it with Ctrl+V. Press Enter to confirm.%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Through Steam, it's easier: in your friends\n"
     u8"list, pick \"Join Game\" on your friend."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"Join a Game\n"
     u8"\n"
     u8"[c:red]Invalid address: %s[/c]\n"
     u8"\n"
     u8"Example: 26.123.45.67\n"
     u8"or, with another port: 26.123.45.67:27600"},
    {u8"(vide)", u8"(empty)"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.",
     u8"Make sure you both have the same version of the game and the mod."},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).",
     u8"The host's game is full (4 players max)."},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"You don't have the same mod version:\n"
     u8"you both need the latest one."},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"You don't have the same game version:\n"
     u8"you both need to update Mewgenics (Steam)."},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"Your game files don't match:\n"
     u8"verify them in Steam (Properties, Installed Files)."},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"The net.control setting in mgmp.json gives the same cat\n"
     u8"to both players: set it back to \"auto\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"[c:red]The connection was refused.[/c]\n"
     u8"%s\n"
     u8"(%s)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"[c:red]Couldn't join %s:[/c]\n"
     u8"%s.\n"
     u8"Leave the lobby, then try again (or join by address)."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"Connecting to %s..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"[c:red]Couldn't reach %s.[/c]\n"
     u8"Check the address, and that your friend\n"
     u8"clicked \"Host\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"Disconnected."},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"The save screen will open on its own."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"Connecting to %s...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"[c:red]The connection to the host was lost.[/c]\n"
     u8"Leave the lobby, then join it again."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"Connecting to %s via Steam...\n"
     u8"(a few seconds)%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"Connected, saying hello to the host...%s"},
    {u8"%s (toi)", u8"%s (you)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"You joined %s's game!\n"
     u8"\n"
     u8"Players (%d/4):\n"
     u8"%s\n"
     u8"Waiting for %s's game:\n"
     u8"as soon as they choose their save,\n"
     u8"the save screen will open on its own."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"You joined %s's game!\n"
     u8"\n"
     u8"Players (%d/4):\n"
     u8"%s\n"
     u8"Waiting for %s's game:\n"
     u8"as soon as they choose their save,\n"
     u8"you can choose yours."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"Multiplayer Lobby\n"
     u8"\n"
     u8"You joined %s's game!\n"
     u8"\n"
     u8"Players (%d/4):\n"
     u8"%s\n"
     u8"Choose your save: your house, your cats.\n"
     u8"The NPCs and story will be %s's,\n"
     u8"and to go on an adventure, you'll join their team."},
    {u8"Héberger", u8"Host"},
    {u8"Rejoindre", u8"Join"},
    {u8"Choisir la sauvegarde", u8"Choose a Save"},
    {u8"Arrêter", u8"Stop"},
    {u8"Choisir ta sauvegarde", u8"Choose Your Save"},
    {u8"Quitter", u8"Leave"},
    {u8"Annuler", u8"Cancel"},
    {u8"Échanger avec %s", u8"Trade with %s"},
    {u8"Donner à qui ?", u8"Give to which player?"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"Up or Down to choose, then Enter."},
    {u8"Pièces à donner : %lld_", u8"Coins to give: %lld_"},
    {u8"Nourriture à donner : %lld_", u8"Food to give: %lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"Type a number (T: all), then Enter."},
    {u8"Échanger avec %s : les objets de ton rangement", u8"Trade with %s: items from your storage"},
    {u8"16 au plus par don : %u choisis.", u8"Up to 16 per gift: %u chosen."},
    {u8"16 au plus par don : %u choisi.", u8"Up to 16 per gift: %u chosen."},
    {u8"Lecture du rangement...", u8"Reading storage..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"Up/Down: browse, Space: select, Left/Right: how many.\n"
     u8"Type letters to search, then Enter."},
    {u8"Tu n'as rien choisi à donner.", u8"You didn't choose anything to give."},
    {u8"Tu n'as rien à donner pour l'instant.", u8"You have nothing to give right now."},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"Give %s from your purse\n"
     u8"to %s?"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"Give %s\n"
     u8"to %s?"},
    {u8"Envoi du don...", u8"Sending gift..."},
    {u8"Personne ne peut recevoir maintenant.", u8"Nobody can receive right now."},
    {u8"Personne d'autre n'est dans la partie.", u8"Nobody else is in the game."},
    {u8"Valider", u8"Lock In!"},
    {u8"Donner", u8"Give"},
    {u8"Recommencer", u8"Start Over"},
    {u8"Fermer", u8"Close"},
    {u8"Pas d'échange possible ici.", u8"You can't trade here."},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"The window closed: the host entered a room.\n"
     u8"What you chose is kept (E to resume)."},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"Trading on an Adventure\n"
     u8"\n"
     u8"On the map, press E to give coins\n"
     u8"or food from your purse to another player."},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
