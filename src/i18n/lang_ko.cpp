// lang_ko.cpp -- Mewgenics Multi : les textes du mod et du lanceur en coreen (ko).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : la forme polie simple des jeux video, comme combined.csv (colonne ko).
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_ko[];
const TrPair kTable_ko[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"mgmp.json 파일을 읽지 못했습니다: 설정이 저장되지 않았습니다."},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"mgmp.json에 주석(// 또는 /* */)이 있어 저장하면 지워집니다. 주석을 지우거나, 파일에서 직접 설정을 바꾸세요."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.json이 올바른 JSON이 아닙니다. 설정을 바꾸기 전에 파일을 고치세요(또는 zip에 든 파일로 되돌리세요)."},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"모드 폴더에 쓸 수 없습니다: 설정이 저장되지 않았습니다."},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"mgmp.json 파일을 바꾸지 못했습니다(다른 곳에서 열려 있나요?). 설정이 저장되지 않았습니다."},
    {u8"Steam est ouvert.", u8"Steam이 실행 중입니다."},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Steam이 없으면 모드가 로드되기 전에 게임이 종료됩니다. Steam을 실행하고 연결될 때까지 기다리세요."},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"Mewgenics\u00A0Multi.exe 옆에 mgmp.json이 없습니다. 설정을 저장하면 다시 만들어집니다."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.json이 올바른 JSON이 아닙니다. 파일을 고치거나 zip에 든 파일로 되돌리세요."},
    {u8"Réglages lus dans mgmp.json.", u8"mgmp.json에서 설정을 읽었습니다."},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe를 찾을 수 없습니다(%s). 설정에서 찾아보기…를 눌러 Mewgenics.exe를 선택하세요."},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"Mewgenics.exe 옆에 resources.gpak이 없습니다. Steam에서 게임 파일을 확인하세요."},
    {u8"Jeu trouvé : %s.", u8"게임 위치: %s."},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.", u8"steam_appid.txt: 게임을 찾은 뒤에 확인합니다."},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Mewgenics.exe 옆에 이 파일이 없습니다. 파일이 없으면 Steam이 모드 없이 게임을 다시 실행합니다."},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"Mewgenics.exe 옆의 파일에 686060이 없습니다. Steam이 모드 없이 게임을 다시 실행하게 됩니다."},
    {u8"steam_appid.txt est en place à côté du jeu.", u8"게임 옆에 steam_appid.txt가 있습니다."},
    {u8"Fichiers du mod présents.", u8"모드 파일이 모두 있습니다."},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Mewgenics\u00A0Multi.exe 옆에 mgmp.dll과 mgmp_loader.exe가 없습니다. zip에서 폴더 전체를 다시 압축 해제하세요."},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Mewgenics\u00A0Multi.exe 옆에 %s이(가) 없습니다. zip에서 폴더 전체를 다시 압축 해제하세요."},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi 빌드 %s, 프로토콜 %u: 모두 같은 버전이어야 합니다."},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"게임을 찾을 수 없습니다. 먼저 설정에서 Mewgenics.exe를 선택하세요."},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"게임 옆에 steam_appid.txt를 쓰지 못했습니다. 직접 복사하세요(LISEZ-MOI 2단계)."},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"Windows 권한 요청이 승인되지 않아 steam_appid.txt를 복사하지 못했습니다. 다시 시도하거나 직접 복사하세요(LISEZ-MOI 2단계)."},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics Multi를 관리자 권한으로 다시 실행하지 못했습니다. steam_appid.txt를 직접 복사하세요(LISEZ-MOI 2단계)."},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"관리자 권한으로 복사하지 못했습니다. steam_appid.txt를 직접 복사하세요(LISEZ-MOI 2단계)."},
    {u8"Le mod n'a pas pu être chargé : %s", u8"모드를 로드하지 못했습니다: %s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exe가 모드를 로드하지 못하고 종료되었습니다. 다시 실행해 보고, 문제가 반복되면 loader.txt를 보내 주세요."},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exe가 모드를 로드하지 못하고 종료되었습니다(코드 %s). 가장 흔한 원인: Steam이 꺼져 있거나 연결이 끊김. Steam을 실행하고 연결될 때까지 기다린 뒤 다시 실행하세요."},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"모드가 로드되기 전에 게임이 시작 도중 종료되었습니다. 가장 흔한 원인: Steam이 꺼져 있거나 연결이 끊김. Steam을 실행하고 연결될 때까지 기다린 뒤 다시 실행하세요."},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"Steam이 게임을 다시 실행하는 중입니다(1~2분 걸릴 수 있습니다)…"},
    {u8"Ouverture du jeu…", u8"게임을 여는 중…"},
    {u8"Le jeu démarre…", u8"게임 시작 중…"},
    {u8"Injection du mod dans le jeu…", u8"게임에 모드를 주입하는 중…"},
    {u8"Le mod est en place, le jeu reprend…", u8"모드 적용 완료, 게임을 계속합니다…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Steam이 다시 실행한 게임을 아직 기다리는 중… Steam이 방금 켜졌다면 확인 창이 떠 있을 수 있습니다."},
    {u8"Jeu relancé trouvé : injection du mod…", u8"다시 실행된 게임을 찾았습니다: 모드 주입 중…"},
    {u8"Le jeu est lancé avec le mod.", u8"모드와 함께 게임이 실행되었습니다."},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.", u8"모드 적용이 늦어졌지만 게임은 계속 진행됩니다."},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.", u8"다시 실행된 게임의 시작이 늦어 그대로 모드를 주입합니다."},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"Mewgenics\u00A0Multi.exe 옆에 mgmp.dll이 없습니다. zip에서 모드 폴더를 다시 압축 해제하세요."},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe를 찾을 수 없습니다. 설정에서 찾아보기…를 눌러 Mewgenics.exe를 선택하세요."},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"mgmp.json을 읽을 수 없습니다. 파일을 고치거나 zip에 든 파일로 되돌리세요."},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steam이 모드 없이 게임을 다시 실행했습니다. Mewgenics.exe 옆의 steam_appid.txt를 확인한 뒤(복구 버튼) 다시 실행하세요."},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"이 게임은 모드가 지원하는 버전(1.1.21239)이 아닙니다. 게임이 업데이트되었나요?"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"다른 프로그램(오버레이, 백신, 안티 치트)이 모드 로드를 막았습니다."},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"게임 시작이 너무 오래 걸려 모드를 로드하지 못했습니다. 다시 실행하세요."},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"모드 주입에 실패해 게임이 종료되었습니다. 다시 실행해 보고, 문제가 반복되면 loader.txt를 보내 주세요."},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"mgmp.json에 게임 위치가 없습니다. 설정에서 찾아보기…를 눌러 Mewgenics.exe를 선택하세요."},
    {u8"build %s · protocole %u", u8"빌드 %s · 프로토콜 %u"},
    {u8"Steam ouvert", u8"Steam 실행 중"},
    {u8"Steam fermé", u8"Steam 꺼짐"},
    {u8"Réglages (mgmp.json)", u8"설정 (mgmp.json)"},
    {u8"Jeu trouvé", u8"게임 찾음"},
    {u8"Jeu incomplet", u8"게임 파일 누락"},
    {u8"Jeu introuvable", u8"게임을 찾을 수 없음"},
    {u8"Fichiers du mod", u8"모드 파일"},
    {u8"Réparer", u8"복구"},
    {u8"Jouer", u8"플레이"},
    {u8"Règle d'abord les croix rouges.", u8"먼저 빨간 X 표시를 해결하세요."},
    {u8"Réglages", u8"설정"},
    {u8"Journaux", u8"로그"},
    {u8"Langue", u8"언어"},
    {u8"Le jeu (Mewgenics.exe)", u8"게임 (Mewgenics.exe)"},
    {u8"Parcourir…", u8"찾아보기…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe와 resources.gpak이 있습니다."},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"옆에 resources.gpak이 없습니다. 올바른 폴더가 아닙니다."},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.", u8"여기에 Mewgenics.exe가 없습니다. 찾아보기로 선택하세요."},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Steam으로 함께 플레이 (친구 목록)"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Steam에 문제가 있으면 체크를 해제하세요. 게임의 멀티플레이 메뉴에서 주소로만 접속하게 됩니다."},
    {u8"Enregistrer", u8"저장"},
    {u8"Retour", u8"뒤로"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"아직 로그가 없습니다. 모드는 게임을 할 때마다 로그를 남깁니다."},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"모드 로그 1개가 Mewgenics\u00A0Multi.exe 옆에 있습니다."},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"모드 로그 %d개가 Mewgenics\u00A0Multi.exe 옆에 있습니다."},
    {u8"Le dernier : %s", u8"최근 로그: %s"},
    {u8"écrit le %s", u8"작성 시각: %s"},
    {u8"Ouvrir le dossier", u8"폴더 열기"},
    {u8"Copier le dernier journal", u8"최근 로그 복사"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"도움이 필요하면 최근 로그를 복사한 뒤 Discord에 Ctrl+V로 붙여 넣으세요."},
    {u8"Lancement", u8"실행"},
    {u8"Le jeu se lance avec le mod…", u8"모드와 함께 게임을 실행하는 중…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.", u8"즐거운 시간 되세요! 이 창은 자동으로 닫힙니다."},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"자세한 내용은 Mewgenics\u00A0Multi.exe 옆의 loader.txt에 있습니다."},
    {u8"Réessayer", u8"다시 시도"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"Steam이 게임을 다시 실행하면 1~2분 걸릴 수 있습니다."},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.", u8"로그를 복사했습니다. Discord에 Ctrl+V로 붙여 넣으세요."},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"클립보드에 복사하지 못했습니다. 다시 시도하거나 폴더를 여세요."},
    {u8"Où est Mewgenics.exe ?", u8"Mewgenics.exe는 어디에 있나요?"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"이 PC에서 Direct2D를 쓸 수 없어 Mewgenics Multi를 표시할 수 없습니다."},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"그래픽 카드를 준비하지 못해 Mewgenics Multi를 표시할 수 없습니다."},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"각자의 집과 고양이,\n"
     u8"모험은 다 함께!"},
    {u8"(illustrations du jeu introuvables)", u8"(게임 일러스트를 찾을 수 없음)"},
    {u8"Comme le jeu (%s)", u8"게임과 동일 (%s)"},
    {u8"Comme Windows (%s)", u8"Windows와 동일 (%s)"},
    {u8"Automatique (%s)", u8"자동 (%s)"},
    {u8"Lancement annulé.", u8"실행이 취소되었습니다."},
    {u8"Lancement du mod…", u8"모드 실행 중…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windows가 실행을 준비하지 못했습니다. Mewgenics Multi를 다시 실행하세요."},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exe를 시작하지 못했습니다. zip에서 모드 폴더를 다시 압축 해제하세요(백신이 격리했을 수도 있습니다)."},
    {u8"la connexion Steam avec l'hôte est coupée", u8"호스트와의 Steam 연결이 끊김"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"호스트가 Steam으로 응답하지 않음(로비가 닫혔거나 모드가 없을 수 있음)"},
    {u8"Steam refuse la connexion", u8"Steam이 연결을 거부함"},
    {u8"À %s de choisir", u8"%s이(가) 고를 차례"},
    {u8"À toi de choisir", u8"내가 고를 차례"},
    {u8"À %s de choisir la capacité à remplacer.", u8"%s이(가) 교체할 능력을 고를 차례입니다."},
    {u8"Ce chat est à %s", u8"%s의 고양이입니다"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"목걸이 선택\n"
     u8"\n"
     u8"공동 선택이 중단되었습니다(게임 오류).\n"
     u8"팀 전체의 목걸이를 직접 고르세요."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"목걸이 선택\n"
     u8"\n"
     u8"준비 완료! %s에게 알렸습니다.\n"
     u8"팀 전체의 출발은 그 플레이어가 확정합니다."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"목걸이 선택\n"
     u8"\n"
     u8"출발은 %s이(가) 확정합니다.\n"
     u8"고양이들이 목걸이를 차면 알려 주세요."},
    {u8"%s et %s", u8"%s, %s"},
    {u8"%s, %s", u8"%s, %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"목걸이 선택\n"
     u8"\n"
     u8"%s이(가) 아직 고양이들의 목걸이를 고르고 있습니다.\n"
     u8"그래도 출발하려면 '확정!'을\n"
     u8"다시 누르세요."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"목걸이 선택\n"
     u8"\n"
     u8"%s이(가) 아직 고양이들의 목걸이를 고르고 있습니다.\n"
     u8"그래도 출발하려면 '확정!'을\n"
     u8"다시 누르세요."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"모험 출발\n"
     u8"\n"
     u8"%s의 출발 화면이 더 이상 표시되지 않습니다.\n"
     u8"남은 화면은 직접 진행하세요. 지도에서\n"
     u8"합류하게 됩니다."},
    {u8"  %s : parti", u8"  %s: 나감"},
    {u8"  %s : prêt !", u8"  %s: 준비 완료!"},
    {u8"  %s : charge ta partie...", u8"  %s: 내 게임을 불러오는 중..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"모험 출발\n"
     u8"\n"
     u8"함께 떠나기 위해 팀원들이 내 게임을 불러오는 중입니다:\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"아이템 선택\n"
     u8"\n"
     u8"여기서 보관함을 불러오지 못했습니다.\n"
     u8"고양이들은 지금 장착한 아이템으로 출발합니다."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"아이템 선택\n"
     u8"\n"
     u8"%s이(가) 아직 장착 중입니다.\n"
     u8"그래도 출발하려면 '확정!'을\n"
     u8"다시 누르세요."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"아이템 선택\n"
     u8"\n"
     u8"%s이(가) 아직 장착 중입니다.\n"
     u8"그래도 출발하려면 '확정!'을\n"
     u8"다시 누르세요."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"아이템 선택\n"
     u8"\n"
     u8"준비 완료! 고양이들이 이 아이템으로 출발합니다.\n"
     u8"팀 전체의 출발은 %s이(가) 확정합니다."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"아이템 선택\n"
     u8"\n"
     u8"고른 아이템을 %s의 모험에 반영하지\n"
     u8"못했습니다. 고양이들은 원래 아이템으로\n"
     u8"출발하며, 보관함은 바뀌지\n"
     u8"않습니다."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"아이템 선택\n"
     u8"\n"
     u8"%s이(가) 이미 출발을 확정했습니다. 고양이들은\n"
     u8"지금까지 고른 아이템으로 출발하며,\n"
     u8"이후 선택은 무시됩니다."},
    {u8"Ta bourse : %s.", u8"내 지갑: %s."},
    {u8"%s n'est plus dans la partie.", u8"%s이(가) 게임에서 나갔습니다."},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"이번 모험에는 내 지갑이 없습니다(고양이를 데려오지 않았습니다)."},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"교환은 집에서, 또는 멀티플레이 모험 중에 지갑으로 할 수 있습니다. 지금은 교환 불가: %s."},
    {u8"pas de partie à plusieurs en cours", u8"진행 중인 멀티플레이 게임 없음"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés", u8"게임 기능을 찾지 못해 집에서의 교환이 꺼짐"},
    {u8"tu es dans la partie de l'hôte", u8"호스트의 게임에 있음"},
    {u8"tu n'es pas dans ta maison", u8"내 집에 있지 않음"},
    {u8"ta save n'est pas chargée", u8"세이브를 불러오지 않음"},
    {u8"un départ en aventure est en cours", u8"모험 출발 진행 중"},
    {u8"ta maison est attaquée", u8"집이 공격받는 중"},
    {u8"ta maison est illisible", u8"집 정보를 읽을 수 없음"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"보관함이 호스트의 모험에서 고양이들이 가져올 것을 기다리는 중(돌아왔다면 세이브를 다시 불러오세요: Esc, '저장 후 메뉴화면으로 나가기', 그다음 '플레이'와 세이브 선택)"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"보관함의 아이템이 아직 새 주인을 기다리는 중(전리품, 구매품, NPC)"},
    {u8"Pas d'échange maintenant : %s.", u8"지금은 교환할 수 없습니다: %s."},
    {u8"Ton don précédent attend encore sa réponse.", u8"이전 선물이 아직 응답을 기다리고 있습니다."},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"모드가 아직 세이브를 인식하지 못했습니다. 메뉴에서 다시 불러오세요."},
    {u8"Choisis un joueur de la partie.", u8"게임에 있는 플레이어를 고르세요."},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.", u8"지금은 교환할 수 없습니다: %s이(가) 집에 없습니다."},
    {u8"Ce montant n'est pas possible.", u8"줄 수 없는 수량입니다."},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.", u8"줄 코인, 먹이 또는 아이템을 고르세요."},
    {u8"16 objets au plus par don.", u8"선물당 아이템 최대 16개."},
    {u8"Pas d'objets maintenant : %s.", u8"지금은 아이템을 줄 수 없습니다: %s."},
    {u8"Ta maison est illisible : pas de don.", u8"집 정보를 읽을 수 없어 선물할 수 없습니다."},
    {u8"Tu n'as pas autant de pièces.", u8"코인이 부족합니다."},
    {u8"Tu n'as pas autant de nourriture.", u8"먹이가 부족합니다."},
    {u8"Ton rangement est illisible : pas de don.", u8"보관함을 읽을 수 없어 선물할 수 없습니다."},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"고른 아이템 중 하나가 아직 끝나지 않은 다른 선물에 들어 있습니다."},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"고른 아이템 중 하나가 보관함에 없습니다. 다시 고르세요."},
    {u8"Un objet choisi ne peut pas se donner.", u8"고른 아이템 중 줄 수 없는 것이 있습니다."},
    {u8"Le don n'a pas pu partir (connexion).", u8"선물을 보내지 못했습니다(연결 문제)."},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…", u8"%s에게 선물을 보냈습니다(%s). 응답을 기다리는 중…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…", u8"호스트에게 선물 처리를 요청했습니다. 응답을 기다리는 중…"},
    {u8"Pas de don : %s.", u8"선물 불가: %s."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%s이(가) 선물을 받았습니다. 아이템 화면을 닫으면 선물한 아이템이 보관함에서 빠집니다."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%s이(가) 선물을 받았습니다. 선물한 아이템은 가능한 한 빨리(출발, 방어전, 고양이들의 귀환 뒤) 보관함에서 빠집니다."},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%s이(가) 선물을 받았습니다. 이 세이브로 돌아오면 선물한 것이 집에서 빠집니다."},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"%s이(가) 선물에 응답하지 않았습니다. 집에서 빠진 것은 없습니다."},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"%s이(가) 선물에 응답하지 않았습니다. 집에서 빠진 것은 없습니다."},
    {u8"Les échanges marchent pendant une partie à plusieurs.", u8"교환은 멀티플레이 게임 중에만 할 수 있습니다."},
    {u8"Dans ta maison : %s.", u8"내 집: %s."},
    {u8"%s n'est pas dans sa maison.", u8"%s이(가) 집에 없습니다."},
    {u8"%lld pièces", u8"%lld 코인"},
    {u8"%lld pièce", u8"%lld 코인"},
    {u8"%lld nourritures", u8"먹이 %lld개"},
    {u8"%lld nourriture", u8"먹이 %lld개"},
    {u8"rien", u8"없음"},
    {u8"%s n'est pas dans sa maison", u8"%s이(가) 집에 없음"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s", u8"%s의 게임에 창이나 아이템 화면이 열려 있음"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"%s의 게임이 아이템을 보관하지 못함"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"%s의 게임이 아이템을 보관하지 못함"},
    {u8"les dons ne marchent pas chez %s", u8"%s에게는 선물이 작동하지 않음"},
    {u8"%s n'est plus dans la partie", u8"%s이(가) 게임에서 나감"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%s의 고양이가 아직 호스트의 모험에 있음(보관함이 귀환을 기다리는 중)"},
    {u8"raison inconnue", u8"알 수 없는 이유"},
    {u8"Cadeau d'%s", u8"%s의 선물"},
    {u8"Cadeau de %s", u8"%s의 선물"},
    {u8"%s t'a donné %s.", u8"%s이(가) 준 선물: %s."},
    {u8"C'est dans ta maison, et ta save est à jour.", u8"집에 들어왔고, 세이브에도 반영되었습니다."},
    {u8"C'est dans ta maison.", u8"집에 들어왔습니다."},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"뒤늦게 %s이(가) %s을(를) 받았습니다. 집에서 빠졌습니다."},
    {u8"%s a bien reçu %s : c'est parti de ta maison.", u8"%s이(가) %s을(를) 받았습니다. 집에서 빠졌습니다."},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"(아이템 1개가 보관함에서 빠지지 않아 내 집에도 남아 있습니다.)"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"(아이템 %u개가 보관함에서 빠지지 않아 내 집에도 남아 있습니다.)"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"%s에게 선물을 보내지 못했습니다: %s. 집에서 빠진 것은 없습니다."},
    {u8"Rien à donner dans ton rangement.", u8"보관함에 줄 수 있는 것이 없습니다."},
    {u8"Tape des lettres pour chercher un objet.", u8"글자를 입력해 아이템을 검색하세요."},
    {u8"Recherche : %s_", u8"검색: %s_"},
    {u8"Aucun objet ne correspond.", u8"일치하는 아이템이 없습니다."},
    {u8"(page %u sur %u)", u8"(%u / %u 페이지)"},
    {u8"Butin : %s.", u8"전리품: %s."},
    {u8"Besoin", u8"필요"},
    {u8"Cupidité", u8"욕심"},
    {u8"Passer", u8"패스"},
    {u8"%u s", u8"%u초"},
    {u8"toi", u8"나"},
    {u8"Joueur %u", u8"플레이어 %u"},
    {u8"Un objet", u8"아이템"},
    {u8"Partage du butin : objet %u sur %u", u8"전리품 분배: 아이템 %u / %u"},
    {u8"Besoin : %s", u8"필요: %s"},
    {u8"Cupidité : %s", u8"욕심: %s"},
    {u8"Passer : %s", u8"패스: %s"},
    {u8"Partage du butin", u8"전리품 분배"},
    {u8"%u objets du sac à partager entre %s.", u8"배낭 아이템 %u개를 %s이(가) 나눠 가집니다."},
    {u8"%u objet du sac à partager entre %s.", u8"배낭 아이템 %u개를 %s이(가) 나눠 가집니다."},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"아이템마다 필요(쓸 데가 있음), 욕심(아무도\n"
     u8"필요 없을 때) 또는 패스를 고르세요. 동점은 100면 주사위로 가립니다."},
    {u8"En attente de %s...", u8"%s을(를) 기다리는 중..."},
    {u8"En attente de %s... (%s)", u8"%s을(를) 기다리는 중... (%s)"},
    {u8"Tout le monde est là : ça commence !", u8"모두 모였습니다. 시작합니다!"},
    {u8"Commencer", u8"시작"},
    {u8"D'accord", u8"확인"},
    {u8"Le veux-tu ? (%s)", u8"가져가시겠습니까? (%s)"},
    {u8"Je le veux", u8"가져가기"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"필요: 쓸 데가 있음. 다른 사람보다 우선합니다.\n"
     u8"욕심: 아무도 필요하지 않을 때 가져갑니다.\n"
     u8"(%s)"},
    {u8"Ton choix : %s.", u8"내 선택: %s."},
    {u8"Ton choix est fait.", u8"선택을 마쳤습니다."},
    {u8"En attente de %s (%s).", u8"%s을(를) 기다리는 중 (%s)."},
    {u8"Le résultat arrive...", u8"결과를 기다리는 중..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s이(가) 자리에 없어 차례를 넘깁니다."},
    {u8"%s n'est pas là : son tour passe.", u8"%s이(가) 자리에 없어 차례를 넘깁니다."},
    {u8"Personne n'en veut : il reste chez toi.", u8"아무도 원하지 않아 나에게 남습니다."},
    {u8"Personne n'en veut : il reste chez %s.", u8"아무도 원하지 않아 %s에게 남습니다."},
    {u8"Tu l'emportes (%s) !", u8"획득했습니다! (%s)"},
    {u8"%s l'emporte (%s).", u8"%s이(가) 획득했습니다 (%s)."},
    {u8"Dés : %s (égalité départagée).", u8"주사위: %s (동점이라 다시 굴림)."},
    {u8"Dés : %s.", u8"주사위: %s."},
    {u8"Partage du butin terminé", u8"전리품 분배 완료"},
    {u8"Tu remportes : %s.", u8"내가 획득한 아이템: %s."},
    {u8"%s remporte : %s.", u8"%s이(가) 획득한 아이템: %s."},
    {u8"Remporté, mais resté chez %s : %s.", u8"획득했지만 %s에게 남음: %s."},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"아무도 원하지 않음(내가 보관): %s."},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"아무도 원하지 않음(%s이(가) 보관): %s."},
    {u8"Aucun objet n'a été partagé.", u8"분배된 아이템이 없습니다."},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"세이브가 이미 열려 있습니다. 다음 창에서 전리품과\n"
     u8"함께 다시 불러오는 방법을 알려 드립니다."},
    {u8"Ton butin t'attend dans ton rangement, chez toi.", u8"전리품은 내 집 보관함에서 기다리고 있습니다."},
    {u8"Ce que les autres remportent quitte ton rangement.", u8"다른 플레이어가 획득한 아이템은 내 보관함에서 빠집니다."},
    {u8"Dr Beanies", u8"닥터 비니스"},
    {u8"Butch", u8"부치"},
    {u8"Tink", u8"팅크"},
    {u8"Frank", u8"프랭크"},
    {u8"Bébé Jack", u8"베이비 잭"},
    {u8"Tracy", u8"트레이시"},
    {u8"Broyeur d'organe", u8"내장 분쇄자"},
    {u8"Steven", u8"스티븐"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"보상 준비 완료\n"
     u8"\n"
     u8"%s이(가) %s에게 준 고양이가\n"
     u8"게이지를 채웠습니다. 내가 집에 들어가면\n"
     u8"대신 보상을 받습니다. 코인, 먹이, 아이템은\n"
     u8"%s에게, 해금 요소는\n"
     u8"모두에게 돌아갑니다."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"%s의 보상\n"
     u8"\n"
     u8"%s의 선물에 대한 보상을\n"
     u8"그 플레이어에게 보내지 못해 내 집에 남습니다."},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"%s에게 가는 보상\n"
     u8"\n"
     u8"%s이(가) %s의 고양이에 답례했습니다:\n"
     u8"%s은(는) 그 플레이어의 집으로 갑니다.\n"
     u8"해금 요소는 모두에게 돌아갑니다."},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"모두를 위한 보상\n"
     u8"\n"
     u8"%s이(가) %s의 고양이에 답례했습니다:\n"
     u8"해금 요소는 모두에게\n"
     u8"돌아갑니다."},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s이(가) 고양이 선물에 답례합니다: %s\n"
     u8"(해금 요소는 모두에게 돌아갑니다)."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"%s의 보상\n"
     u8"\n"
     u8"%s이(가) 고양이 선물에 답례합니다: %s.\n"
     u8"보상은 다음에 세이브를 불러올 때(모험에서\n"
     u8"돌아올 때) 내 집에 들어옵니다.\n"
     u8"해금 요소는\n"
     u8"모두에게 돌아갑니다."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"%s의 보상\n"
     u8"\n"
     u8"내 고양이가 %s의 게이지를 채웠습니다.\n"
     u8"보상인 해금 요소는 모두에게\n"
     u8"돌아갑니다. 다음에 세이브를 불러올 때\n"
     u8"적용됩니다."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"%s의 보상\n"
     u8"\n"
     u8"%s이(가) 채운 %s의 게이지 보상은\n"
     u8"그 플레이어가 없어서 내 집에 남습니다."},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"보상 준비 완료\n"
     u8"\n"
     u8"내 고양이가 %s의 게이지를 채웠습니다!\n"
     u8"%s이(가) 자기 집에 들어가는 즉시\n"
     u8"대신 보상을 받아 줍니다."},
    {u8"les bourses ne marchent pas dans cette partie", u8"이 게임에서는 지갑을 쓸 수 없음"},
    {u8"rien à donner", u8"줄 것이 없음"},
    {u8"c'est trop d'un coup", u8"한 번에 너무 많음"},
    {u8"choisis un autre joueur", u8"다른 플레이어를 골라야 함"},
    {u8"ton don précédent attend encore la réponse de l'hôte", u8"이전 선물이 아직 호스트의 응답을 기다리는 중"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"이번 모험에 내 지갑이 없음"},
    {u8"envoi impossible (connexion)", u8"보낼 수 없음(연결 문제)"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"모험 종료\n"
     u8"\n"
     u8"각자 지갑을 자기 집으로 가져갑니다:"},
    {u8"%s n'est plus là : sa bourse reste chez toi.", u8"%s이(가) 나가서 그 지갑은 내 집에 남습니다."},
    {u8"%s : rien.", u8"%s: 없음."},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.", u8"%s에게 지갑을 보내지 못해 내 집에 남습니다."},
    {u8"%s : %s.", u8"%s: %s."},
    {u8"Toi : %s.", u8"나: %s."},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"호스트가 아직 선물에 응답하지 않았습니다. 경로 선택 창에서 지갑을 확인하세요."},
    {u8"Ton don n'est pas passé : %s.", u8"선물을 보내지 못했습니다: %s."},
    {u8"(et d'autres trouvailles)", u8"(그 밖에도 더 있습니다)"},
    {u8"pas de bourse pour ce joueur dans cette aventure", u8"이번 모험에 이 플레이어의 지갑이 없음"},
    {u8"ta bourse n'en a pas autant", u8"지갑에 그만큼 없음"},
    {u8"l'aventure n'est plus suivie", u8"모험이 더 이상 추적되지 않음"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s이(가) %s을(를) 줬습니다. 내 지갑에 들어왔습니다."},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"%s을(를) %s에게 줬습니다. 상대의 지갑에 들어갔습니다."},
    {u8"%s a donné %s à %s.", u8"%s이(가) %s을(를) %s에게 줬습니다."},
    {u8"Un chat", u8"어떤 고양이"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%s이(가) %s을(를) 얻고 %s을(를) 잃었습니다. 내 지갑에 반영됩니다."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s이(가) %s을(를) 얻고 %s을(를) 잃었습니다. %s의 지갑에 반영됩니다."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s이(가) %s을(를) 얻고 %s을(를) 잃었습니다. %s의 지갑에 반영됩니다."},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s이(가) %s을(를) 찾아 내 지갑에 넣었습니다."},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s이(가) %s을(를) 찾아 %s의 지갑에 넣었습니다."},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s이(가) %s을(를) 찾아 %s의 지갑에 넣었습니다."},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s이(가) %s을(를) 썼습니다. 내 지갑에서 빠집니다."},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.", u8"%s이(가) %s을(를) 썼습니다. %s의 지갑에서 빠집니다."},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.", u8"%s이(가) %s을(를) 썼습니다. %s의 지갑에서 빠집니다."},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s이(가) %s을(를) 잃었습니다. 내 지갑에서 빠집니다."},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%s이(가) %s을(를) 잃었습니다. %s의 지갑에서 빠집니다."},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%s이(가) %s을(를) 잃었습니다. %s의 지갑에서 빠집니다."},
    {u8"%s n'a rien trouvé.", u8"%s이(가) 아무것도 찾지 못했습니다."},
    {u8"un objet", u8"아이템"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.", u8"%s을(를) %s에 샀습니다. 내 지갑에서 빠집니다."},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.", u8"%s이(가) %s을(를) %s에 샀습니다. 자기 지갑에서 빠집니다."},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"%s을(를) 샀습니다. 내 지갑에 %s 추가."},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%s이(가) %s을(를) 샀습니다. 자기 지갑에 %s 추가."},
    {u8"Tu as acheté %s (gratuit).", u8"%s을(를) 샀습니다(무료)."},
    {u8"%s a acheté %s (gratuit).", u8"%s이(가) %s을(를) 샀습니다(무료)."},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"전투에서 내 고양이들이 %s을(를) 얻고 %s을(를) 잃었습니다. 내 지갑에 반영됩니다."},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"전투에서 내 고양이들이 %s을(를) 찾아 내 지갑에 넣었습니다."},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"전투에서 내 고양이들이 %s을(를) 잃었습니다. 내 지갑에서 빠집니다."},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"전투에서 내 고양이들이 아무것도 찾지 못했습니다."},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"전투에서 %s의 고양이들이 %s을(를) 얻고 %s을(를) 잃었습니다. 자기 지갑에 반영됩니다."},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"전투에서 %s의 고양이들이 %s을(를) 얻고 %s을(를) 잃었습니다. 자기 지갑에 반영됩니다."},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"전투에서 %s의 고양이들이 %s을(를) 찾아 자기 지갑에 넣었습니다."},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"전투에서 %s의 고양이들이 %s을(를) 찾아 자기 지갑에 넣었습니다."},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"전투에서 %s의 고양이들이 %s을(를) 잃었습니다. 자기 지갑에서 빠집니다."},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"전투에서 %s의 고양이들이 %s을(를) 잃었습니다. 자기 지갑에서 빠집니다."},
    {u8"En combat, les chats d'%s n'ont rien trouvé.", u8"전투에서 %s의 고양이들이 아무것도 찾지 못했습니다."},
    {u8"En combat, les chats de %s n'ont rien trouvé.", u8"전투에서 %s의 고양이들이 아무것도 찾지 못했습니다."},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"배낭이 열려 있습니다\n"
     u8"\n"
     u8"호스트가 다른 칸에 들어갔습니다.\n"
     u8"따라가려면 배낭을 닫으세요."},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"아이템\n"
     u8"\n"
     u8"%s의 고양이입니다. 주인만\n"
     u8"아이템을 바꿀 수 있습니다."},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"아이템\n"
     u8"\n"
     u8"칸을 진행하는 동안(상점, 레벨 업\n"
     u8"등)에는 호스트만 아이템을 바꿀 수 있습니다.\n"
     u8"아이템 장착은 지도에서 하세요."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"아이템\n"
     u8"\n"
     u8"호스트가 아직 지도로 돌아오지 않았습니다.\n"
     u8"기다리면 배낭 잠금이 자동으로 풀립니다."},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"아이템\n"
     u8"\n"
     u8"배낭을 호스트의 배낭과 맞추는 중입니다.\n"
     u8"닫았다가 잠시 후 다시 여세요."},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"아이템\n"
     u8"\n"
     u8"다른 플레이어가 먼저 이 아이템을 가져갔습니다.\n"
     u8"변경이 적용되지 않았습니다. 배낭을 닫으면\n"
     u8"호스트의 배낭과 같아집니다."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"아이템\n"
     u8"\n"
     u8"마지막 변경이 도착하기 전에 호스트가\n"
     u8"다른 칸에 들어가 변경이 취소되었습니다.\n"
     u8"지도에서 다시 하세요."},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"아이템\n"
     u8"\n"
     u8"이 아이템은 이제 없습니다. 다른 플레이어가\n"
     u8"먼저 가져갔습니다."},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).", u8"%s이(가) 나가서 구매품이 내 집에 남습니다(%s)."},
    {u8"%s rapporte ses achats : %s.", u8"%s이(가) 구매품을 가져갑니다: %s."},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%s의 구매품을 보내지 못해 내 집에 남습니다."},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%s의 구매품을 보내지 못해 내 집에 남습니다."},
    {u8"Achats de l'aventure", u8"모험 중 구매품"},
    {u8"Boutique", u8"상점"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s이(가) 아직 이 상점에서\n"
     u8"쇼핑 중입니다. 기다려 주세요(나가면 더는\n"
     u8"그 플레이어들 대신 구매할 수 없습니다).\n"
     u8"그래도 나가려면 한 번 더 누르세요."},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s이(가) 아직 이 상점에서\n"
     u8"쇼핑 중입니다. 기다려 주세요(나가면 더는\n"
     u8"그 플레이어 대신 구매할 수 없습니다).\n"
     u8"그래도 나가려면 한 번 더 누르세요."},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"호스트에게 구매를 요청하지 못해\n"
     u8"아무것도 사지 않았습니다. 잠시 후 다시 시도하세요."},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"호스트가 이 상점을 나갔습니다.\n"
     u8"구매가 이루어지지 않았습니다."},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"호스트 쪽에서 이 아이템을 더 이상 팔지 않습니다.\n"
     u8"구매가 이루어지지 않았습니다."},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"이 아이템은 방금 팔렸습니다.\n"
     u8"구매가 이루어지지 않았습니다."},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"가격은 %s인데 지갑에는 %lld뿐입니다."},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"지갑이 부족해 이 아이템을 살 수 없습니다."},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"(나를 포함해 모두 자기 지갑으로 냅니다.)"},
    {u8"(Chacun paie avec sa bourse.)", u8"(각자 자기 지갑으로 냅니다.)"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"지금은 캔디와 패밀리어를 호스트만\n"
     u8"살 수 있습니다(호스트의 지갑으로)."},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.", u8"호스트의 게임에서 구매를 처리하지 못했습니다."},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"이번 모험에는 지갑이 없습니다.\n"
     u8"호스트만 구매할 수 있습니다."},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"이번 모험에 고양이를 데려오지\n"
     u8"않아서 물건을 살 지갑이\n"
     u8"없습니다."},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"이번 모험에는 내 지갑이 없습니다\n"
     u8"(고양이를 데려오지 않았습니다). 구매 불가."},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"호스트가 지금 대신 구매할 수 없습니다\n"
     u8"(호스트 쪽에 창이 열려 있습니다).\n"
     u8"아무것도 사지 않았습니다. 잠시 후 다시 시도하세요."},
    {u8"un bonbon", u8"캔디"},
    {u8"un familier", u8"패밀리어"},
    {u8"un article", u8"상품"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"모험 종료\n"
     u8"\n"
     u8"%s이(가) %s의 집으로 돌아갑니다."},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"모험 종료\n"
     u8"\n"
     u8"%s이(가) %s의 집으로 돌아갑니다."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"모험 출발\n"
     u8"\n"
     u8"다른 플레이어의 고양이들이 출발하지 못했고\n"
     u8"내 출발 상자도 비어 있어 이번에는 모험을\n"
     u8"떠나지 않습니다. (이유는 모드 로그 mgmp.log에 있습니다.)"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"모험 출발\n"
     u8"\n"
     u8"출발 상자가 비어 있고 고양이를 데려오는\n"
     u8"사람도 없어 이번에는 모험을 떠나지 않습니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"혼합 팀\n"
     u8"\n"
     u8"다른 플레이어의 고양이들이 팀에 합류하지\n"
     u8"못해 내 고양이들만 출발합니다.\n"
     u8"(이유는 모드 로그 mgmp.log에 있습니다.)"},
    {u8"Retour de l'aventure", u8"모험에서 귀환"},
    {u8"%s est de retour à la maison.", u8"%s이(가) 집으로 돌아왔습니다."},
    {u8"%s sont de retour à la maison.", u8"%s이(가) 집으로 돌아왔습니다."},
    {u8"%s a péri pendant l'aventure.", u8"%s이(가) 모험 중에 죽었습니다."},
    {u8"%s ont péri pendant l'aventure.", u8"%s이(가) 모험 중에 죽었습니다."},
    {u8"%s ne revient pas de l'aventure.", u8"%s이(가) 모험에서 돌아오지 않습니다."},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s이(가) 모험에서 돌아오지 않습니다."},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s이(가) 출발 후에 바뀌어서\n"
     u8"모험 결과가 반영되지 않았습니다."},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s이(가) 출발 후에 바뀌어서\n"
     u8"모험 결과가 반영되지 않았습니다."},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"출발 후에 보관함이 바뀌어서\n"
     u8"출발할 때 장착한 아이템을 보관함에서 빼지 않았습니다."},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"보관함은 바뀌지 않았습니다. 출발할 때\n"
     u8"장착한 아이템이 아직 보관함에 있습니다."},
    {u8"Tu rapportes %s dans ta maison.", u8"%s을(를) 집으로 가져갑니다."},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.",
     u8"NPC들이 고양이 선물에 답례했습니다: %s을(를) 집으로 보냈습니다."},
    {u8"Cadeaux des PNJ", u8"NPC의 선물"},
    {u8"Ton butin est dans ton rangement : %s.", u8"전리품이 보관함에 들어왔습니다: %s."},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.", u8"NPC의 선물이 보관함에 들어왔습니다: %s."},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.", u8"모험 중에 합류한 %s이(가) 내 집으로 옵니다."},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.", u8"모험 중에 합류한 %s이(가) 내 집으로 옵니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%s이(가) 팀에 합류하지 못했습니다.\n"
     u8"이번 모험에서는 팀 고양이 중 일부를\n"
     u8"내가 조종합니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%s이(가) 팀에 합류하지 못했습니다.\n"
     u8"이번 모험에서는 팀 고양이 중 일부를\n"
     u8"내가 조종합니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%s이(가) 팀에 합류하지 못했습니다.\n"
     u8"조종할 고양이 없이 모험을 따라갑니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%s이(가) 팀에 합류하지 못했습니다.\n"
     u8"조종할 고양이 없이 모험을 따라갑니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%s이(가) 출발하지 못하고 집에 남습니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%s이(가) 출발하지 못하고 집에 남습니다."},
    {u8"Tes chats sont rentrés de l'aventure.", u8"내 고양이들이 모험에서 돌아왔습니다."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"새 고양이 합류\n"
     u8"\n"
     u8"%s이(가) %s의 팀에 합류했습니다. 이 고양이는\n"
     u8"내 것입니다! 내가 없는 동안 전투는 게임 AI가,\n"
     u8"선택은 %s이(가) 대신하며,\n"
     u8"모험이 끝나면 내 집으로 돌아옵니다."},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"새 고양이: %s이(가) 내 집으로 옵니다."},
    {u8"Nouveaux chats : %s arrive chez toi.", u8"새 고양이: %s이(가) 내 집으로 옵니다."},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"4번째 자리\n"
     u8"\n"
     u8"3인이면 팀에 빈자리가 하나 있습니다.\n"
     u8"내 자리이므로 고양이 2마리를 데려갑니다."},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"4번째 자리\n"
     u8"\n"
     u8"빈자리는 이미 %s이(가) 차지했습니다.\n"
     u8"3인이면 고양이 1마리만 데려갑니다.\n"
     u8"출발 상자에서 고양이 1마리를 빼세요."},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"4번째 자리\n"
     u8"\n"
     u8"4번째 플레이어가 있어 각자 고양이를\n"
     u8"1마리씩 데려갑니다. 출발 상자에서 고양이 1마리를 빼세요."},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"4번째 자리는 출발 상자에 고양이를 2마리째 먼저 넣은 사람이 갖습니다."},
    {u8"La 4e place est à %s.", u8"4번째 자리: %s."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%d인이면 고양이를 최대 %d마리 데려갑니다.\n"
     u8"출발 상자에서 고양이 %d마리를 빼세요.%s\n"
     u8"나머지 자리는 다른 플레이어의 고양이로 채워집니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%d인이면 고양이를 최대 %d마리 데려갑니다.\n"
     u8"출발 상자에서 고양이 %d마리를 빼세요.%s\n"
     u8"나머지 자리는 다른 플레이어의 고양이로 채워집니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%d인이면 고양이를 최대 %d마리 데려갑니다.\n"
     u8"출발 상자에서 고양이 %d마리를 빼세요.%s\n"
     u8"나머지 자리는 다른 플레이어의 고양이로 채워집니다."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"혼합 팀\n"
     u8"\n"
     u8"%d인이면 고양이를 최대 %d마리 데려갑니다.\n"
     u8"출발 상자에서 고양이 %d마리를 빼세요.%s\n"
     u8"나머지 자리는 다른 플레이어의 고양이로 채워집니다."},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"내 집에 있지 않아서\n"
     u8"다른 플레이어의 고양이로 플레이합니다."},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"출발 상자가 아직 해금되지 않아서\n"
     u8"다른 플레이어의 고양이로 플레이합니다."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"출발 상자가 비어 있어 다른 플레이어의 고양이로 플레이합니다.\n"
     u8"내 고양이를 데려가려면: '아직이요'를 누르고 1~2마리를 부치의 출발 상자에\n"
     u8"넣은 뒤, 내 '출발!' 표지판을 누르세요."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"출발 상자가 비어 있어 다른 플레이어의 고양이로 플레이합니다.\n"
     u8"내 고양이를 데려가려면: '아직이요'를 누르고 1마리를 부치의 출발 상자에\n"
     u8"넣은 뒤, 내 '출발!' 표지판을 누르세요."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"출발 상자에 고양이가 %d마리 있습니다(최대 %d마리).\n"
     u8"'아직이요'로 답하고 %d마리를 뺀 뒤,\n"
     u8"내 '출발!' 표지판을 누르세요.%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).", u8"내 고양이들은 부치의 출발 상자에서 출발합니다."},
    {u8"Tu emmènes %s.", u8"데려갈 고양이: %s."},
    {u8"Départ en aventure", u8"모험 출발"},
    {u8"La session n'est pas prête : réessaie dans un instant.", u8"세션이 아직 준비되지 않았습니다. 잠시 후 다시 시도하세요."},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"4번째 자리를 배정받는 중입니다.\n"
     u8"잠시 후 다시 물어봅니다."},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"모험에 보낼 고양이를\n"
     u8"준비하지 못했습니다(mgmp.log 참고)."},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s이(가) 모험에서 돌아왔지만, 그 결과가\n"
     u8"아직 세이브에 반영되지 않았습니다. 다시 떠나기 전에\n"
     u8"세이브를 다시 불러오세요('다시 불러오기'는 이번\n"
     u8"출발 후에 표시됩니다. 아니면 Esc, '저장 후 메뉴화면으로\n"
     u8"나가기', 그다음 '플레이'와 세이브 선택)."},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s이(가) 모험에서 돌아왔지만, 그 결과가\n"
     u8"아직 세이브에 반영되지 않았습니다. 다시 떠나기 전에\n"
     u8"세이브를 다시 불러오세요('다시 불러오기'는 이번\n"
     u8"출발 후에 표시됩니다. 아니면 Esc, '저장 후 메뉴화면으로\n"
     u8"나가기', 그다음 '플레이'와 세이브 선택)."},
    {u8"Le retour de %s attend d'être recopié.", u8"%s의 귀환 결과가 아직 반영되지 않았습니다."},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"호스트에게 응답을 보내지 못했습니다.\n"
     u8"다시 해 주세요."},
    {u8"Ta bourse de l'aventure : %s.", u8"이번 모험의 내 지갑: %s."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"출발 상자에 고양이가 %d마리 있습니다(최대 %d마리).\n"
     u8"%d마리를 뺀 뒤 내 '출발!' 표지판을 누르세요."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"새 고양이 합류\n"
     u8"\n"
     u8"%s이(가) 팀에 합류했습니다. 내 고양이입니다!\n"
     u8"내가 조종하며, 모험이 끝나면\n"
     u8"내 집으로 돌아옵니다."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"새 고양이 합류\n"
     u8"\n"
     u8"%s이(가) 팀에 합류했습니다. 내 고양이입니다."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"새 고양이 합류\n"
     u8"\n"
     u8"%s이(가) 팀에 합류했습니다. %s의 고양이입니다."},
    {u8"Un nouveau chat", u8"새 고양이"},
    {u8"en bas", u8"아래"},
    {u8"en haut", u8"위"},
    {u8"à droite", u8"오른쪽"},
    {u8"à gauche", u8"왼쪽"},
    {u8"au milieu", u8"가운데"},
    {u8"n°%d", u8"%d번"},
    {u8"%s (%s)", u8"%s (%s)"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]투표가 반영되지 않았습니다: %s.[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"경로 선택\n"
     u8"\n"},
    {u8"Même chemin", u8"같은 경로"},
    {u8"Choisir", u8"선택"},
    {u8"Changer", u8"변경"},
    {u8"Sortie", u8"출구"},
    {u8"Retour à la maison", u8"집으로 귀환"},
    {u8"Combat", u8"전투"},
    {u8"Combat difficile", u8"어려운 전투"},
    {u8"Mini-boss", u8"미니 보스"},
    {u8"Boss", u8"보스"},
    {u8"Événement", u8"이벤트"},
    {u8"Événement spécial", u8"특별 이벤트"},
    {u8"Trésor", u8"보물"},
    {u8"Meuble", u8"가구"},
    {u8"Nourriture", u8"먹이"},
    {u8"Bonus", u8"보너스"},
    {u8"Case vide", u8"빈 칸"},
    {u8"Case", u8"칸"},
    {u8"l'hôte n'est pas sur la carte", u8"호스트가 지도에 없음"},
    {u8"vos cartes ne sont pas les mêmes", u8"서로의 지도가 다름"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"호스트가 그 칸으로 갈 수 없음"},
    {u8"le chemin est déjà choisi", u8"경로가 이미 정해짐"},
    {u8"ce tour est fini", u8"이번 투표가 끝남"},
    {u8"l'hôte ne l'a pas pris", u8"호스트가 받아들이지 않음"},
    {u8"pas de vote", u8"투표 안 함"},
    {u8"absent, ne vote pas", u8"자리 비움, 투표 안 함"},
    {u8"pas encore sur la carte", u8"아직 지도에 없음"},
    {u8"pas encore voté", u8"아직 투표 안 함"},
    {u8"%s (toi) : %s", u8"%s (나): %s"},
    {u8"%s : %s", u8"%s: %s"},
    {u8"En pause : on attend tout le monde sur la carte.", u8"일시 정지: 모두가 지도에 올 때까지 기다립니다."},
    {u8"À toi de choisir : encore %u s.", u8"내가 고를 차례: %u초 남음."},
    {u8"En attente des autres : encore %u s.", u8"다른 플레이어를 기다리는 중: %u초 남음."},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"%s 사이 동점:\n"
     u8"무작위로 %s이(가) 뽑혔습니다."},
    {u8"Tout le monde a choisi %s.", u8"모두가 %s을(를) 골랐습니다."},
    {u8"%s l'emporte.", u8"%s(으)로 결정."},
    {u8"%s : prêt", u8"%s: 준비 완료"},
    {u8"%s : défend sa maison", u8"%s: 집을 지키는 중"},
    {u8"%s : pas encore", u8"%s: 아직"},
    {u8"Tout le monde est prêt !", u8"모두 준비되었습니다!"},
    {u8"En attente d'un joueur.", u8"플레이어 1명을 기다리는 중입니다."},
    {u8"En attente de %d joueurs.", u8"플레이어 %d명을 기다리는 중입니다."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"하루 마치기\n"
     u8"\n"
     u8"밤을 보낼 준비가 되었습니다.\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"내 표지판은 모험 출발용입니다.\n"
     u8"하루를 마치려면 팀에서 고양이를 빼세요."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"하루 마치기\n"
     u8"\n"
     u8"플레이어 1명이 밤을 보내려고 기다립니다.\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"하루 마치기\n"
     u8"\n"
     u8"플레이어 %d명이 밤을 보내려고 기다립니다.\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1명 대기 중"},
    {u8"%d joueurs attendent", u8"%d명 대기 중"},
    {u8"Passer la nuit", u8"밤 보내기"},
    {u8"Plus tard", u8"나중에"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"계속하려면 Esc를 누른 뒤\n"
     u8"'저장 후 메뉴화면으로 나가기'를 누르세요."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"받을 것\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"세이브가 이미 열려 있어 다음에 불러올 때\n"
     u8"반영됩니다. '다시 불러오기'를 누르면\n"
     u8"모드가 메뉴를 거쳐 세이브를 다시 엽니다."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"받을 것\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"세이브가 이미 열려 있어 다음에 불러올 때\n"
     u8"반영됩니다. 세이브를 다시 불러오세요:\n"
     u8"Esc, '저장 후 메뉴화면으로 나가기',\n"
     u8"그다음 '플레이'와 세이브 선택."},
    {u8"%s : j'arrive !", u8"%s: 갈게요!"},
    {u8"%s : réfléchit...", u8"%s: 고민 중..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"모험 출발\n"
     u8"\n"
     u8"모험을 떠나려고 합니다.\n"
     u8"팀원들의 대답:\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s: 아직 안 옴"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"팀원을 기다리는 중\n"
     u8"\n"
     u8"%s\n"
     u8"기다리지 않고 진행하면, 도착할 때까지\n"
     u8"그 고양이들은 게임 AI가 전투에서\n"
     u8"조종합니다."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"모험 출발\n"
     u8"\n"
     u8"%s이(가) 모험을 떠납니다!\n"
     u8"각자 자기 고양이를 데려가 조종합니다.\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"모험 진행 중\n"
     u8"\n"
     u8"%s이(가) 모험 중입니다!\n"
     u8"내 고양이는 함께하지 않으므로, 조종할\n"
     u8"고양이 없이 모험을 따라갑니다."},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"모험 진행 중\n"
     u8"\n"
     u8"%s이(가) 내 고양이들과 모험 중입니다!\n"
     u8"내가 없는 동안 전투는 게임 AI가,\n"
     u8"선택은 %s이(가) 대신합니다.\n"
     u8"팀에 합류하시겠습니까?"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"모험 진행 중\n"
     u8"\n"
     u8"%s이(가) 모험 중입니다!\n"
     u8"팀에 합류하시겠습니까?"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"모험 나가기\n"
     u8"\n"
     u8"%s의 모험이 끝났습니다.\n"
     u8"내 집으로 돌아갑니다."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"모험 나가기\n"
     u8"\n"
     u8"%s의 모험이므로 그 플레이어만\n"
     u8"포기할 수 있습니다. 모험은 나 없이 계속됩니다.\n"
     u8"\n"
     u8"집으로 돌아가시겠습니까? 다시 합류하려면\n"
     u8"집의 표지판을 누르세요."},
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
     u8"모험 나가기\n"
     u8"\n"
     u8"%s의 모험이므로 그 플레이어만\n"
     u8"포기할 수 있습니다. 모험은 나 없이 계속되며,\n"
     u8"내 고양이들은 그 팀에 남습니다. 전투는\n"
     u8"게임 AI가 조종하고, 모험이 끝나면\n"
     u8"내 집으로 돌아옵니다.\n"
     u8"\n"
     u8"집으로 돌아가시겠습니까? 다시 합류하려면\n"
     u8"집의 표지판을 누르세요."},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"모험 합류\n"
     u8"\n"
     u8"%s의 게임이 진행 중인 모험을 보내는 중입니다.\n"
     u8"그 플레이어가 방금 칸에서 선택을 했다면\n"
     u8"(이벤트, 레벨 업) 지도로 돌아온 뒤에\n"
     u8"합류하게 됩니다."},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"모험으로 출발\n"
     u8"\n"
     u8"%s의 게임을 불러옵니다.\n"
     u8"함께 출발합니다."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"집으로 귀환\n"
     u8"\n"
     u8"%s와(과)의 연결이 끊어졌습니다.\n"
     u8"내 집으로 돌아갑니다."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"집으로 귀환\n"
     u8"\n"
     u8"%s와(과)의 출발이 취소되었습니다.\n"
     u8"내 집으로 돌아갑니다."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"집으로 귀환\n"
     u8"\n"
     u8"%s의 모험이 끝났습니다.\n"
     u8"내 집으로 돌아갑니다."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"집으로 귀환\n"
     u8"\n"
     u8"%s의 전투는 끝났지만 내 쪽 전투는\n"
     u8"계속되고 있었습니다. 동기화가 어긋나\n"
     u8"아무도 그 전투를 진행하지 않았습니다.\n"
     u8"집으로 돌아왔고, 고양이들은 그 팀에 남습니다.\n"
     u8"\n"
     u8"모험에 다시 합류하려면\n"
     u8"집의 표지판을 누르세요."},
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
     u8"집으로 귀환\n"
     u8"\n"
     u8"내 전투는 끝났지만 %s의 전투는\n"
     u8"계속되고 있었습니다. 동기화가\n"
     u8"어긋났습니다. 집으로 돌아왔고, 고양이들은\n"
     u8"그 팀에 남아 전투가 끝날 때까지\n"
     u8"게임 AI가 조종합니다.\n"
     u8"\n"
     u8"모험에 다시 합류하려면\n"
     u8"집의 표지판을 누르세요."},
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
     u8"집으로 귀환\n"
     u8"\n"
     u8"%s이(가) 들어간 칸을 내 게임이\n"
     u8"따라가지 못했습니다. 내 쪽 모험이 더 이상\n"
     u8"같지 않습니다. 집으로 돌아왔고,\n"
     u8"고양이들은 그 팀에 남아 게임 AI가\n"
     u8"조종합니다.\n"
     u8"\n"
     u8"모험에 다시 합류하려면\n"
     u8"집의 표지판을 누르세요."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"집으로 귀환\n"
     u8"\n"
     u8"내 쪽 전투가 %s의 전투와\n"
     u8"어긋나 전투가 나 없이 계속되었으므로\n"
     u8"집으로 돌아왔습니다. 고양이들은 그 팀에\n"
     u8"남아 게임 AI가 조종합니다.\n"
     u8"\n"
     u8"모험에 다시 합류하려면\n"
     u8"집의 표지판을 누르세요."},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"이 세이브는 모험 진행 중입니다.\n"
     u8"멀티플레이에서는 호스트와 함께 모험합니다.\n"
     u8"메뉴로 돌아가 다른 세이브를 고르거나,\n"
     u8"이 세이브는 싱글플레이로 마치세요."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"모험 출발\n"
     u8"\n"
     u8"%s의 모험을 불러오는 중입니다."},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%s이(가) 모험을 다시 불러왔습니다.\n"
     u8"내 게임도 다시 불러옵니다."},
    {u8"Rentrer", u8"집으로 가기"},
    {u8"Rester", u8"머무르기"},
    {u8"Ne pas attendre", u8"기다리지 않기"},
    {u8"Recharger", u8"다시 불러오기"},
    {u8"J'arrive", u8"갈게요"},
    {u8"Pas encore", u8"아직이요"},
    {u8"Échanger", u8"교환"},
    {u8"Multijoueur", u8"멀티플레이"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"이 파이프는 %s의 집에 있는 것입니다.\n"
     u8"NPC에게 고양이를 보내려면\n"
     u8"내 집에서 하세요."},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"NPC는 %s의 게임을 따릅니다.\n"
     u8"이 버전의 게임에서는 파이프로 고양이를\n"
     u8"보낼 수 없습니다."},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"팀 전체의 패배입니다.\n"
     u8"%s의 고양이들도 돌아오지 않습니다."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"%s의 게임 참가\n"
     u8"\n"
     u8"이미 멀티플레이 로비에 있습니다.\n"
     u8"먼저 로비에서 나간 뒤(멀티플레이에서 '나가기'),\n"
     u8"'게임 참가'를 다시 누르세요."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"%s의 게임 참가\n"
     u8"\n"
     u8"게임이 메인 메뉴로 돌아간 뒤(진행 상황은\n"
     u8"'저장 후 메뉴화면으로 나가기'처럼 저장됩니다)\n"
     u8"%s의 로비에 참가합니다.\n"
     u8"\n"
     u8"나중에: 메인 메뉴로 갈 때\n"
     u8"참가합니다."},
    {u8"Y aller", u8"지금 가기"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"%s의 게임 참가\n"
     u8"\n"
     u8"메인 메뉴로 가는 즉시\n"
     u8"%s의 로비에 참가합니다\n"
     u8"(Esc, 그다음 '저장 후 메뉴화면으로 나가기')."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"%s의 게임 참가\n"
     u8"\n"
     u8"지금은 Steam으로 연결할 수 없습니다.\n"
     u8"잠시 후 다시 시도하거나 주소로 참가하세요\n"
     u8"(멀티플레이에서 '참가하기')."},
    {u8"%s a annulé le départ.", u8"%s이(가) 출발을 취소했습니다."},
    {u8"réseau local", u8"로컬 네트워크"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"모험 출발\n"
     u8"\n"
     u8"내 고양이들은 이미 %s의 팀과 함께 떠났습니다.\n"
     u8"내가 없는 동안 전투는 게임 AI가,\n"
     u8"선택은 %s이(가) 대신하며,\n"
     u8"모험이 끝나면 내 집으로 돌아옵니다."},
    {u8"Internet", u8"인터넷"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s이(가) 집을 지키는 중입니다. 자기 고양이들로\n"
     u8"혼자 싸우는 전투이며, 끝나면 돌아옵니다."},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s이(가) 집을 지키는 중입니다. 각자 자기\n"
     u8"고양이들로 혼자 싸우며, 끝나면 돌아옵니다."},
    {u8"%s a fini de défendre sa maison.", u8"%s이(가) 집 지키기를 마쳤습니다."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s이(가) 집 지키기를 마쳤습니다.\n"
     u8"%s은(는) 아직 집을 지키는 중입니다."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s이(가) 집 지키기를 마쳤습니다.\n"
     u8"%s은(는) 아직 집을 지키는 중입니다."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"모험 출발\n"
     u8"\n"
     u8"투표 후에 취소했으므로 %s이(가) 나와\n"
     u8"내 고양이 없이 떠납니다. 그 플레이어가 지도에\n"
     u8"도착하면 모험에 합류할 수 있습니다."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"모험 출발\n"
     u8"\n"
     u8"이번에는 출발할 수 있는 고양이가 없어\n"
     u8"%s이(가) 모험을 떠나지 않습니다."},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"전리품 분배가 아직 끝나지 않았습니다.\n"
     u8"아이템이 모두 지나갈 때까지 기다리세요."},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"%s의 모험에 합류하러 갑니다.\n"
     u8"집은 돌아온 뒤에 지키게 됩니다."},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"모험 출발은 %s이(가) 합니다.\n"
     u8"그 플레이어에게 '출발!'을 눌러 달라고 하세요."},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"멀티플레이\n"
     u8"\n"
     u8"방을 만들어 친구들이 참가하게 하거나,\n"
     u8"친구의 게임에 참가하세요."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"[c:red]포트 %u번을 열 수 없습니다.[/c]\n"
     u8"다른 프로그램이 사용 중일 수 있습니다.\n"
     u8"로비를 닫은 뒤 다시 시도하세요."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"로비를 여는 중..."},
    {u8"%s (hôte)", u8"%s (호스트)"},
    {u8"En attente d'un ami...", u8"친구를 기다리는 중..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Steam 친구는 클릭 한 번으로 참가할 수 있습니다.\n"
     u8"Steam 친구 목록에서 내 이름의 '게임 참가'를 누르면 됩니다.\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"또는 주소로:\n"
     u8"%s   (%s, 포트 %u)\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"친구에게 이 주소를 알려 주세요:\n"
     u8"%s   (%s, 포트 %u)\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"주소가 이미 복사되었습니다. Ctrl+V로 붙여 넣어 전달하세요.\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"이 PC의 주소를 찾을 수 없습니다(포트 %u).\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"%s\n"
     u8"플레이어 (%d/4):\n"
     u8"%s\n"
     u8"모두 모이면 세이브를 고르세요."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"게임 참가\n"
     u8"\n"
     u8"호스트 주소:\n"
     u8"%s_\n"
     u8"\n"
     u8"친구에게 받은 주소를 입력하거나\n"
     u8"Ctrl+V로 붙여 넣으세요. Enter로 확정합니다.%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Steam이 더 간단합니다. 친구 목록에서\n"
     u8"친구 이름의 '게임 참가'를 누르세요."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"게임 참가\n"
     u8"\n"
     u8"[c:red]잘못된 주소: %s[/c]\n"
     u8"\n"
     u8"예: 26.123.45.67\n"
     u8"다른 포트를 쓸 때: 26.123.45.67:27600"},
    {u8"(vide)", u8"(비어 있음)"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.", u8"두 사람의 게임과 모드 버전이 같은지 확인하세요."},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).", u8"호스트의 게임이 가득 찼습니다(최대 4명)."},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"모드 버전이 서로 다릅니다.\n"
     u8"두 사람 모두 최신 버전을 받으세요."},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"게임 버전이 서로 다릅니다.\n"
     u8"두 사람 모두 Mewgenics를 업데이트하세요(Steam)."},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"게임 파일이 서로 다릅니다.\n"
     u8"Steam에서 확인하세요(속성, 설치된 파일)."},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"mgmp.json의 net.control 설정이 두 플레이어에게\n"
     u8"같은 고양이를 줍니다. \"auto\"로 되돌리세요."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"[c:red]연결이 거부되었습니다.[/c]\n"
     u8"%s\n"
     u8"(%s)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"[c:red]%s의 게임에 참가할 수 없습니다:[/c]\n"
     u8"%s.\n"
     u8"로비에서 나간 뒤 다시 시도하세요(또는 주소로)."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"%s에 연결하는 중..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"[c:red]%s에 연결할 수 없습니다.[/c]\n"
     u8"주소가 맞는지, 친구가\n"
     u8"'방 만들기'를 눌렀는지 확인하세요."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"연결 끊김."},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"세이브 선택 화면이 자동으로 열립니다."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"%s에 연결하는 중...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"[c:red]호스트와의 연결이 끊어졌습니다.[/c]\n"
     u8"로비에서 나간 뒤 다시 참가하세요."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"Steam으로 %s에 연결하는 중...\n"
     u8"(몇 초 걸립니다)%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"연결됨, 호스트에게 인사하는 중...%s"},
    {u8"%s (toi)", u8"%s (나)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"%s의 게임에 참가했습니다!\n"
     u8"\n"
     u8"플레이어 (%d/4):\n"
     u8"%s\n"
     u8"%s의 게임을 기다리는 중입니다.\n"
     u8"호스트가 세이브를 고르면\n"
     u8"세이브 선택 화면이 자동으로 열립니다."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"%s의 게임에 참가했습니다!\n"
     u8"\n"
     u8"플레이어 (%d/4):\n"
     u8"%s\n"
     u8"%s의 게임을 기다리는 중입니다.\n"
     u8"호스트가 세이브를 고르면\n"
     u8"내 세이브를 고를 수 있습니다."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"멀티플레이 로비\n"
     u8"\n"
     u8"%s의 게임에 참가했습니다!\n"
     u8"\n"
     u8"플레이어 (%d/4):\n"
     u8"%s\n"
     u8"내 세이브를 고르세요: 내 집, 내 고양이.\n"
     u8"NPC와 스토리는 %s의 것을 따르고,\n"
     u8"모험을 떠날 때는 그 플레이어의 팀에 합류합니다."},
    {u8"Héberger", u8"방 만들기"},
    {u8"Rejoindre", u8"참가하기"},
    {u8"Choisir la sauvegarde", u8"세이브 선택"},
    {u8"Arrêter", u8"닫기"},
    {u8"Choisir ta sauvegarde", u8"내 세이브 선택"},
    {u8"Quitter", u8"나가기"},
    {u8"Annuler", u8"취소"},
    {u8"Échanger avec %s", u8"%s와(과) 교환"},
    {u8"Donner à qui ?", u8"누구에게 줄까요?"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"위/아래로 고른 뒤 Enter를 누르세요."},
    {u8"Pièces à donner : %lld_", u8"줄 코인: %lld_"},
    {u8"Nourriture à donner : %lld_", u8"줄 먹이: %lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"숫자를 입력하고(T: 전부) Enter를 누르세요."},
    {u8"Échanger avec %s : les objets de ton rangement", u8"%s와(과) 교환: 내 보관함의 아이템"},
    {u8"16 au plus par don : %u choisis.", u8"선물당 최대 16개: %u개 선택됨."},
    {u8"16 au plus par don : %u choisi.", u8"선물당 최대 16개: %u개 선택됨."},
    {u8"Lecture du rangement...", u8"보관함을 읽는 중..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"위/아래: 선택, Space: 체크, 왼쪽/오른쪽: 수량.\n"
     u8"글자를 입력해 검색, 끝나면 Enter."},
    {u8"Tu n'as rien choisi à donner.", u8"줄 것을 고르지 않았습니다."},
    {u8"Tu n'as rien à donner pour l'instant.", u8"지금은 줄 것이 없습니다."},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"내 지갑에서 %s 줄까요?\n"
     u8"대상: %s"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"%s 줄까요?\n"
     u8"대상: %s"},
    {u8"Envoi du don...", u8"선물 보내는 중..."},
    {u8"Personne ne peut recevoir maintenant.", u8"지금은 받을 수 있는 사람이 없습니다."},
    {u8"Personne d'autre n'est dans la partie.", u8"게임에 다른 플레이어가 없습니다."},
    {u8"Confirmer", u8"결정"},
    {u8"Donner", u8"주기"},
    {u8"Recommencer", u8"다시 하기"},
    {u8"Fermer", u8"닫기"},
    {u8"Pas d'échange possible ici.", u8"여기서는 교환할 수 없습니다."},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"호스트가 다른 칸에 들어가 창이 닫혔습니다.\n"
     u8"고른 내용은 저장되어 있습니다(E로 다시 열기)."},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"모험 중 교환\n"
     u8"\n"
     u8"지도에서 E를 누르면 지갑의 코인이나\n"
     u8"먹이를 다른 플레이어에게 줄 수 있습니다."},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
