// lang_ru.cpp -- Mewgenics Multi : les textes du mod et du lanceur en russe (ru).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : le tutoiement russe, comme le tutoiement du francais du mod.
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_ru[];
const TrPair kTable_ru[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"Не удалось прочитать mgmp.json: настройки не сохранены."},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"В mgmp.json есть комментарии (// или /* */), а сохранение их сотрёт: удали их или измени настройку в файле вручную."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.json — некорректный JSON: исправь его (или верни файл из zip-архива), прежде чем менять настройки."},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"Не удалось записать в папку мода: настройки не сохранены."},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"Не удалось заменить mgmp.json (он открыт в другой программе?): настройки не сохранены."},
    {u8"Steam est ouvert.", u8"Steam запущен."},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Без него игра закроется раньше, чем загрузится мод. Запусти Steam и дождись подключения."},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"Рядом с Mewgenics\u00A0Multi.exe нет mgmp.json: сохрани настройки, чтобы создать его заново."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.json — некорректный JSON: исправь его или верни файл из zip-архива."},
    {u8"Réglages lus dans mgmp.json.", u8"Настройки прочитаны из mgmp.json."},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe не найден (%s): в «Настройках» нажми «Обзор…» и выбери Mewgenics.exe."},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"Рядом с Mewgenics.exe нет resources.gpak: проверь файлы игры в Steam."},
    {u8"Jeu trouvé : %s.", u8"Игра найдена: %s."},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.",
     u8"steam_appid.txt: проверим, когда игра будет найдена."},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Его нет рядом с Mewgenics.exe: без него Steam перезапустит игру без мода."},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"Файл рядом с Mewgenics.exe не содержит 686060: Steam перезапустил бы игру без мода."},
    {u8"steam_appid.txt est en place à côté du jeu.", u8"steam_appid.txt на месте, рядом с игрой."},
    {u8"Fichiers du mod présents.", u8"Файлы мода на месте."},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Рядом с Mewgenics\u00A0Multi.exe нет mgmp.dll и mgmp_loader.exe: заново распакуй всю папку из zip-архива."},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Рядом с Mewgenics\u00A0Multi.exe нет %s: заново распакуй всю папку из zip-архива."},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi, сборка %s, протокол %u: у всех игроков они должны совпадать."},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"Игра не найдена: сначала выбери Mewgenics.exe в «Настройках»."},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"Не удалось записать steam_appid.txt рядом с игрой: скопируй его вручную (LISEZ-MOI, шаг 2)."},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"Windows не получила твоего согласия: steam_appid.txt не скопирован. Можешь попробовать ещё раз или скопировать его вручную (LISEZ-MOI, шаг 2)."},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics Multi не смог перезапуститься от имени администратора: скопируй steam_appid.txt вручную (LISEZ-MOI, шаг 2)."},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Копирование от имени администратора не удалось: скопируй steam_appid.txt вручную (LISEZ-MOI, шаг 2)."},
    {u8"%02u/%02u/%04u à %02u:%02u", u8"%02u.%02u.%04u в %02u:%02u"},
    {u8"Le mod n'a pas pu être chargé : %s", u8"Не удалось загрузить мод: %s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exe завершился, не загрузив мод. Запусти снова; если это повторится, пришли loader.txt."},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exe завершился (код %s), не загрузив мод. Скорее всего, Steam закрыт или не в сети. Запусти Steam, дождись подключения и попробуй снова."},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"Игра закрылась во время запуска, ещё до мода. Чаще всего причина в том, что Steam закрыт или не в сети. Запусти Steam, дождись подключения и попробуй снова."},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"Steam перезапускает игру (это может занять минуту-другую)…"},
    {u8"Ouverture du jeu…", u8"Открываем игру…"},
    {u8"Le jeu démarre…", u8"Игра запускается…"},
    {u8"Injection du mod dans le jeu…", u8"Внедряем мод в игру…"},
    {u8"Le mod est en place, le jeu reprend…", u8"Мод на месте, игра продолжает запуск…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Всё ещё ждём игру, перезапущенную Steam… Если Steam только что открылся, возможно, он задаёт тебе вопрос."},
    {u8"Jeu relancé trouvé : injection du mod…", u8"Перезапущенная игра найдена: внедряем мод…"},
    {u8"Le jeu est lancé avec le mod.", u8"Игра запущена с модом."},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.",
     u8"Мод долго не загружался; игра всё равно продолжает запуск."},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.",
     u8"Перезапущенная игра долго стартует; внедряем всё равно."},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"Рядом с Mewgenics\u00A0Multi.exe не найден mgmp.dll: заново распакуй папку мода из zip-архива."},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe не найден: в «Настройках» нажми «Обзор…» и выбери Mewgenics.exe."},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"mgmp.json не читается: исправь его или верни файл из zip-архива."},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steam перезапустил игру без мода: проверь steam_appid.txt рядом с Mewgenics.exe (кнопка «Исправить») и попробуй снова."},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"Это не та версия игры, для которой сделан мод (1.1.21239): может, игра обновилась?"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"Какая-то программа (оверлей, антивирус, античит) помешала загрузке мода."},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"Игра слишком долго запускалась: мод не загружен. Попробуй снова."},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"Внедрить мод не удалось, игра закрыта. Запусти снова; если это повторится, пришли loader.txt."},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"В mgmp.json не указано, где игра: в «Настройках» нажми «Обзор…» и выбери Mewgenics.exe."},
    {u8"build %s · protocole %u", u8"сборка %s · протокол %u"},
    {u8"Steam ouvert", u8"Steam запущен"},
    {u8"Steam fermé", u8"Steam закрыт"},
    {u8"Réglages (mgmp.json)", u8"Настройки (mgmp.json)"},
    {u8"Jeu trouvé", u8"Игра найдена"},
    {u8"Jeu incomplet", u8"Игра неполная"},
    {u8"Jeu introuvable", u8"Игра не найдена"},
    {u8"Fichiers du mod", u8"Файлы мода"},
    {u8"Réparer", u8"Исправить"},
    {u8"Jouer", u8"Играть"},
    {u8"Règle d'abord les croix rouges.", u8"Сначала исправь красные крестики."},
    {u8"Réglages", u8"Настройки"},
    {u8"Journaux", u8"Журналы"},
    {u8"Langue", u8"Язык"},
    {u8"Le jeu (Mewgenics.exe)", u8"Игра (Mewgenics.exe)"},
    {u8"Parcourir…", u8"Обзор…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe и resources.gpak на месте."},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"Рядом нет resources.gpak: это не та папка."},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.",
     u8"Здесь нет Mewgenics.exe: выбери его через «Обзор…»."},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Играть вместе через Steam (список друзей)"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Сними галочку, если со Steam проблемы: останется только адрес, в меню «Мультиплеер» игры."},
    {u8"Enregistrer", u8"Сохранить"},
    {u8"Retour", u8"Назад"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"Журналов пока нет: мод создаёт новый журнал в каждой игре."},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"1 журнал мода, рядом с Mewgenics\u00A0Multi.exe."},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"Журналов мода: %d, рядом с Mewgenics\u00A0Multi.exe."},
    {u8"Le dernier : %s", u8"Последний: %s"},
    {u8"écrit le %s", u8"записан %s"},
    {u8"Ouvrir le dossier", u8"Открыть папку"},
    {u8"Copier le dernier journal", u8"Копировать журнал"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"Чтобы попросить помощи, скопируй последний журнал и вставь его в Discord через Ctrl+V."},
    {u8"Lancement", u8"Запуск"},
    {u8"Le jeu se lance avec le mod…", u8"Игра запускается с модом…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.", u8"Приятной игры! Это окно закроется само."},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"Подробности — в loader.txt, рядом с Mewgenics\u00A0Multi.exe."},
    {u8"Réessayer", u8"Повторить"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"Если Steam перезапустит игру, это может занять минуту-другую."},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.",
     u8"Журнал скопирован: вставь его в Discord через Ctrl+V."},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"Не удалось скопировать в буфер обмена: попробуй ещё раз или открой папку."},
    {u8"Où est Mewgenics.exe ?", u8"Где Mewgenics.exe?"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"Direct2D недоступен на этом ПК: Mewgenics Multi не может отобразиться."},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"Не удалось подготовить видеокарту: Mewgenics Multi не может отобразиться."},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"У каждого свой дом и котейки,\n"
     u8"а поход — общий!"},
    {u8"(illustrations du jeu introuvables)", u8"(иллюстрации игры не найдены)"},
    {u8"Comme le jeu (%s)", u8"Как в игре (%s)"},
    {u8"Comme Windows (%s)", u8"Как в Windows (%s)"},
    {u8"Automatique (%s)", u8"Автоматически (%s)"},
    {u8"Lancement annulé.", u8"Запуск отменён."},
    {u8"Lancement du mod…", u8"Запуск мода…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windows не смогла подготовить запуск. Перезапусти Mewgenics Multi."},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exe не запустился: заново распакуй папку мода из zip-архива (или антивирус поместил его в карантин)."},
    {u8"la connexion Steam avec l'hôte est coupée", u8"связь с хостом через Steam прервалась"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"хост не отвечает через Steam (лобби закрыто или у него нет мода?)"},
    {u8"Steam refuse la connexion", u8"Steam отклоняет подключение"},
    {u8"À %s de choisir", u8"Выбирает %s"},
    {u8"À toi de choisir", u8"Выбираешь ты"},
    {u8"À %s de choisir la capacité à remplacer.", u8"%s выбирает абилку для замены."},
    {u8"Ce chat est à %s", u8"Это котейка игрока %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"Выбор ошейников\n"
     u8"\n"
     u8"Общий выбор прервался (ошибка игры):\n"
     u8"выбери ошейники для всей партии."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"Выбор ошейников\n"
     u8"\n"
     u8"Готово! Хост %s знает, что у тебя всё:\n"
     u8"он подтвердит выход за всю партию."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"Выбор ошейников\n"
     u8"\n"
     u8"Выход подтверждает %s:\n"
     u8"скажи, когда твои котейки будут в ошейниках."},
    {u8"%s et %s", u8"%s и %s"},
    {u8"%s, %s", u8"%s, %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Выбор ошейников\n"
     u8"\n"
     u8"%s ещё выбирают ошейники.\n"
     u8"Нажми «Подтвердить!» ещё раз,\n"
     u8"чтобы не ждать."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Выбор ошейников\n"
     u8"\n"
     u8"%s ещё выбирает ошейники.\n"
     u8"Нажми «Подтвердить!» ещё раз,\n"
     u8"чтобы не ждать."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Сборы игрока %s здесь больше не видны.\n"
     u8"Пройди эти экраны самостоятельно: вы\n"
     u8"встретитесь на карте."},
    {u8"  %s : parti", u8"  %s: в пути"},
    {u8"  %s : prêt !", u8"  %s: всё готово!"},
    {u8"  %s : charge ta partie...", u8"  %s: загружает твою игру..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Команда загружает твою игру, чтобы пойти с тобой:\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"Выбор предметов\n"
     u8"\n"
     u8"Твою кладовую не удалось загрузить здесь:\n"
     u8"котейки уйдут с теми предметами, что на них."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Выбор предметов\n"
     u8"\n"
     u8"%s ещё снаряжают котеек.\n"
     u8"Нажми «Подтвердить!» ещё раз,\n"
     u8"чтобы не ждать."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Выбор предметов\n"
     u8"\n"
     u8"%s ещё снаряжает котеек.\n"
     u8"Нажми «Подтвердить!» ещё раз,\n"
     u8"чтобы не ждать."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"Выбор предметов\n"
     u8"\n"
     u8"Готово! Котейки уйдут с этими предметами:\n"
     u8"%s подтвердит выход за всю партию."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"Выбор предметов\n"
     u8"\n"
     u8"Предметы, выбранные для твоих котеек, не удалось\n"
     u8"перенести в поход игрока %s: котейки\n"
     u8"уйдут с тем, что у них было, а кладовая\n"
     u8"не изменится."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"Выбор предметов\n"
     u8"\n"
     u8"Игрок %s уже подтвердил выход:\n"
     u8"котейки уйдут с тем, что выбрано,\n"
     u8"остальное не в счёт."},
    {u8"Ta bourse : %s.", u8"Твой кошелёк: %s."},
    {u8"%s n'est plus dans la partie.", u8"Игрока %s больше нет в игре."},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"В этом походе у тебя нет кошелька (в нём нет твоих котеек)."},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"Обмен работает у тебя дома или в совместном походе через кошелёк. Сейчас: %s."},
    {u8"pas de partie à plusieurs en cours", u8"сейчас нет совместной игры"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés",
     u8"не хватает функции игры, обмен дома отключён"},
    {u8"tu es dans la partie de l'hôte", u8"ты в игре хоста"},
    {u8"tu n'es pas dans ta maison", u8"ты не у себя дома"},
    {u8"ta save n'est pas chargée", u8"твоё сохранение не загружено"},
    {u8"un départ en aventure est en cours", u8"идут сборы в поход"},
    {u8"ta maison est attaquée", u8"твой дом атакуют"},
    {u8"ta maison est illisible", u8"не удаётся прочитать данные дома"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"твоя кладовая ждёт добычу котеек из похода хоста (если они уже вернулись, загрузи сохранение заново: Esc, «Сохраниться и выйти в меню», затем «Играть» и твоё сохранение)"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"предметы из твоей кладовой ещё ждут нового владельца (добыча, покупки, NPC)"},
    {u8"Pas d'échange maintenant : %s.", u8"Сейчас обмен невозможен: %s."},
    {u8"Ton don précédent attend encore sa réponse.", u8"Твой прошлый подарок ещё ждёт ответа."},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"Мод ещё не знает твоё сохранение: загрузи его заново из меню."},
    {u8"Choisis un joueur de la partie.", u8"Выбери игрока из этой игры."},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.",
     u8"Сейчас обмен невозможен: игрока %s нет дома."},
    {u8"Ce montant n'est pas possible.", u8"Такое количество недопустимо."},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.",
     u8"Выбери монеты, еду или предметы для подарка."},
    {u8"16 objets au plus par don.", u8"Максимум 16 предметов в подарке."},
    {u8"Pas d'objets maintenant : %s.", u8"Сейчас предметы недоступны: %s."},
    {u8"Ta maison est illisible : pas de don.", u8"Данные дома не читаются: подарка не будет."},
    {u8"Tu n'as pas autant de pièces.", u8"У тебя нет столько монет."},
    {u8"Tu n'as pas autant de nourriture.", u8"У тебя нет столько еды."},
    {u8"Ton rangement est illisible : pas de don.", u8"Данные кладовой не читаются: подарка не будет."},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"Выбранный предмет уже обещан в незавершённом подарке."},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"Выбранного предмета больше нет в кладовой: выбери заново."},
    {u8"Un objet choisi ne peut pas se donner.", u8"Выбранный предмет нельзя подарить."},
    {u8"Le don n'a pas pu partir (connexion).", u8"Подарок не удалось отправить (связь)."},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…",
     u8"Подарок отправлен игроку %s (%s): ждём ответа…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…",
     u8"Запрос на подарок отправлен хосту: ждём ответа…"},
    {u8"Pas de don : %s.", u8"Подарка не будет: %s."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"Игрок %s получил твой подарок: предметы покинут кладовую, как только закроется экран предметов."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"Игрок %s получил твой подарок: предметы покинут кладовую при первой возможности (после выхода в поход, защиты дома или возвращения котеек)."},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"Игрок %s получил твой подарок: подаренное покинет твой дом, когда ты вернёшься в это сохранение."},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"Игрок %s не ответил на подарок: из дома ничего не ушло."},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"Игрок %s не ответил на подарок: из дома ничего не ушло."},
    {u8"Les échanges marchent pendant une partie à plusieurs.",
     u8"Обмен работает только в совместной игре."},
    {u8"Dans ta maison : %s.", u8"У тебя дома: %s."},
    {u8"%s n'est pas dans sa maison.", u8"Игрока %s нет дома."},
    {u8"%lld pièces", u8"%lld мон."},
    {u8"%lld pièce", u8"%lld мон."},
    {u8"%lld nourritures", u8"%lld ед. еды"},
    {u8"%lld nourriture", u8"%lld ед. еды"},
    {u8"rien", u8"ничего"},
    {u8"%s n'est pas dans sa maison", u8"игрока %s нет дома"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s",
     u8"у игрока %s открыто окно или экран предметов"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"игре игрока %s не удалось их разместить"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"игре игрока %s не удалось их разместить"},
    {u8"les dons ne marchent pas chez %s", u8"у игрока %s подарки не работают"},
    {u8"%s n'est plus dans la partie", u8"игрока %s больше нет в игре"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"у игрока %s ещё есть котейки в походе хоста (его кладовая ждёт их возвращения)"},
    {u8"raison inconnue", u8"причина неизвестна"},
    {u8"Cadeau d'%s", u8"Подарок от %s"},
    {u8"Cadeau de %s", u8"Подарок от %s"},
    {u8"%s t'a donné %s.", u8"%s дарит тебе: %s."},
    {u8"C'est dans ta maison, et ta save est à jour.", u8"Всё уже у тебя дома, и сохранение обновлено."},
    {u8"C'est dans ta maison.", u8"Всё уже у тебя дома."},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"Подарок всё же дошёл до игрока %s (%s) и ушёл из дома."},
    {u8"%s a bien reçu %s : c'est parti de ta maison.",
     u8"Подарок дошёл до игрока %s (%s) и ушёл из дома."},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"(1 предмет не смог покинуть кладовую: он остался и у тебя.)"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"(Предметов, не покинувших кладовую: %u. Они остались и у тебя.)"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"Подарок игроку %s не дошёл: %s. Из дома ничего не ушло."},
    {u8"Rien à donner dans ton rangement.", u8"В кладовой нечего дарить."},
    {u8"Tape des lettres pour chercher un objet.", u8"Набирай буквы, чтобы найти предмет."},
    {u8"Recherche : %s_", u8"Поиск: %s_"},
    {u8"Aucun objet ne correspond.", u8"Ничего не найдено."},
    {u8"(page %u sur %u)", u8"(стр. %u из %u)"},
    {u8"Butin : %s.", u8"Добыча: %s."},
    {u8"Besoin", u8"Нужно"},
    {u8"Cupidité", u8"Не откажусь"},
    {u8"Passer", u8"Пропустить"},
    {u8"%u s", u8"%u с"},
    {u8"toi", u8"ты"},
    {u8"Joueur %u", u8"Игрок %u"},
    {u8"Un objet", u8"Предмет"},
    {u8"Partage du butin : objet %u sur %u", u8"Дележ добычи: предмет %u из %u"},
    {u8"Besoin : %s", u8"Нужно: %s"},
    {u8"Cupidité : %s", u8"Не откажусь: %s"},
    {u8"Passer : %s", u8"Пропустить: %s"},
    {u8"Partage du butin", u8"Дележ добычи"},
    {u8"%u objets du sac à partager entre %s.", u8"Предметов из рюкзака к дележу: %u. Участники: %s."},
    {u8"%u objet du sac à partager entre %s.", u8"%u предмет из рюкзака к дележу. Участники: %s."},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"Для каждого предмета: Нужно (он тебе пригодится), Не откажусь (если\n"
     u8"он никому не нужен) или Пропустить. Ничью решает бросок d100."},
    {u8"En attente de %s...", u8"Ждём: %s..."},
    {u8"En attente de %s... (%s)", u8"Ждём: %s... (%s)"},
    {u8"Tout le monde est là : ça commence !", u8"Все на месте: начинаем!"},
    {u8"Commencer", u8"Начать"},
    {u8"D'accord", u8"Хорошо"},
    {u8"Le veux-tu ? (%s)", u8"Берёшь? (%s)"},
    {u8"Je le veux", u8"Беру"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"Нужно: он тебе пригодится, ты в приоритете.\n"
     u8"Не откажусь: берёшь, если он никому не нужен.\n"
     u8"(%s)"},
    {u8"Ton choix : %s.", u8"Твой выбор: %s."},
    {u8"Ton choix est fait.", u8"Выбор сделан."},
    {u8"En attente de %s (%s).", u8"Ждём: %s (%s)."},
    {u8"Le résultat arrive...", u8"Результат на подходе..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s отсутствуют: их ход пропущен."},
    {u8"%s n'est pas là : son tour passe.", u8"%s отсутствует: ход пропущен."},
    {u8"Personne n'en veut : il reste chez toi.", u8"Никому не нужен: остаётся у тебя."},
    {u8"Personne n'en veut : il reste chez %s.", u8"Никому не нужен: остаётся у игрока %s."},
    {u8"Tu l'emportes (%s) !", u8"Предмет твой (%s)!"},
    {u8"%s l'emporte (%s).", u8"%s забирает предмет (%s)."},
    {u8"Dés : %s (égalité départagée).", u8"Броски: %s (ничья переброшена)."},
    {u8"Dés : %s.", u8"Броски: %s."},
    {u8"Partage du butin terminé", u8"Дележ добычи окончен"},
    {u8"Tu remportes : %s.", u8"Тебе достаётся: %s."},
    {u8"%s remporte : %s.", u8"%s получает: %s."},
    {u8"Remporté, mais resté chez %s : %s.", u8"Выиграно, но осталось у игрока %s: %s."},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"Никому не нужно (остаётся у тебя): %s."},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"Никому не нужно (остаётся у игрока %s): %s."},
    {u8"Aucun objet n'a été partagé.", u8"Ни один предмет не разыгран."},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"Твоё сохранение уже открыто: следующее окно\n"
     u8"подскажет, как загрузить его заново с добычей."},
    {u8"Ton butin t'attend dans ton rangement, chez toi.", u8"Добыча ждёт тебя дома, в кладовой."},
    {u8"Ce que les autres remportent quitte ton rangement.",
     u8"То, что выиграли другие, уходит из твоей кладовой."},
    {u8"Beanies", u8"Душечкин"},
    {u8"Butch", u8"Бутч"},
    {u8"Tink", u8"Тинк"},
    {u8"Frank", u8"Фрэнк"},
    {u8"Jack", u8"Джек"},
    {u8"Tracy", u8"Трейси"},
    {u8"l'orgue de barbarie", u8"Органоид"},
    {u8"Steven", u8"Стивен"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"Награда готова\n"
     u8"\n"
     u8"Котейка, которого игрок %s отдал NPC %s,\n"
     u8"заполнил шкалу. Ты заберёшь награду за этого игрока,\n"
     u8"как только будешь дома: монеты, еда\n"
     u8"и предметы уйдут игроку %s,\n"
     u8"а разблокировки — всем."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"Награда: %s\n"
     u8"\n"
     u8"Награду за подарок игрока %s не удалось\n"
     u8"ему отправить: она остаётся у тебя."},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"Награда для игрока %s\n"
     u8"\n"
     u8"%s награждает за котейку игрока %s:\n"
     u8"ему достаётся: %s.\n"
     u8"Разблокировки — для всех."},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Награда для всех\n"
     u8"\n"
     u8"%s награждает за котейку игрока %s:\n"
     u8"разблокировки достаются\n"
     u8"всем."},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s благодарит за котейку: %s\n"
     u8"(разблокировки — для всех)."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Награда: %s\n"
     u8"\n"
     u8"%s благодарит за котейку: %s.\n"
     u8"Всё это будет дома после следующей\n"
     u8"загрузки сохранения (по возвращении\n"
     u8"из похода). Разблокировки — для\n"
     u8"всех."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"Награда: %s\n"
     u8"\n"
     u8"Твой котейка заполнил шкалу NPC %s:\n"
     u8"награда — разблокировки — для\n"
     u8"всех. Она появится в твоём сохранении\n"
     u8"при следующей загрузке."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"Награда: %s\n"
     u8"\n"
     u8"Игрок %s, заполнивший шкалу NPC %s,\n"
     u8"уже ушёл: награда остаётся у тебя."},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"Награда готова\n"
     u8"\n"
     u8"Твой котейка заполнил шкалу NPC %s!\n"
     u8"%s заберёт награду за тебя,\n"
     u8"как только будет у себя дома."},
    {u8"les bourses ne marchent pas dans cette partie", u8"кошельки в этой игре не работают"},
    {u8"rien à donner", u8"нечего дарить"},
    {u8"c'est trop d'un coup", u8"слишком много за раз"},
    {u8"choisis un autre joueur", u8"выбери другого игрока"},
    {u8"ton don précédent attend encore la réponse de l'hôte",
     u8"твой прошлый подарок ещё ждёт ответа хоста"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"у тебя нет кошелька в этом походе"},
    {u8"envoi impossible (connexion)", u8"отправить не удалось (связь)"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"Конец похода\n"
     u8"\n"
     u8"Каждый уносит свой кошелёк домой:"},
    {u8"%s n'est plus là : sa bourse reste chez toi.",
     u8"Игрока %s больше нет: его кошелёк остаётся у тебя."},
    {u8"%s : rien.", u8"%s: ничего."},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.",
     u8"Игрок %s не смог получить кошелёк: он остаётся у тебя."},
    {u8"%s : %s.", u8"%s: %s."},
    {u8"Toi : %s.", u8"Ты: %s."},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"Хост пока не ответил на подарок: проверь кошелёк в окне выбора пути."},
    {u8"Ton don n'est pas passé : %s.", u8"Подарок не дошёл: %s."},
    {u8"(et d'autres trouvailles)", u8"(и другие находки)"},
    {u8"pas de bourse pour ce joueur dans cette aventure", u8"у этого игрока нет кошелька в этом походе"},
    {u8"ta bourse n'en a pas autant", u8"в твоём кошельке столько нет"},
    {u8"l'aventure n'est plus suivie", u8"поход больше не отслеживается"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s дарит тебе %s: это в твоём кошельке."},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"Ты даришь %s игроку %s: это в его кошельке."},
    {u8"%s a donné %s à %s.", u8"%s дарит %s игроку %s."},
    {u8"Un chat", u8"Котейка"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%s находит %s и теряет %s: это идёт в твой кошелёк."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s находит %s и теряет %s: это идёт в кошелёк игрока %s."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s находит %s и теряет %s: это идёт в кошелёк игрока %s."},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s находит %s: это в твой кошелёк."},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s находит %s: это в кошелёк игрока %s."},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s находит %s: это в кошелёк игрока %s."},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s тратит %s: это из твоего кошелька."},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.", u8"%s тратит %s: это из кошелька игрока %s."},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.", u8"%s тратит %s: это из кошелька игрока %s."},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s теряет %s: это из твоего кошелька."},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%s теряет %s: это из кошелька игрока %s."},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%s теряет %s: это из кошелька игрока %s."},
    {u8"%s n'a rien trouvé.", u8"%s ничего не находит."},
    {u8"un objet", u8"предмет"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.",
     u8"Твоя покупка: %s за %s — из твоего кошелька."},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.",
     u8"Покупка игрока %s: %s за %s — из его кошелька."},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"Твоя покупка: %s, в кошельке на %s больше."},
    {u8"%s a acheté %s : %s de plus pour sa bourse.",
     u8"Покупка игрока %s: %s, в его кошельке на %s больше."},
    {u8"Tu as acheté %s (gratuit).", u8"Твоя покупка: %s (бесплатно)."},
    {u8"%s a acheté %s (gratuit).", u8"Покупка игрока %s: %s (бесплатно)."},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"В бою твои котейки нашли %s и потеряли %s: это идёт в твой кошелёк."},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"В бою твои котейки нашли %s: это в твой кошелёк."},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"В бою твои котейки потеряли %s: это из твоего кошелька."},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"В бою твои котейки ничего не нашли."},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"В бою котейки игрока %s нашли %s и потеряли %s: это идёт в его кошелёк."},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"В бою котейки игрока %s нашли %s и потеряли %s: это идёт в его кошелёк."},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"В бою котейки игрока %s нашли %s: это в его кошелёк."},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"В бою котейки игрока %s нашли %s: это в его кошелёк."},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"В бою котейки игрока %s потеряли %s: это из его кошелька."},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"В бою котейки игрока %s потеряли %s: это из его кошелька."},
    {u8"En combat, les chats d'%s n'ont rien trouvé.", u8"В бою котейки игрока %s ничего не нашли."},
    {u8"En combat, les chats de %s n'ont rien trouvé.", u8"В бою котейки игрока %s ничего не нашли."},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"Твой рюкзак открыт\n"
     u8"\n"
     u8"Хост вошёл в локацию:\n"
     u8"закрой рюкзак, чтобы идти за ним."},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"Предметы\n"
     u8"\n"
     u8"Это котейка игрока %s: менять его предметы\n"
     u8"может только владелец."},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"Предметы\n"
     u8"\n"
     u8"Внутри локации (магазин, повышение\n"
     u8"уровня...) предметы меняет только хост.\n"
     u8"Снаряжай котеек на карте."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"Предметы\n"
     u8"\n"
     u8"Хост ещё не вернулся на карту:\n"
     u8"подожди, рюкзак разблокируется сам."},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"Предметы\n"
     u8"\n"
     u8"Твой рюкзак обновляется по данным хоста:\n"
     u8"закрой его и открой снова через мгновение."},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"Предметы\n"
     u8"\n"
     u8"Другой игрок опередил тебя:\n"
     u8"твоё изменение не принято. Закрой\n"
     u8"рюкзак — он вернётся к версии хоста."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"Предметы\n"
     u8"\n"
     u8"Хост вошёл в локацию раньше, чем дошло\n"
     u8"твоё последнее изменение: оно отменено.\n"
     u8"Повтори его на карте."},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"Предметы\n"
     u8"\n"
     u8"Этого предмета больше нет: другой игрок\n"
     u8"взял его прямо перед тобой."},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).",
     u8"Игрока %s больше нет: его покупки остаются у тебя (%s)."},
    {u8"%s rapporte ses achats : %s.", u8"%s уносит свои покупки: %s."},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Покупки игрока %s не удалось ему отправить: они остаются у тебя."},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Покупки игрока %s не удалось ему отправить: они остаются у тебя."},
    {u8"Achats de l'aventure", u8"Покупки в походе"},
    {u8"Boutique", u8"Магазин"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s ещё не закончили в этом\n"
     u8"магазине: подожди их (когда выйдешь, ты\n"
     u8"уже не сможешь покупать за них).\n"
     u8"Нажми ещё раз, чтобы всё равно выйти."},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"Игрок %s ещё не закончил в этом\n"
     u8"магазине: подожди его (когда выйдешь, ты\n"
     u8"уже не сможешь покупать за него).\n"
     u8"Нажми ещё раз, чтобы всё равно выйти."},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"Не удалось отправить хосту запрос на покупку:\n"
     u8"ничего не куплено. Попробуй ещё раз чуть позже."},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Хоста больше нет в этом магазине:\n"
     u8"покупка не состоялась."},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"У хоста этот предмет больше не продаётся:\n"
     u8"покупка не состоялась."},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Этот предмет только что продан:\n"
     u8"покупка не состоялась."},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"Цена — %s, а в кошельке — %lld."},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"Не хватает монет в кошельке."},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"(Каждый платит из своего кошелька, ты тоже.)"},
    {u8"(Chacun paie avec sa bourse.)", u8"(Каждый платит из своего кошелька.)"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"Пока что конфетки и фамильяров покупает\n"
     u8"только хост (из своего кошелька)."},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.", u8"Игра не смогла совершить покупку у хоста."},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"В этом походе нет кошельков:\n"
     u8"покупает только хост."},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"В этом походе нет твоих котеек,\n"
     u8"поэтому нет и кошелька для\n"
     u8"покупок."},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"В этом походе у тебя нет кошелька\n"
     u8"(в нём нет твоих котеек): покупки не будет."},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"Хост сейчас не может купить для тебя\n"
     u8"(у него открыто какое-то окно):\n"
     u8"ничего не куплено. Попробуй ещё раз чуть позже."},
    {u8"un bonbon", u8"конфетка"},
    {u8"un familier", u8"фамильяр"},
    {u8"un article", u8"товар"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"Конец похода\n"
     u8"\n"
     u8"%s возвращаются к игроку %s."},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"Конец похода\n"
     u8"\n"
     u8"%s возвращается к игроку %s."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Котейки других игроков не смогли отправиться,\n"
     u8"а твоя коробка похода пуста: похода\n"
     u8"в этот раз не будет. (Причина — в журнале мода, mgmp.log.)"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Твоя коробка похода пуста, и никто не берёт\n"
     u8"котеек: похода в этот раз не будет."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Смешанная партия\n"
     u8"\n"
     u8"Котейки других игроков не смогли\n"
     u8"вступить в партию: ты идёшь со своими.\n"
     u8"(Причина — в журнале мода, mgmp.log.)"},
    {u8"Retour de l'aventure", u8"Возвращение из похода"},
    {u8"%s est de retour à la maison.", u8"%s снова дома."},
    {u8"%s sont de retour à la maison.", u8"%s снова дома."},
    {u8"%s a péri pendant l'aventure.", u8"%s погибает в походе."},
    {u8"%s ont péri pendant l'aventure.", u8"%s погибли в походе."},
    {u8"%s ne revient pas de l'aventure.", u8"%s не возвращается из похода."},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s не возвращаются из похода."},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s — данные изменились после выхода:\n"
     u8"возвращение из похода не перенесено."},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s изменились после выхода:\n"
     u8"их возвращение из похода не перенесено."},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"Кладовая изменилась после выхода:\n"
     u8"взятые в поход предметы из неё не убраны."},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"Кладовая не изменена: взятые в поход\n"
     u8"предметы всё ещё в ней."},
    {u8"Tu rapportes %s dans ta maison.", u8"Ты приносишь домой %s."},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.",
     u8"NPC благодарят тебя за котеек: %s у тебя дома."},
    {u8"Cadeaux des PNJ", u8"Подарки NPC"},
    {u8"Ton butin est dans ton rangement : %s.", u8"Твоя добыча в кладовой: %s."},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.", u8"Подарки NPC в кладовой: %s."},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.",
     u8"Котейки %s, найденные в походе, прибывают к тебе домой."},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.",
     u8"Котейка %s, найденный в походе, прибывает к тебе домой."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"%s не смогли вступить в партию:\n"
     u8"в этом походе ты управляешь частью\n"
     u8"котеек партии."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"%s не может вступить в партию:\n"
     u8"в этом походе ты управляешь частью\n"
     u8"котеек партии."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"%s не смогли вступить в партию:\n"
     u8"ты следишь за походом, не управляя котейками."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"%s не может вступить в партию:\n"
     u8"ты следишь за походом, не управляя котейками."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"%s не смогли пойти и остаются дома."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"%s не может пойти и остаётся дома."},
    {u8"Tes chats sont rentrés de l'aventure.", u8"Твои котейки вернулись из похода."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"Новый котейка в партии\n"
     u8"\n"
     u8"%s вступает в партию игрока %s: этот котейка\n"
     u8"твой! Пока тебя нет, в бою за него играет\n"
     u8"ИИ игры, а %s выбирает за него,\n"
     u8"и он вернётся к тебе домой в конце похода."},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"Новые котейки: %s прибывают к тебе."},
    {u8"Nouveaux chats : %s arrive chez toi.", u8"Новый котейка: %s прибывает к тебе."},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"4-е место\n"
     u8"\n"
     u8"При 3 игроках есть свободное место:\n"
     u8"оно твоё: ты берёшь двух котеек."},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"4-е место\n"
     u8"\n"
     u8"Место уже занято игроком %s:\n"
     u8"при 3 игроках ты берёшь 1 котейку.\n"
     u8"Убери лишнего из коробки похода."},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"4-е место\n"
     u8"\n"
     u8"Есть четвёртый игрок: каждый берёт по\n"
     u8"1 котейке. Убери одного из коробки похода."},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"4-е место достанется тому, кто первым положит в коробку двух котеек."},
    {u8"La 4e place est à %s.", u8"4-е место — у игрока %s."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"При %d игроках ты берёшь не больше %d котеек:\n"
     u8"убери %d котеек из коробки похода.%s\n"
     u8"Остальные игроки дополнят партию своими."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"При %d игроках ты берёшь не больше %d котеек:\n"
     u8"убери %d котейку из коробки похода.%s\n"
     u8"Остальные игроки дополнят партию своими."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"При %d игроках ты берёшь не больше %d котейки:\n"
     u8"убери %d котеек из коробки похода.%s\n"
     u8"Остальные игроки дополнят партию своими."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Смешанная партия\n"
     u8"\n"
     u8"При %d игроках ты берёшь не больше %d котейки:\n"
     u8"убери %d котейку из коробки похода.%s\n"
     u8"Остальные игроки дополнят партию своими."},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Ты не у себя дома:\n"
     u8"ты будешь играть котейками других."},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Коробка похода ещё не разблокирована:\n"
     u8"ты будешь играть котейками других."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Коробка похода пуста: ты будешь играть котейками других.\n"
     u8"Чтобы взять своих: «Ещё нет», положи 1 или 2 в коробку\n"
     u8"(у Бутча), затем нажми на свою табличку «В путь!»."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Коробка похода пуста: ты будешь играть котейками других.\n"
     u8"Чтобы взять своих: «Ещё нет», положи одну в коробку\n"
     u8"(у Бутча), затем нажми на свою табличку «В путь!»."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"В коробке похода котеек: %d, а можно не больше %d:\n"
     u8"ответь «Ещё нет», убери лишних (%d),\n"
     u8"затем нажми на свою табличку «В путь!».%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).",
     u8"Твои котейки отправятся из коробки похода (у Бутча)."},
    {u8"Tu emmènes %s.", u8"Ты берёшь: %s."},
    {u8"Départ en aventure", u8"Отправка в поход"},
    {u8"La session n'est pas prête : réessaie dans un instant.",
     u8"Сессия не готова: попробуй ещё раз чуть позже."},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"4-е место закрепляется за тобой:\n"
     u8"вопрос сейчас появится снова."},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"Не удалось подготовить твоих котеек\n"
     u8"к походу (см. mgmp.log)."},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s вернулись из похода, но их возвращение\n"
     u8"ещё не перенесено в сохранение: загрузи его заново,\n"
     u8"прежде чем снова отправлять их («Загрузить»\n"
     u8"предложат после этого выхода; или Esc, «Сохраниться\n"
     u8"и выйти в меню», затем «Играть» и твоё сохранение)."},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"Котейка %s вернулся из похода, но его возвращение\n"
     u8"ещё не перенесено в сохранение: загрузи сохранение заново,\n"
     u8"прежде чем снова отправлять котейку («Загрузить»\n"
     u8"предложат после этого выхода; или Esc, «Сохраниться\n"
     u8"и выйти в меню», затем «Играть» и твоё сохранение)."},
    {u8"Le retour de %s attend d'être recopié.", u8"%s: возвращение ждёт переноса."},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"Не удалось отправить ответ хосту:\n"
     u8"попробуй снова."},
    {u8"Ta bourse de l'aventure : %s.", u8"Твой кошелёк из похода: %s."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"В коробке похода котеек: %d, а можно не больше %d:\n"
     u8"убери лишних (%d), затем нажми на свою табличку «В путь!»."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"Новый котейка в партии\n"
     u8"\n"
     u8"%s в партии, и это твой котейка!\n"
     u8"Ты им управляешь, а в конце похода\n"
     u8"он вернётся к тебе домой."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"Новый котейка в партии\n"
     u8"\n"
     u8"%s вступает в партию: этот котейка твой."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"Новый котейка в партии\n"
     u8"\n"
     u8"%s вступает в партию: это котейка игрока %s."},
    {u8"Un nouveau chat", u8"Новый котейка"},
    {u8"en bas", u8"внизу"},
    {u8"en haut", u8"вверху"},
    {u8"à droite", u8"справа"},
    {u8"à gauche", u8"слева"},
    {u8"au milieu", u8"посередине"},
    {u8"n°%d", u8"№%d"},
    {u8"%s (%s)", u8"%s (%s)"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]Твой голос не принят: %s.[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"Выбор пути\n"
     u8"\n"},
    {u8"Même chemin", u8"Тот же путь"},
    {u8"Choisir", u8"Выбрать"},
    {u8"Changer", u8"Изменить"},
    {u8"Sortie", u8"Выход"},
    {u8"Retour à la maison", u8"Возвращение домой"},
    {u8"Combat", u8"Бой"},
    {u8"Combat difficile", u8"Трудный бой"},
    {u8"Mini-boss", u8"Мини-босс"},
    {u8"Boss", u8"Босс"},
    {u8"Événement", u8"Событие"},
    {u8"Événement spécial", u8"Особое событие"},
    {u8"Trésor", u8"Сокровище"},
    {u8"Meuble", u8"Мебель"},
    {u8"Nourriture", u8"Еда"},
    {u8"Bonus", u8"Бонус"},
    {u8"Case vide", u8"Пустая локация"},
    {u8"Case", u8"Локация"},
    {u8"l'hôte n'est pas sur la carte", u8"хоста нет на карте"},
    {u8"vos cartes ne sont pas les mêmes", u8"ваши карты не совпадают"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"хост не может пойти в эту локацию"},
    {u8"le chemin est déjà choisi", u8"путь уже выбран"},
    {u8"ce tour est fini", u8"этот раунд окончен"},
    {u8"l'hôte ne l'a pas pris", u8"хост его не принял"},
    {u8"pas de vote", u8"нет голоса"},
    {u8"absent, ne vote pas", u8"отсутствует, не голосует"},
    {u8"pas encore sur la carte", u8"ещё не на карте"},
    {u8"pas encore voté", u8"голоса пока нет"},
    {u8"%s (toi) : %s", u8"%s (ты): %s"},
    {u8"%s : %s", u8"%s: %s"},
    {u8"En pause : on attend tout le monde sur la carte.", u8"Пауза: ждём всех на карте."},
    {u8"À toi de choisir : encore %u s.", u8"Выбирай: осталось %u с."},
    {u8"En attente des autres : encore %u s.", u8"Ждём остальных: осталось %u с."},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"Ничья: %s.\n"
     u8"Жребий выбирает: %s."},
    {u8"Tout le monde a choisi %s.", u8"Все выбрали: %s."},
    {u8"%s l'emporte.", u8"Побеждает %s."},
    {u8"%s : prêt", u8"%s: готово"},
    {u8"%s : défend sa maison", u8"%s: защищает свой дом"},
    {u8"%s : pas encore", u8"%s: ещё нет"},
    {u8"Tout le monde est prêt !", u8"Все готовы!"},
    {u8"En attente d'un joueur.", u8"Ждём одного игрока."},
    {u8"En attente de %d joueurs.", u8"Ждём игроков: %d."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"Конец дня\n"
     u8"\n"
     u8"С твоей стороны всё готово к ночи.\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"Твоя табличка сейчас для отправки в поход:\n"
     u8"убери котеек из партии, чтобы закончить день."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Конец дня\n"
     u8"\n"
     u8"1 игрок ждёт перехода к ночи.\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Конец дня\n"
     u8"\n"
     u8"Перехода к ночи ждут игроков: %d.\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1 игрок ждёт"},
    {u8"%d joueurs attendent", u8"Ждут игроков: %d"},
    {u8"Passer la nuit", u8"Закончить день"},
    {u8"Plus tard", u8"Позже"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"Чтобы продолжить, нажми Esc,\n"
     u8"затем «Сохраниться и выйти в меню»."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"Что тебя ждёт\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Сохранение уже открыто, и всё это\n"
     u8"появится только при следующей загрузке.\n"
     u8"«Загрузить»: мод выйдет в меню и откроет его."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"Что тебя ждёт\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Сохранение уже открыто, и всё это\n"
     u8"появится только при следующей загрузке.\n"
     u8"Загрузи его: Esc, «Сохраниться и выйти в меню»,\n"
     u8"затем «Играть» и твоё сохранение."},
    {u8"%s : j'arrive !", u8"%s: иду!"},
    {u8"%s : réfléchit...", u8"%s: думает..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Ты хочешь отправиться в поход.\n"
     u8"Ответы команды:\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s: ещё не здесь"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"Ждём команду\n"
     u8"\n"
     u8"%s\n"
     u8"Если пойдёшь дальше без неё, ИИ игры\n"
     u8"будет играть её котейками в боях, пока\n"
     u8"она не придёт."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"Отправка в поход\n"
     u8"\n"
     u8"%s отправляется в поход!\n"
     u8"Каждый берёт своих котеек и управляет ими.\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"Поход в разгаре\n"
     u8"\n"
     u8"%s в походе!\n"
     u8"Твоих котеек там нет: ты будешь следить\n"
     u8"за походом, не управляя котейками."},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"Поход в разгаре\n"
     u8"\n"
     u8"%s в походе с твоими котейками!\n"
     u8"Пока тебя нет, в бою ими играет ИИ игры,\n"
     u8"а %s выбирает за них.\n"
     u8"Присоединишься к команде?"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"Поход в разгаре\n"
     u8"\n"
     u8"%s в походе!\n"
     u8"Присоединишься к команде?"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"Выход из похода\n"
     u8"\n"
     u8"Поход игрока %s окончен:\n"
     u8"ты возвращаешься домой."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"Выход из похода\n"
     u8"\n"
     u8"Поход ведёт %s: бросить его\n"
     u8"может лишь хост. Поход идёт без тебя.\n"
     u8"\n"
     u8"Домой? Чтобы вернуться в поход,\n"
     u8"нажми на табличку у себя дома."},
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
     u8"Выход из похода\n"
     u8"\n"
     u8"Поход ведёт %s: бросить его\n"
     u8"может лишь хост. Поход идёт без тебя,\n"
     u8"а твои котейки — в его партии: в бою\n"
     u8"ими играет ИИ игры, а потом они\n"
     u8"вернутся домой.\n"
     u8"\n"
     u8"Домой? Чтобы вернуться в поход,\n"
     u8"нажми на табличку у себя дома."},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"Присоединение к походу\n"
     u8"\n"
     u8"Игра игрока %s передаёт тебе текущий поход.\n"
     u8"Если он сейчас делает выбор в локации\n"
     u8"(событие, повышение уровня), ты появишься,\n"
     u8"когда он вернётся на карту."},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"Сборы в поход\n"
     u8"\n"
     u8"Игра игрока %s загружается:\n"
     u8"ты отправишься с ним."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Возвращение домой\n"
     u8"\n"
     u8"Связь с игроком %s прервалась.\n"
     u8"Ты возвращаешься домой."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"Возвращение домой\n"
     u8"\n"
     u8"Отправка с игроком %s не состоялась.\n"
     u8"Ты возвращаешься домой."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Возвращение домой\n"
     u8"\n"
     u8"Поход игрока %s окончен.\n"
     u8"Ты возвращаешься домой."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Возвращение домой\n"
     u8"\n"
     u8"Бой игрока %s закончился, а твоя копия\n"
     u8"продолжалась: она рассинхронизировалась,\n"
     u8"и этот бой уже никто не играл. Ты теперь\n"
     u8"дома; твои котейки остаются в его партии.\n"
     u8"\n"
     u8"Чтобы вернуться в поход, нажми на\n"
     u8"табличку у себя дома."},
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
     u8"Возвращение домой\n"
     u8"\n"
     u8"Твой бой закончился, а бой игрока %s\n"
     u8"продолжался: твоя копия\n"
     u8"рассинхронизировалась. Ты теперь дома;\n"
     u8"твои котейки остаются в его партии, ими\n"
     u8"играет ИИ игры до конца боя.\n"
     u8"\n"
     u8"Чтобы вернуться в поход, нажми на\n"
     u8"табличку у себя дома."},
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
     u8"Возвращение домой\n"
     u8"\n"
     u8"Игрок %s вошёл в локацию, куда твоя\n"
     u8"игра не смогла последовать: твоя копия похода\n"
     u8"уже не совпадала. Ты теперь дома; твои\n"
     u8"котейки остаются в его партии, ими играет\n"
     u8"ИИ игры.\n"
     u8"\n"
     u8"Чтобы вернуться в поход, нажми на\n"
     u8"табличку у себя дома."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Возвращение домой\n"
     u8"\n"
     u8"Твоя копия боя рассинхронизировалась с копией\n"
     u8"игрока %s: бой шёл без тебя, поэтому\n"
     u8"ты теперь дома. Твои котейки остаются в\n"
     u8"его партии, ими играет ИИ игры.\n"
     u8"\n"
     u8"Чтобы вернуться в поход, нажми на\n"
     u8"табличку у себя дома."},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"Твоё сохранение — в разгаре похода.\n"
     u8"В мультиплеере походы проходят вместе с хостом:\n"
     u8"вернись в меню и выбери другое сохранение\n"
     u8"или закончи этот поход в одиночку."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Загружается поход игрока %s."},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"Игрок %s перезагрузил поход:\n"
     u8"ты тоже загружаешь его заново."},
    {u8"Rentrer", u8"Домой"},
    {u8"Rester", u8"Остаться"},
    {u8"Ne pas attendre", u8"Не ждать"},
    {u8"Recharger", u8"Загрузить"},
    {u8"J'arrive", u8"Иду!"},
    {u8"Pas encore", u8"Ещё нет"},
    {u8"Échanger", u8"Обмен"},
    {u8"Multijoueur", u8"Мультиплеер"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"Эта труба — в доме игрока %s:\n"
     u8"отправляй котеек к NPC\n"
     u8"из своего дома."},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"NPC привязаны к игре игрока %s:\n"
     u8"отправлять котеек по трубе\n"
     u8"в этой версии игры нельзя."},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"Это поражение всей партии:\n"
     u8"котейки игрока %s тоже не вернутся."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"Подключение: %s\n"
     u8"\n"
     u8"Ты уже в лобби мультиплеера:\n"
     u8"сначала выйди из него («Мультиплеер», затем «Выйти»),\n"
     u8"а потом снова нажми «Присоединиться к игре»."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"Подключение: %s\n"
     u8"\n"
     u8"Игра выйдет в главное меню\n"
     u8"(с сохранением, как при «Сохраниться и выйти в меню»),\n"
     u8"затем — в лобби игрока %s.\n"
     u8"\n"
     u8"«Позже»: игра подключится, когда ты\n"
     u8"выйдешь в главное меню."},
    {u8"Y aller", u8"Перейти"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"Подключение: %s\n"
     u8"\n"
     u8"Игра войдёт в лобби игрока %s,\n"
     u8"когда ты будешь в главном меню\n"
     u8"(Esc, «Сохраниться и выйти в меню»)."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"Подключение: %s\n"
     u8"\n"
     u8"Через Steam сейчас не подключиться.\n"
     u8"Попробуй позже или зайди по адресу\n"
     u8"(«Мультиплеер», «Подключиться»)."},
    {u8"%s a annulé le départ.", u8"Игрок %s отменил отправку."},
    {u8"réseau local", u8"локальная сеть"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Твои котейки уже ушли с партией игрока %s:\n"
     u8"пока тебя нет, в бою ими играет ИИ игры,\n"
     u8"а %s выбирает за них; они вернутся\n"
     u8"к тебе домой в конце похода."},
    {u8"Internet", u8"Интернет"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s защищает свой дом: это одиночный\n"
     u8"бой со своими котейками. Потом вернётся."},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s защищают свои дома: каждый играет\n"
     u8"свой бой в одиночку, своими котейками. Потом вернутся."},
    {u8"%s a fini de défendre sa maison.", u8"%s: защита дома окончена."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s: защита дома окончена.\n"
     u8"%s ещё защищает свой."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s: защита дома окончена.\n"
     u8"%s ещё защищают свои."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Отмена после голосования: %s уходит\n"
     u8"без тебя и твоих котеек. Присоединиться\n"
     u8"к походу можно, когда хост будет на карте."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"Отправка в поход\n"
     u8"\n"
     u8"Ни один котейка не может пойти:\n"
     u8"%s не отправляется в поход."},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"Дележ добычи ещё не окончен:\n"
     u8"дождись конца показа предметов."},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"Ты идёшь в поход игрока %s:\n"
     u8"защитишь дом, когда вернёшься."},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"Поход начинает %s:\n"
     u8"попроси этого игрока нажать «В путь!»."},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"Мультиплеер\n"
     u8"\n"
     u8"Создай игру, чтобы друзья к тебе присоединились,\n"
     u8"или подключись к игре друга."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"[c:red]Не удалось открыть порт %u.[/c]\n"
     u8"Возможно, его занимает другая программа.\n"
     u8"Закрой лобби и попробуй снова."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Открываем лобби..."},
    {u8"%s (hôte)", u8"%s (хост)"},
    {u8"En attente d'un ami...", u8"Ждём друга..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Друзья в Steam подключатся в один клик: в списке\n"
     u8"друзей Steam — «Присоединиться к игре» у твоего имени.\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Или по адресу:\n"
     u8"%s   (%s, порт %u)\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Дай этот адрес другу:\n"
     u8"%s   (%s, порт %u)\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"Он уже скопирован: вставь его другу в чат (Ctrl+V).\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"Адрес на этом ПК не найден (порт %u).\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"%s\n"
     u8"Игроки (%d/4):\n"
     u8"%s\n"
     u8"Когда все соберутся, выбери сохранение."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"Подключение к игре\n"
     u8"\n"
     u8"Адрес хоста:\n"
     u8"%s_\n"
     u8"\n"
     u8"Введи адрес, который дал тебе друг,\n"
     u8"или вставь его через Ctrl+V. Enter — подтвердить.%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Через Steam проще: в списке друзей нажми\n"
     u8"«Присоединиться к игре» у своего друга."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"Подключение к игре\n"
     u8"\n"
     u8"[c:red]Неверный адрес: %s[/c]\n"
     u8"\n"
     u8"Пример: 26.123.45.67\n"
     u8"или с другим портом: 26.123.45.67:27600"},
    {u8"(vide)", u8"(пусто)"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.",
     u8"Проверьте, что у вас одинаковые версии игры и мода."},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).",
     u8"Игра хоста заполнена (не больше 4 игроков)."},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"У вас разные версии мода:\n"
     u8"обновитесь оба до последней."},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"У вас разные версии игры:\n"
     u8"обновите Mewgenics у обоих (через Steam)."},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"Файлы игры различаются:\n"
     u8"проверьте их в Steam (Свойства, Установленные файлы)."},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"Настройка net.control в mgmp.json даёт обоим игрокам\n"
     u8"одного и того же котейку: верните значение \"auto\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"[c:red]В подключении отказано.[/c]\n"
     u8"%s\n"
     u8"(%s)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"[c:red]Ошибка подключения (%s):[/c]\n"
     u8"%s.\n"
     u8"Выйди из лобби и попробуй снова (или по адресу)."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Подключение: %s..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"[c:red]Не удаётся связаться: %s.[/c]\n"
     u8"Проверь адрес и попроси друга\n"
     u8"нажать «Создать»."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Отключено."},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"Экран сохранений откроется сам."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Подключение: %s...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"[c:red]Связь с хостом прервалась.[/c]\n"
     u8"Выйди из лобби и подключись снова."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Подключение через Steam: %s...\n"
     u8"(несколько секунд)%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Подключено, знакомимся с хостом...%s"},
    {u8"%s (toi)", u8"%s (ты)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Ты в игре игрока %s!\n"
     u8"\n"
     u8"Игроки (%d/4):\n"
     u8"%s\n"
     u8"Ждём игру игрока %s:\n"
     u8"как только он выберет сохранение,\n"
     u8"экран сохранений откроется сам."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Ты в игре игрока %s!\n"
     u8"\n"
     u8"Игроки (%d/4):\n"
     u8"%s\n"
     u8"Ждём игру игрока %s:\n"
     u8"как только он выберет сохранение,\n"
     u8"ты сможешь выбрать своё."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"Лобби мультиплеера\n"
     u8"\n"
     u8"Ты в игре игрока %s!\n"
     u8"\n"
     u8"Игроки (%d/4):\n"
     u8"%s\n"
     u8"Выбери своё сохранение: твой дом, твои котейки.\n"
     u8"NPC и сюжет будут как у игрока %s,\n"
     u8"а в поход ты пойдёшь с его командой."},
    {u8"Héberger", u8"Создать"},
    {u8"Rejoindre", u8"Подключиться"},
    {u8"Choisir la sauvegarde", u8"Выбрать сохранение"},
    {u8"Arrêter", u8"Закрыть"},
    {u8"Choisir ta sauvegarde", u8"Выбрать своё сохранение"},
    {u8"Quitter", u8"Выйти"},
    {u8"Annuler", u8"Отмена"},
    {u8"Échanger avec %s", u8"Обмен с игроком %s"},
    {u8"Donner à qui ?", u8"Кому подарить?"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"Вверх/Вниз — выбрать, затем Enter."},
    {u8"Pièces à donner : %lld_", u8"Сколько монет подарить: %lld_"},
    {u8"Nourriture à donner : %lld_", u8"Сколько еды подарить: %lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"Введи число (T — всё), затем Enter."},
    {u8"Échanger avec %s : les objets de ton rangement", u8"Обмен с игроком %s: предметы из кладовой"},
    {u8"16 au plus par don : %u choisis.", u8"Максимум 16 в подарке, выбрано: %u."},
    {u8"16 au plus par don : %u choisi.", u8"Максимум 16 в подарке, выбрано: %u."},
    {u8"Lecture du rangement...", u8"Читаем кладовую..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"Вверх/Вниз — выбор, Пробел — отметить, Влево/Вправо — сколько.\n"
     u8"Буквы — поиск, затем Enter."},
    {u8"Tu n'as rien choisi à donner.", u8"Ничего не выбрано для подарка."},
    {u8"Tu n'as rien à donner pour l'instant.", u8"Тебе пока нечего подарить."},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"Подарить %s из кошелька\n"
     u8"игроку %s?"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"Подарить %s\n"
     u8"игроку %s?"},
    {u8"Envoi du don...", u8"Отправка подарка..."},
    {u8"Personne ne peut recevoir maintenant.", u8"Сейчас подарок никто не примет."},
    {u8"Personne d'autre n'est dans la partie.", u8"В игре больше никого нет."},
    {u8"Valider", u8"Подтвердить"},
    {u8"Donner", u8"Подарить"},
    {u8"Recommencer", u8"Заново"},
    {u8"Fermer", u8"Закрыть"},
    {u8"Pas d'échange possible ici.", u8"Здесь обмен невозможен."},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"Окно закрылось: хост вошёл в локацию.\n"
     u8"Твой выбор сохранён (E — продолжить)."},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"Обмен в походе\n"
     u8"\n"
     u8"На карте нажми E, чтобы подарить монеты\n"
     u8"или еду из кошелька другому игроку."},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
