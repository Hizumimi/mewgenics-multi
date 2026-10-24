// lang_it.cpp -- Mewgenics Multi : les textes du mod et du lanceur en italien (it).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : le tutoiement (« tu »), comme le tutoiement du francais du mod.
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_it[];
const TrPair kTable_it[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"Impossibile leggere mgmp.json: impostazioni non salvate."},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"mgmp.json contiene dei commenti (// o /* */) che verrebbero cancellati salvando: rimuovili o cambia l'impostazione a mano nel file."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.json non è un JSON valido: correggilo (o rimetti quello dello zip) prima di cambiare le impostazioni."},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"Impossibile scrivere nella cartella del mod: impostazioni non salvate."},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"Impossibile sostituire mgmp.json (aperto altrove?): impostazioni non salvate."},
    {u8"Steam est ouvert.", u8"Steam è aperto."},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Senza Steam, il gioco si chiude prima che il mod si carichi. Apri Steam e aspetta che sia connesso."},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"mgmp.json manca accanto a Mewgenics\u00A0Multi.exe: salva le impostazioni per ricrearlo."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.json non è un JSON valido: correggilo o rimetti quello dello zip."},
    {u8"Réglages lus dans mgmp.json.", u8"Impostazioni lette da mgmp.json."},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe non trovato (%s): in Impostazioni, clicca su Sfoglia… e scegli Mewgenics.exe."},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"resources.gpak manca accanto a Mewgenics.exe: verifica i file del gioco su Steam."},
    {u8"Jeu trouvé : %s.", u8"Gioco trovato: %s."},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.",
     u8"steam_appid.txt: da verificare una volta trovato il gioco."},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Manca accanto a Mewgenics.exe: senza questo file, Steam riavvia il gioco senza il mod."},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"Quello accanto a Mewgenics.exe non contiene 686060: Steam riavvierebbe il gioco senza il mod."},
    {u8"steam_appid.txt est en place à côté du jeu.",
     u8"steam_appid.txt è al suo posto accanto al gioco."},
    {u8"Fichiers du mod présents.", u8"File del mod presenti."},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Mancano mgmp.dll e mgmp_loader.exe accanto a Mewgenics\u00A0Multi.exe: estrai di nuovo tutta la cartella dallo zip."},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Manca %s accanto a Mewgenics\u00A0Multi.exe: estrai di nuovo tutta la cartella dallo zip."},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi, build %s, protocollo %u: tutti devono avere la stessa versione."},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"Gioco non trovato: scegli prima Mewgenics.exe in Impostazioni."},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"Impossibile scrivere steam_appid.txt accanto al gioco: copialo a mano (LISEZ-MOI, passo 2)."},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"Non hai dato il permesso a Windows: steam_appid.txt non è stato copiato. Puoi riprovare o copiarlo a mano (LISEZ-MOI, passo 2)."},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics Multi non è riuscito a riavviarsi come amministratore: copia steam_appid.txt a mano (LISEZ-MOI, passo 2)."},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"La copia come amministratore non è riuscita: copia steam_appid.txt a mano (LISEZ-MOI, passo 2)."},
    {u8"Le mod n'a pas pu être chargé : %s", u8"Impossibile caricare il mod: %s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exe si è fermato senza caricare il mod. Riprova; se succede di nuovo, invia loader.txt."},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exe si è fermato (codice %s) senza caricare il mod. Causa più probabile: Steam chiuso o disconnesso. Apri Steam, aspetta che sia connesso, poi riprova."},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"Il gioco si è chiuso durante l'avvio, prima del mod. Causa più frequente: Steam chiuso o disconnesso. Apri Steam, aspetta che sia connesso, poi riprova."},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"Steam riavvia il gioco (può volerci un minuto o due)…"},
    {u8"Ouverture du jeu…", u8"Apertura del gioco…"},
    {u8"Le jeu démarre…", u8"Avvio del gioco…"},
    {u8"Injection du mod dans le jeu…", u8"Iniezione del mod nel gioco…"},
    {u8"Le mod est en place, le jeu reprend…", u8"Il mod è pronto, il gioco riprende…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Ancora in attesa del gioco riavviato da Steam… Se Steam si è appena aperto, forse ti sta facendo una domanda."},
    {u8"Jeu relancé trouvé : injection du mod…", u8"Gioco riavviato trovato: iniezione del mod…"},
    {u8"Le jeu est lancé avec le mod.", u8"Il gioco è avviato con il mod."},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.",
     u8"Il mod ci ha messo molto a prepararsi; il gioco riprende comunque."},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.",
     u8"Il gioco riavviato tarda ad avviarsi; il mod viene iniettato comunque."},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"mgmp.dll non trovato accanto a Mewgenics\u00A0Multi.exe: estrai di nuovo la cartella del mod dallo zip."},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe non trovato: in Impostazioni, clicca su Sfoglia… e scegli Mewgenics.exe."},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"mgmp.json è illeggibile: correggilo o rimetti quello dello zip."},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steam ha riavviato il gioco senza il mod: verifica steam_appid.txt accanto a Mewgenics.exe (pulsante Ripara), poi riprova."},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"Questo gioco non è la versione prevista dal mod (1.1.21239): è stato aggiornato?"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"Un programma (overlay, antivirus, anti-cheat) ha impedito il caricamento del mod."},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"Il gioco ci ha messo troppo ad avviarsi: il mod non è caricato. Riprova."},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"L'iniezione del mod non è riuscita e il gioco è stato chiuso. Riprova; se succede di nuovo, invia loader.txt."},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"mgmp.json non dice dov'è il gioco: in Impostazioni, clicca su Sfoglia… e scegli Mewgenics.exe."},
    {u8"build %s · protocole %u", u8"build %s · protocollo %u"},
    {u8"Steam ouvert", u8"Steam aperto"},
    {u8"Steam fermé", u8"Steam chiuso"},
    {u8"Réglages (mgmp.json)", u8"Impostazioni (mgmp.json)"},
    {u8"Jeu trouvé", u8"Gioco trovato"},
    {u8"Jeu incomplet", u8"Gioco incompleto"},
    {u8"Jeu introuvable", u8"Gioco non trovato"},
    {u8"Fichiers du mod", u8"File del mod"},
    {u8"Réparer", u8"Ripara"},
    {u8"Jouer", u8"Gioca"},
    {u8"Règle d'abord les croix rouges.", u8"Prima risolvi le croci rosse."},
    {u8"Réglages", u8"Impostazioni"},
    {u8"Journaux", u8"Log"},
    {u8"Langue", u8"Lingua"},
    {u8"Le jeu (Mewgenics.exe)", u8"Il gioco (Mewgenics.exe)"},
    {u8"Parcourir…", u8"Sfoglia…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe e resources.gpak ci sono."},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"Qui manca resources.gpak: non è la cartella giusta."},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.",
     u8"Nessun Mewgenics.exe qui: sceglilo con Sfoglia."},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Gioca insieme su Steam (lista amici)"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Togli la spunta se Steam dà problemi: potrai collegarti solo tramite indirizzo, dal menu Multigiocatore del gioco."},
    {u8"Enregistrer", u8"Salva"},
    {u8"Retour", u8"Indietro"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"Ancora nessun log: il mod ne scrive uno a ogni partita."},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"1 log del mod, accanto a Mewgenics\u00A0Multi.exe."},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"%d log del mod, accanto a Mewgenics\u00A0Multi.exe."},
    {u8"Le dernier : %s", u8"L'ultimo: %s"},
    {u8"écrit le %s", u8"scritto il %s"},
    {u8"Ouvrir le dossier", u8"Apri la cartella"},
    {u8"Copier le dernier journal", u8"Copia l'ultimo log"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"Per chiedere aiuto: copia l'ultimo log, poi incollalo su Discord con Ctrl+V."},
    {u8"Lancement", u8"Avvio"},
    {u8"Le jeu se lance avec le mod…", u8"Il gioco si avvia con il mod…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.",
     u8"Buona partita! Questa finestra si chiude da sola."},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"I dettagli sono in loader.txt, accanto a Mewgenics\u00A0Multi.exe."},
    {u8"Réessayer", u8"Riprova"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"Se Steam riavvia il gioco, può volerci un minuto o due."},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.",
     u8"Log copiato: incollalo su Discord con Ctrl+V."},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"Impossibile riempire gli appunti: riprova o apri la cartella."},
    {u8"Où est Mewgenics.exe ?", u8"Dov'è Mewgenics.exe?"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"Direct2D non è disponibile su questo PC: Mewgenics Multi non può essere visualizzato."},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"Impossibile preparare la scheda grafica: Mewgenics Multi non può essere visualizzato."},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"Ognuno la sua casa e i suoi gatti,\n"
     u8"l'avventura insieme!"},
    {u8"(illustrations du jeu introuvables)", u8"(illustrazioni del gioco non trovate)"},
    {u8"Comme le jeu (%s)", u8"Come il gioco (%s)"},
    {u8"Comme Windows (%s)", u8"Come Windows (%s)"},
    {u8"Automatique (%s)", u8"Automatica (%s)"},
    {u8"Lancement annulé.", u8"Avvio annullato."},
    {u8"Lancement du mod…", u8"Avvio del mod…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windows non è riuscito a preparare l'avvio. Riavvia Mewgenics Multi."},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exe non è riuscito ad avviarsi: estrai di nuovo la cartella del mod dallo zip (o il tuo antivirus l'ha messo in quarantena)."},
    {u8"la connexion Steam avec l'hôte est coupée", u8"la connessione Steam con l'host si è interrotta"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"l'host non risponde tramite Steam (lobby chiusa, o non ha il mod?)"},
    {u8"Steam refuse la connexion", u8"Steam rifiuta la connessione"},
    {u8"À %s de choisir", u8"Tocca a %s scegliere"},
    {u8"À toi de choisir", u8"Tocca a te scegliere"},
    {u8"À %s de choisir la capacité à remplacer.", u8"Tocca a %s scegliere l'abilità da sostituire."},
    {u8"Ce chat est à %s", u8"Questo gatto è di %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"Scelta dei collari\n"
     u8"\n"
     u8"La scelta condivisa si è interrotta (errore del gioco):\n"
     u8"scegli i collari per tutta la squadra."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"Scelta dei collari\n"
     u8"\n"
     u8"Fatto! %s sa che hai finito:\n"
     u8"confermerà la partenza per tutta la squadra."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"Scelta dei collari\n"
     u8"\n"
     u8"È %s a confermare la partenza:\n"
     u8"digli quando i tuoi gatti hanno i collari."},
    {u8"%s et %s", u8"%s e %s"},
    {u8"%s, %s", u8"%s, %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Scelta dei collari\n"
     u8"\n"
     u8"%s scelgono ancora i collari dei loro gatti.\n"
     u8"Clicca di nuovo su \"Conferma!\" per partire\n"
     u8"comunque."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Scelta dei collari\n"
     u8"\n"
     u8"%s sceglie ancora i collari dei suoi gatti.\n"
     u8"Clicca di nuovo su \"Conferma!\" per partire\n"
     u8"comunque."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"La partenza di %s non si vede più qui.\n"
     u8"Finisci tu queste schermate: lo raggiungerai\n"
     u8"sulla mappa."},
    {u8"  %s : parti", u8"  %s: non c'è più"},
    {u8"  %s : prêt !", u8"  %s: pronto!"},
    {u8"  %s : charge ta partie...", u8"  %s: carica la tua partita..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"La tua squadra carica la tua partita per partire con te:\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"Scelta degli oggetti\n"
     u8"\n"
     u8"Impossibile caricare qui il tuo deposito:\n"
     u8"i tuoi gatti partono con gli oggetti che hanno."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Scelta degli oggetti\n"
     u8"\n"
     u8"%s equipaggiano ancora i loro gatti.\n"
     u8"Clicca di nuovo su \"Conferma!\" per partire\n"
     u8"comunque."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Scelta degli oggetti\n"
     u8"\n"
     u8"%s equipaggia ancora i suoi gatti.\n"
     u8"Clicca di nuovo su \"Conferma!\" per partire\n"
     u8"comunque."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"Scelta degli oggetti\n"
     u8"\n"
     u8"Fatto! I tuoi gatti partono con questi oggetti:\n"
     u8"%s confermerà la partenza per tutta la squadra."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"Scelta degli oggetti\n"
     u8"\n"
     u8"Gli oggetti scelti per i tuoi gatti non sono\n"
     u8"stati trasferiti nell'avventura di %s: i tuoi gatti\n"
     u8"partono con quelli che avevano, il tuo deposito\n"
     u8"non cambia."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"Scelta degli oggetti\n"
     u8"\n"
     u8"%s ha già confermato la partenza: i tuoi gatti\n"
     u8"partono con gli oggetti scelti finora, il resto\n"
     u8"non conta più."},
    {u8"Ta bourse : %s.", u8"La tua borsa: %s."},
    {u8"%s n'est plus dans la partie.", u8"%s non è più nella partita."},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"Non hai una borsa in questa avventura (non ci hai portato gatti)."},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"Gli scambi funzionano a casa tua, o in un'avventura multigiocatore con la tua borsa. Per ora: %s."},
    {u8"pas de partie à plusieurs en cours", u8"non sei in multigiocatore"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés",
     u8"manca una funzione del gioco, gli scambi a casa sono disattivati"},
    {u8"tu es dans la partie de l'hôte", u8"sei nella partita dell'host"},
    {u8"tu n'es pas dans ta maison", u8"non sei a casa tua"},
    {u8"ta save n'est pas chargée", u8"il tuo salvataggio non è caricato"},
    {u8"un départ en aventure est en cours", u8"c'è una partenza in corso"},
    {u8"ta maison est attaquée", u8"la tua casa è sotto attacco"},
    {u8"ta maison est illisible", u8"la tua casa è illeggibile"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"il tuo deposito aspetta ciò che i tuoi gatti riportano dall'avventura dell'host (se sono tornati, ricarica il salvataggio: Esc, \"Salva ed esci al menu\", poi \"Gioca\" e il tuo salvataggio)"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"alcuni oggetti del tuo deposito aspettano ancora il nuovo proprietario (bottino, acquisti, PNG)"},
    {u8"Pas d'échange maintenant : %s.", u8"Niente scambi ora: %s."},
    {u8"Ton don précédent attend encore sa réponse.",
     u8"Il tuo dono precedente attende ancora una risposta."},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"Il mod non conosce ancora il tuo salvataggio: ricaricalo dal menu."},
    {u8"Choisis un joueur de la partie.", u8"Scegli un giocatore della partita."},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.",
     u8"Niente scambi ora: %s non è a casa sua."},
    {u8"Ce montant n'est pas possible.", u8"Importo non valido."},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.",
     u8"Scegli monete, cibo od oggetti da dare."},
    {u8"16 objets au plus par don.", u8"Al massimo 16 oggetti per dono."},
    {u8"Pas d'objets maintenant : %s.", u8"Niente oggetti ora: %s."},
    {u8"Ta maison est illisible : pas de don.", u8"La tua casa è illeggibile: niente dono."},
    {u8"Tu n'as pas autant de pièces.", u8"Non hai così tante monete."},
    {u8"Tu n'as pas autant de nourriture.", u8"Non hai così tanto cibo."},
    {u8"Ton rangement est illisible : pas de don.", u8"Il tuo deposito è illeggibile: niente dono."},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"Un oggetto scelto fa già parte di un dono non ancora concluso."},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"Un oggetto scelto non è più nel tuo deposito: scegli di nuovo."},
    {u8"Un objet choisi ne peut pas se donner.", u8"Un oggetto scelto non si può dare."},
    {u8"Le don n'a pas pu partir (connexion).", u8"Il dono non è partito (connessione)."},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…",
     u8"Dono inviato a %s (%s): in attesa della sua risposta…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…",
     u8"Dono richiesto all'host: in attesa della sua risposta…"},
    {u8"Pas de don : %s.", u8"Niente dono: %s."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%s ha ricevuto il tuo dono: i tuoi oggetti lasceranno il deposito appena la schermata degli oggetti sarà chiusa."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%s ha ricevuto il tuo dono: i tuoi oggetti lasceranno il deposito appena possibile (dopo la partenza, la difesa della tua casa o il ritorno dei tuoi gatti)."},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%s ha ricevuto il tuo dono: ciò che hai dato lascerà la tua casa quando tornerai in questo salvataggio."},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"Nessuna risposta da %s al tuo dono: dalla tua casa non è uscito nulla."},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"Nessuna risposta da %s al tuo dono: dalla tua casa non è uscito nulla."},
    {u8"Les échanges marchent pendant une partie à plusieurs.",
     u8"Gli scambi funzionano durante una partita multigiocatore."},
    {u8"Dans ta maison : %s.", u8"A casa tua: %s."},
    {u8"%s n'est pas dans sa maison.", u8"%s non è a casa sua."},
    {u8"%lld pièces", u8"%lld monete"},
    {u8"%lld pièce", u8"%lld moneta"},
    {u8"%lld nourritures", u8"%lld unità di cibo"},
    {u8"%lld nourriture", u8"%lld unità di cibo"},
    {u8"rien", u8"niente"},
    {u8"%s n'est pas dans sa maison", u8"%s non è a casa sua"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s",
     u8"una finestra o la schermata degli oggetti è rimasta aperta da %s"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"il gioco di %s non è riuscito a riporli"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"il gioco di %s non è riuscito a riporli"},
    {u8"les dons ne marchent pas chez %s", u8"i doni non funzionano da %s"},
    {u8"%s n'est plus dans la partie", u8"%s non è più nella partita"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%s ha ancora dei gatti nell'avventura dell'host (il suo deposito attende il loro ritorno)"},
    {u8"raison inconnue", u8"motivo sconosciuto"},
    {u8"Cadeau d'%s", u8"Regalo da %s"},
    {u8"Cadeau de %s", u8"Regalo da %s"},
    {u8"%s t'a donné %s.", u8"%s ti ha dato %s."},
    {u8"C'est dans ta maison, et ta save est à jour.",
     u8"È a casa tua, e il tuo salvataggio è aggiornato."},
    {u8"C'est dans ta maison.", u8"È a casa tua."},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"Alla fine, %s ha ricevuto %s: il dono è uscito dalla tua casa."},
    {u8"%s a bien reçu %s : c'est parti de ta maison.",
     u8"%s ha ricevuto %s: il dono è uscito dalla tua casa."},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"(1 oggetto non è potuto uscire dal tuo deposito: ce l'hai ancora anche tu.)"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"(%u oggetti non sono potuti uscire dal tuo deposito: li hai ancora anche tu.)"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"Il dono a %s è fallito: %s. Dalla tua casa non è uscito nulla."},
    {u8"Rien à donner dans ton rangement.", u8"Niente da dare nel tuo deposito."},
    {u8"Tape des lettres pour chercher un objet.", u8"Digita delle lettere per cercare un oggetto."},
    {u8"Recherche : %s_", u8"Cerca: %s_"},
    {u8"Aucun objet ne correspond.", u8"Nessun oggetto corrisponde."},
    {u8"(page %u sur %u)", u8"(pagina %u di %u)"},
    {u8"Butin : %s.", u8"Bottino: %s."},
    {u8"Besoin", u8"Bisogno"},
    {u8"Cupidité", u8"Avidità"},
    {u8"Passer", u8"Passa"},
    {u8"%u s", u8"%u s"},
    {u8"toi", u8"te"},
    {u8"Joueur %u", u8"Giocatore %u"},
    {u8"Un objet", u8"Un oggetto"},
    {u8"Partage du butin : objet %u sur %u", u8"Bottino: oggetto %u di %u"},
    {u8"Besoin : %s", u8"Bisogno: %s"},
    {u8"Cupidité : %s", u8"Avidità: %s"},
    {u8"Passer : %s", u8"Passa: %s"},
    {u8"Partage du butin", u8"Spartizione del bottino"},
    {u8"%u objets du sac à partager entre %s.", u8"Zaino: %u oggetti da spartire tra %s."},
    {u8"%u objet du sac à partager entre %s.", u8"Zaino: %u oggetto da spartire tra %s."},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"Per ogni oggetto: Bisogno (ti serve), Avidità (se\n"
     u8"nessuno ne ha bisogno) o Passa. Un dado da 100 decide i pareggi."},
    {u8"En attente de %s...", u8"In attesa di %s..."},
    {u8"En attente de %s... (%s)", u8"In attesa di %s... (%s)"},
    {u8"Tout le monde est là : ça commence !", u8"Ci sono tutti: si comincia!"},
    {u8"Commencer", u8"Inizia"},
    {u8"D'accord", u8"Ok"},
    {u8"Le veux-tu ? (%s)", u8"Lo vuoi? (%s)"},
    {u8"Je le veux", u8"Lo voglio"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"Bisogno: ti serve, hai la precedenza sugli altri.\n"
     u8"Avidità: lo prendi se nessuno ne ha bisogno.\n"
     u8"(%s)"},
    {u8"Ton choix : %s.", u8"La tua scelta: %s."},
    {u8"Ton choix est fait.", u8"Hai fatto la tua scelta."},
    {u8"En attente de %s (%s).", u8"In attesa di %s (%s)."},
    {u8"Le résultat arrive...", u8"Il risultato sta arrivando..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s non ci sono: saltano il turno."},
    {u8"%s n'est pas là : son tour passe.", u8"%s non c'è: salta il turno."},
    {u8"Personne n'en veut : il reste chez toi.", u8"Nessuno lo vuole: resta a te."},
    {u8"Personne n'en veut : il reste chez %s.", u8"Nessuno lo vuole: resta a %s."},
    {u8"Tu l'emportes (%s) !", u8"Lo vinci tu (%s)!"},
    {u8"%s l'emporte (%s).", u8"Lo vince %s (%s)."},
    {u8"Dés : %s (égalité départagée).", u8"Dadi: %s (spareggio)."},
    {u8"Dés : %s.", u8"Dadi: %s."},
    {u8"Partage du butin terminé", u8"Spartizione del bottino conclusa"},
    {u8"Tu remportes : %s.", u8"Vinci: %s."},
    {u8"%s remporte : %s.", u8"%s vince: %s."},
    {u8"Remporté, mais resté chez %s : %s.", u8"Vinto, ma rimasto a %s: %s."},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"Nessuno li ha voluti (restano a te): %s."},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"Nessuno li ha voluti (restano a %s): %s."},
    {u8"Aucun objet n'a été partagé.", u8"Nessun oggetto è stato spartito."},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"Il tuo salvataggio è già aperto: la prossima\n"
     u8"finestra ti dice come ricaricarlo col bottino."},
    {u8"Ton butin t'attend dans ton rangement, chez toi.",
     u8"Il tuo bottino ti aspetta nel deposito, a casa tua."},
    {u8"Ce que les autres remportent quitte ton rangement.",
     u8"Ciò che vincono gli altri lascia il tuo deposito."},
    {u8"Dr Beanies", u8"Dr. Beanies"},
    {u8"Butch", u8"Butch"},
    {u8"Tink", u8"Tink"},
    {u8"Frank", u8"Frank"},
    {u8"Bébé Jack", u8"Baby Jack"},
    {u8"Tracy", u8"Tracy"},
    {u8"Broyeur d'organe", u8"Pelaorgani"},
    {u8"Steven", u8"Steven"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"Ricompensa pronta\n"
     u8"\n"
     u8"Il gatto che %s ha dato a %s ha riempito\n"
     u8"la sua barra. La ricompensa sarà ritirata per lui\n"
     u8"appena sarai a casa tua: le monete,\n"
     u8"il cibo e gli oggetti andranno a %s,\n"
     u8"gli sblocchi a tutti."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"Ricompensa di %s\n"
     u8"\n"
     u8"Non è stato possibile inviare a %s la\n"
     u8"ricompensa del suo dono: resta a te."},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"Ricompensa per %s\n"
     u8"\n"
     u8"%s ha premiato il gatto ricevuto\n"
     u8"da %s: %s vanno a casa sua.\n"
     u8"Gli sblocchi sono per tutti."},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Ricompensa per tutti\n"
     u8"\n"
     u8"%s ha premiato il gatto ricevuto\n"
     u8"da %s: i suoi sblocchi sono per\n"
     u8"tutti."},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s ti ringrazia per il tuo gatto: %s\n"
     u8"(i suoi sblocchi sono per tutti)."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Ricompensa di %s\n"
     u8"\n"
     u8"%s ti ringrazia per il tuo gatto: %s.\n"
     u8"Arriveranno a casa tua al prossimo\n"
     u8"caricamento del salvataggio (dopo\n"
     u8"l'avventura). I suoi sblocchi sono per\n"
     u8"tutti."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"Ricompensa di %s\n"
     u8"\n"
     u8"Il tuo gatto ha riempito la barra di %s:\n"
     u8"la sua ricompensa, fatta di sblocchi, è per\n"
     u8"tutti. Arriverà nel tuo salvataggio\n"
     u8"al prossimo caricamento."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"Ricompensa di %s\n"
     u8"\n"
     u8"%s, che aveva riempito la barra di %s,\n"
     u8"non c'è più: la ricompensa resta a te."},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"Ricompensa pronta\n"
     u8"\n"
     u8"Il tuo gatto ha riempito la barra di %s!\n"
     u8"%s ritirerà la ricompensa per te\n"
     u8"appena sarà a casa sua."},
    {u8"les bourses ne marchent pas dans cette partie", u8"le borse non funzionano in questa partita"},
    {u8"rien à donner", u8"niente da dare"},
    {u8"c'est trop d'un coup", u8"è troppo in una volta"},
    {u8"choisis un autre joueur", u8"scegli un altro giocatore"},
    {u8"ton don précédent attend encore la réponse de l'hôte",
     u8"il tuo dono precedente attende ancora la risposta dell'host"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"non hai una borsa in questa avventura"},
    {u8"envoi impossible (connexion)", u8"invio impossibile (connessione)"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"Fine dell'avventura\n"
     u8"\n"
     u8"Ognuno riporta la sua borsa a casa sua:"},
    {u8"%s n'est plus là : sa bourse reste chez toi.", u8"%s non c'è più: la sua borsa resta a te."},
    {u8"%s : rien.", u8"%s: niente."},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.",
     u8"%s non ha potuto ricevere la sua borsa: resta a te."},
    {u8"%s : %s.", u8"%s: %s."},
    {u8"Toi : %s.", u8"Tu: %s."},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"Ancora nessuna risposta dell'host per il tuo dono: controlla la tua borsa nella finestra del percorso."},
    {u8"Ton don n'est pas passé : %s.", u8"Il tuo dono è fallito: %s."},
    {u8"(et d'autres trouvailles)", u8"(e altri ritrovamenti)"},
    {u8"pas de bourse pour ce joueur dans cette aventure",
     u8"nessuna borsa per questo giocatore in questa avventura"},
    {u8"ta bourse n'en a pas autant", u8"la tua borsa non ne ha abbastanza"},
    {u8"l'aventure n'est plus suivie", u8"l'avventura non è più tracciata"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s ti ha dato %s: è nella tua borsa."},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"Hai dato %s a %s: è nella sua borsa."},
    {u8"%s a donné %s à %s.", u8"%s ha dato %s a %s."},
    {u8"Un chat", u8"Un gatto"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%s ha trovato %s e perso %s: conta per la tua borsa."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s ha trovato %s e perso %s: conta per la borsa di %s."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s ha trovato %s e perso %s: conta per la borsa di %s."},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s ha trovato %s: va nella tua borsa."},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s ha trovato %s: va nella borsa di %s."},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s ha trovato %s: va nella borsa di %s."},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s ha speso %s: a carico della tua borsa."},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.",
     u8"%s ha speso %s: a carico della borsa di %s."},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.",
     u8"%s ha speso %s: a carico della borsa di %s."},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s ha perso %s: a carico della tua borsa."},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%s ha perso %s: a carico della borsa di %s."},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%s ha perso %s: a carico della borsa di %s."},
    {u8"%s n'a rien trouvé.", u8"%s non ha trovato niente."},
    {u8"un objet", u8"un oggetto"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.",
     u8"Hai comprato %s per %s: a carico della tua borsa."},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.",
     u8"%s ha comprato %s per %s: a carico della sua borsa."},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"Hai comprato %s: %s in più per la tua borsa."},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%s ha comprato %s: %s in più per la sua borsa."},
    {u8"Tu as acheté %s (gratuit).", u8"Hai comprato %s (gratis)."},
    {u8"%s a acheté %s (gratuit).", u8"%s ha comprato %s (gratis)."},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"In battaglia i tuoi gatti hanno trovato %s e perso %s: conta per la tua borsa."},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"In battaglia i tuoi gatti hanno trovato %s: va nella tua borsa."},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"In battaglia i tuoi gatti hanno perso %s: a carico della tua borsa."},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"In battaglia non hai trovato nulla."},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"In battaglia i gatti di %s hanno trovato %s e perso %s: conta per la sua borsa."},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"In battaglia i gatti di %s hanno trovato %s e perso %s: conta per la sua borsa."},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"In battaglia i gatti di %s hanno trovato %s: va nella sua borsa."},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"In battaglia i gatti di %s hanno trovato %s: va nella sua borsa."},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"In battaglia i gatti di %s hanno perso %s: a carico della sua borsa."},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"In battaglia i gatti di %s hanno perso %s: a carico della sua borsa."},
    {u8"En combat, les chats d'%s n'ont rien trouvé.",
     u8"In battaglia i gatti di %s non hanno trovato nulla."},
    {u8"En combat, les chats de %s n'ont rien trouvé.",
     u8"In battaglia i gatti di %s non hanno trovato nulla."},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"Il tuo zaino è aperto\n"
     u8"\n"
     u8"L'host è entrato in una casella:\n"
     u8"chiudi lo zaino per seguirlo."},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"Oggetti\n"
     u8"\n"
     u8"Questo gatto è di %s: solo il suo proprietario\n"
     u8"può cambiarne gli oggetti."},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"Oggetti\n"
     u8"\n"
     u8"In una casella (un negozio, un aumento di\n"
     u8"livello...), solo l'host cambia gli oggetti.\n"
     u8"Equipaggia i tuoi gatti sulla mappa."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"Oggetti\n"
     u8"\n"
     u8"L'host non è ancora tornato sulla mappa:\n"
     u8"aspettalo, il tuo zaino si sbloccherà da solo."},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"Oggetti\n"
     u8"\n"
     u8"Il tuo zaino si aggiorna con quello dell'host:\n"
     u8"chiudilo, poi riaprilo tra un istante."},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"Oggetti\n"
     u8"\n"
     u8"Un altro giocatore ha preso quest'oggetto prima di te:\n"
     u8"la tua modifica non è stata applicata. Lo zaino torna\n"
     u8"come quello dell'host quando lo chiudi."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"Oggetti\n"
     u8"\n"
     u8"L'host è entrato in una casella prima che la tua\n"
     u8"ultima modifica arrivasse: è stata annullata.\n"
     u8"Rifalla sulla mappa."},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"Oggetti\n"
     u8"\n"
     u8"Quest'oggetto non c'è più: un altro giocatore\n"
     u8"l'ha preso appena prima di te."},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).",
     u8"%s non c'è più: i suoi acquisti restano a te (%s)."},
    {u8"%s rapporte ses achats : %s.", u8"%s riporta i suoi acquisti: %s."},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Impossibile inviare a %s i suoi acquisti: restano a te."},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Impossibile inviare a %s i suoi acquisti: restano a te."},
    {u8"Achats de l'aventure", u8"Acquisti dell'avventura"},
    {u8"Boutique", u8"Negozio"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s non hanno finito in questo\n"
     u8"negozio: aspettali (una volta uscito, non\n"
     u8"potresti più comprare per loro).\n"
     u8"Clicca ancora per uscire comunque."},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s non ha finito in questo\n"
     u8"negozio: aspettalo (una volta uscito, non\n"
     u8"potresti più comprare per lui).\n"
     u8"Clicca ancora per uscire comunque."},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"Impossibile inviare la richiesta d'acquisto all'host:\n"
     u8"non è stato comprato niente. Riprova tra un istante."},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"L'host non è più in questo negozio:\n"
     u8"l'acquisto non è avvenuto."},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Quest'oggetto non è più in vendita dall'host:\n"
     u8"l'acquisto non è avvenuto."},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Quest'oggetto è già stato venduto:\n"
     u8"l'acquisto non è avvenuto."},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"Costa %s; la tua borsa ne ha %lld."},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"La tua borsa non basta per quest'oggetto."},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"(Ognuno paga con la sua borsa, anche tu.)"},
    {u8"(Chacun paie avec sa bourse.)", u8"(Ognuno paga con la sua borsa.)"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"Per ora solo l'host compra le caramelle\n"
     u8"e i famigli (con la sua borsa)."},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.",
     u8"Il gioco dell'host non è riuscito a fare l'acquisto."},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"Niente borse in questa avventura:\n"
     u8"compra solo l'host."},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"Non hai portato gatti in questa\n"
     u8"avventura: non hai una borsa per\n"
     u8"comprare."},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"Non hai una borsa in questa avventura\n"
     u8"(non ci hai portato gatti): niente acquisti."},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"L'host non può comprare per te in questo\n"
     u8"momento (ha una finestra aperta):\n"
     u8"non è stato comprato niente. Riprova tra un istante."},
    {u8"un bonbon", u8"una caramella"},
    {u8"un familier", u8"un famiglio"},
    {u8"un article", u8"un articolo"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"Fine dell'avventura\n"
     u8"\n"
     u8"%s tornano a casa di %s."},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"Fine dell'avventura\n"
     u8"\n"
     u8"%s torna a casa di %s."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"I gatti degli altri giocatori non hanno potuto partire\n"
     u8"e la tua scatola di partenza è vuota: niente avventura\n"
     u8"stavolta. (Il log del mod, mgmp.log, spiega perché.)"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"La tua scatola di partenza è vuota e nessuno\n"
     u8"porta gatti: niente avventura stavolta."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Squadra mista\n"
     u8"\n"
     u8"I gatti degli altri giocatori non hanno potuto\n"
     u8"unirsi alla squadra: parti con i tuoi.\n"
     u8"(Il log del mod, mgmp.log, spiega perché.)"},
    {u8"Retour de l'aventure", u8"Ritorno dall'avventura"},
    {u8"%s est de retour à la maison.", u8"%s è di nuovo a casa."},
    {u8"%s sont de retour à la maison.", u8"%s sono di nuovo a casa."},
    {u8"%s a péri pendant l'aventure.", u8"%s ha perso la vita nell'avventura."},
    {u8"%s ont péri pendant l'aventure.", u8"%s hanno perso la vita nell'avventura."},
    {u8"%s ne revient pas de l'aventure.", u8"%s non torna dall'avventura."},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s non tornano dall'avventura."},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s non è più come alla partenza:\n"
     u8"i risultati dell'avventura non sono stati copiati."},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s non sono più come alla partenza:\n"
     u8"i risultati dell'avventura non sono stati copiati."},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"Il tuo deposito è cambiato dalla partenza:\n"
     u8"gli oggetti equipaggiati alla partenza non ne sono stati tolti."},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"Il tuo deposito non è cambiato: gli oggetti equipaggiati\n"
     u8"alla partenza ci sono ancora."},
    {u8"Tu rapportes %s dans ta maison.", u8"Riporti %s a casa tua."},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.",
     u8"I PNG ti ringraziano per i tuoi gatti: %s a casa tua."},
    {u8"Cadeaux des PNJ", u8"Regali dei PNG"},
    {u8"Ton butin est dans ton rangement : %s.", u8"Il tuo bottino è nel tuo deposito: %s."},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.",
     u8"I regali dei PNG sono nel tuo deposito: %s."},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.",
     u8"%s, reclutati durante l'avventura, arrivano a casa tua."},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.",
     u8"%s, reclutato durante l'avventura, arriva a casa tua."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Squadra mista\n"
     u8"\n"
     u8"%s restano fuori dalla squadra:\n"
     u8"in questa avventura controlli una parte\n"
     u8"dei gatti della squadra."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Squadra mista\n"
     u8"\n"
     u8"%s resta fuori dalla squadra:\n"
     u8"in questa avventura controlli una parte\n"
     u8"dei gatti della squadra."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Squadra mista\n"
     u8"\n"
     u8"%s restano fuori dalla squadra:\n"
     u8"segui l'avventura da spettatore."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Squadra mista\n"
     u8"\n"
     u8"%s resta fuori dalla squadra:\n"
     u8"segui l'avventura da spettatore."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"Squadra mista\n"
     u8"\n"
     u8"%s non hanno potuto partire e restano a casa."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"Squadra mista\n"
     u8"\n"
     u8"%s non ha potuto partire e resta a casa."},
    {u8"Tes chats sont rentrés de l'aventure.", u8"I tuoi gatti sono tornati a casa."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"Nuovo gatto in squadra\n"
     u8"\n"
     u8"%s entra nella squadra di %s: questo gatto\n"
     u8"è tuo! Finché non ci sei, l'IA del gioco\n"
     u8"lo controlla in battaglia e %s sceglie per lui,\n"
     u8"e tornerà a casa tua alla fine dell'avventura."},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"Nuovi gatti: %s arrivano da te."},
    {u8"Nouveaux chats : %s arrive chez toi.", u8"Nuovo gatto: %s arriva da te."},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"Il quarto posto\n"
     u8"\n"
     u8"Con 3 giocatori c'è un posto libero:\n"
     u8"è tuo, porti 2 gatti."},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"Il quarto posto\n"
     u8"\n"
     u8"Il posto libero è già di %s:\n"
     u8"con 3 giocatori, porti 1 gatto.\n"
     u8"Togline uno dalla scatola di partenza."},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"Il quarto posto\n"
     u8"\n"
     u8"C'è un quarto giocatore: ognuno porta\n"
     u8"1 gatto. Togline uno dalla scatola di partenza."},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"Il quarto posto va al primo che mette un secondo gatto nella scatola."},
    {u8"La 4e place est à %s.", u8"Il quarto posto è di %s."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Squadra mista\n"
     u8"\n"
     u8"Con %d giocatori, porti al massimo %d gatti:\n"
     u8"togline %d dalla scatola di partenza.%s\n"
     u8"Gli altri giocatori completeranno la squadra con i loro."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Squadra mista\n"
     u8"\n"
     u8"Con %d giocatori, porti al massimo %d gatti:\n"
     u8"togline %d dalla scatola di partenza.%s\n"
     u8"Gli altri giocatori completeranno la squadra con i loro."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Squadra mista\n"
     u8"\n"
     u8"Con %d giocatori, porti al massimo %d gatto:\n"
     u8"togline %d dalla scatola di partenza.%s\n"
     u8"Gli altri giocatori completeranno la squadra con i loro."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Squadra mista\n"
     u8"\n"
     u8"Con %d giocatori, porti al massimo %d gatto:\n"
     u8"togline %d dalla scatola di partenza.%s\n"
     u8"Gli altri giocatori completeranno la squadra con i loro."},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Non sei a casa tua:\n"
     u8"giocherai con i gatti degli altri."},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"La tua scatola di partenza non è ancora sbloccata:\n"
     u8"giocherai con i gatti degli altri."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"La tua scatola di partenza è vuota: giocherai con i gatti degli altri.\n"
     u8"Per portare i tuoi: rispondi \"Non ancora\", mettine 1 o 2 nella tua scatola\n"
     u8"(da Butch), poi clicca sul tuo cartello \"Partenza!\"."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"La tua scatola di partenza è vuota: giocherai con i gatti degli altri.\n"
     u8"Per portare i tuoi: rispondi \"Non ancora\", mettine uno nella tua scatola\n"
     u8"(da Butch), poi clicca sul tuo cartello \"Partenza!\"."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"La tua scatola di partenza contiene %d gatti, al massimo %d:\n"
     u8"rispondi \"Non ancora\", togline %d,\n"
     u8"poi clicca sul tuo cartello \"Partenza!\".%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).",
     u8"I tuoi gatti partono dalla tua scatola (da Butch)."},
    {u8"Tu emmènes %s.", u8"Porti %s."},
    {u8"Départ en aventure", u8"Partenza per l'avventura"},
    {u8"La session n'est pas prête : réessaie dans un instant.",
     u8"La sessione non è pronta: riprova tra un istante."},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"Assegnazione del quarto posto in corso:\n"
     u8"la domanda tornerà tra un istante."},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"Impossibile preparare i tuoi gatti\n"
     u8"per l'avventura (vedi mgmp.log)."},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s tornano da un'avventura, ma i loro risultati\n"
     u8"non sono ancora nel tuo salvataggio: ricaricalo prima\n"
     u8"di farli ripartire (\"Ricarica\" ti verrà proposto\n"
     u8"dopo questa partenza; altrimenti Esc,\n"
     u8"\"Salva ed esci al menu\", poi \"Gioca\" e il salvataggio)."},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s torna da un'avventura, ma i suoi risultati\n"
     u8"non sono ancora nel tuo salvataggio: ricaricalo prima\n"
     u8"di farlo ripartire (\"Ricarica\" ti verrà proposto\n"
     u8"dopo questa partenza; altrimenti Esc,\n"
     u8"\"Salva ed esci al menu\", poi \"Gioca\" e il salvataggio)."},
    {u8"Le retour de %s attend d'être recopié.", u8"I risultati di %s sono da copiare."},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"L'invio della tua risposta all'host non è riuscito:\n"
     u8"riprova."},
    {u8"Ta bourse de l'aventure : %s.", u8"La tua borsa dell'avventura: %s."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"La tua scatola di partenza contiene %d gatti, al massimo %d:\n"
     u8"togline %d, poi clicca sul tuo cartello \"Partenza!\"."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"Nuovo gatto in squadra\n"
     u8"\n"
     u8"%s entra in squadra: è tuo!\n"
     u8"Lo controlli tu e tornerà a casa tua\n"
     u8"alla fine dell'avventura."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"Nuovo gatto in squadra\n"
     u8"\n"
     u8"%s entra in squadra: è tuo."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"Nuovo gatto in squadra\n"
     u8"\n"
     u8"%s entra in squadra: è di %s."},
    {u8"Un nouveau chat", u8"Un nuovo gatto"},
    {u8"en bas", u8"in basso"},
    {u8"en haut", u8"in alto"},
    {u8"à droite", u8"a destra"},
    {u8"à gauche", u8"a sinistra"},
    {u8"au milieu", u8"al centro"},
    {u8"n°%d", u8"n°%d"},
    {u8"%s (%s)", u8"%s (%s)"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]",
     u8"[c:red]Il tuo voto non è stato accettato: %s.[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"Scelta del percorso\n"
     u8"\n"},
    {u8"Même chemin", u8"Stesso percorso"},
    {u8"Choisir", u8"Scegli"},
    {u8"Changer", u8"Cambia"},
    {u8"Sortie", u8"Uscita"},
    {u8"Retour à la maison", u8"Ritorno a casa"},
    {u8"Combat", u8"Battaglia"},
    {u8"Combat difficile", u8"Battaglia difficile"},
    {u8"Mini-boss", u8"Miniboss"},
    {u8"Boss", u8"Boss"},
    {u8"Événement", u8"Evento"},
    {u8"Événement spécial", u8"Evento speciale"},
    {u8"Trésor", u8"Tesoro"},
    {u8"Meuble", u8"Mobili"},
    {u8"Nourriture", u8"Cibo"},
    {u8"Bonus", u8"Bonus"},
    {u8"Case vide", u8"Casella vuota"},
    {u8"Case", u8"Casella"},
    {u8"l'hôte n'est pas sur la carte", u8"l'host non è sulla mappa"},
    {u8"vos cartes ne sont pas les mêmes", u8"le vostre mappe non sono uguali"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"l'host non può andarci"},
    {u8"le chemin est déjà choisi", u8"il percorso è già scelto"},
    {u8"ce tour est fini", u8"questo turno è finito"},
    {u8"l'hôte ne l'a pas pris", u8"l'host non l'ha accettato"},
    {u8"pas de vote", u8"nessun voto"},
    {u8"absent, ne vote pas", u8"assente, non vota"},
    {u8"pas encore sur la carte", u8"non ancora sulla mappa"},
    {u8"pas encore voté", u8"non ha ancora votato"},
    {u8"%s (toi) : %s", u8"%s (tu): %s"},
    {u8"%s : %s", u8"%s: %s"},
    {u8"En pause : on attend tout le monde sur la carte.",
     u8"In pausa: si aspetta che tutti siano sulla mappa."},
    {u8"À toi de choisir : encore %u s.", u8"Tocca a te scegliere: ancora %u s."},
    {u8"En attente des autres : encore %u s.", u8"In attesa degli altri: ancora %u s."},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"Pareggio tra %s:\n"
     u8"la sorte sceglie %s."},
    {u8"Tout le monde a choisi %s.", u8"Tutti hanno scelto %s."},
    {u8"%s l'emporte.", u8"Vince %s."},
    {u8"%s : prêt", u8"%s: pronto"},
    {u8"%s : défend sa maison", u8"%s: difende la sua casa"},
    {u8"%s : pas encore", u8"%s: non ancora"},
    {u8"Tout le monde est prêt !", u8"Sono tutti pronti!"},
    {u8"En attente d'un joueur.", u8"In attesa di un giocatore."},
    {u8"En attente de %d joueurs.", u8"In attesa di %d giocatori."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"Fine giornata\n"
     u8"\n"
     u8"Sei pronto a passare la notte.\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"Il tuo cartello serve a partire per l'avventura:\n"
     u8"togli i tuoi gatti dalla squadra per finire la giornata."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Fine giornata\n"
     u8"\n"
     u8"1 giocatore vuole passare la notte.\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Fine giornata\n"
     u8"\n"
     u8"%d giocatori vogliono passare la notte.\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1 giocatore aspetta"},
    {u8"%d joueurs attendent", u8"%d giocatori aspettano"},
    {u8"Passer la nuit", u8"Fine giornata"},
    {u8"Plus tard", u8"Più tardi"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"Per continuare, premi Esc\n"
     u8"e poi \"Salva ed esci al menu\"."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"Cosa ti aspetta\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Il tuo salvataggio è già aperto: tutto questo ci\n"
     u8"sarà solo al prossimo caricamento. \"Ricarica\":\n"
     u8"il mod passa dal menu e lo riapre."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"Cosa ti aspetta\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Il tuo salvataggio è già aperto: tutto questo ci\n"
     u8"sarà solo al prossimo caricamento. Ricaricalo:\n"
     u8"Esc, \"Salva ed esci al menu\",\n"
     u8"poi \"Gioca\" e il tuo salvataggio."},
    {u8"%s : j'arrive !", u8"%s: arrivo!"},
    {u8"%s : réfléchit...", u8"%s: ci pensa..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"Vuoi partire all'avventura.\n"
     u8"La tua squadra risponde:\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s: non ancora qui"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"In attesa della tua squadra\n"
     u8"\n"
     u8"%s\n"
     u8"Se avanzi senza aspettarla, l'IA del gioco\n"
     u8"controlla i suoi gatti in battaglia fino al\n"
     u8"suo arrivo."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"%s parte all'avventura!\n"
     u8"Ognuno porta e controlla i propri gatti.\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"Avventura in corso\n"
     u8"\n"
     u8"%s è in avventura!\n"
     u8"I tuoi gatti non partecipano: seguirai\n"
     u8"l'avventura da spettatore."},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"Avventura in corso\n"
     u8"\n"
     u8"%s è in avventura con i tuoi gatti!\n"
     u8"Finché non ci sei, l'IA del gioco li controlla\n"
     u8"in battaglia e %s sceglie per loro.\n"
     u8"Ti unisci alla sua squadra?"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"Avventura in corso\n"
     u8"\n"
     u8"%s è in avventura!\n"
     u8"Ti unisci alla sua squadra?"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"Lasciare l'avventura\n"
     u8"\n"
     u8"L'avventura di %s è finita:\n"
     u8"torni a casa tua."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"Lasciare l'avventura\n"
     u8"\n"
     u8"È l'avventura di %s: solo lui può\n"
     u8"abbandonarla. Continua senza di te.\n"
     u8"\n"
     u8"Torni a casa tua? Per rientrare,\n"
     u8"clicca su un cartello della tua casa."},
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
     u8"Lasciare l'avventura\n"
     u8"\n"
     u8"È l'avventura di %s: solo lui può\n"
     u8"abbandonarla. Continua senza di te, e i tuoi\n"
     u8"gatti restano nella sua squadra: l'IA del gioco\n"
     u8"li controlla in battaglia, poi tornano a\n"
     u8"casa tua alla fine.\n"
     u8"\n"
     u8"Torni a casa tua? Per rientrare,\n"
     u8"clicca su un cartello della tua casa."},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"Unirsi all'avventura\n"
     u8"\n"
     u8"Il gioco di %s ti invia l'avventura in corso.\n"
     u8"Se ha appena fatto una scelta in una casella\n"
     u8"(evento, aumento di livello), arriverai\n"
     u8"al suo ritorno sulla mappa."},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"In viaggio verso l'avventura\n"
     u8"\n"
     u8"La partita di %s sta per caricarsi:\n"
     u8"partirai con lui."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Ritorno a casa\n"
     u8"\n"
     u8"La connessione con %s si è interrotta.\n"
     u8"Torni a casa tua."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"Ritorno a casa\n"
     u8"\n"
     u8"La partenza con %s non è avvenuta.\n"
     u8"Torni a casa tua."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Ritorno a casa\n"
     u8"\n"
     u8"L'avventura di %s è finita.\n"
     u8"Torni a casa tua."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Ritorno a casa\n"
     u8"\n"
     u8"La battaglia di %s era finita, ma la tua\n"
     u8"copia continuava: si era desincronizzata,\n"
     u8"nessuno giocava più quella battaglia. Sei\n"
     u8"tornato; i tuoi gatti restano nella sua squadra.\n"
     u8"\n"
     u8"Per tornare nell'avventura, clicca su\n"
     u8"un cartello della tua casa."},
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
     u8"Ritorno a casa\n"
     u8"\n"
     u8"La tua battaglia è finita, ma non quella\n"
     u8"di %s: la tua copia si era\n"
     u8"desincronizzata. Sei tornato; i tuoi\n"
     u8"gatti restano nella sua squadra e l'IA\n"
     u8"del gioco li controlla fino alla fine.\n"
     u8"\n"
     u8"Per tornare nell'avventura, clicca su\n"
     u8"un cartello della tua casa."},
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
     u8"Ritorno a casa\n"
     u8"\n"
     u8"%s è entrato in una casella che il tuo gioco\n"
     u8"non ha potuto seguire: la tua copia dell'avventura\n"
     u8"non era più la stessa. Sei tornato; i tuoi\n"
     u8"gatti restano nella sua squadra, l'IA del gioco\n"
     u8"li controlla.\n"
     u8"\n"
     u8"Per tornare nell'avventura, clicca su\n"
     u8"un cartello della tua casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Ritorno a casa\n"
     u8"\n"
     u8"La tua copia della battaglia si è desincronizzata\n"
     u8"da quella di %s: la battaglia continuava senza\n"
     u8"di te, quindi sei tornato. I tuoi gatti restano nella\n"
     u8"sua squadra, l'IA del gioco li controlla.\n"
     u8"\n"
     u8"Per tornare nell'avventura, clicca su\n"
     u8"un cartello della tua casa."},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"Il tuo salvataggio è a metà avventura.\n"
     u8"In multigiocatore, le avventure si giocano con l'host:\n"
     u8"torna al menu e scegli un altro salvataggio,\n"
     u8"o finisci questo da solo."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"Il tuo gioco carica l'avventura di %s."},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%s ha ricaricato l'avventura:\n"
     u8"la ricarichi anche tu."},
    {u8"Rentrer", u8"Torna a casa"},
    {u8"Rester", u8"Resta"},
    {u8"Ne pas attendre", u8"Non aspettare"},
    {u8"Recharger", u8"Ricarica"},
    {u8"J'arrive", u8"Arrivo!"},
    {u8"Pas encore", u8"Non ancora"},
    {u8"Échanger", u8"Scambia"},
    {u8"Multijoueur", u8"Multigiocatore"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"Questo tubo è della casa di %s:\n"
     u8"manda i tuoi gatti ai PNG dalla\n"
     u8"tua casa."},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"I PNG seguono la partita di %s:\n"
     u8"non puoi mandare i tuoi gatti nel tubo\n"
     u8"con questa versione del gioco."},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"È una sconfitta per tutta la squadra:\n"
     u8"neanche i gatti di %s torneranno a casa."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"Unisciti a %s\n"
     u8"\n"
     u8"Sei già in una lobby multigiocatore:\n"
     u8"esci prima da lì (Multigiocatore, poi \"Esci\"),\n"
     u8"poi clicca di nuovo su \"Unisciti alla partita\"."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"Unisciti a %s\n"
     u8"\n"
     u8"Il tuo gioco tornerà al menu principale (la partita\n"
     u8"viene salvata, come con \"Salva ed esci al menu\"),\n"
     u8"poi entrerà nella lobby di %s.\n"
     u8"\n"
     u8"Più tardi: ci entrerà quando andrai\n"
     u8"al menu principale."},
    {u8"Y aller", u8"Vai"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"Unisciti a %s\n"
     u8"\n"
     u8"Il tuo gioco entrerà nella lobby di %s\n"
     u8"appena sarai al menu principale\n"
     u8"(Esc, poi \"Salva ed esci al menu\")."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"Unisciti a %s\n"
     u8"\n"
     u8"La connessione tramite Steam per ora è impossibile.\n"
     u8"Riprova tra poco, o unisciti tramite indirizzo\n"
     u8"(Multigiocatore, poi \"Unisciti\")."},
    {u8"%s a annulé le départ.", u8"%s ha annullato la partenza."},
    {u8"réseau local", u8"rete locale"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"I tuoi gatti sono già partiti con la squadra di %s:\n"
     u8"finché non ci sei, l'IA del gioco li controlla\n"
     u8"in battaglia e %s sceglie per loro; torneranno\n"
     u8"a casa tua alla fine dell'avventura."},
    {u8"Internet", u8"Internet"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s difende la sua casa: una battaglia che gioca\n"
     u8"da solo, con i suoi gatti. Poi torna."},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s difendono la loro casa: ognuno gioca\n"
     u8"la sua battaglia da solo, con i suoi gatti. Poi tornano."},
    {u8"%s a fini de défendre sa maison.", u8"%s ha finito di difendere la sua casa."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s ha finito di difendere la sua casa.\n"
     u8"%s difende ancora la sua."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s ha finito di difendere la sua casa.\n"
     u8"%s difendono ancora la loro."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"Hai annullato dopo il voto: %s parte\n"
     u8"senza di te né i tuoi gatti. Potrai unirti\n"
     u8"alla sua avventura quando sarà sulla mappa."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"Partenza per l'avventura\n"
     u8"\n"
     u8"Nessun gatto può partire stavolta:\n"
     u8"%s non parte per l'avventura."},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"La spartizione del bottino non è finita:\n"
     u8"aspetta che sfilino tutti gli oggetti."},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"Parti per unirti all'avventura di %s:\n"
     u8"difenderai la tua casa al tuo ritorno."},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"È %s ad avviare l'avventura:\n"
     u8"chiedigli di cliccare su \"Partenza!\"."},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"Multigiocatore\n"
     u8"\n"
     u8"Ospita una partita a cui i tuoi amici possano unirsi,\n"
     u8"o unisciti alla partita di un amico."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"[c:red]Impossibile aprire la porta %u.[/c]\n"
     u8"Forse un altro programma la sta usando.\n"
     u8"Chiudi la lobby, poi riprova."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Apertura della lobby..."},
    {u8"%s (hôte)", u8"%s (host)"},
    {u8"En attente d'un ami...", u8"In attesa di un amico..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"I tuoi amici Steam si uniscono con un clic: nella loro\n"
     u8"lista amici di Steam, \"Unisciti alla partita\" sul tuo nome.\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Oppure tramite indirizzo:\n"
     u8"%s   (%s, porta %u)\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Dai questo indirizzo al tuo amico:\n"
     u8"%s   (%s, porta %u)\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"È già stato copiato: incollaglielo con Ctrl+V.\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"Indirizzo non trovato su questo PC (porta %u).\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"%s\n"
     u8"Giocatori (%d/4):\n"
     u8"%s\n"
     u8"Quando ci sono tutti, scegli il tuo salvataggio."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"Unisciti a una partita\n"
     u8"\n"
     u8"Indirizzo dell'host:\n"
     u8"%s_\n"
     u8"\n"
     u8"Digita l'indirizzo del tuo amico,\n"
     u8"o incollalo con Ctrl+V e premi Invio.%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Con Steam è più semplice: nella tua lista\n"
     u8"amici, \"Unisciti alla partita\" sul tuo amico."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"Unisciti a una partita\n"
     u8"\n"
     u8"[c:red]Indirizzo non valido: %s[/c]\n"
     u8"\n"
     u8"Esempio: 26.123.45.67\n"
     u8"o, con un'altra porta: 26.123.45.67:27600"},
    {u8"(vide)", u8"(vuoto)"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.",
     u8"Verificate di avere la stessa versione del gioco e del mod."},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).",
     u8"La partita dell'host è piena (massimo 4 giocatori)."},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"Non avete la stessa versione del mod:\n"
     u8"scaricate entrambi l'ultima."},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"Non avete la stessa versione del gioco:\n"
     u8"aggiornate entrambi Mewgenics (Steam)."},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"I file del gioco non sono uguali:\n"
     u8"verificateli su Steam (Proprietà, File installati)."},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"L'impostazione net.control di mgmp.json dà lo stesso gatto\n"
     u8"ai due giocatori: rimettetela su \"auto\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"[c:red]La connessione è stata rifiutata.[/c]\n"
     u8"%s\n"
     u8"(%s)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"[c:red]Impossibile unirsi a %s:[/c]\n"
     u8"%s.\n"
     u8"Esci dalla lobby, poi riprova (o tramite indirizzo)."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Connessione a %s..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"[c:red]Impossibile raggiungere %s.[/c]\n"
     u8"Controlla l'indirizzo e che il tuo amico\n"
     u8"abbia cliccato su \"Ospita\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Disconnesso."},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"La schermata dei salvataggi si aprirà da sola."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Connessione a %s...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"[c:red]La connessione con l'host si è interrotta.[/c]\n"
     u8"Esci dalla lobby, poi rientraci."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Connessione a %s tramite Steam...\n"
     u8"(qualche secondo)%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Connesso, presentazione all'host...%s"},
    {u8"%s (toi)", u8"%s (tu)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Hai raggiunto la partita di %s!\n"
     u8"\n"
     u8"Giocatori (%d/4):\n"
     u8"%s\n"
     u8"In attesa della partita di %s:\n"
     u8"appena avrà scelto il salvataggio,\n"
     u8"la schermata dei salvataggi si aprirà da sola."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Hai raggiunto la partita di %s!\n"
     u8"\n"
     u8"Giocatori (%d/4):\n"
     u8"%s\n"
     u8"In attesa della partita di %s:\n"
     u8"appena avrà scelto il salvataggio,\n"
     u8"potrai scegliere il tuo."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"Lobby multigiocatore\n"
     u8"\n"
     u8"Hai raggiunto la partita di %s!\n"
     u8"\n"
     u8"Giocatori (%d/4):\n"
     u8"%s\n"
     u8"Scegli il tuo salvataggio: la tua casa, i tuoi gatti.\n"
     u8"I PNG e la storia saranno quelli di %s,\n"
     u8"e per partire all'avventura, ti unirai alla sua squadra."},
    {u8"Héberger", u8"Ospita"},
    {u8"Rejoindre", u8"Unisciti"},
    {u8"Choisir la sauvegarde", u8"Scegli il salvataggio"},
    {u8"Arrêter", u8"Chiudi"},
    {u8"Choisir ta sauvegarde", u8"Scegli il tuo salvataggio"},
    {u8"Quitter", u8"Esci"},
    {u8"Annuler", u8"Annulla"},
    {u8"Échanger avec %s", u8"Scambia con %s"},
    {u8"Donner à qui ?", u8"A chi dare?"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"Su o Giù per scegliere, poi Invio."},
    {u8"Pièces à donner : %lld_", u8"Monete da dare: %lld_"},
    {u8"Nourriture à donner : %lld_", u8"Cibo da dare: %lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"Digita un numero (T: tutto), poi Invio."},
    {u8"Échanger avec %s : les objets de ton rangement", u8"Scambia con %s: gli oggetti del tuo deposito"},
    {u8"16 au plus par don : %u choisis.", u8"Massimo 16 per dono: %u scelti."},
    {u8"16 au plus par don : %u choisi.", u8"Massimo 16 per dono: %u scelto."},
    {u8"Lecture du rangement...", u8"Lettura del deposito..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"Su/Giù: scegli, Spazio: seleziona, Sinistra/Destra: quanti.\n"
     u8"Lettere per cercare, poi Invio."},
    {u8"Tu n'as rien choisi à donner.", u8"Non hai scelto niente da dare."},
    {u8"Tu n'as rien à donner pour l'instant.", u8"Per ora non hai niente da dare."},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"Dare %s dalla tua borsa\n"
     u8"a %s?"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"Dare %s\n"
     u8"a %s?"},
    {u8"Envoi du don...", u8"Invio del dono..."},
    {u8"Personne ne peut recevoir maintenant.", u8"Nessuno può ricevere adesso."},
    {u8"Personne d'autre n'est dans la partie.", u8"Non c'è nessun altro nella partita."},
    {u8"Confirmer", u8"Conferma"},
    {u8"Donner", u8"Dai"},
    {u8"Recommencer", u8"Ricomincia"},
    {u8"Fermer", u8"Chiudi"},
    {u8"Pas d'échange possible ici.", u8"Nessuno scambio possibile qui."},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"La finestra si è chiusa: l'host è entrato in una casella.\n"
     u8"Ciò che avevi scelto è conservato (E per riprendere)."},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"Scambi in avventura\n"
     u8"\n"
     u8"Sulla mappa, premi E per dare monete\n"
     u8"o cibo dalla tua borsa a un altro giocatore."},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
