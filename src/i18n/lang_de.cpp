// lang_de.cpp -- Mewgenics Multi : les textes du mod et du lanceur en allemand (de).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : le tutoiement (« du »), comme le tutoiement du francais du mod.
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_de[];
const TrPair kTable_de[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"mgmp.json konnte nicht gelesen werden: Einstellungen nicht gespeichert."},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"mgmp.json enthält Kommentare (// oder /* */), die beim Speichern verloren gehen würden: Entferne sie oder ändere die Einstellung von Hand in der Datei."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.json ist kein gültiges JSON: Korrigiere die Datei (oder nimm die aus dem Zip), bevor du die Einstellungen änderst."},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"Der Mod-Ordner ist nicht beschreibbar: Einstellungen nicht gespeichert."},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"mgmp.json konnte nicht ersetzt werden (woanders geöffnet?): Einstellungen nicht gespeichert."},
    {u8"Steam est ouvert.", u8"Steam läuft."},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Ohne Steam schließt sich das Spiel, bevor der Mod geladen ist. Starte Steam und warte, bis es verbunden ist."},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"mgmp.json fehlt neben Mewgenics\u00A0Multi.exe: Speichere die Einstellungen, um die Datei neu anzulegen."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.json ist kein gültiges JSON: Korrigiere die Datei oder nimm die aus dem Zip."},
    {u8"Réglages lus dans mgmp.json.", u8"Einstellungen aus mgmp.json gelesen."},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe nicht gefunden (%s): Klicke in den Einstellungen auf Suchen… und wähle Mewgenics.exe."},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"resources.gpak fehlt neben Mewgenics.exe: Überprüfe die Spieldateien in Steam."},
    {u8"Jeu trouvé : %s.", u8"Spiel gefunden: %s."},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.",
     u8"steam_appid.txt: wird geprüft, sobald das Spiel gefunden ist."},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Fehlt neben Mewgenics.exe: Ohne die Datei startet Steam das Spiel ohne den Mod neu."},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"Die Datei neben Mewgenics.exe enthält nicht 686060: Steam würde das Spiel ohne den Mod neu starten."},
    {u8"steam_appid.txt est en place à côté du jeu.", u8"steam_appid.txt liegt neben dem Spiel."},
    {u8"Fichiers du mod présents.", u8"Mod-Dateien vorhanden."},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"mgmp.dll und mgmp_loader.exe fehlen neben Mewgenics\u00A0Multi.exe: Entpacke den ganzen Ordner erneut aus dem Zip."},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"%s fehlt neben Mewgenics\u00A0Multi.exe: Entpacke den ganzen Ordner erneut aus dem Zip."},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi, Build %s, Protokoll %u: Alle brauchen dieselbe Version."},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"Spiel nicht gefunden: Wähle zuerst Mewgenics.exe in den Einstellungen."},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"steam_appid.txt konnte nicht neben das Spiel geschrieben werden: Kopiere die Datei von Hand (LISEZ-MOI, Schritt 2)."},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"Windows hat deine Zustimmung nicht erhalten: steam_appid.txt wurde nicht kopiert. Versuch es erneut oder kopiere die Datei von Hand (LISEZ-MOI, Schritt 2)."},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics Multi konnte sich nicht als Administrator neu starten: Kopiere steam_appid.txt von Hand (LISEZ-MOI, Schritt 2)."},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Das Kopieren als Administrator hat nicht geklappt: Kopiere steam_appid.txt von Hand (LISEZ-MOI, Schritt 2)."},
    {u8"Le mod n'a pas pu être chargé : %s", u8"Der Mod konnte nicht geladen werden: %s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exe wurde beendet, ohne den Mod zu laden. Versuch es noch einmal; passiert es wieder, schick loader.txt."},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exe wurde beendet (Code %s), ohne den Mod zu laden. Wahrscheinlichste Ursache: Steam ist geschlossen oder getrennt. Starte Steam, warte, bis es verbunden ist, und versuch es dann noch einmal."},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"Das Spiel hat sich beim Start geschlossen, noch vor dem Mod. Häufigste Ursache: Steam ist geschlossen oder getrennt. Starte Steam, warte, bis es verbunden ist, und versuch es dann noch einmal."},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"Steam startet das Spiel neu (das kann ein, zwei Minuten dauern)…"},
    {u8"Ouverture du jeu…", u8"Spiel wird geöffnet…"},
    {u8"Le jeu démarre…", u8"Das Spiel startet…"},
    {u8"Injection du mod dans le jeu…", u8"Mod wird ins Spiel injiziert…"},
    {u8"Le mod est en place, le jeu reprend…", u8"Der Mod ist geladen, das Spiel läuft weiter…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Warte noch auf das von Steam neu gestartete Spiel… Wenn sich Steam gerade erst geöffnet hat, stellt es dir vielleicht eine Frage."},
    {u8"Jeu relancé trouvé : injection du mod…", u8"Spiel neu gestartet: Mod wird injiziert…"},
    {u8"Le jeu est lancé avec le mod.", u8"Das Spiel läuft mit dem Mod."},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.",
     u8"Der Mod hat lange zum Laden gebraucht; das Spiel läuft trotzdem weiter."},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.",
     u8"Das neu gestartete Spiel braucht lange; der Mod wird trotzdem injiziert."},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"mgmp.dll fehlt neben Mewgenics\u00A0Multi.exe: Entpacke den Mod-Ordner erneut aus dem Zip."},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe nicht gefunden: Klicke in den Einstellungen auf Suchen… und wähle Mewgenics.exe."},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"mgmp.json ist nicht lesbar: Korrigiere die Datei oder nimm die aus dem Zip."},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steam hat das Spiel ohne den Mod neu gestartet: Prüfe steam_appid.txt neben Mewgenics.exe (Schaltfläche Beheben) und versuch es dann noch einmal."},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"Dieses Spiel ist nicht die Version, für die der Mod gemacht ist (1.1.21239): Wurde es aktualisiert?"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"Ein Programm (Overlay, Virenscanner, Anti-Cheat) hat das Laden des Mods verhindert."},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"Das Spiel hat zu lange zum Starten gebraucht: Der Mod ist nicht geladen. Versuch es noch einmal."},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"Injizieren fehlgeschlagen, das Spiel wurde geschlossen. Versuch es noch einmal; passiert es wieder, schick loader.txt."},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"In mgmp.json steht nicht, wo das Spiel ist: Klicke in den Einstellungen auf Suchen… und wähle Mewgenics.exe."},
    {u8"build %s · protocole %u", u8"Build %s · Protokoll %u"},
    {u8"Steam ouvert", u8"Steam läuft"},
    {u8"Steam fermé", u8"Steam geschlossen"},
    {u8"Réglages (mgmp.json)", u8"Einstellungen (mgmp.json)"},
    {u8"Jeu trouvé", u8"Spiel gefunden"},
    {u8"Jeu incomplet", u8"Spiel unvollständig"},
    {u8"Jeu introuvable", u8"Spiel nicht gefunden"},
    {u8"Fichiers du mod", u8"Mod-Dateien"},
    {u8"Réparer", u8"Beheben"},
    {u8"Jouer", u8"Spielen"},
    {u8"Règle d'abord les croix rouges.", u8"Kümmere dich zuerst um die roten Kreuze."},
    {u8"Réglages", u8"Einstellungen"},
    {u8"Journaux", u8"Logs"},
    {u8"Langue", u8"Sprache"},
    {u8"Le jeu (Mewgenics.exe)", u8"Spiel (Mewgenics.exe)"},
    {u8"Parcourir…", u8"Suchen…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe und resources.gpak sind da."},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"resources.gpak fehlt daneben: Das ist nicht der richtige Ordner."},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.",
     u8"Hier ist keine Mewgenics.exe: Wähle sie mit Suchen… aus."},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Über Steam spielen (Freundesliste)"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Bei Problemen mit Steam Haken entfernen: Dann bleibt nur die Adresse im Mehrspieler-Menü."},
    {u8"Enregistrer", u8"Speichern"},
    {u8"Retour", u8"Zurück"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"Noch kein Log vorhanden: Der Mod schreibt bei jedem Spiel eines."},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"1 Mod-Log, neben Mewgenics\u00A0Multi.exe."},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"%d Mod-Logs, neben Mewgenics\u00A0Multi.exe."},
    {u8"Le dernier : %s", u8"Das neueste: %s"},
    {u8"écrit le %s", u8"geschrieben am %s"},
    {u8"Ouvrir le dossier", u8"Ordner öffnen"},
    {u8"Copier le dernier journal", u8"Neuestes Log kopieren"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"Um Hilfe zu bitten: Kopiere das neueste Log und füge es mit Strg+V in Discord ein."},
    {u8"Lancement", u8"Start"},
    {u8"Le jeu se lance avec le mod…", u8"Das Spiel startet mit dem Mod…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.",
     u8"Viel Spaß! Dieses Fenster schließt sich von selbst."},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"Details stehen in loader.txt, neben Mewgenics\u00A0Multi.exe."},
    {u8"Réessayer", u8"Nochmal"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"Wenn Steam das Spiel neu startet, kann das ein, zwei Minuten dauern."},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.",
     u8"Log kopiert: Füge es mit Strg+V in Discord ein."},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"Die Zwischenablage konnte nicht befüllt werden: Versuch es erneut oder öffne den Ordner."},
    {u8"Où est Mewgenics.exe ?", u8"Wo ist Mewgenics.exe?"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"Direct2D ist auf diesem PC nicht verfügbar: Mewgenics Multi kann nicht angezeigt werden."},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"Die Grafikkarte konnte nicht vorbereitet werden: Mewgenics Multi kann nicht angezeigt werden."},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"Eigenes Haus, eigene Katzen,\n"
     u8"das Abenteuer zusammen!"},
    {u8"(illustrations du jeu introuvables)", u8"(Spielgrafiken nicht gefunden)"},
    {u8"Comme le jeu (%s)", u8"Wie das Spiel (%s)"},
    {u8"Comme Windows (%s)", u8"Wie Windows (%s)"},
    {u8"Automatique (%s)", u8"Automatisch (%s)"},
    {u8"Lancement annulé.", u8"Start abgebrochen."},
    {u8"Lancement du mod…", u8"Mod wird gestartet…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windows konnte den Start nicht vorbereiten. Starte Mewgenics Multi neu."},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exe konnte nicht starten: Entpacke den Mod-Ordner erneut aus dem Zip (oder dein Virenscanner hat die Datei in Quarantäne verschoben)."},
    {u8"la connexion Steam avec l'hôte est coupée", u8"die Steam-Verbindung zum Host ist unterbrochen"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"der Host antwortet nicht über Steam (Lobby geschlossen oder kein Mod?)"},
    {u8"Steam refuse la connexion", u8"Steam verweigert die Verbindung"},
    {u8"À %s de choisir", u8"%s entscheidet"},
    {u8"À toi de choisir", u8"Du entscheidest"},
    {u8"À %s de choisir la capacité à remplacer.", u8"%s wählt die Fähigkeit, die ersetzt wird."},
    {u8"Ce chat est à %s", u8"Diese Katze gehört %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"Halsbänder wählen\n"
     u8"\n"
     u8"Die gemeinsame Wahl ist abgebrochen (Spielfehler):\n"
     u8"Wähle die Halsbänder für das Team."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"Halsbänder wählen\n"
     u8"\n"
     u8"Bereit! %s weiß, dass du fertig bist,\n"
     u8"und bestätigt den Aufbruch für das ganze Team."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"Halsbänder wählen\n"
     u8"\n"
     u8"%s bestätigt den Aufbruch:\n"
     u8"Sag Bescheid, wenn deine Katzen ihre Halsbänder haben."},
    {u8"%s et %s", u8"%s und %s"},
    {u8"%s, %s", u8"%s, %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Halsbänder wählen\n"
     u8"\n"
     u8"%s wählen noch die Halsbänder aus.\n"
     u8"Klicke noch einmal auf „Fertig!“, um trotzdem\n"
     u8"aufzubrechen."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Halsbänder wählen\n"
     u8"\n"
     u8"%s wählt noch die Halsbänder aus.\n"
     u8"Klicke noch einmal auf „Fertig!“, um trotzdem\n"
     u8"aufzubrechen."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Den Aufbruch von %s siehst du hier nicht mehr.\n"
     u8"Klick dich selbst durch: Du stößt\n"
     u8"auf der Karte dazu."},
    {u8"  %s : parti", u8"  %s: nicht mehr da"},
    {u8"  %s : prêt !", u8"  %s: bereit!"},
    {u8"  %s : charge ta partie...", u8"  %s: lädt dein Spiel..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Dein Team lädt dein Spiel, um mit dir aufzubrechen:\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"Gegenstände wählen\n"
     u8"\n"
     u8"Dein Lager konnte hier nicht geladen werden:\n"
     u8"Deine Katzen brechen mit dem auf, was sie tragen."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Gegenstände wählen\n"
     u8"\n"
     u8"%s sind noch beim Ausrüsten.\n"
     u8"Klicke noch einmal auf „Fertig!“, um trotzdem\n"
     u8"aufzubrechen."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Gegenstände wählen\n"
     u8"\n"
     u8"%s ist noch beim Ausrüsten.\n"
     u8"Klicke noch einmal auf „Fertig!“, um trotzdem\n"
     u8"aufzubrechen."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"Gegenstände wählen\n"
     u8"\n"
     u8"Bereit! Deine Katzen nehmen diese Gegenstände mit:\n"
     u8"%s bestätigt den Aufbruch für das ganze Team."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"Gegenstände wählen\n"
     u8"\n"
     u8"Die Gegenstände für deine Katzen konnten nicht\n"
     u8"ins Abenteuer von %s übernommen werden: Deine\n"
     u8"Katzen brechen mit ihren bisherigen auf, dein Lager\n"
     u8"bleibt unverändert."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"Gegenstände wählen\n"
     u8"\n"
     u8"%s hat den Aufbruch schon bestätigt: Deine Katzen\n"
     u8"nehmen die bisher gewählten Gegenstände mit;\n"
     u8"Weiteres zählt nicht."},
    {u8"Ta bourse : %s.", u8"Dein Beutel: %s."},
    {u8"%s n'est plus dans la partie.", u8"%s ist nicht mehr im Spiel."},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"Du hast in diesem Abenteuer keinen Beutel (du hast keine Katze mitgebracht)."},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"Tauschen geht bei dir zu Hause oder mit deinem Beutel im gemeinsamen Abenteuer. Im Moment: %s."},
    {u8"pas de partie à plusieurs en cours", u8"kein Mehrspieler-Spiel aktiv"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés",
     u8"eine Spielfunktion fehlt, Tauschen zu Hause ist abgeschaltet"},
    {u8"tu es dans la partie de l'hôte", u8"du bist im Spiel des Hosts"},
    {u8"tu n'es pas dans ta maison", u8"du bist nicht zu Hause"},
    {u8"ta save n'est pas chargée", u8"dein Speicherstand ist nicht geladen"},
    {u8"un départ en aventure est en cours", u8"ein Aufbruch ins Abenteuer läuft"},
    {u8"ta maison est attaquée", u8"Angriff auf dein Haus"},
    {u8"ta maison est illisible", u8"dein Haus ist nicht lesbar"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"dein Lager wartet auf das, was deine Katzen aus dem Abenteuer des Hosts mitbringen (sind sie zurück, lade deinen Speicherstand neu: Esc, „Speichern und zurück zum Menü“, dann „Spielen“ und wähle ihn)"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"Gegenstände aus deinem Lager warten noch auf ihren neuen Besitzer (Beute, Einkäufe, NPCs)"},
    {u8"Pas d'échange maintenant : %s.", u8"Gerade kein Tausch: %s."},
    {u8"Ton don précédent attend encore sa réponse.", u8"Dein letztes Geschenk wartet noch auf Antwort."},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"Der Mod kennt deinen Speicherstand noch nicht: Lade ihn übers Menü neu."},
    {u8"Choisis un joueur de la partie.", u8"Wähle einen Mitspieler."},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.",
     u8"Gerade kein Tausch: %s ist nicht zu Hause."},
    {u8"Ce montant n'est pas possible.", u8"Dieser Betrag geht nicht."},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.",
     u8"Wähle Münzen, Futter oder Gegenstände zum Verschenken."},
    {u8"16 objets au plus par don.", u8"Max. 16 Gegenstände pro Geschenk."},
    {u8"Pas d'objets maintenant : %s.", u8"Gerade keine Gegenstände: %s."},
    {u8"Ta maison est illisible : pas de don.", u8"Dein Haus ist unlesbar: kein Geschenk."},
    {u8"Tu n'as pas autant de pièces.", u8"So viele Münzen hast du nicht."},
    {u8"Tu n'as pas autant de nourriture.", u8"So viel Futter hast du nicht."},
    {u8"Ton rangement est illisible : pas de don.", u8"Dein Lager ist nicht lesbar: kein Geschenk."},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"Ein gewählter Gegenstand steckt schon in einem offenen Geschenk."},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"Ein gewählter Gegenstand ist nicht mehr in deinem Lager: Wähle neu."},
    {u8"Un objet choisi ne peut pas se donner.", u8"Ein Gegenstand ist nicht verschenkbar."},
    {u8"Le don n'a pas pu partir (connexion).", u8"Geschenk nicht gesendet (Verbindung)."},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…",
     u8"Geschenk an %s gesendet (%s): Warte auf Antwort…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…",
     u8"Geschenk beim Host angefragt: Warte auf Antwort…"},
    {u8"Pas de don : %s.", u8"Kein Geschenk: %s."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%s hat dein Geschenk erhalten: Es verlässt dein Lager, sobald der Gegenstandsbildschirm zu ist."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%s hat dein Geschenk erhalten: Deine Gegenstände verlassen dein Lager so bald wie möglich (nach dem Aufbruch, der Verteidigung deines Hauses oder der Rückkehr deiner Katzen)."},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%s hat dein Geschenk erhalten: Es verlässt dein Haus, wenn du in diesen Speicherstand zurückkehrst."},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"Keine Antwort von %s aufs Geschenk: Nichts hat dein Haus verlassen."},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"Keine Antwort von %s aufs Geschenk: Nichts hat dein Haus verlassen."},
    {u8"Les échanges marchent pendant une partie à plusieurs.",
     u8"Tauschen geht nur in einem Mehrspieler-Spiel."},
    {u8"Dans ta maison : %s.", u8"Bei dir zu Hause: %s."},
    {u8"%s n'est pas dans sa maison.", u8"%s ist nicht zu Hause."},
    {u8"%lld pièces", u8"%lld\u00A0Münzen"},
    {u8"%lld pièce", u8"%lld\u00A0Münze"},
    {u8"%lld nourritures", u8"%lld\u00A0Futter"},
    {u8"%lld nourriture", u8"%lld\u00A0Futter"},
    {u8"rien", u8"nichts"},
    {u8"%s n'est pas dans sa maison", u8"%s ist nicht zu Hause"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s",
     u8"bei %s ist ein Fenster oder der Gegenstandsbildschirm offen"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"bei %s ließen sie sich nicht einlagern"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"bei %s ließen sie sich nicht einlagern"},
    {u8"les dons ne marchent pas chez %s", u8"Geschenke funktionieren bei %s nicht"},
    {u8"%s n'est plus dans la partie", u8"%s ist nicht mehr im Spiel"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%s hat noch Katzen im Abenteuer des Hosts (sein Lager wartet auf ihre Rückkehr)"},
    {u8"raison inconnue", u8"unbekannter Grund"},
    {u8"Cadeau d'%s", u8"Geschenk von %s"},
    {u8"Cadeau de %s", u8"Geschenk von %s"},
    {u8"%s t'a donné %s.", u8"%s hat dir %s geschenkt."},
    {u8"C'est dans ta maison, et ta save est à jour.",
     u8"Es ist bei dir zu Hause, dein Speicherstand ist aktuell."},
    {u8"C'est dans ta maison.", u8"Es ist bei dir zu Hause."},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"%s hat %s doch noch erhalten: Es hat dein Haus verlassen."},
    {u8"%s a bien reçu %s : c'est parti de ta maison.",
     u8"%s hat %s erhalten: Es hat dein Haus verlassen."},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"(1 Gegenstand blieb auch in deinem Lager.)"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"(%u Gegenstände blieben auch in deinem Lager.)"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"Das Geschenk an %s ging nicht durch: %s. Nichts hat dein Haus verlassen."},
    {u8"Rien à donner dans ton rangement.", u8"Im Lager ist nichts zu verschenken."},
    {u8"Tape des lettres pour chercher un objet.", u8"Tippe Buchstaben, um einen Gegenstand zu suchen."},
    {u8"Recherche : %s_", u8"Suche: %s_"},
    {u8"Aucun objet ne correspond.", u8"Kein Gegenstand passt."},
    {u8"(page %u sur %u)", u8"(Seite %u von %u)"},
    {u8"Butin : %s.", u8"Beute: %s."},
    {u8"Besoin", u8"Bedarf"},
    {u8"Cupidité", u8"Gier"},
    {u8"Passer", u8"Passen"},
    {u8"%u s", u8"%u s"},
    {u8"toi", u8"du"},
    {u8"Joueur %u", u8"Spieler %u"},
    {u8"Un objet", u8"Ein Gegenstand"},
    {u8"Partage du butin : objet %u sur %u", u8"Beute: Gegenstand %u von %u"},
    {u8"Besoin : %s", u8"Bedarf: %s"},
    {u8"Cupidité : %s", u8"Gier: %s"},
    {u8"Passer : %s", u8"Passen: %s"},
    {u8"Partage du butin", u8"Beuteverteilung"},
    {u8"%u objets du sac à partager entre %s.", u8"%u Gegenstände. Mit dabei: %s."},
    {u8"%u objet du sac à partager entre %s.", u8"%u Gegenstand. Mit dabei: %s."},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"Für jeden Gegenstand: Bedarf (du brauchst ihn), Gier (wenn\n"
     u8"ihn niemand braucht) oder Passen. Ein Wurf von 1 bis 100 entscheidet."},
    {u8"En attente de %s...", u8"Warte auf %s..."},
    {u8"En attente de %s... (%s)", u8"Warte auf %s... (%s)"},
    {u8"Tout le monde est là : ça commence !", u8"Alle sind da: Es geht los!"},
    {u8"Commencer", u8"Starten"},
    {u8"D'accord", u8"OK"},
    {u8"Le veux-tu ? (%s)", u8"Willst du ihn? (%s)"},
    {u8"Je le veux", u8"Ich will ihn"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"Bedarf: Du brauchst ihn, du hast Vorrang.\n"
     u8"Gier: Du nimmst ihn, wenn ihn niemand braucht.\n"
     u8"(%s)"},
    {u8"Ton choix : %s.", u8"Deine Wahl: %s."},
    {u8"Ton choix est fait.", u8"Du hast gewählt."},
    {u8"En attente de %s (%s).", u8"Warte auf %s (%s)."},
    {u8"Le résultat arrive...", u8"Das Ergebnis kommt..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s sind nicht da und setzen aus."},
    {u8"%s n'est pas là : son tour passe.", u8"%s ist nicht da und setzt aus."},
    {u8"Personne n'en veut : il reste chez toi.", u8"Niemand will ihn: Er bleibt bei dir."},
    {u8"Personne n'en veut : il reste chez %s.", u8"Niemand will ihn: Er bleibt bei %s."},
    {u8"Tu l'emportes (%s) !", u8"Du bekommst ihn (%s)!"},
    {u8"%s l'emporte (%s).", u8"%s bekommt ihn (%s)."},
    {u8"Dés : %s (égalité départagée).", u8"Würfel: %s (Gleichstand aufgelöst)."},
    {u8"Dés : %s.", u8"Würfel: %s."},
    {u8"Partage du butin terminé", u8"Beuteverteilung beendet"},
    {u8"Tu remportes : %s.", u8"Du bekommst: %s."},
    {u8"%s remporte : %s.", u8"%s bekommt: %s."},
    {u8"Remporté, mais resté chez %s : %s.", u8"Gewonnen, aber noch bei %s: %s."},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"Wollte niemand (bleiben bei dir): %s."},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"Wollte niemand (bleiben bei %s): %s."},
    {u8"Aucun objet n'a été partagé.", u8"Kein Gegenstand wurde verteilt."},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"Dein Speicherstand ist schon offen: Das nächste\n"
     u8"Fenster sagt dir, wie du ihn mit der Beute neu lädst."},
    {u8"Ton butin t'attend dans ton rangement, chez toi.",
     u8"Deine Beute wartet zu Hause in deinem Lager."},
    {u8"Ce que les autres remportent quitte ton rangement.",
     u8"Was die anderen bekommen, verlässt dein Lager."},
    {u8"Dr Beanies", u8"Dr. Beanies"},
    {u8"Butch", u8"Butch"},
    {u8"Tink", u8"Tink"},
    {u8"Frank", u8"Frank"},
    {u8"Bébé Jack", u8"Baby Jack"},
    {u8"Tracy", u8"Tracy"},
    {u8"Broyeur d'organe", u8"Leichensammler"},
    {u8"Steven", u8"Steven"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"Belohnung bereit\n"
     u8"\n"
     u8"Die Katze, die %s an %s gab, hat\n"
     u8"die Leiste gefüllt. Die Belohnung wird\n"
     u8"für ihn abgeholt, sobald du zu Hause bist:\n"
     u8"Münzen, Futter und Gegenstände an %s,\n"
     u8"Freischaltungen an alle."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"Belohnung von %s\n"
     u8"\n"
     u8"Die Belohnung für %s ließ sich nicht\n"
     u8"senden: Sie bleibt bei dir."},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"Belohnung für %s\n"
     u8"\n"
     u8"%s hat die Katze von %s belohnt:\n"
     u8"%s gehen zu ihm nach Hause.\n"
     u8"Die Freischaltungen sind für alle."},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Belohnung für alle\n"
     u8"\n"
     u8"%s hat die Katze von %s belohnt:\n"
     u8"Die Freischaltungen sind\n"
     u8"für alle."},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s dankt dir für deine Katze: %s\n"
     u8"(die Freischaltungen sind für alle)."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Belohnung von %s\n"
     u8"\n"
     u8"%s dankt dir für deine Katze: %s.\n"
     u8"Sie kommen in dein Haus, sobald dein\n"
     u8"Speicherstand neu geladen wird (nach dem\n"
     u8"Abenteuer). Die Freischaltungen sind\n"
     u8"für alle."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"Belohnung von %s\n"
     u8"\n"
     u8"Deine Katze hat die Leiste von %s gefüllt:\n"
     u8"Die Belohnung sind Freischaltungen\n"
     u8"für alle. Sie kommen beim nächsten\n"
     u8"Laden in deinen Speicherstand."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"Belohnung von %s\n"
     u8"\n"
     u8"%s hat die Leiste von %s gefüllt,\n"
     u8"ist aber nicht mehr da: Die Belohnung bleibt bei dir."},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"Belohnung bereit\n"
     u8"\n"
     u8"Deine Katze füllte die Leiste von %s!\n"
     u8"%s holt die Belohnung für dich ab,\n"
     u8"sobald er zu Hause ist."},
    {u8"les bourses ne marchent pas dans cette partie", u8"Beutel funktionieren in diesem Spiel nicht"},
    {u8"rien à donner", u8"nichts zu verschenken"},
    {u8"c'est trop d'un coup", u8"das ist zu viel auf einmal"},
    {u8"choisis un autre joueur", u8"wähle einen anderen Spieler"},
    {u8"ton don précédent attend encore la réponse de l'hôte",
     u8"dein letztes Geschenk wartet noch auf die Antwort des Hosts"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"du hast in diesem Abenteuer keinen Beutel"},
    {u8"envoi impossible (connexion)", u8"Senden nicht möglich (Verbindung)"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"Ende des Abenteuers\n"
     u8"\n"
     u8"Jeder bringt seinen Beutel mit nach Hause:"},
    {u8"%s n'est plus là : sa bourse reste chez toi.",
     u8"%s ist nicht mehr da: Sein Beutel bleibt bei dir."},
    {u8"%s : rien.", u8"%s: nichts."},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.",
     u8"%s konnte seinen Beutel nicht empfangen: Er bleibt bei dir."},
    {u8"%s : %s.", u8"%s: %s."},
    {u8"Toi : %s.", u8"Du: %s."},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"Noch keine Antwort des Hosts auf dein Geschenk: Sieh in der Wegwahl nach deinem Beutel."},
    {u8"Ton don n'est pas passé : %s.", u8"Dein Geschenk ging nicht durch: %s."},
    {u8"(et d'autres trouvailles)", u8"(und weitere Funde)"},
    {u8"pas de bourse pour ce joueur dans cette aventure",
     u8"kein Beutel für diesen Spieler in diesem Abenteuer"},
    {u8"ta bourse n'en a pas autant", u8"so viel hat dein Beutel nicht"},
    {u8"l'aventure n'est plus suivie", u8"das Abenteuer ist nicht mehr aktiv"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s hat dir %s in den Beutel gelegt."},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.",
     u8"Du hast %s an %s gegeben: Es ist in seinem Beutel."},
    {u8"%s a donné %s à %s.", u8"%s hat %s an %s gegeben."},
    {u8"Un chat", u8"Eine Katze"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%s hat %s gefunden und %s verloren: Das zählt für deinen Beutel."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s hat %s gefunden und %s verloren: Das zählt für den Beutel von %s."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s hat %s gefunden und %s verloren: Das zählt für den Beutel von %s."},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s hat %s für deinen Beutel gefunden."},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s hat %s für den Beutel von %s gefunden."},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s hat %s für den Beutel von %s gefunden."},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s hat %s ausgegeben: Das zahlt dein Beutel."},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.",
     u8"%s hat %s ausgegeben: Das zahlt der Beutel von %s."},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.",
     u8"%s hat %s ausgegeben: Das zahlt der Beutel von %s."},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s hat %s verloren: Das fehlt in deinem Beutel."},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.",
     u8"%s hat %s verloren: Das fehlt im Beutel von %s."},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.",
     u8"%s hat %s verloren: Das fehlt im Beutel von %s."},
    {u8"%s n'a rien trouvé.", u8"%s hat nichts gefunden."},
    {u8"un objet", u8"etwas"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.",
     u8"Du hast %s für %s gekauft: Das zahlt dein Beutel."},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.",
     u8"%s hat %s für %s gekauft: Das zahlt sein Beutel."},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"Du hast %s gekauft: %s mehr für deinen Beutel."},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%s hat %s gekauft: %s mehr für seinen Beutel."},
    {u8"Tu as acheté %s (gratuit).", u8"Du hast %s gekauft (gratis)."},
    {u8"%s a acheté %s (gratuit).", u8"%s hat %s gekauft (gratis)."},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"Im Kampf: %s gefunden, %s verloren. Das zählt für deinen Beutel."},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"Im Kampf fanden deine Katzen %s: Das kommt in deinen Beutel."},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"Im Kampf verloren deine Katzen %s: Das fehlt in deinem Beutel."},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"Im Kampf fanden deine Katzen nichts."},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"Im Kampf fanden die Katzen von %s %s und verloren %s: Das zählt für seinen Beutel."},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"Im Kampf fanden die Katzen von %s %s und verloren %s: Das zählt für seinen Beutel."},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"Im Kampf fanden die Katzen von %s %s: Das kommt in seinen Beutel."},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"Im Kampf fanden die Katzen von %s %s: Das kommt in seinen Beutel."},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"Im Kampf verloren die Katzen von %s %s: Das fehlt in seinem Beutel."},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"Im Kampf verloren die Katzen von %s %s: Das fehlt in seinem Beutel."},
    {u8"En combat, les chats d'%s n'ont rien trouvé.", u8"Im Kampf fanden die Katzen von %s nichts."},
    {u8"En combat, les chats de %s n'ont rien trouvé.", u8"Im Kampf fanden die Katzen von %s nichts."},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"Dein Rucksack ist offen\n"
     u8"\n"
     u8"Der Host hat ein Feld betreten:\n"
     u8"Schließ ihn, um dem Host zu folgen."},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"Gegenstände\n"
     u8"\n"
     u8"Diese Katze gehört %s: Nur der Besitzer\n"
     u8"kann ihre Gegenstände ändern."},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"Gegenstände\n"
     u8"\n"
     u8"Solange ihr in einem Feld seid (Shop, Level-up...),\n"
     u8"ändert nur der Host die Gegenstände.\n"
     u8"Rüste deine Katzen auf der Karte aus."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"Gegenstände\n"
     u8"\n"
     u8"Der Host ist noch nicht zurück auf der Karte:\n"
     u8"Warte auf ihn, dein Rucksack wird von selbst frei."},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"Gegenstände\n"
     u8"\n"
     u8"Dein Rucksack wird mit dem des Hosts abgeglichen:\n"
     u8"Schließ ihn und öffne ihn gleich wieder."},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"Gegenstände\n"
     u8"\n"
     u8"Jemand hat den Gegenstand vor dir genommen:\n"
     u8"Deine Änderung zählt nicht.\n"
     u8"Beim Schließen wird dein Rucksack wie beim Host."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"Gegenstände\n"
     u8"\n"
     u8"Der Host hat ein Feld betreten, bevor deine\n"
     u8"letzte Änderung ankam: Sie wurde verworfen.\n"
     u8"Mach sie auf der Karte noch einmal."},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"Gegenstände\n"
     u8"\n"
     u8"Dieser Gegenstand ist weg: Ein anderer Spieler\n"
     u8"hat ihn kurz vor dir genommen."},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).",
     u8"%s ist nicht mehr da: Seine Einkäufe bleiben bei dir (%s)."},
    {u8"%s rapporte ses achats : %s.", u8"%s bringt seine Einkäufe mit: %s."},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Die Einkäufe von %s konnten nicht gesendet werden: Sie bleiben bei dir."},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Die Einkäufe von %s konnten nicht gesendet werden: Sie bleiben bei dir."},
    {u8"Achats de l'aventure", u8"Einkäufe im Abenteuer"},
    {u8"Boutique", u8"Shop"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s sind in diesem Shop noch\n"
     u8"nicht fertig: Warte auf sie (draußen könntest du\n"
     u8"nichts mehr für sie kaufen).\n"
     u8"Klicke noch einmal, um trotzdem zu gehen."},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s ist in diesem Shop noch\n"
     u8"nicht fertig: Warte auf ihn (draußen könntest du\n"
     u8"nichts mehr für ihn kaufen).\n"
     u8"Klicke noch einmal, um trotzdem zu gehen."},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"Dein Kauf konnte nicht beim Host angefragt werden:\n"
     u8"Nichts wurde gekauft. Versuch es gleich noch einmal."},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Der Host ist nicht mehr im Shop:\n"
     u8"Der Kauf hat nicht geklappt."},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Der Gegenstand ist beim Host nicht mehr zu haben:\n"
     u8"Der Kauf hat nicht geklappt."},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Das wurde gerade verkauft:\n"
     u8"Der Kauf hat nicht geklappt."},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"Er kostet %s; dein Beutel hat %lld."},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"Dein Beutel reicht für diesen Gegenstand nicht."},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"(Jeder zahlt mit seinem Beutel, du auch.)"},
    {u8"(Chacun paie avec sa bourse.)", u8"(Jeder zahlt mit seinem Beutel.)"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"Vorerst kauft nur der Host Bonbons\n"
     u8"und Begleiter (mit seinem Beutel)."},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.",
     u8"Das Spiel konnte den Kauf beim Host nicht ausführen."},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"Keine Beutel in diesem Abenteuer:\n"
     u8"Nur der Host kauft."},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"Ohne eigene Katze in diesem\n"
     u8"Abenteuer hast du keinen Beutel\n"
     u8"zum Einkaufen."},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"Du hast in diesem Abenteuer keinen Beutel\n"
     u8"(keine Katzen mitgebracht): kein Einkauf."},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"Der Host kann gerade nicht für dich kaufen\n"
     u8"(bei ihm ist ein Fenster offen):\n"
     u8"Nichts wurde gekauft. Versuch es gleich noch einmal."},
    {u8"un bonbon", u8"ein Bonbon"},
    {u8"un familier", u8"einen Begleiter"},
    {u8"un article", u8"einen Artikel"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"Ende des Abenteuers\n"
     u8"\n"
     u8"%s gehen nach Hause zu %s."},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"Ende des Abenteuers\n"
     u8"\n"
     u8"%s geht nach Hause zu %s."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Die Katzen der anderen Spieler konnten nicht aufbrechen\n"
     u8"und deine Abenteuerbox ist leer: diesmal kein Abenteuer.\n"
     u8"(Das Mod-Log, mgmp.log, sagt, warum.)"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Deine Abenteuerbox ist leer und niemand\n"
     u8"bringt eine Katze mit: kein Abenteuer."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"Die Katzen der anderen Spieler konnten dem\n"
     u8"Team nicht beitreten: Du brichst mit deinen auf.\n"
     u8"(Das Mod-Log, mgmp.log, sagt, warum.)"},
    {u8"Retour de l'aventure", u8"Zurück vom Abenteuer"},
    {u8"%s est de retour à la maison.", u8"%s ist wieder zu Hause."},
    {u8"%s sont de retour à la maison.", u8"%s sind wieder zu Hause."},
    {u8"%s a péri pendant l'aventure.", u8"%s ist im Abenteuer gestorben."},
    {u8"%s ont péri pendant l'aventure.", u8"%s sind im Abenteuer gestorben."},
    {u8"%s ne revient pas de l'aventure.", u8"%s kommt nicht zurück."},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s kommen nicht zurück."},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s ist seit dem Aufbruch verändert:\n"
     u8"Der Stand nach dem Abenteuer wurde nicht übernommen."},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s sind seit dem Aufbruch verändert:\n"
     u8"Ihr Stand nach dem Abenteuer wurde nicht übernommen."},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"Dein Lager hat sich seit dem Aufbruch geändert:\n"
     u8"Die beim Aufbruch ausgerüsteten Gegenstände wurden nicht entfernt."},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"Dein Lager wurde nicht geändert: Die Ausrüstung\n"
     u8"vom Aufbruch ist noch darin."},
    {u8"Tu rapportes %s dans ta maison.", u8"Du bringst %s mit nach Hause."},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.",
     u8"Die NPCs danken dir für deine Katzen: %s bei dir zu Hause."},
    {u8"Cadeaux des PNJ", u8"Geschenke der NPCs"},
    {u8"Ton butin est dans ton rangement : %s.", u8"Deine Beute ist in deinem Lager: %s."},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.",
     u8"Die Geschenke der NPCs sind in deinem Lager: %s."},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.",
     u8"%s, im Abenteuer rekrutiert, ziehen bei dir ein."},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.",
     u8"%s, im Abenteuer rekrutiert, zieht bei dir ein."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"%s schafften es nicht ins Team:\n"
     u8"In diesem Abenteuer steuerst du einen Teil\n"
     u8"der Katzen im Team."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"%s schaffte es nicht ins Team:\n"
     u8"In diesem Abenteuer steuerst du einen Teil\n"
     u8"der Katzen im Team."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"%s schafften es nicht ins Team:\n"
     u8"Du bist ohne eigene Katze dabei."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"%s schaffte es nicht ins Team:\n"
     u8"Du bist ohne eigene Katze dabei."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"%s konnten nicht aufbrechen und bleiben zu Hause."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"%s konnte nicht aufbrechen und bleibt zu Hause."},
    {u8"Tes chats sont rentrés de l'aventure.", u8"Deine Katzen sind wieder zu Hause."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"Neue Katze im Team\n"
     u8"\n"
     u8"%s ist neu im Team von %s: Die Katze\n"
     u8"gehört dir! Bis du da bist, steuert die\n"
     u8"Spiel-KI sie im Kampf und %s wählt für sie;\n"
     u8"nach dem Abenteuer zieht sie bei dir ein."},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"Neue Katzen: %s ziehen bei dir ein."},
    {u8"Nouveau chat : %s arrive chez toi.", u8"Neue Katze: %s zieht bei dir ein."},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"Der 4. Platz\n"
     u8"\n"
     u8"Bei 3 Spielern ist ein Platz frei:\n"
     u8"Er gehört dir, du nimmst 2 Katzen mit."},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"Der 4. Platz\n"
     u8"\n"
     u8"Der freie Platz gehört schon %s:\n"
     u8"Bei 3 Spielern nimmst du 1 Katze mit.\n"
     u8"Nimm eine aus deiner Box heraus."},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"Der 4. Platz\n"
     u8"\n"
     u8"Ein vierter Spieler ist da: Jeder nimmt 1 Katze\n"
     u8"mit. Nimm eine Katze aus deiner Abenteuerbox heraus."},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"Der 4. Platz geht an den Ersten, der eine 2. Katze in seine Box setzt."},
    {u8"La 4e place est à %s.", u8"Der 4. Platz gehört %s."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"Bei %d Spielern nimmst du höchstens %d Katzen mit:\n"
     u8"Nimm %d Katzen aus deiner Abenteuerbox heraus.%s\n"
     u8"Die anderen füllen das Team auf."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"Bei %d Spielern nimmst du höchstens %d Katzen mit:\n"
     u8"Nimm %d Katze aus deiner Abenteuerbox heraus.%s\n"
     u8"Die anderen füllen das Team auf."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"Bei %d Spielern nimmst du höchstens %d Katze mit:\n"
     u8"Nimm %d Katzen aus deiner Abenteuerbox heraus.%s\n"
     u8"Die anderen füllen das Team auf."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Gemischtes Team\n"
     u8"\n"
     u8"Bei %d Spielern nimmst du höchstens %d Katze mit:\n"
     u8"Nimm %d Katze aus deiner Abenteuerbox heraus.%s\n"
     u8"Die anderen füllen das Team auf."},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Du bist nicht zu Hause:\n"
     u8"Du spielst mit Katzen der anderen."},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Deine Abenteuerbox ist noch gesperrt:\n"
     u8"Du spielst mit Katzen der anderen."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Deine Abenteuerbox ist leer: Du spielst mit den Katzen der anderen.\n"
     u8"Um deine mitzunehmen: Wähle „Noch nicht“, setz 1 oder 2 in deine Box\n"
     u8"(bei Butch), dann klick auf dein Schild „Aufbrechen!“."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Deine Abenteuerbox ist leer: Du spielst mit den Katzen der anderen.\n"
     u8"Um deine mitzunehmen: Wähle „Noch nicht“, setz eine in deine Box\n"
     u8"(bei Butch), dann klick auf dein Schild „Aufbrechen!“."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"In deiner Abenteuerbox sind %d Katzen, erlaubt sind %d:\n"
     u8"Antworte „Noch nicht“, nimm %d heraus,\n"
     u8"dann klick auf dein Schild „Aufbrechen!“.%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).",
     u8"Deine Katzen brechen aus deiner Abenteuerbox auf (bei Butch)."},
    {u8"Tu emmènes %s.", u8"Du nimmst %s mit."},
    {u8"Départ en aventure", u8"Aufbruch ins Abenteuer"},
    {u8"La session n'est pas prête : réessaie dans un instant.",
     u8"Die Sitzung ist nicht bereit: Versuch es gleich noch einmal."},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"Der 4. Platz wird dir gerade zugeteilt:\n"
     u8"Die Frage kommt gleich wieder."},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"Deine Katzen konnten nicht\n"
     u8"vorbereitet werden (siehe mgmp.log)."},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s kommen aus einem Abenteuer zurück, aber ihre\n"
     u8"Rückkehr ist noch nicht in deinem Speicherstand:\n"
     u8"Lade ihn neu, bevor sie wieder aufbrechen („Neu laden“\n"
     u8"wird dir danach angeboten; sonst Esc, „Speichern und\n"
     u8"zurück zum Menü“, dann „Spielen“ und wähle ihn)."},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s kommt aus einem Abenteuer zurück, aber seine\n"
     u8"Rückkehr ist noch nicht in deinem Speicherstand:\n"
     u8"Lade ihn neu, bevor er wieder aufbricht („Neu laden“\n"
     u8"wird dir danach angeboten; sonst Esc, „Speichern und\n"
     u8"zurück zum Menü“, dann „Spielen“ und wähle ihn)."},
    {u8"Le retour de %s attend d'être recopié.", u8"%s: Rückkehr noch nicht übernommen."},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"Deine Antwort an den Host kam nicht an:\n"
     u8"Versuch's neu."},
    {u8"Ta bourse de l'aventure : %s.", u8"Dein Beutel aus dem Abenteuer: %s."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"In deiner Abenteuerbox sind %d Katzen, erlaubt sind %d:\n"
     u8"Nimm %d heraus, dann klick auf dein Schild „Aufbrechen!“."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"Neue Katze im Team\n"
     u8"\n"
     u8"%s ist neu im Team: Sie gehört dir!\n"
     u8"Du steuerst sie, und nach dem\n"
     u8"Abenteuer zieht sie bei dir ein."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"Neue Katze im Team\n"
     u8"\n"
     u8"%s ist neu im Team: Sie gehört dir."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"Neue Katze im Team\n"
     u8"\n"
     u8"%s ist neu im Team: Sie gehört %s."},
    {u8"Un nouveau chat", u8"Eine neue Katze"},
    {u8"en bas", u8"unten"},
    {u8"en haut", u8"oben"},
    {u8"à droite", u8"rechts"},
    {u8"à gauche", u8"links"},
    {u8"au milieu", u8"in der Mitte"},
    {u8"n°%d", u8"Nr. %d"},
    {u8"%s (%s)", u8"%s (%s)"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]Deine Stimme wurde nicht gezählt: %s.[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"Wegwahl\n"
     u8"\n"},
    {u8"Même chemin", u8"Gleicher Weg"},
    {u8"Choisir", u8"Wählen"},
    {u8"Changer", u8"Ändern"},
    {u8"Sortie", u8"Ausgang"},
    {u8"Retour à la maison", u8"Rückkehr nach Hause"},
    {u8"Combat", u8"Kampf"},
    {u8"Combat difficile", u8"Schwerer Kampf"},
    {u8"Mini-boss", u8"Mini-Boss"},
    {u8"Boss", u8"Boss"},
    {u8"Événement", u8"Ereignis"},
    {u8"Événement spécial", u8"Besonderes Ereignis"},
    {u8"Trésor", u8"Schatz"},
    {u8"Meuble", u8"Möbel"},
    {u8"Nourriture", u8"Futter"},
    {u8"Bonus", u8"Bonus"},
    {u8"Case vide", u8"Leeres Feld"},
    {u8"Case", u8"Feld"},
    {u8"l'hôte n'est pas sur la carte", u8"der Host ist nicht auf der Karte"},
    {u8"vos cartes ne sont pas les mêmes", u8"eure Karten sind nicht gleich"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"der Host kann nicht auf dieses Feld"},
    {u8"le chemin est déjà choisi", u8"der Weg ist schon gewählt"},
    {u8"ce tour est fini", u8"diese Runde ist vorbei"},
    {u8"l'hôte ne l'a pas pris", u8"der Host hat sie nicht angenommen"},
    {u8"pas de vote", u8"keine Stimme"},
    {u8"absent, ne vote pas", u8"abwesend, stimmt nicht ab"},
    {u8"pas encore sur la carte", u8"noch nicht auf der Karte"},
    {u8"pas encore voté", u8"noch nicht abgestimmt"},
    {u8"%s (toi) : %s", u8"%s (du): %s"},
    {u8"%s : %s", u8"%s: %s"},
    {u8"En pause : on attend tout le monde sur la carte.",
     u8"Pause: Wir warten, bis alle auf der Karte sind."},
    {u8"À toi de choisir : encore %u s.", u8"Du bist dran: noch %u s."},
    {u8"En attente des autres : encore %u s.", u8"Warte auf die anderen: noch %u s."},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"Gleichstand zwischen %s:\n"
     u8"Das Los fällt auf %s."},
    {u8"Tout le monde a choisi %s.", u8"Alle haben %s gewählt."},
    {u8"%s l'emporte.", u8"%s gewinnt."},
    {u8"%s : prêt", u8"%s: bereit"},
    {u8"%s : défend sa maison", u8"%s: verteidigt sein Haus"},
    {u8"%s : pas encore", u8"%s: noch nicht"},
    {u8"Tout le monde est prêt !", u8"Alle sind bereit!"},
    {u8"En attente d'un joueur.", u8"Warte auf einen Spieler."},
    {u8"En attente de %d joueurs.", u8"Warte auf %d Spieler."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"Tag beenden\n"
     u8"\n"
     u8"Du bist bereit für die Nacht.\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"Dein Schild steht auf Aufbruch ins Abenteuer:\n"
     u8"Nimm deine Katzen aus dem Team, um den Tag zu beenden."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Tag beenden\n"
     u8"\n"
     u8"1 Spieler wartet auf die Nacht.\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Tag beenden\n"
     u8"\n"
     u8"%d Spieler warten auf die Nacht.\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1 Spieler wartet"},
    {u8"%d joueurs attendent", u8"%d Spieler warten"},
    {u8"Passer la nuit", u8"Tag beenden"},
    {u8"Plus tard", u8"Später"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"Um weiterzumachen, drück Esc\n"
     u8"und dann „Speichern und zurück zum Menü“."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"Das wartet auf dich\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Dein Speicherstand ist schon offen:\n"
     u8"Das alles kommt erst beim nächsten Laden.\n"
     u8"„Neu laden“: Der Mod öffnet ihn übers Menü neu."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"Das wartet auf dich\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Dein Speicherstand ist schon offen: Das alles\n"
     u8"kommt erst beim nächsten Laden. Lade ihn neu:\n"
     u8"Esc, „Speichern und zurück zum Menü“,\n"
     u8"dann „Spielen“ und wähle ihn."},
    {u8"%s : j'arrive !", u8"%s: Ich komme!"},
    {u8"%s : réfléchit...", u8"%s: überlegt..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Du willst ins Abenteuer aufbrechen.\n"
     u8"Dein Team antwortet:\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s: noch nicht da"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"Warte auf dein Team\n"
     u8"\n"
     u8"%s\n"
     u8"Gehst du ohne zu warten weiter, steuert die Spiel-KI\n"
     u8"die Katzen der anderen in den Kämpfen, bis sie\n"
     u8"ankommen."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"%s bricht ins Abenteuer auf!\n"
     u8"Jeder nimmt seine Katzen mit und steuert sie.\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"Laufendes Abenteuer\n"
     u8"\n"
     u8"%s ist im Abenteuer!\n"
     u8"Deine Katzen sind nicht dabei: Du begleitest\n"
     u8"das Abenteuer ohne eigene Katze."},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"Laufendes Abenteuer\n"
     u8"\n"
     u8"%s ist mit deinen Katzen unterwegs!\n"
     u8"Bis du da bist, steuert die Spiel-KI sie\n"
     u8"im Kampf und %s wählt für sie.\n"
     u8"Seinem Team beitreten?"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"Laufendes Abenteuer\n"
     u8"\n"
     u8"%s ist im Abenteuer!\n"
     u8"Seinem Team beitreten?"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"Abenteuer verlassen\n"
     u8"\n"
     u8"Das Abenteuer von %s ist vorbei:\n"
     u8"Du kehrst nach Hause zurück."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"Abenteuer verlassen\n"
     u8"\n"
     u8"Nur %s kann dieses Abenteuer\n"
     u8"abbrechen. Es geht ohne dich weiter.\n"
     u8"\n"
     u8"Nach Hause gehen? Um zurückzukommen,\n"
     u8"klick auf ein Schild in deinem Haus."},
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
     u8"Abenteuer verlassen\n"
     u8"\n"
     u8"Nur %s kann dieses Abenteuer\n"
     u8"abbrechen. Es geht ohne dich weiter,\n"
     u8"deine Katzen bleiben in seinem Team:\n"
     u8"Die Spiel-KI steuert sie im Kampf, am Ende\n"
     u8"kommen sie zu dir.\n"
     u8"\n"
     u8"Nach Hause gehen? Um zurückzukommen,\n"
     u8"klick auf ein Schild in deinem Haus."},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"Abenteuer beitreten\n"
     u8"\n"
     u8"Das Spiel von %s schickt dir das Abenteuer.\n"
     u8"Hat er gerade eine Wahl auf einem Feld getroffen\n"
     u8"(Ereignis, Level-up), kommst du dazu, sobald\n"
     u8"er zurück auf der Karte ist."},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"Auf ins Abenteuer\n"
     u8"\n"
     u8"Das Spiel von %s wird geladen:\n"
     u8"Du brichst mit ihm auf."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Rückkehr nach Hause\n"
     u8"\n"
     u8"Verbindung zu %s unterbrochen.\n"
     u8"Du kehrst nach Hause zurück."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"Rückkehr nach Hause\n"
     u8"\n"
     u8"Der Aufbruch mit %s fand nicht statt.\n"
     u8"Du kehrst nach Hause zurück."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Rückkehr nach Hause\n"
     u8"\n"
     u8"Das Abenteuer von %s ist vorbei.\n"
     u8"Du kehrst nach Hause zurück."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Rückkehr nach Hause\n"
     u8"\n"
     u8"Der Kampf von %s war vorbei, deine\n"
     u8"Kopie lief aber weiter: Sie war\n"
     u8"desynchronisiert, niemand spielte mehr.\n"
     u8"Du bist zurück; deine Katzen bleiben in seinem Team.\n"
     u8"\n"
     u8"Um wieder ins Abenteuer zu kommen,\n"
     u8"klick auf ein Schild in deinem Haus."},
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
     u8"Rückkehr nach Hause\n"
     u8"\n"
     u8"Dein Kampf war vorbei, der von %s\n"
     u8"lief aber weiter: Deine Kopie war\n"
     u8"desynchronisiert. Du bist zurück;\n"
     u8"deine Katzen bleiben in seinem Team,\n"
     u8"die Spiel-KI steuert sie bis Kampfende.\n"
     u8"\n"
     u8"Um wieder ins Abenteuer zu kommen,\n"
     u8"klick auf ein Schild in deinem Haus."},
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
     u8"Rückkehr nach Hause\n"
     u8"\n"
     u8"%s hat ein Feld betreten, dem dein\n"
     u8"Spiel nicht folgen konnte: Deine Kopie\n"
     u8"des Abenteuers wich ab. Du bist zurück;\n"
     u8"deine Katzen bleiben in seinem Team, die Spiel-KI\n"
     u8"steuert sie.\n"
     u8"\n"
     u8"Um wieder ins Abenteuer zu kommen,\n"
     u8"klick auf ein Schild in deinem Haus."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Rückkehr nach Hause\n"
     u8"\n"
     u8"Deine Kopie des Kampfes war\n"
     u8"nicht mehr synchron mit der von %s:\n"
     u8"Der Kampf ging ohne dich weiter. Deine Katzen\n"
     u8"bleiben im Team, die Spiel-KI steuert sie.\n"
     u8"\n"
     u8"Um wieder ins Abenteuer zu kommen,\n"
     u8"klick auf ein Schild in deinem Haus."},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"Dein Speicherstand ist im Abenteuer.\n"
     u8"Im Mehrspielermodus spielt man Abenteuer mit dem Host:\n"
     u8"Wähle im Menü einen anderen\n"
     u8"oder spiel diesen allein zu Ende."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Dein Spiel lädt das Abenteuer von %s."},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%s hat das Abenteuer neu geladen:\n"
     u8"Du lädst es auch neu."},
    {u8"Rentrer", u8"Nach Hause"},
    {u8"Rester", u8"Bleiben"},
    {u8"Ne pas attendre", u8"Nicht warten"},
    {u8"Recharger", u8"Neu laden"},
    {u8"J'arrive", u8"Ich komme"},
    {u8"Pas encore", u8"Noch nicht"},
    {u8"Échanger", u8"Tauschen"},
    {u8"Multijoueur", u8"Mehrspieler"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"Dieses Rohr gehört zum Haus von %s:\n"
     u8"Schick deine Katzen von deinem eigenen\n"
     u8"Haus aus zu den NPCs."},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"Die NPCs folgen dem Spiel von %s:\n"
     u8"Katzen ins Rohr schicken geht mit\n"
     u8"dieser Spielversion nicht."},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"Eine Niederlage fürs ganze Team:\n"
     u8"Auch die Katzen von %s kommen nicht zurück."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"%s beitreten\n"
     u8"\n"
     u8"Du bist schon in einer Lobby:\n"
     u8"Verlass sie zuerst (Mehrspieler, „Verlassen“),\n"
     u8"dann klick erneut auf „Spiel beitreten“."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"%s beitreten\n"
     u8"\n"
     u8"Dein Spiel kehrt zum Hauptmenü zurück (dein Fortschritt\n"
     u8"wird gespeichert wie mit „Speichern und zurück zum Menü“)\n"
     u8"und tritt dann der Lobby von %s bei.\n"
     u8"\n"
     u8"Später: Dein Spiel tritt bei, sobald du ins\n"
     u8"Hauptmenü gehst."},
    {u8"Y aller", u8"Jetzt los"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"%s beitreten\n"
     u8"\n"
     u8"Dein Spiel tritt der Lobby von %s bei,\n"
     u8"sobald du im Hauptmenü bist\n"
     u8"(Esc, dann „Speichern und zurück zum Menü“)."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"%s beitreten\n"
     u8"\n"
     u8"Eine Verbindung über Steam ist gerade nicht möglich.\n"
     u8"Versuch es später noch einmal oder tritt per Adresse bei\n"
     u8"(Mehrspieler, dann „Beitreten“)."},
    {u8"%s a annulé le départ.", u8"%s hat den Aufbruch abgebrochen."},
    {u8"réseau local", u8"lokales Netz"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Deine Katzen sind schon mit dem Team von %s los:\n"
     u8"Bis du da bist, steuert die Spiel-KI sie\n"
     u8"im Kampf und %s wählt für sie; nach dem\n"
     u8"Abenteuer kommen sie zu dir nach Hause."},
    {u8"Internet", u8"Internet"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s verteidigt sein Haus – allein,\n"
     u8"mit seinen Katzen. Danach ist er zurück."},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s verteidigen ihre Häuser – jeder\n"
     u8"allein mit seinen Katzen. Danach sind sie zurück."},
    {u8"%s a fini de défendre sa maison.", u8"%s hat sein Haus verteidigt."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s hat sein Haus verteidigt.\n"
     u8"%s verteidigt seines noch."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s hat sein Haus verteidigt.\n"
     u8"%s verteidigen ihres noch."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Du hast nach der Wahl abgesagt: %s\n"
     u8"bricht ohne dich und deine Katzen auf.\n"
     u8"Du kannst beitreten, sobald er auf der Karte ist."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"Aufbruch ins Abenteuer\n"
     u8"\n"
     u8"Diesmal kann keine Katze aufbrechen:\n"
     u8"%s bricht nicht ins Abenteuer auf."},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"Die Beuteverteilung läuft noch:\n"
     u8"Warte, bis alles verteilt ist."},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"Du gehst ins Abenteuer von %s:\n"
     u8"Dein Haus verteidigst du danach."},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"%s startet das Abenteuer:\n"
     u8"Bitte ihn, auf „Aufbrechen!“ zu klicken."},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"Mehrspieler\n"
     u8"\n"
     u8"Hoste ein Spiel, damit deine Freunde beitreten,\n"
     u8"oder spiel bei einem Freund mit."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"[c:red]Port %u lässt sich nicht öffnen.[/c]\n"
     u8"Vielleicht belegt ihn ein Programm.\n"
     u8"Beende die Lobby und versuch es erneut."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Lobby wird geöffnet..."},
    {u8"%s (hôte)", u8"%s (Host)"},
    {u8"En attente d'un ami...", u8"Warte auf einen Freund..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Deine Steam-Freunde treten mit einem Klick bei: in ihrer\n"
     u8"Steam-Freundesliste „Spiel beitreten“ bei deinem Namen.\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Oder per Adresse:\n"
     u8"%s   (%s, Port %u)\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Gib deinem Freund diese Adresse:\n"
     u8"%s   (%s, Port %u)\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"Schon kopiert: Mit Strg+V kannst du sie ihm schicken.\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"Keine Adresse auf diesem PC gefunden (Port %u).\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"%s\n"
     u8"Spieler (%d/4):\n"
     u8"%s\n"
     u8"Wenn alle da sind, wähle deinen Speicherstand."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"Einem Spiel beitreten\n"
     u8"\n"
     u8"Adresse des Hosts:\n"
     u8"%s_\n"
     u8"\n"
     u8"Tippe die Adresse deines Freundes ein\n"
     u8"oder füg sie mit Strg+V ein. Enter zum Bestätigen.%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Über Steam ist es einfacher: in deiner Freundesliste\n"
     u8"„Spiel beitreten“ bei deinem Freund."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"Einem Spiel beitreten\n"
     u8"\n"
     u8"[c:red]Ungültige Adresse: %s[/c]\n"
     u8"\n"
     u8"Beispiel: 26.123.45.67\n"
     u8"oder mit anderem Port: 26.123.45.67:27600"},
    {u8"(vide)", u8"(leer)"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.",
     u8"Prüft, ob ihr dieselbe Version von Spiel und Mod habt."},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).",
     u8"Das Spiel des Hosts ist voll (höchstens 4 Spieler)."},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"Ihr habt nicht dieselbe Mod-Version:\n"
     u8"Holt euch beide die neueste."},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"Ihr habt nicht dieselbe Spielversion:\n"
     u8"Aktualisiert beide Mewgenics (Steam)."},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"Die Spieldateien sind nicht gleich:\n"
     u8"Überprüft sie in Steam (Eigenschaften, Installierte Dateien)."},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"Die Einstellung net.control in mgmp.json gibt beiden\n"
     u8"Spielern dieselbe Katze: Stellt sie zurück auf \"auto\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"[c:red]Die Verbindung wurde abgelehnt.[/c]\n"
     u8"%s\n"
     u8"(%s)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"[c:red]Beitritt zu %s nicht möglich:[/c]\n"
     u8"%s.\n"
     u8"Verlass die Lobby und versuch es erneut (oder per Adresse)."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Verbinde mit %s..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"[c:red]%s ist nicht erreichbar.[/c]\n"
     u8"Prüf die Adresse und ob dein Freund\n"
     u8"auf „Hosten“ geklickt hat."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Getrennt."},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"Die Speicherstand-Auswahl öffnet sich von selbst."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Verbinde mit %s...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"[c:red]Verbindung zum Host unterbrochen.[/c]\n"
     u8"Verlass die Lobby und tritt erneut bei."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Verbinde mit %s über Steam...\n"
     u8"(ein paar Sekunden)%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Verbunden, Anmeldung beim Host...%s"},
    {u8"%s (toi)", u8"%s (du)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Du bist im Spiel von %s!\n"
     u8"\n"
     u8"Spieler (%d/4):\n"
     u8"%s\n"
     u8"Warte auf das Spiel von %s:\n"
     u8"Sobald er seinen Speicherstand wählt,\n"
     u8"öffnet sich die Auswahl von selbst."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Du bist im Spiel von %s!\n"
     u8"\n"
     u8"Spieler (%d/4):\n"
     u8"%s\n"
     u8"Warte auf das Spiel von %s:\n"
     u8"Sobald er seinen Speicherstand wählt,\n"
     u8"kannst du deinen wählen."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"Mehrspieler-Lobby\n"
     u8"\n"
     u8"Du bist im Spiel von %s!\n"
     u8"\n"
     u8"Spieler (%d/4):\n"
     u8"%s\n"
     u8"Wähle deinen Speicherstand: dein Haus, deine Katzen.\n"
     u8"NPCs und Geschichte sind die von %s,\n"
     u8"und ins Abenteuer ziehst du mit seinem Team."},
    {u8"Héberger", u8"Hosten"},
    {u8"Rejoindre", u8"Beitreten"},
    {u8"Choisir la sauvegarde", u8"Speicherstand wählen"},
    {u8"Arrêter", u8"Beenden"},
    {u8"Choisir ta sauvegarde", u8"Speicherstand wählen"},
    {u8"Quitter", u8"Verlassen"},
    {u8"Annuler", u8"Abbrechen"},
    {u8"Échanger avec %s", u8"Tauschen mit %s"},
    {u8"Donner à qui ?", u8"Wem schenken?"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"Wähle mit Hoch/Runter, dann Enter."},
    {u8"Pièces à donner : %lld_", u8"Münzen zum Verschenken: %lld_"},
    {u8"Nourriture à donner : %lld_", u8"Futter zum Verschenken: %lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"Tippe eine Zahl (T: alles), dann Enter."},
    {u8"Échanger avec %s : les objets de ton rangement",
     u8"Tauschen mit %s: Gegenstände aus deinem Lager"},
    {u8"16 au plus par don : %u choisis.", u8"Max. 16 pro Geschenk: %u gewählt."},
    {u8"16 au plus par don : %u choisi.", u8"Max. 16 pro Geschenk: %u gewählt."},
    {u8"Lecture du rangement...", u8"Lager wird gelesen..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"Hoch/Runter: wählen, Leertaste: markieren, Links/Rechts: Anzahl.\n"
     u8"Buchstaben zum Suchen, dann Enter."},
    {u8"Tu n'as rien choisi à donner.", u8"Du hast nichts zum Schenken gewählt."},
    {u8"Tu n'as rien à donner pour l'instant.", u8"Du hast gerade nichts zu verschenken."},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"%s aus deinem Beutel schenken\n"
     u8"an %s?"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"%s schenken\n"
     u8"an %s?"},
    {u8"Envoi du don...", u8"Sende Geschenk..."},
    {u8"Personne ne peut recevoir maintenant.", u8"Gerade kann niemand etwas empfangen."},
    {u8"Personne d'autre n'est dans la partie.", u8"Sonst ist niemand im Spiel."},
    {u8"Confirmer", u8"Bestätigen"},
    {u8"Donner", u8"Schenken"},
    {u8"Recommencer", u8"Neu anfangen"},
    {u8"Fermer", u8"Schließen"},
    {u8"Pas d'échange possible ici.", u8"Hier ist kein Tausch möglich."},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"Das Fenster wurde geschlossen: Der Host hat ein Feld betreten.\n"
     u8"Deine Auswahl bleibt erhalten (E zum Fortsetzen)."},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"Tauschen im Abenteuer\n"
     u8"\n"
     u8"Drück auf der Karte E, um Münzen oder Futter\n"
     u8"aus deinem Beutel einem anderen Spieler zu schenken."},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
