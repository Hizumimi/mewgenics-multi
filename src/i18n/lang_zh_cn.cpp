// lang_zh_cn.cpp -- Mewgenics Multi : les textes du mod et du lanceur en chinois simplifie (zh-cn).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : simple et direct, comme combined.csv. \u200B aux coupures permises.
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_zh_cn[];
const TrPair kTable_zh_cn[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.", u8"无法\u200B读取\u200Bmgmp.json：\u200B设置\u200B未保存。"},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"mgmp.json\u200B里\u200B有\u200B注释\u200B（// 或 /* */），\u200B保存\u200B会\u200B把\u200B它们\u200B删掉：\u200B先\u200B删掉\u200B注释，\u200B或者\u200B直接\u200B在\u200B文件\u200B里\u200B手动\u200B修改\u200B设置。"},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.json\u200B不\u200B是\u200B有效\u200B的\u200BJSON：\u200B修改\u200B设置\u200B前，\u200B先\u200B把\u200B它\u200B改好\u200B（或\u200B换回\u200Bzip\u200B里\u200B的\u200B那\u200B份）。"},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"无法\u200B写入\u200B模组\u200B文件夹：\u200B设置\u200B未保存。"},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"无法\u200B替换\u200Bmgmp.json\u200B（被\u200B其他\u200B程序\u200B打开\u200B了？）：\u200B设置\u200B未保存。"},
    {u8"Steam est ouvert.", u8"Steam\u200B已\u200B打开。"},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"没有\u200BSteam，\u200B游戏\u200B会\u200B在\u200B模组\u200B加载\u200B前\u200B关闭。\u200B打开\u200BSteam，\u200B等\u200B它\u200B连接\u200B上。"},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"Mewgenics\u00A0Multi.exe\u200B旁边\u200B缺少\u200Bmgmp.json：\u200B保存\u200B一次\u200B设置\u200B即可\u200B重新\u200B创建。"},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.json\u200B不\u200B是\u200B有效\u200B的\u200BJSON：\u200B把\u200B它\u200B改好，\u200B或\u200B换回\u200Bzip\u200B里\u200B的\u200B那\u200B份。"},
    {u8"Réglages lus dans mgmp.json.", u8"已\u200B从\u200Bmgmp.json\u200B读取\u200B设置。"},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"找不到\u200BMewgenics.exe\u200B（%s）：\u200B在\u200B“设置”\u200B里\u200B点击\u200B“浏览…”，\u200B选择\u200BMewgenics.exe。"},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"Mewgenics.exe\u200B旁边\u200B缺少\u200Bresources.gpak：\u200B在\u200BSteam\u200B里\u200B验证\u200B游戏\u200B文件。"},
    {u8"Jeu trouvé : %s.", u8"已\u200B找到\u200B游戏：\u200B%s。"},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.", u8"steam_appid.txt：\u200B找到\u200B游戏\u200B后\u200B再\u200B检查。"},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Mewgenics.exe\u200B旁边\u200B缺少\u200B这个\u200B文件：\u200B没有\u200B它，\u200BSteam\u200B会\u200B不\u200B带\u200B模组\u200B重新启动\u200B游戏。"},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"Mewgenics.exe\u200B旁边\u200B的\u200B这个\u200B文件\u200B里\u200B没有\u200B686060：\u200BSteam\u200B会\u200B不\u200B带\u200B模组\u200B重新启动\u200B游戏。"},
    {u8"steam_appid.txt est en place à côté du jeu.", u8"steam_appid.txt\u200B已\u200B在\u200B游戏\u200B旁边\u200B就位。"},
    {u8"Fichiers du mod présents.", u8"模组\u200B文件\u200B齐全。"},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Mewgenics\u00A0Multi.exe\u200B旁边\u200B缺少\u200Bmgmp.dll\u200B和\u200Bmgmp_loader.exe：\u200B从\u200Bzip\u200B里\u200B重新\u200B解压\u200B整个\u200B文件夹。"},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Mewgenics\u00A0Multi.exe\u200B旁边\u200B缺少\u200B%s：\u200B从\u200Bzip\u200B里\u200B重新\u200B解压\u200B整个\u200B文件夹。"},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi，\u200B版本\u200B%s，\u200B协议\u200B%u：\u200B所有人\u200B的\u200B版本\u200B必须\u200B一致。"},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"找不到\u200B游戏：\u200B先\u200B在\u200B“设置”\u200B里\u200B选择\u200BMewgenics.exe。"},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"无法\u200B在\u200B游戏\u200B旁边\u200B写入\u200Bsteam_appid.txt：\u200B手动\u200B复制\u200B它\u200B（见\u200BLISEZ-MOI\u200B第2步）。"},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"你\u200B没有\u200B同意\u200BWindows\u200B的\u200B授权\u200B请求：\u200Bsteam_appid.txt\u200B未\u200B复制。\u200B你\u200B可以\u200B重试，\u200B或\u200B手动\u200B复制\u200B（见\u200BLISEZ-MOI\u200B第2步）。"},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics Multi\u200B无法\u200B以\u200B管理员\u200B身份\u200B重新启动：\u200B手动\u200B复制\u200Bsteam_appid.txt\u200B（见\u200BLISEZ-MOI\u200B第2步）。"},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"以\u200B管理员\u200B身份\u200B复制\u200B失败：\u200B手动\u200B复制\u200Bsteam_appid.txt\u200B（见\u200BLISEZ-MOI\u200B第2步）。"},
    {u8"%02u/%02u/%04u à %02u:%02u", u8"%02u/%02u/%04u %02u:%02u"},
    {u8"Le mod n'a pas pu être chargé : %s", u8"模组\u200B加载\u200B失败：\u200B%s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exe\u200B没有\u200B加载\u200B模组\u200B就\u200B停止\u200B了。\u200B重新启动；\u200B如果\u200B还是\u200B这样，\u200B请\u200B发送\u200Bloader.txt。"},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exe\u200B没有\u200B加载\u200B模组\u200B就\u200B停止\u200B了\u200B（代码\u200B%s）。\u200B最可能\u200B的\u200B原因：\u200BSteam\u200B未\u200B打开\u200B或\u200B未\u200B连接。\u200B打开\u200BSteam，\u200B等\u200B它\u200B连接\u200B上，\u200B然后\u200B重新启动。"},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"游戏\u200B在\u200B启动\u200B时、\u200B模组\u200B加载\u200B前\u200B就\u200B关闭\u200B了。\u200B最\u200B常见\u200B的\u200B原因：\u200BSteam\u200B未\u200B打开\u200B或\u200B未\u200B连接。\u200B打开\u200BSteam，\u200B等\u200B它\u200B连接\u200B上，\u200B然后\u200B重新启动。"},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…", u8"Steam\u200B正在\u200B重新启动\u200B游戏\u200B（可能\u200B需要\u200B一两\u200B分钟）……"},
    {u8"Ouverture du jeu…", u8"正在\u200B打开\u200B游戏……"},
    {u8"Le jeu démarre…", u8"游戏启动中……"},
    {u8"Injection du mod dans le jeu…", u8"正在\u200B向\u200B游戏\u200B注入\u200B模组……"},
    {u8"Le mod est en place, le jeu reprend…", u8"模组\u200B已\u200B就位，\u200B游戏\u200B继续\u200B运行……"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"仍在\u200B等待\u200BSteam\u200B重新启动\u200B的\u200B游戏……\u200B如果\u200BSteam\u200B刚刚\u200B打开，\u200B它\u200B可能\u200B正在\u200B弹窗\u200B询问\u200B你。"},
    {u8"Jeu relancé trouvé : injection du mod…", u8"已\u200B找到\u200B重新启动\u200B的\u200B游戏：\u200B正在\u200B注入\u200B模组……"},
    {u8"Le jeu est lancé avec le mod.", u8"游戏\u200B已\u200B带\u200B模组\u200B启动。"},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.", u8"模组\u200B就位\u200B较\u200B慢；\u200B游戏\u200B照常\u200B继续。"},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.", u8"重新启动\u200B的\u200B游戏\u200B迟迟\u200B没有\u200B运行；\u200B照样\u200B注入\u200B模组。"},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"Mewgenics\u00A0Multi.exe\u200B旁边\u200B找不到\u200Bmgmp.dll：\u200B从\u200Bzip\u200B里\u200B重新\u200B解压\u200B模组\u200B文件夹。"},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"找不到\u200BMewgenics.exe：\u200B在\u200B“设置”\u200B里\u200B点击\u200B“浏览…”，\u200B选择\u200BMewgenics.exe。"},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"mgmp.json\u200B无法\u200B读取：\u200B把\u200B它\u200B改好，\u200B或\u200B换回\u200Bzip\u200B里\u200B的\u200B那\u200B份。"},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steam\u200B在\u200B没有\u200B模组\u200B的\u200B情况\u200B下\u200B重新启动\u200B了\u200B游戏：\u200B检查\u200BMewgenics.exe\u200B旁边\u200B的\u200Bsteam_appid.txt\u200B（“修复”\u200B按钮），\u200B然后\u200B重新启动。"},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"这个\u200B游戏\u200B不\u200B是\u200B模组\u200B对应\u200B的\u200B版本\u200B（1.1.21239）：\u200B游戏\u200B更新\u200B了\u200B吗？"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"有\u200B程序\u200B（覆盖层、\u200B杀毒软件、\u200B反作弊）\u200B阻止\u200B了\u200B模组\u200B加载。"},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"游戏\u200B启动\u200B时间\u200B太\u200B长：\u200B模组\u200B未\u200B加载。\u200B请\u200B重新启动。"},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"模组\u200B注入\u200B失败，\u200B游戏\u200B已\u200B关闭。\u200B重新启动；\u200B如果\u200B还是\u200B这样，\u200B请\u200B发送\u200Bloader.txt。"},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"mgmp.json\u200B里\u200B没有\u200B游戏\u200B位置：\u200B在\u200B“设置”\u200B里\u200B点击\u200B“浏览…”，\u200B选择\u200BMewgenics.exe。"},
    {u8"build %s · protocole %u", u8"版本\u200B%s · 协议\u200B%u"},
    {u8"Steam ouvert", u8"Steam已打开"},
    {u8"Steam fermé", u8"Steam未打开"},
    {u8"Réglages (mgmp.json)", u8"设置\u200B（mgmp.json）"},
    {u8"Jeu trouvé", u8"已找到游戏"},
    {u8"Jeu incomplet", u8"游戏不完整"},
    {u8"Jeu introuvable", u8"找不到游戏"},
    {u8"Fichiers du mod", u8"模组文件"},
    {u8"Réparer", u8"修复"},
    {u8"Jouer", u8"开始游戏"},
    {u8"Règle d'abord les croix rouges.", u8"先\u200B解决\u200B红叉\u200B的\u200B问题。"},
    {u8"Réglages", u8"设置"},
    {u8"Journaux", u8"日志"},
    {u8"Langue", u8"语言"},
    {u8"Le jeu (Mewgenics.exe)", u8"游戏\u200B（Mewgenics.exe）"},
    {u8"Parcourir…", u8"浏览…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe\u200B和\u200Bresources.gpak\u200B都\u200B在。"},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.", u8"旁边\u200B缺少\u200Bresources.gpak：\u200B文件夹\u200B不\u200B对。"},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.", u8"这里\u200B没有\u200BMewgenics.exe：\u200B用\u200B“浏览…”\u200B选择\u200B它。"},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"通过\u200BSteam\u200B一起\u200B玩\u200B（好友\u200B列表）"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"如果\u200BSteam\u200B出\u200B问题\u200B就\u200B取消\u200B勾选：\u200B游戏\u200B的\u200B“多人游戏”\u200B菜单\u200B里\u200B将\u200B只\u200B剩\u200B地址\u200B连接。"},
    {u8"Enregistrer", u8"保存"},
    {u8"Retour", u8"返回"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"暂时\u200B没有\u200B日志：\u200B模组\u200B每\u200B次\u200B游戏\u200B都\u200B会\u200B写\u200B一\u200B份。"},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"1份\u200B模组\u200B日志，\u200B在\u200BMewgenics\u00A0Multi.exe\u200B旁边。"},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"%d份\u200B模组\u200B日志，\u200B在\u200BMewgenics\u00A0Multi.exe\u200B旁边。"},
    {u8"Le dernier : %s", u8"最新：%s"},
    {u8"écrit le %s", u8"写于%s"},
    {u8"Ouvrir le dossier", u8"打开文件夹"},
    {u8"Copier le dernier journal", u8"复制最新日志"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"需要\u200B帮助\u200B时：\u200B复制\u200B最新\u200B日志，\u200B然后\u200B在\u200BDiscord\u200B里\u200B按\u200BCtrl+V\u200B粘贴。"},
    {u8"Lancement", u8"启动"},
    {u8"Le jeu se lance avec le mod…", u8"游戏\u200B正在\u200B带\u200B模组\u200B启动……"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.", u8"玩得\u200B开心！\u200B这个\u200B窗口\u200B会\u200B自动\u200B关闭。"},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"详情\u200B见\u200BMewgenics\u00A0Multi.exe\u200B旁边\u200B的\u200Bloader.txt。"},
    {u8"Réessayer", u8"重试"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.", u8"如果\u200BSteam\u200B重新启动\u200B游戏，\u200B可能\u200B需要\u200B一两\u200B分钟。"},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.", u8"日志\u200B已\u200B复制：\u200B在\u200BDiscord\u200B里\u200B按\u200BCtrl+V\u200B粘贴。"},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"无法\u200B写入\u200B剪贴板：\u200B请\u200B重试，\u200B或\u200B打开\u200B文件夹。"},
    {u8"Où est Mewgenics.exe ?", u8"Mewgenics.exe\u200B在\u200B哪里？"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"这\u200B台\u200B电脑\u200B无法\u200B使用\u200BDirect2D：\u200BMewgenics Multi\u200B无法\u200B显示。"},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"显卡\u200B初始化\u200B失败：\u200BMewgenics Multi\u200B无法\u200B显示。"},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"各有各的\u200B家\u200B和\u200B猫咪，\n"
     u8"一起\u200B去\u200B冒险！"},
    {u8"(illustrations du jeu introuvables)", u8"（找不到\u200B游戏\u200B插图）"},
    {u8"Comme le jeu (%s)", u8"跟随\u200B游戏\u200B（%s）"},
    {u8"Comme Windows (%s)", u8"跟随\u200BWindows\u200B（%s）"},
    {u8"Automatique (%s)", u8"自动（%s）"},
    {u8"Lancement annulé.", u8"启动已取消。"},
    {u8"Lancement du mod…", u8"正在\u200B启动\u200B模组……"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windows\u200B无法\u200B准备\u200B启动。\u200B请\u200B重新\u200B打开\u200BMewgenics Multi。"},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exe\u200B无法\u200B启动：\u200B从\u200Bzip\u200B里\u200B重新\u200B解压\u200B模组\u200B文件夹\u200B（也\u200B可能\u200B被\u200B杀毒软件\u200B隔离\u200B了）。"},
    {u8"la connexion Steam avec l'hôte est coupée", u8"与\u200B房主\u200B的\u200BSteam\u200B连接\u200B已\u200B断开"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"房主\u200B没有\u200B通过\u200BSteam\u200B回应\u200B（房间\u200B已\u200B关闭，\u200B或\u200B没\u200B装\u200B模组？）"},
    {u8"Steam refuse la connexion", u8"Steam\u200B拒绝\u200B了\u200B连接"},
    {u8"À %s de choisir", u8"轮到%s选择"},
    {u8"À toi de choisir", u8"轮到你选择"},
    {u8"À %s de choisir la capacité à remplacer.", u8"轮到\u200B%s\u200B选择\u200B要\u200B替换\u200B的\u200B能力。"},
    {u8"Ce chat est à %s", u8"这只猫属于%s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"选择\u200B项圈\n"
     u8"\n"
     u8"共同\u200B选择\u200B中断\u200B了\u200B（游戏\u200B出错）：\n"
     u8"由\u200B你\u200B为\u200B整个\u200B队伍\u200B选择\u200B项圈。"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"选择\u200B项圈\n"
     u8"\n"
     u8"准备\u200B好\u200B了！\u200B%s\u200B知道\u200B你\u200B选\u200B完\u200B了：\n"
     u8"他\u200B会\u200B为\u200B整个\u200B队伍\u200B确认\u200B出发。"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"选择\u200B项圈\n"
     u8"\n"
     u8"由\u200B%s\u200B确认\u200B出发：\n"
     u8"你\u200B的\u200B猫戴\u200B好\u200B项圈\u200B后\u200B告诉\u200B他。"},
    {u8"%s et %s", u8"%s和%s"},
    {u8"%s, %s", u8"%s、%s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"选择\u200B项圈\n"
     u8"\n"
     u8"%s\u200B还在\u200B给\u200B自己\u200B的\u200B猫\u200B选\u200B项圈。\n"
     u8"确定\u200B不\u200B等\u200B的话，\u200B再\u200B点\u200B一次\u200B“开冲！”\n"
     u8"直接\u200B出发。"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"选择\u200B项圈\n"
     u8"\n"
     u8"%s\u200B还在\u200B给\u200B自己\u200B的\u200B猫\u200B选\u200B项圈。\n"
     u8"确定\u200B不\u200B等\u200B的话，\u200B再\u200B点\u200B一次\u200B“开冲！”\n"
     u8"直接\u200B出发。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"这里\u200B不\u200B再\u200B显示\u200B%s\u200B的\u200B出发\u200B画面。\n"
     u8"自己\u200B完成\u200B这些\u200B画面：\u200B你\u200B会\u200B在\u200B地图\u200B上\n"
     u8"和\u200B他\u200B会合。"},
    {u8"  %s : parti", u8"  %s：已出发"},
    {u8"  %s : prêt !", u8"  %s：\u200B准备\u200B好\u200B了！"},
    {u8"  %s : charge ta partie...", u8"  %s：\u200B正在\u200B加载\u200B你\u200B的\u200B游戏……"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"队友\u200B正在\u200B加载\u200B你\u200B的\u200B游戏，\u200B好\u200B和\u200B你\u200B一起\u200B出发：\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"选择\u200B物品\n"
     u8"\n"
     u8"这里\u200B无法\u200B加载\u200B你\u200B的\u200B仓库：\n"
     u8"你\u200B的\u200B猫\u200B将\u200B带着\u200B身上\u200B的\u200B物品\u200B出发。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"选择\u200B物品\n"
     u8"\n"
     u8"%s\u200B还在\u200B给\u200B自己\u200B的\u200B猫\u200B装备\u200B物品。\n"
     u8"确定\u200B不\u200B等\u200B的话，\u200B再\u200B点\u200B一次\u200B“开冲！”\n"
     u8"直接\u200B出发。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"选择\u200B物品\n"
     u8"\n"
     u8"%s\u200B还在\u200B给\u200B自己\u200B的\u200B猫\u200B装备\u200B物品。\n"
     u8"确定\u200B不\u200B等\u200B的话，\u200B再\u200B点\u200B一次\u200B“开冲！”\n"
     u8"直接\u200B出发。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"选择\u200B物品\n"
     u8"\n"
     u8"准备\u200B好\u200B了！\u200B你\u200B的\u200B猫\u200B将\u200B带着\u200B这些\u200B物品\u200B出发：\n"
     u8"%s\u200B会\u200B为\u200B整个\u200B队伍\u200B确认\u200B出发。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"选择\u200B物品\n"
     u8"\n"
     u8"给\u200B你\u200B的\u200B猫\u200B选\u200B的\u200B物品\u200B没能\n"
     u8"带\u200B进\u200B%s\u200B的\u200B冒险：\u200B你\u200B的\u200B猫\n"
     u8"带着\u200B原来\u200B的\u200B物品\u200B出发，\u200B你\u200B的\u200B仓库\n"
     u8"不\u200B变。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"选择\u200B物品\n"
     u8"\n"
     u8"%s\u200B已经\u200B确认\u200B出发：\u200B你\u200B的\u200B猫\u200B将\u200B带着\n"
     u8"目前\u200B选\u200B好\u200B的\u200B物品\u200B出发，\u200B之后\u200B的\n"
     u8"更改\u200B不\u200B再\u200B算数。"},
    {u8"Ta bourse : %s.", u8"你\u200B的\u200B钱袋：\u200B%s。"},
    {u8"%s n'est plus dans la partie.", u8"%s\u200B已\u200B不\u200B在\u200B游戏\u200B中。"},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"你\u200B在\u200B这\u200B次\u200B冒险\u200B中\u200B没有\u200B钱袋\u200B（你\u200B没有\u200B带\u200B猫\u200B来）。"},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"交易\u200B可以\u200B在\u200B家里\u200B进行，\u200B或\u200B在\u200B多人\u200B冒险\u200B中\u200B用\u200B钱袋\u200B进行。\u200B目前：\u200B%s。"},
    {u8"pas de partie à plusieurs en cours", u8"当前\u200B没有\u200B进行\u200B中\u200B的\u200B多人游戏"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés", u8"缺少\u200B一\u200B项\u200B游戏\u200B功能，\u200B家中\u200B交易\u200B已\u200B关闭"},
    {u8"tu es dans la partie de l'hôte", u8"你\u200B在\u200B房主\u200B的\u200B游戏\u200B里"},
    {u8"tu n'es pas dans ta maison", u8"你\u200B不\u200B在\u200B自己\u200B家里"},
    {u8"ta save n'est pas chargée", u8"你\u200B的\u200B存档\u200B未\u200B加载"},
    {u8"un départ en aventure est en cours", u8"正在出发冒险"},
    {u8"ta maison est attaquée", u8"你的家正遭到攻击"},
    {u8"ta maison est illisible", u8"无法读取你的家"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"你\u200B的\u200B仓库\u200B在\u200B等\u200B你\u200B的\u200B猫\u200B从\u200B房主\u200B的\u200B冒险\u200B中\u200B带回\u200B的\u200B东西\u200B（如果\u200B它们\u200B已经\u200B回来，\u200B重新加载\u200B存档：\u200B按\u200BEsc，\u200B选\u200B“保存然后退到主菜单”，\u200B再\u200B选\u200B“开始游戏”\u200B和\u200B你\u200B的\u200B存档）"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"你\u200B仓库\u200B里\u200B有些\u200B物品\u200B还在\u200B等待\u200B新\u200B主人\u200B（战利品、\u200B购买、\u200BNPC）"},
    {u8"Pas d'échange maintenant : %s.", u8"现在\u200B无法\u200B交易：\u200B%s。"},
    {u8"Ton don précédent attend encore sa réponse.", u8"你\u200B上\u200B一次\u200B的\u200B赠送\u200B还在\u200B等待\u200B对方\u200B回应。"},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"模组\u200B还\u200B没有\u200B识别\u200B你\u200B的\u200B存档：\u200B从\u200B菜单\u200B重新加载\u200B它。"},
    {u8"Choisis un joueur de la partie.", u8"选择\u200B一\u200B名\u200B游戏\u200B中\u200B的\u200B玩家。"},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.", u8"现在\u200B无法\u200B交易：\u200B%s\u200B不\u200B在\u200B自己\u200B家里。"},
    {u8"Ce montant n'est pas possible.", u8"这个\u200B数量\u200B不行。"},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.", u8"选择\u200B要\u200B赠送\u200B的\u200B钱、\u200B食物\u200B或\u200B物品。"},
    {u8"16 objets au plus par don.", u8"每\u200B次\u200B最多\u200B赠送\u200B16件\u200B物品。"},
    {u8"Pas d'objets maintenant : %s.", u8"现在\u200B无法\u200B赠送\u200B物品：\u200B%s。"},
    {u8"Ta maison est illisible : pas de don.", u8"无法\u200B读取\u200B你\u200B的\u200B家：\u200B无法\u200B赠送。"},
    {u8"Tu n'as pas autant de pièces.", u8"你\u200B没有\u200B那么\u200B多\u200B钱。"},
    {u8"Tu n'as pas autant de nourriture.", u8"你\u200B没有\u200B那么\u200B多\u200B食物。"},
    {u8"Ton rangement est illisible : pas de don.", u8"无法\u200B读取\u200B你\u200B的\u200B仓库：\u200B无法\u200B赠送。"},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.", u8"选中\u200B的\u200B某\u200B件\u200B物品\u200B已\u200B用于\u200B一次\u200B尚未\u200B完成\u200B的\u200B赠送。"},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"选中\u200B的\u200B某\u200B件\u200B物品\u200B已\u200B不\u200B在\u200B你\u200B的\u200B仓库\u200B里：\u200B请\u200B重新\u200B选择。"},
    {u8"Un objet choisi ne peut pas se donner.", u8"选中\u200B的\u200B某\u200B件\u200B物品\u200B不\u200B能\u200B赠送。"},
    {u8"Le don n'a pas pu partir (connexion).", u8"赠送\u200B没能\u200B发出\u200B（连接\u200B问题）。"},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…", u8"已\u200B向\u200B%s\u200B赠送\u200B（%s）：\u200B等待\u200B对方\u200B回应……"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…", u8"已\u200B向\u200B房主\u200B申请\u200B赠送：\u200B等待\u200B回应……"},
    {u8"Pas de don : %s.", u8"无法\u200B赠送：\u200B%s。"},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%s\u200B已\u200B收到\u200B你\u200B的\u200B赠送：\u200B物品\u200B画面\u200B关闭\u200B后，\u200B你\u200B的\u200B物品\u200B就\u200B会\u200B离开\u200B你\u200B的\u200B仓库。"},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%s\u200B已\u200B收到\u200B你\u200B的\u200B赠送：\u200B你\u200B的\u200B物品会\u200B尽快\u200B离开\u200B你\u200B的\u200B仓库\u200B（在\u200B出发、\u200B守家\u200B或\u200B你\u200B的\u200B猫\u200B回来\u200B之后）。"},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%s\u200B已\u200B收到\u200B你\u200B的\u200B赠送：\u200B等\u200B你\u200B回到\u200B这个\u200B存档\u200B时，\u200B送出\u200B的\u200B东西\u200B就\u200B会\u200B离开\u200B你\u200B的\u200B家。"},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"%s\u200B没有\u200B回应\u200B你\u200B的\u200B赠送：\u200B你\u200B家里\u200B什么\u200B都\u200B没\u200B少。"},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"%s\u200B没有\u200B回应\u200B你\u200B的\u200B赠送：\u200B你\u200B家里\u200B什么\u200B都\u200B没\u200B少。"},
    {u8"Les échanges marchent pendant une partie à plusieurs.", u8"交易\u200B只能\u200B在\u200B多人游戏\u200B中\u200B进行。"},
    {u8"Dans ta maison : %s.", u8"家中\u200B现有：\u200B%s。"},
    {u8"%s n'est pas dans sa maison.", u8"%s\u200B不\u200B在\u200B自己\u200B家里。"},
    {u8"%lld pièces", u8"%lld块钱"},
    {u8"%lld pièce", u8"%lld块钱"},
    {u8"%lld nourritures", u8"%lld份食物"},
    {u8"%lld nourriture", u8"%lld份食物"},
    {u8"rien", u8"什么都没有"},
    {u8"%s n'est pas dans sa maison", u8"%s\u200B不\u200B在\u200B自己\u200B家里"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s", u8"%s\u200B那边\u200B还\u200B开着\u200B某个\u200B窗口\u200B或\u200B物品\u200B画面"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"%s\u200B的\u200B游戏\u200B没能\u200B把\u200B它们\u200B收\u200B进\u200B仓库"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"%s\u200B的\u200B游戏\u200B没能\u200B把\u200B它们\u200B收\u200B进\u200B仓库"},
    {u8"les dons ne marchent pas chez %s", u8"%s\u200B那边\u200B无法\u200B接收\u200B赠送"},
    {u8"%s n'est plus dans la partie", u8"%s\u200B已\u200B不\u200B在\u200B游戏\u200B中"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%s\u200B还\u200B有\u200B猫\u200B在\u200B房主\u200B的\u200B冒险\u200B中\u200B（他\u200B的\u200B仓库\u200B在\u200B等\u200B它们\u200B回来）"},
    {u8"raison inconnue", u8"原因不明"},
    {u8"Cadeau d'%s", u8"%s的礼物"},
    {u8"Cadeau de %s", u8"%s的礼物"},
    {u8"%s t'a donné %s.", u8"%s送给你%s。"},
    {u8"C'est dans ta maison, et ta save est à jour.", u8"东西\u200B已\u200B放进\u200B你\u200B家，\u200B存档\u200B也\u200B已\u200B更新。"},
    {u8"C'est dans ta maison.", u8"东西\u200B已\u200B放进\u200B你\u200B家。"},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.", u8"%s\u200B最终\u200B还是\u200B收到\u200B了\u200B%s：\u200B东西\u200B已\u200B离开\u200B你\u200B家。"},
    {u8"%s a bien reçu %s : c'est parti de ta maison.", u8"%s\u200B已\u200B收到\u200B%s：\u200B东西\u200B已\u200B离开\u200B你\u200B家。"},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"（有\u200B1件\u200B物品\u200B没能\u200B离开\u200B你\u200B的\u200B仓库：\u200B它\u200B也\u200B还在\u200B你\u200B这里。）"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"（有\u200B%u件\u200B物品\u200B没能\u200B离开\u200B你\u200B的\u200B仓库：\u200B它们\u200B也\u200B还在\u200B你\u200B这里。）"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"给\u200B%s\u200B的\u200B赠送\u200B没有\u200B成功：\u200B%s。\u200B你\u200B家里\u200B什么\u200B都\u200B没\u200B少。"},
    {u8"Rien à donner dans ton rangement.", u8"你\u200B的\u200B仓库\u200B里\u200B没有\u200B可\u200B赠送\u200B的\u200B东西。"},
    {u8"Tape des lettres pour chercher un objet.", u8"输入\u200B字母\u200B来\u200B搜索\u200B物品。"},
    {u8"Recherche : %s_", u8"搜索：%s_"},
    {u8"Aucun objet ne correspond.", u8"没有\u200B匹配\u200B的\u200B物品。"},
    {u8"(page %u sur %u)", u8"（第%u页，\u200B共\u200B%u页）"},
    {u8"Butin : %s.", u8"战利品：%s。"},
    {u8"Besoin", u8"需求"},
    {u8"Cupidité", u8"贪婪"},
    {u8"Passer", u8"跳过"},
    {u8"%u s", u8"%u秒"},
    {u8"toi", u8"你"},
    {u8"Joueur %u", u8"玩家%u"},
    {u8"Un objet", u8"一件物品"},
    {u8"Partage du butin : objet %u sur %u", u8"分配\u200B战利品：\u200B第%u件，\u200B共\u200B%u件"},
    {u8"Besoin : %s", u8"需求：%s"},
    {u8"Cupidité : %s", u8"贪婪：%s"},
    {u8"Passer : %s", u8"跳过：%s"},
    {u8"Partage du butin", u8"分配战利品"},
    {u8"%u objets du sac à partager entre %s.", u8"背包里\u200B的\u200B%u件\u200B物品\u200B将\u200B在\u200B%s\u200B之间\u200B分配。"},
    {u8"%u objet du sac à partager entre %s.", u8"背包里\u200B的\u200B%u件\u200B物品\u200B将\u200B在\u200B%s\u200B之间\u200B分配。"},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"每\u200B件\u200B物品\u200B可\u200B选：\u200B需求\u200B（对\u200B你\u200B有用）、\u200B贪婪\u200B（如果\n"
     u8"没\u200B人\u200B需要）\u200B或\u200B跳过。\u200B掷\u200B100面\u200B骰\u200B决定\u200B归属。"},
    {u8"En attente de %s...", u8"正在等待%s……"},
    {u8"En attente de %s... (%s)", u8"正在\u200B等待\u200B%s……\u200B（%s）"},
    {u8"Tout le monde est là : ça commence !", u8"大家\u200B都\u200B到齐\u200B了：\u200B开始！"},
    {u8"Commencer", u8"开始"},
    {u8"D'accord", u8"好"},
    {u8"Le veux-tu ? (%s)", u8"你\u200B想要\u200B吗？\u200B（%s）"},
    {u8"Je le veux", u8"我要"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"需求：\u200B对\u200B你\u200B有用，\u200B优先\u200B于\u200B其他\u200B人。\n"
     u8"贪婪：\u200B没\u200B人\u200B需要\u200B时\u200B才\u200B归\u200B你。\n"
     u8"（%s）"},
    {u8"Ton choix : %s.", u8"你\u200B的\u200B选择：\u200B%s。"},
    {u8"Ton choix est fait.", u8"你\u200B已\u200B做\u200B出\u200B选择。"},
    {u8"En attente de %s (%s).", u8"正在\u200B等待\u200B%s\u200B（%s）。"},
    {u8"Le résultat arrive...", u8"结果\u200B马上\u200B揭晓……"},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s\u200B不\u200B在，\u200B视为\u200B跳过。"},
    {u8"%s n'est pas là : son tour passe.", u8"%s\u200B不\u200B在，\u200B视为\u200B跳过。"},
    {u8"Personne n'en veut : il reste chez toi.", u8"没\u200B人\u200B想要：\u200B它\u200B留\u200B在\u200B你\u200B这里。"},
    {u8"Personne n'en veut : il reste chez %s.", u8"没\u200B人\u200B想要：\u200B它\u200B留\u200B在\u200B%s\u200B那\u200B里。"},
    {u8"Tu l'emportes (%s) !", u8"你\u200B得到\u200B了\u200B它\u200B（%s）！"},
    {u8"%s l'emporte (%s).", u8"%s\u200B得到\u200B了\u200B它\u200B（%s）。"},
    {u8"Dés : %s (égalité départagée).", u8"骰子：\u200B%s\u200B（平局\u200B已\u200B重掷）。"},
    {u8"Dés : %s.", u8"骰子：%s。"},
    {u8"Partage du butin terminé", u8"战利品\u200B分配\u200B完毕"},
    {u8"Tu remportes : %s.", u8"你获得：%s。"},
    {u8"%s remporte : %s.", u8"%s获得：%s。"},
    {u8"Remporté, mais resté chez %s : %s.", u8"已\u200B获得，\u200B但\u200B仍\u200B留\u200B在\u200B%s\u200B那\u200B里：\u200B%s。"},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"没\u200B人\u200B想要\u200B（留\u200B在\u200B你\u200B这里）：\u200B%s。"},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"没\u200B人\u200B想要\u200B（留\u200B在\u200B%s\u200B那\u200B里）：\u200B%s。"},
    {u8"Aucun objet n'a été partagé.", u8"没有\u200B分配\u200B任何\u200B物品。"},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"你\u200B的\u200B存档\u200B已经\u200B打开：\u200B下一\u200B个\u200B窗口\u200B会\u200B告诉\u200B你\n"
     u8"如何\u200B重新加载，\u200B拿到\u200B你\u200B的\u200B战利品。"},
    {u8"Ton butin t'attend dans ton rangement, chez toi.", u8"你\u200B的\u200B战利品\u200B在\u200B你\u200B家\u200B的\u200B仓库\u200B里\u200B等\u200B你。"},
    {u8"Ce que les autres remportent quitte ton rangement.", u8"其他\u200B人\u200B获得\u200B的\u200B物品会\u200B离开\u200B你\u200B的\u200B仓库。"},
    {u8"Beanies", u8"比尼斯博士"},
    {u8"Butch", u8"布奇"},
    {u8"Tink", u8"汀可"},
    {u8"Frank", u8"弗兰克"},
    {u8"Jack", u8"杰克"},
    {u8"Tracy", u8"特蕾茜"},
    {u8"l'orgue de barbarie", u8"脏器佬"},
    {u8"Steven", u8"史蒂文"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"奖励\u200B已\u200B就绪\n"
     u8"\n"
     u8"%s\u200B送\u200B给\u200B%s\u200B的\u200B猫\u200B填满\u200B了\n"
     u8"好感条。\u200B等\u200B你\u200B回到\u200B家，\u200B就\u200B会\u200B替\u200B他\n"
     u8"领取\u200B奖励：\u200B其中\u200B的\u200B钱、\n"
     u8"食物\u200B和\u200B物品\u200B归\u200B%s，\n"
     u8"解锁内容\u200B归\u200B所有人。"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"%s\u200B的\u200B奖励\n"
     u8"\n"
     u8"%s\u200B赠送\u200B所得\u200B的\u200B奖励\u200B没能\n"
     u8"发\u200B给\u200B他：\u200B奖励\u200B留\u200B在\u200B你\u200B这里。"},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"给\u200B%s\u200B的\u200B奖励\n"
     u8"\n"
     u8"%s\u200B奖励\u200B了\u200B%s\u200B送来\u200B的\n"
     u8"猫：\u200B%s\u200B会\u200B送到\u200B他\u200B家。\n"
     u8"解锁内容\u200B归\u200B所有人。"},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"给\u200B所有人\u200B的\u200B奖励\n"
     u8"\n"
     u8"%s\u200B奖励\u200B了\u200B%s\u200B送来\u200B的\n"
     u8"猫：\u200B奖励\u200B的\u200B解锁内容\n"
     u8"归\u200B所有人。"},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s\u200B感谢\u200B你\u200B送\u200B的\u200B猫：\u200B%s\n"
     u8"（解锁内容\u200B归\u200B所有人）。"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"%s\u200B的\u200B奖励\n"
     u8"\n"
     u8"%s\u200B感谢\u200B你\u200B送\u200B的\u200B猫：\u200B%s。\n"
     u8"下次\u200B加载\u200B存档\u200B时\u200B（冒险\n"
     u8"回来\u200B后），\u200B这些\u200B会\u200B送到\n"
     u8"你\u200B家。\u200B解锁内容\u200B归\n"
     u8"所有人。"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"%s\u200B的\u200B奖励\n"
     u8"\n"
     u8"你\u200B的\u200B猫\u200B填满\u200B了\u200B%s\u200B的\u200B好感条：\n"
     u8"奖励\u200B是\u200B解锁内容，\u200B归\n"
     u8"所有人。\u200B下次\u200B加载\u200B存档\u200B时\n"
     u8"生效。"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"%s\u200B的\u200B奖励\n"
     u8"\n"
     u8"%s\u200B之前\u200B填满\u200B了\u200B%s\u200B的\u200B好感条，\n"
     u8"但\u200B已经\u200B离开\u200B了：\u200B奖励\u200B留\u200B在\u200B你\u200B这里。"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"奖励\u200B已\u200B就绪\n"
     u8"\n"
     u8"你\u200B的\u200B猫\u200B填满\u200B了\u200B%s\u200B的\u200B好感条！\n"
     u8"%s\u200B回到\u200B家\u200B后，\u200B就\u200B会\u200B替\u200B你\n"
     u8"领取\u200B奖励。"},
    {u8"les bourses ne marchent pas dans cette partie", u8"这\u200B局\u200B游戏\u200B不\u200B能\u200B使用\u200B钱袋"},
    {u8"rien à donner", u8"没有\u200B可\u200B送\u200B的\u200B东西"},
    {u8"c'est trop d'un coup", u8"一次送太多了"},
    {u8"choisis un autre joueur", u8"选择其他玩家"},
    {u8"ton don précédent attend encore la réponse de l'hôte", u8"你\u200B上\u200B一次\u200B的\u200B赠送\u200B还在\u200B等\u200B房主\u200B回应"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"你\u200B在\u200B这\u200B次\u200B冒险\u200B中\u200B没有\u200B钱袋"},
    {u8"envoi impossible (connexion)", u8"无法\u200B发送\u200B（连接\u200B问题）"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"冒险\u200B结束\n"
     u8"\n"
     u8"每个\u200B人\u200B把\u200B自己\u200B的\u200B钱袋\u200B带\u200B回家："},
    {u8"%s n'est plus là : sa bourse reste chez toi.", u8"%s\u200B已\u200B不\u200B在：\u200B他\u200B的\u200B钱袋\u200B留\u200B在\u200B你\u200B这里。"},
    {u8"%s : rien.", u8"%s：\u200B什么\u200B都\u200B没有。"},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.", u8"%s\u200B没能\u200B收到\u200B钱袋：\u200B钱袋\u200B留\u200B在\u200B你\u200B这里。"},
    {u8"%s : %s.", u8"%s：%s。"},
    {u8"Toi : %s.", u8"你：%s。"},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"房主\u200B还\u200B没\u200B回应\u200B你\u200B的\u200B赠送：\u200B在\u200B路线\u200B窗口\u200B里\u200B查看\u200B你\u200B的\u200B钱袋。"},
    {u8"Ton don n'est pas passé : %s.", u8"你\u200B的\u200B赠送\u200B没有\u200B成功：\u200B%s。"},
    {u8"(et d'autres trouvailles)", u8"（还\u200B有\u200B其他\u200B收获）"},
    {u8"pas de bourse pour ce joueur dans cette aventure", u8"这\u200B名\u200B玩家\u200B在\u200B这\u200B次\u200B冒险\u200B中\u200B没有\u200B钱袋"},
    {u8"ta bourse n'en a pas autant", u8"你\u200B的\u200B钱袋\u200B里\u200B没有\u200B那么\u200B多"},
    {u8"l'aventure n'est plus suivie", u8"这\u200B次\u200B冒险\u200B已\u200B不再\u200B同步"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s\u200B送\u200B给\u200B你\u200B%s：\u200B已\u200B放进\u200B你\u200B的\u200B钱袋。"},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"你\u200B把\u200B%s\u200B送\u200B给\u200B了\u200B%s：\u200B已\u200B放进\u200B对方\u200B的\u200B钱袋。"},
    {u8"%s a donné %s à %s.", u8"%s\u200B把\u200B%s\u200B送\u200B给\u200B了\u200B%s。"},
    {u8"Un chat", u8"一只猫"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.", u8"%s\u200B找到\u200B了\u200B%s，\u200B失去\u200B了\u200B%s：\u200B计\u200B入\u200B你\u200B的\u200B钱袋。"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s\u200B找到\u200B了\u200B%s，\u200B失去\u200B了\u200B%s：\u200B计\u200B入\u200B%s\u200B的\u200B钱袋。"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s\u200B找到\u200B了\u200B%s，\u200B失去\u200B了\u200B%s：\u200B计\u200B入\u200B%s\u200B的\u200B钱袋。"},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s\u200B找到\u200B了\u200B%s：\u200B归\u200B你\u200B的\u200B钱袋。"},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s\u200B找到\u200B了\u200B%s：\u200B归\u200B%s\u200B的\u200B钱袋。"},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s\u200B找到\u200B了\u200B%s：\u200B归\u200B%s\u200B的\u200B钱袋。"},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s\u200B花掉\u200B了\u200B%s：\u200B从\u200B你\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.", u8"%s\u200B花掉\u200B了\u200B%s：\u200B从\u200B%s\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.", u8"%s\u200B花掉\u200B了\u200B%s：\u200B从\u200B%s\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s\u200B失去\u200B了\u200B%s：\u200B从\u200B你\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%s\u200B失去\u200B了\u200B%s：\u200B从\u200B%s\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%s\u200B失去\u200B了\u200B%s：\u200B从\u200B%s\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"%s n'a rien trouvé.", u8"%s\u200B什么\u200B都\u200B没\u200B找到。"},
    {u8"un objet", u8"一件物品"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.", u8"你\u200B买\u200B了\u200B%s，\u200B花费\u200B%s：\u200B从\u200B你\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.", u8"%s\u200B买\u200B了\u200B%s，\u200B花费\u200B%s：\u200B从\u200B他\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"你\u200B买\u200B了\u200B%s：\u200B你\u200B的\u200B钱袋\u200B增加\u200B了\u200B%s。"},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%s\u200B买\u200B了\u200B%s：\u200B他\u200B的\u200B钱袋\u200B增加\u200B了\u200B%s。"},
    {u8"Tu as acheté %s (gratuit).", u8"你\u200B买\u200B了\u200B%s\u200B（免费）。"},
    {u8"%s a acheté %s (gratuit).", u8"%s\u200B买\u200B了\u200B%s\u200B（免费）。"},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"战斗\u200B中，\u200B你\u200B的\u200B猫\u200B找到\u200B了\u200B%s，\u200B失去\u200B了\u200B%s：\u200B计\u200B入\u200B你\u200B的\u200B钱袋。"},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.", u8"战斗\u200B中，\u200B你\u200B的\u200B猫\u200B找到\u200B了\u200B%s：\u200B归\u200B你\u200B的\u200B钱袋。"},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"战斗\u200B中，\u200B你\u200B的\u200B猫\u200B失去\u200B了\u200B%s：\u200B从\u200B你\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"战斗\u200B中，\u200B你\u200B的\u200B猫\u200B什么\u200B都\u200B没\u200B找到。"},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B找到\u200B了\u200B%s，\u200B失去\u200B了\u200B%s：\u200B计\u200B入\u200B他\u200B的\u200B钱袋。"},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B找到\u200B了\u200B%s，\u200B失去\u200B了\u200B%s：\u200B计\u200B入\u200B他\u200B的\u200B钱袋。"},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B找到\u200B了\u200B%s：\u200B归\u200B他\u200B的\u200B钱袋。"},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B找到\u200B了\u200B%s：\u200B归\u200B他\u200B的\u200B钱袋。"},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B失去\u200B了\u200B%s：\u200B从\u200B他\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B失去\u200B了\u200B%s：\u200B从\u200B他\u200B的\u200B钱袋\u200B里\u200B扣除。"},
    {u8"En combat, les chats d'%s n'ont rien trouvé.", u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B什么\u200B都\u200B没\u200B找到。"},
    {u8"En combat, les chats de %s n'ont rien trouvé.", u8"战斗\u200B中，\u200B%s\u200B的\u200B猫\u200B什么\u200B都\u200B没\u200B找到。"},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"背包\u200B已\u200B打开\n"
     u8"\n"
     u8"房主\u200B进入\u200B了\u200B一\u200B个\u200B地点：\n"
     u8"关上\u200B背包\u200B才\u200B能\u200B跟上\u200B他。"},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"物品\n"
     u8"\n"
     u8"这\u200B只猫\u200B属于\u200B%s：\u200B只有\u200B它\u200B的\u200B主人\n"
     u8"才\u200B能\u200B更换\u200B它\u200B的\u200B物品。"},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"物品\n"
     u8"\n"
     u8"身处\u200B某个\u200B地点\u200B时\u200B（商店、\u200B升级\n"
     u8"等），\u200B只有\u200B房主\u200B能\u200B更换\u200B物品。\n"
     u8"请\u200B在\u200B地图\u200B上\u200B给\u200B你\u200B的\u200B猫\u200B装备。"},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"物品\n"
     u8"\n"
     u8"房主\u200B还\u200B没\u200B回到\u200B地图\u200B上：\n"
     u8"等\u200B他\u200B回来，\u200B你\u200B的\u200B背包会\u200B自动\u200B解锁。"},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"物品\n"
     u8"\n"
     u8"你\u200B的\u200B背包\u200B正在\u200B与\u200B房主\u200B的\u200B同步：\n"
     u8"先\u200B关上，\u200B稍后\u200B再\u200B打开。"},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"物品\n"
     u8"\n"
     u8"另\u200B一\u200B名\u200B玩家\u200B抢先\u200B拿走\u200B了\u200B这\u200B件\u200B物品：\n"
     u8"你\u200B的\u200B更改\u200B无效。\u200B关上\u200B背包\u200B后，\n"
     u8"它\u200B会\u200B恢复\u200B成\u200B房主\u200B那边\u200B的\u200B样子。"},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"物品\n"
     u8"\n"
     u8"你\u200B最后\u200B的\u200B更改\u200B送达\u200B前，\u200B房主\n"
     u8"已\u200B进入\u200B一\u200B个\u200B地点：\u200B更改\u200B已\u200B取消。\n"
     u8"请\u200B在\u200B地图\u200B上\u200B重新\u200B操作。"},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"物品\n"
     u8"\n"
     u8"这\u200B件\u200B物品\u200B已经\u200B不\u200B在\u200B了：\u200B另\u200B一\u200B名\u200B玩家\n"
     u8"抢先\u200B拿走\u200B了\u200B它。"},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).", u8"%s\u200B已\u200B不\u200B在：\u200B他\u200B买\u200B的\u200B东西\u200B留\u200B在\u200B你\u200B这里\u200B（%s）。"},
    {u8"%s rapporte ses achats : %s.", u8"%s\u200B带回\u200B了\u200B他\u200B买\u200B的\u200B东西：\u200B%s。"},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%s\u200B买\u200B的\u200B东西\u200B没能\u200B发\u200B给\u200B他：\u200B留\u200B在\u200B你\u200B这里。"},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%s\u200B买\u200B的\u200B东西\u200B没能\u200B发\u200B给\u200B他：\u200B留\u200B在\u200B你\u200B这里。"},
    {u8"Achats de l'aventure", u8"冒险\u200B中\u200B买\u200B的\u200B东西"},
    {u8"Boutique", u8"商店"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s\u200B还\u200B没\u200B在\u200B这\u200B家\n"
     u8"商店\u200B买\u200B完：\u200B等等\u200B他们\u200B（你\u200B一旦\u200B离开，\n"
     u8"就\u200B没法\u200B再\u200B替\u200B他们\u200B购买\u200B了）。\n"
     u8"再\u200B点\u200B一次\u200B即可\u200B直接\u200B离开。"},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s\u200B还\u200B没\u200B在\u200B这\u200B家\n"
     u8"商店\u200B买\u200B完：\u200B等等\u200B他\u200B（你\u200B一旦\u200B离开，\n"
     u8"就\u200B没法\u200B再\u200B替\u200B他\u200B购买\u200B了）。\n"
     u8"再\u200B点\u200B一次\u200B即可\u200B直接\u200B离开。"},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"无法\u200B向\u200B房主\u200B发出\u200B购买\u200B请求：\n"
     u8"什么\u200B都\u200B没\u200B买。\u200B请\u200B稍后\u200B再\u200B试。"},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"房主\u200B已\u200B不\u200B在\u200B这\u200B家\u200B商店：\n"
     u8"购买\u200B没有\u200B完成。"},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"房主\u200B那边\u200B已\u200B不\u200B再\u200B出售\u200B这\u200B件\u200B物品：\n"
     u8"购买\u200B没有\u200B完成。"},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"这\u200B件\u200B物品\u200B刚刚\u200B被\u200B卖掉\u200B了：\n"
     u8"购买\u200B没有\u200B完成。"},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"价格\u200B是\u200B%s；\u200B你\u200B的\u200B钱袋\u200B里\u200B只有\u200B%lld块钱。"},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"你\u200B的\u200B钱袋\u200B不够\u200B买\u200B这\u200B件\u200B物品。"},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"（每个\u200B人\u200B都\u200B用\u200B自己\u200B的\u200B钱袋\u200B付钱，\u200B你\u200B也\u200B一样。）"},
    {u8"(Chacun paie avec sa bourse.)", u8"（每个\u200B人\u200B都\u200B用\u200B自己\u200B的\u200B钱袋\u200B付钱。）"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"目前\u200B只有\u200B房主\u200B能\u200B购买\n"
     u8"糖果\u200B和\u200B跟班\u200B（用\u200B他\u200B的\u200B钱袋）。"},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.", u8"游戏\u200B没能\u200B在\u200B房主\u200B那边\u200B完成\u200B购买。"},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"这\u200B次\u200B冒险\u200B没有\u200B钱袋：\n"
     u8"只有\u200B房主\u200B能\u200B购买。"},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"你\u200B没有\u200B带\u200B猫\u200B参加\u200B这\u200B次\n"
     u8"冒险：\u200B没有\u200B钱袋\u200B可以\n"
     u8"购物。"},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"你\u200B在\u200B这\u200B次\u200B冒险\u200B中\u200B没有\u200B钱袋\n"
     u8"（你\u200B没有\u200B带\u200B猫\u200B来）：\u200B无法\u200B购买。"},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"房主\u200B现在\u200B无法\u200B替\u200B你\n"
     u8"购买\u200B（他\u200B那边\u200B开着\u200B一\u200B个\u200B窗口）：\n"
     u8"什么\u200B都\u200B没\u200B买。\u200B请\u200B稍后\u200B再\u200B试。"},
    {u8"un bonbon", u8"一块糖果"},
    {u8"un familier", u8"一个跟班"},
    {u8"un article", u8"一件商品"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"冒险\u200B结束\n"
     u8"\n"
     u8"%s\u200B回到\u200B了\u200B%s\u200B的\u200B家。"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"冒险\u200B结束\n"
     u8"\n"
     u8"%s\u200B回到\u200B了\u200B%s\u200B的\u200B家。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"其他\u200B玩家\u200B的\u200B猫\u200B没能\u200B出发，\n"
     u8"而\u200B你\u200B的\u200B冒险箱\u200B是\u200B空\u200B的：\u200B这\u200B次\n"
     u8"无法\u200B冒险。\u200B（原因\u200B见\u200B模组\u200B日志\u200Bmgmp.log。）"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"你\u200B的\u200B冒险箱\u200B是\u200B空\u200B的，\u200B也\u200B没\u200B人\u200B带\n"
     u8"猫：\u200B这\u200B次\u200B无法\u200B冒险。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"混合队伍\n"
     u8"\n"
     u8"其他\u200B玩家\u200B的\u200B猫\u200B没能\n"
     u8"加入\u200B队伍：\u200B你\u200B带着\u200B自己\u200B的\u200B猫\u200B出发。\n"
     u8"（原因\u200B见\u200B模组\u200B日志\u200Bmgmp.log。）"},
    {u8"Retour de l'aventure", u8"冒险归来"},
    {u8"%s est de retour à la maison.", u8"%s回家了。"},
    {u8"%s sont de retour à la maison.", u8"%s回家了。"},
    {u8"%s a péri pendant l'aventure.", u8"%s\u200B在\u200B冒险\u200B中\u200B死掉\u200B了。"},
    {u8"%s ont péri pendant l'aventure.", u8"%s\u200B在\u200B冒险\u200B中\u200B死掉\u200B了。"},
    {u8"%s ne revient pas de l'aventure.", u8"%s\u200B没有\u200B从\u200B冒险\u200B中\u200B回来。"},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s\u200B没有\u200B从\u200B冒险\u200B中\u200B回来。"},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s\u200B在\u200B出发\u200B后\u200B有\u200B了\u200B变化：\n"
     u8"冒险\u200B归来\u200B的\u200B结果\u200B没有\u200B同步\u200B过来。"},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s\u200B在\u200B出发\u200B后\u200B有\u200B了\u200B变化：\n"
     u8"他们\u200B冒险\u200B归来\u200B的\u200B结果\u200B没有\u200B同步\u200B过来。"},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"你\u200B的\u200B仓库\u200B在\u200B出发\u200B后\u200B有\u200B了\u200B变化：\n"
     u8"出发\u200B时\u200B装备\u200B的\u200B物品\u200B没有\u200B从\u200B仓库\u200B中\u200B移除。"},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"你\u200B的\u200B仓库\u200B没有\u200B被\u200B修改：\u200B出发\u200B时\n"
     u8"装备\u200B的\u200B物品\u200B还在\u200B里面。"},
    {u8"Tu rapportes %s dans ta maison.", u8"你\u200B把\u200B%s\u200B带回\u200B了\u200B家。"},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.", u8"NPC\u200B感谢\u200B你\u200B送\u200B的\u200B猫：\u200B%s\u200B已\u200B送到\u200B你\u200B家。"},
    {u8"Cadeaux des PNJ", u8"NPC的礼物"},
    {u8"Ton butin est dans ton rangement : %s.", u8"你\u200B的\u200B战利品\u200B已\u200B放进\u200B仓库：\u200B%s。"},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.", u8"NPC\u200B的\u200B礼物\u200B已\u200B放进\u200B你\u200B的\u200B仓库：\u200B%s。"},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.", u8"冒险\u200B中\u200B招募\u200B的\u200B%s\u200B来\u200B到\u200B了\u200B你\u200B家。"},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.", u8"冒险\u200B中\u200B招募\u200B的\u200B%s\u200B来\u200B到\u200B了\u200B你\u200B家。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"混合队伍\n"
     u8"\n"
     u8"%s\u200B没能\u200B加入\u200B队伍：\n"
     u8"这\u200B次\u200B冒险\u200B中，\u200B你\u200B来\u200B操控\u200B队伍\u200B里\u200B的\n"
     u8"一部分\u200B猫。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"混合队伍\n"
     u8"\n"
     u8"%s\u200B没能\u200B加入\u200B队伍：\n"
     u8"这\u200B次\u200B冒险\u200B中，\u200B你\u200B来\u200B操控\u200B队伍\u200B里\u200B的\n"
     u8"一部分\u200B猫。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"混合队伍\n"
     u8"\n"
     u8"%s\u200B没能\u200B加入\u200B队伍：\n"
     u8"你\u200B会\u200B随队\u200B冒险，\u200B但\u200B不\u200B操控\u200B任何\u200B猫。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"混合队伍\n"
     u8"\n"
     u8"%s\u200B没能\u200B加入\u200B队伍：\n"
     u8"你\u200B会\u200B随队\u200B冒险，\u200B但\u200B不\u200B操控\u200B任何\u200B猫。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"混合队伍\n"
     u8"\n"
     u8"%s\u200B没能\u200B出发，\u200B留\u200B在\u200B了\u200B家里。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"混合队伍\n"
     u8"\n"
     u8"%s\u200B没能\u200B出发，\u200B留\u200B在\u200B了\u200B家里。"},
    {u8"Tes chats sont rentrés de l'aventure.", u8"你\u200B的\u200B猫\u200B从\u200B冒险\u200B中\u200B回来\u200B了。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"队伍\u200B里来\u200B了\u200B新猫\n"
     u8"\n"
     u8"%s\u200B加入\u200B了\u200B%s\u200B的\u200B队伍：\u200B这\u200B只猫\n"
     u8"归\u200B你！\u200B你\u200B不\u200B在\u200B的\u200B时候，\u200B战斗\u200B中\u200B由\u200B游戏\u200BAI\n"
     u8"操控\u200B它，\u200B%s\u200B替\u200B它\u200B做\u200B选择，\n"
     u8"冒险\u200B结束\u200B后\u200B它\u200B会\u200B回到\u200B你\u200B家。"},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"新猫：\u200B%s\u200B来\u200B到\u200B你\u200B家。"},
    {u8"Nouveaux chats : %s arrive chez toi.", u8"新猫：\u200B%s\u200B来\u200B到\u200B你\u200B家。"},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"第4个\u200B位置\n"
     u8"\n"
     u8"3人\u200B游戏\u200B时，\u200B队伍\u200B有\u200B一\u200B个\u200B空位：\n"
     u8"它\u200B归\u200B你\u200B了，\u200B你\u200B可以\u200B带\u200B2只猫。"},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"第4个\u200B位置\n"
     u8"\n"
     u8"空位\u200B已经\u200B被\u200B%s\u200B占\u200B了：\n"
     u8"3人\u200B游戏\u200B时，\u200B你\u200B只能\u200B带\u200B1只猫。\n"
     u8"从\u200B你\u200B的\u200B冒险箱\u200B里\u200B拿出\u200B一\u200B只猫。"},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"第4个\u200B位置\n"
     u8"\n"
     u8"第四\u200B名\u200B玩家\u200B来\u200B了：\u200B每人\u200B只能\u200B带\n"
     u8"1只猫。\u200B从\u200B你\u200B的\u200B冒险箱\u200B里\u200B拿出\u200B一\u200B只猫。"},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"第4个\u200B位置\u200B归\u200B第一\u200B个\u200B往\u200B冒险箱\u200B里\u200B放\u200B第2只猫\u200B的\u200B人。"},
    {u8"La 4e place est à %s.", u8"第4个\u200B位置\u200B归\u200B%s。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混合队伍\n"
     u8"\n"
     u8"%d人\u200B游戏\u200B时，\u200B你\u200B最多\u200B带\u200B%d只猫：\n"
     u8"从\u200B你\u200B的\u200B冒险箱\u200B里\u200B拿出\u200B%d只猫。\u200B%s\n"
     u8"其他\u200B玩家\u200B会\u200B用\u200B他们\u200B的\u200B猫\u200B补满\u200B队伍。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混合队伍\n"
     u8"\n"
     u8"%d人\u200B游戏\u200B时，\u200B你\u200B最多\u200B带\u200B%d只猫：\n"
     u8"从\u200B你\u200B的\u200B冒险箱\u200B里\u200B拿出\u200B%d只猫。\u200B%s\n"
     u8"其他\u200B玩家\u200B会\u200B用\u200B他们\u200B的\u200B猫\u200B补满\u200B队伍。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混合队伍\n"
     u8"\n"
     u8"%d人\u200B游戏\u200B时，\u200B你\u200B最多\u200B带\u200B%d只猫：\n"
     u8"从\u200B你\u200B的\u200B冒险箱\u200B里\u200B拿出\u200B%d只猫。\u200B%s\n"
     u8"其他\u200B玩家\u200B会\u200B用\u200B他们\u200B的\u200B猫\u200B补满\u200B队伍。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混合队伍\n"
     u8"\n"
     u8"%d人\u200B游戏\u200B时，\u200B你\u200B最多\u200B带\u200B%d只猫：\n"
     u8"从\u200B你\u200B的\u200B冒险箱\u200B里\u200B拿出\u200B%d只猫。\u200B%s\n"
     u8"其他\u200B玩家\u200B会\u200B用\u200B他们\u200B的\u200B猫\u200B补满\u200B队伍。"},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"你\u200B不\u200B在\u200B自己\u200B家里：\n"
     u8"你\u200B将\u200B使用\u200B其他\u200B人\u200B的\u200B猫。"},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"你\u200B的\u200B冒险箱\u200B还\u200B没\u200B解锁：\n"
     u8"你\u200B将\u200B使用\u200B其他\u200B人\u200B的\u200B猫。"},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"你\u200B的\u200B冒险箱\u200B是\u200B空\u200B的：\u200B你\u200B将\u200B使用\u200B其他\u200B人\u200B的\u200B猫。\n"
     u8"想\u200B带\u200B自己\u200B的\u200B猫：\u200B选\u200B“还没好”，\u200B往\u200B冒险箱\u200B里\u200B放\u200B1到\u200B2只\n"
     u8"（在\u200B布奇\u200B那\u200B里），\u200B再\u200B点\u200B你\u200B的\u200B“出发！”\u200B牌子。"},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"你\u200B的\u200B冒险箱\u200B是\u200B空\u200B的：\u200B你\u200B将\u200B使用\u200B其他\u200B人\u200B的\u200B猫。\n"
     u8"想\u200B带\u200B自己\u200B的\u200B猫：\u200B选\u200B“还没好”，\u200B往\u200B冒险箱\u200B里\u200B放\u200B1只\n"
     u8"（在\u200B布奇\u200B那\u200B里），\u200B再\u200B点\u200B你\u200B的\u200B“出发！”\u200B牌子。"},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"你\u200B的\u200B冒险箱\u200B里\u200B有\u200B%d只猫，\u200B最多\u200B只能\u200B带\u200B%d只：\n"
     u8"选\u200B“还没好”，\u200B拿出\u200B%d只，\n"
     u8"再\u200B点\u200B你\u200B的\u200B“出发！”\u200B牌子。\u200B%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).", u8"你\u200B的\u200B猫\u200B从\u200B你\u200B的\u200B冒险箱\u200B出发\u200B（在\u200B布奇\u200B那\u200B里）。"},
    {u8"Tu emmènes %s.", u8"你带上了%s。"},
    {u8"Départ en aventure", u8"出发冒险"},
    {u8"La session n'est pas prête : réessaie dans un instant.", u8"会话\u200B还\u200B没\u200B准备\u200B好：\u200B请\u200B稍后\u200B再\u200B试。"},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"正在\u200B把\u200B第4个\u200B位置\u200B分配\u200B给\u200B你：\n"
     u8"稍后\u200B会\u200B再\u200B问\u200B你\u200B一次。"},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"你\u200B的\u200B猫\u200B没能\u200B做好\n"
     u8"冒险\u200B准备\u200B（见\u200Bmgmp.log）。"},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s\u200B刚\u200B从\u200B冒险\u200B中\u200B回来，\u200B但\u200B结果\n"
     u8"还\u200B没\u200B同步\u200B到\u200B你\u200B的\u200B存档：\u200B让\u200B它们\u200B再\u200B出发\u200B前\n"
     u8"先\u200B重新加载\u200B存档\u200B（这\u200B次\u200B出发\u200B后\u200B会\u200B出现\u200B“重新加载”\n"
     u8"选项；\u200B也\u200B可以\u200B按\u200BEsc，\u200B选\u200B“保存然后退到主菜单”，\n"
     u8"再\u200B选\u200B“开始游戏”\u200B和\u200B你\u200B的\u200B存档）。"},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s\u200B刚\u200B从\u200B冒险\u200B中\u200B回来，\u200B但\u200B结果\n"
     u8"还\u200B没\u200B同步\u200B到\u200B你\u200B的\u200B存档：\u200B让\u200B它\u200B再\u200B出发\u200B前\n"
     u8"先\u200B重新加载\u200B存档\u200B（这\u200B次\u200B出发\u200B后\u200B会\u200B出现\u200B“重新加载”\n"
     u8"选项；\u200B也\u200B可以\u200B按\u200BEsc，\u200B选\u200B“保存然后退到主菜单”，\n"
     u8"再\u200B选\u200B“开始游戏”\u200B和\u200B你\u200B的\u200B存档）。"},
    {u8"Le retour de %s attend d'être recopié.", u8"%s\u200B的\u200B冒险\u200B结果\u200B还在\u200B等待\u200B同步。"},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"你\u200B的\u200B回复\u200B没能\u200B发\u200B给\u200B房主：\n"
     u8"请\u200B重试。"},
    {u8"Ta bourse de l'aventure : %s.", u8"你\u200B这\u200B次\u200B冒险\u200B的\u200B钱袋：\u200B%s。"},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"你\u200B的\u200B冒险箱\u200B里\u200B有\u200B%d只猫，\u200B最多\u200B只能\u200B带\u200B%d只：\n"
     u8"拿出\u200B%d只，\u200B再\u200B点\u200B你\u200B的\u200B“出发！”\u200B牌子。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"队伍\u200B里来\u200B了\u200B新猫\n"
     u8"\n"
     u8"%s\u200B加入\u200B了\u200B队伍：\u200B这\u200B只猫\u200B归\u200B你！\n"
     u8"由\u200B你\u200B来\u200B操控\u200B它，\u200B冒险\u200B结束\u200B后\n"
     u8"它\u200B会\u200B回到\u200B你\u200B家。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"队伍\u200B里来\u200B了\u200B新猫\n"
     u8"\n"
     u8"%s\u200B加入\u200B了\u200B队伍：\u200B这\u200B只猫\u200B归\u200B你。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"队伍\u200B里来\u200B了\u200B新猫\n"
     u8"\n"
     u8"%s\u200B加入\u200B了\u200B队伍：\u200B这\u200B只猫\u200B归\u200B%s。"},
    {u8"Un nouveau chat", u8"一只新猫"},
    {u8"en bas", u8"下方"},
    {u8"en haut", u8"上方"},
    {u8"à droite", u8"右侧"},
    {u8"à gauche", u8"左侧"},
    {u8"au milieu", u8"中间"},
    {u8"n°%d", u8"%d号"},
    {u8"%s (%s)", u8"%s（%s）"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]你\u200B的\u200B投票\u200B无效：\u200B%s。[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"选择路线\n"
     u8"\n"},
    {u8"Même chemin", u8"走同一条路"},
    {u8"Choisir", u8"选择"},
    {u8"Changer", u8"更改"},
    {u8"Sortie", u8"出口"},
    {u8"Retour à la maison", u8"回家"},
    {u8"Combat", u8"战斗"},
    {u8"Combat difficile", u8"困难战斗"},
    {u8"Mini-boss", u8"小头目"},
    {u8"Boss", u8"头目"},
    {u8"Événement", u8"事件"},
    {u8"Événement spécial", u8"特殊事件"},
    {u8"Trésor", u8"宝藏"},
    {u8"Meuble", u8"家具"},
    {u8"Nourriture", u8"食物"},
    {u8"Bonus", u8"奖励"},
    {u8"Case vide", u8"空地点"},
    {u8"Case", u8"地点"},
    {u8"l'hôte n'est pas sur la carte", u8"房主\u200B不\u200B在\u200B地图\u200B上"},
    {u8"vos cartes ne sont pas les mêmes", u8"你们\u200B的\u200B地图\u200B不\u200B一样"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"房主\u200B无法\u200B前往\u200B这个\u200B地点"},
    {u8"le chemin est déjà choisi", u8"路线\u200B已经\u200B选\u200B好\u200B了"},
    {u8"ce tour est fini", u8"这一轮已结束"},
    {u8"l'hôte ne l'a pas pris", u8"房主没有采纳"},
    {u8"pas de vote", u8"未投票"},
    {u8"absent, ne vote pas", u8"不\u200B在，\u200B不\u200B参与\u200B投票"},
    {u8"pas encore sur la carte", u8"还没到地图上"},
    {u8"pas encore voté", u8"还没投票"},
    {u8"%s (toi) : %s", u8"%s（你）：%s"},
    {u8"%s : %s", u8"%s：%s"},
    {u8"En pause : on attend tout le monde sur la carte.", u8"暂停\u200B中：\u200B等待\u200B所有人\u200B到达\u200B地图。"},
    {u8"À toi de choisir : encore %u s.", u8"轮到\u200B你\u200B选择：\u200B还剩\u200B%u秒。"},
    {u8"En attente des autres : encore %u s.", u8"等待\u200B其他\u200B人：\u200B还剩\u200B%u秒。"},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"%s\u200B平票：\n"
     u8"随机\u200B选中\u200B了\u200B%s。"},
    {u8"Tout le monde a choisi %s.", u8"所有人\u200B都\u200B选\u200B了\u200B%s。"},
    {u8"%s l'emporte.", u8"%s胜出。"},
    {u8"%s : prêt", u8"%s：准备好了"},
    {u8"%s : défend sa maison", u8"%s：正在守家"},
    {u8"%s : pas encore", u8"%s：还没好"},
    {u8"Tout le monde est prêt !", u8"所有人\u200B都\u200B准备\u200B好\u200B了！"},
    {u8"En attente d'un joueur.", u8"正在\u200B等待\u200B1名\u200B玩家。"},
    {u8"En attente de %d joueurs.", u8"正在\u200B等待\u200B%d名\u200B玩家。"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"结束今天\n"
     u8"\n"
     u8"你\u200B已\u200B准备\u200B好\u200B过夜。\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"你\u200B的\u200B牌子\u200B现在\u200B用于\u200B出发\u200B冒险：\n"
     u8"把\u200B你\u200B的\u200B猫\u200B从\u200B队伍\u200B中\u200B撤下\u200B才\u200B能\u200B结束今天。"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"结束今天\n"
     u8"\n"
     u8"有\u200B1名\u200B玩家\u200B在\u200B等\u200B着\u200B过夜。\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"结束今天\n"
     u8"\n"
     u8"有\u200B%d名\u200B玩家\u200B在\u200B等\u200B着\u200B过夜。\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1名玩家在等"},
    {u8"%d joueurs attendent", u8"%d名玩家在等"},
    {u8"Passer la nuit", u8"过夜"},
    {u8"Plus tard", u8"以后再说"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"要\u200B继续，\u200B请\u200B按\u200BEsc，\n"
     u8"然后\u200B选\u200B“保存然后退到主菜单”。"},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"等着\u200B你\u200B的\u200B东西\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"你\u200B的\u200B存档\u200B已经\u200B打开：\u200B这些\u200B要\u200B等\n"
     u8"下次\u200B加载\u200B存档\u200B时\u200B才\u200B会\u200B出现。\u200B选\u200B“重新加载”：\n"
     u8"模组\u200B会\u200B先\u200B回到\u200B主菜单，\u200B再\u200B重新\u200B打开\u200B存档。"},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"等\u200B着\u200B你\u200B的\u200B东西\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"你\u200B的\u200B存档\u200B已经\u200B打开：\u200B这些\u200B要\u200B等\n"
     u8"下次\u200B加载\u200B存档\u200B时\u200B才\u200B会\u200B出现。\u200B重新加载\u200B它：\n"
     u8"按\u200BEsc，\u200B选\u200B“保存然后退到主菜单”，\n"
     u8"再\u200B选\u200B“开始游戏”\u200B和\u200B你\u200B的\u200B存档。"},
    {u8"%s : j'arrive !", u8"%s：我来了！"},
    {u8"%s : réfléchit...", u8"%s：考虑中……"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"你\u200B想\u200B出发\u200B去\u200B冒险。\n"
     u8"队友\u200B们\u200B的\u200B回答：\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s：还没到"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"等待\u200B你\u200B的\u200B队友\n"
     u8"\n"
     u8"%s\n"
     u8"如果\u200B你\u200B不\u200B等\u200B就\u200B先\u200B走，\u200B在\u200B他们\u200B到达\u200B之前，\n"
     u8"游戏\u200BAI\u200B会\u200B在\u200B战斗\u200B中\u200B操控\n"
     u8"他们\u200B的\u200B猫。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"%s\u200B要\u200B出发\u200B去\u200B冒险\u200B了！\n"
     u8"每个\u200B人\u200B带上\u200B并\u200B操控\u200B自己\u200B的\u200B猫。\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"冒险\u200B进行\u200B中\n"
     u8"\n"
     u8"%s\u200B正在\u200B冒险！\n"
     u8"你\u200B的\u200B猫\u200B没有\u200B同行：\u200B你\u200B会\u200B随队\n"
     u8"冒险，\u200B但\u200B不\u200B操控\u200B任何\u200B猫。"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"冒险\u200B进行\u200B中\n"
     u8"\n"
     u8"%s\u200B正\u200B带着\u200B你\u200B的\u200B猫\u200B冒险！\n"
     u8"你\u200B不\u200B在\u200B的\u200B时候，\u200B战斗\u200B中\u200B由\u200B游戏\u200BAI\u200B操控\u200B它们，\n"
     u8"%s\u200B替\u200B它们\u200B做\u200B选择。\n"
     u8"要\u200B加入\u200B他\u200B的\u200B队伍\u200B吗？"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"冒险\u200B进行\u200B中\n"
     u8"\n"
     u8"%s\u200B正在\u200B冒险！\n"
     u8"要\u200B加入\u200B他\u200B的\u200B队伍\u200B吗？"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"离开\u200B冒险\n"
     u8"\n"
     u8"%s\u200B的\u200B冒险\u200B已\u200B结束：\n"
     u8"你\u200B将\u200B回到\u200B自己\u200B家。"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"离开\u200B冒险\n"
     u8"\n"
     u8"这\u200B是\u200B%s\u200B的\u200B冒险：\u200B只有\u200B他\u200B能\n"
     u8"放弃。\u200B冒险\u200B会\u200B在\u200B没有\u200B你\u200B的\u200B情况\u200B下\u200B继续。\n"
     u8"\n"
     u8"要\u200B回\u200B自己\u200B家\u200B吗？\u200B想\u200B回来\u200B的话，\n"
     u8"点\u200B你\u200B家里\u200B的\u200B任意\u200B一\u200B个\u200B牌子。"},
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
     u8"离开\u200B冒险\n"
     u8"\n"
     u8"这\u200B是\u200B%s\u200B的\u200B冒险：\u200B只有\u200B他\u200B能\n"
     u8"放弃。\u200B冒险\u200B会\u200B在\u200B没有\u200B你\u200B的\u200B情况\u200B下\u200B继续，\u200B你\u200B的\n"
     u8"猫会\u200B留\u200B在\u200B他\u200B的\u200B队伍\u200B里：\u200B战斗\u200B中\u200B由\u200B游戏\u200BAI\n"
     u8"操控\u200B它们，\u200B冒险\u200B结束\u200B后\u200B它们\u200B会\u200B回到\n"
     u8"你\u200B家。\n"
     u8"\n"
     u8"要\u200B回\u200B自己\u200B家\u200B吗？\u200B想\u200B回来\u200B的话，\n"
     u8"点\u200B你\u200B家里\u200B的\u200B任意\u200B一\u200B个\u200B牌子。"},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"加入\u200B冒险\n"
     u8"\n"
     u8"%s\u200B的\u200B游戏\u200B正在\u200B把\u200B当前\u200B冒险\u200B发\u200B给\u200B你。\n"
     u8"如果\u200B他\u200B刚\u200B在\u200B某个\u200B地点\u200B做\u200B了\u200B选择\n"
     u8"（事件、\u200B升级），\u200B你\u200B会\u200B在\u200B他\n"
     u8"回到\u200B地图\u200B时\u200B加入。"},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"踏上\u200B冒险\u200B之旅\n"
     u8"\n"
     u8"即将\u200B加载\u200B%s\u200B的\u200B游戏：\n"
     u8"你\u200B会\u200B和\u200B他\u200B一起\u200B出发。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"回家\n"
     u8"\n"
     u8"与\u200B%s\u200B的\u200B连接\u200B已\u200B断开。\n"
     u8"你\u200B将\u200B回到\u200B自己\u200B家。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"回家\n"
     u8"\n"
     u8"和\u200B%s\u200B的\u200B出发\u200B没有\u200B成行。\n"
     u8"你\u200B将\u200B回到\u200B自己\u200B家。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"回家\n"
     u8"\n"
     u8"%s\u200B的\u200B冒险\u200B已\u200B结束。\n"
     u8"你\u200B将\u200B回到\u200B自己\u200B家。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"回家\n"
     u8"\n"
     u8"%s\u200B的\u200B战斗\u200B已经\u200B结束，\u200B但\u200B你\u200B这边\u200B的\n"
     u8"战斗\u200B还在\u200B继续：\u200B它\u200B已经\u200B不\u200B同步\u200B了，\n"
     u8"没\u200B人\u200B再\u200B进行\u200B这\u200B场\u200B战斗。\u200B你\u200B已\n"
     u8"回家；\u200B你\u200B的\u200B猫\u200B留\u200B在\u200B他\u200B的\u200B队伍\u200B里。\n"
     u8"\n"
     u8"想\u200B回到\u200B冒险\u200B中，\u200B点\u200B你\u200B家里\u200B的\n"
     u8"任意\u200B一\u200B个\u200B牌子。"},
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
     u8"回家\n"
     u8"\n"
     u8"你\u200B的\u200B战斗\u200B结束\u200B了，\u200B但\u200B%s\u200B的\u200B战斗\n"
     u8"还在\u200B继续：\u200B你\u200B这边\u200B已经\n"
     u8"不\u200B同步。\u200B你\u200B已\u200B回家；\u200B你\u200B的\u200B猫\n"
     u8"留\u200B在\u200B他\u200B的\u200B队伍\u200B里，\u200B由\u200B游戏\u200BAI\n"
     u8"操控\u200B到\u200B战斗\u200B结束。\n"
     u8"\n"
     u8"想\u200B回到\u200B冒险\u200B中，\u200B点\u200B你\u200B家里\u200B的\n"
     u8"任意\u200B一\u200B个\u200B牌子。"},
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
     u8"回家\n"
     u8"\n"
     u8"%s\u200B进入\u200B了\u200B一\u200B个\u200B你\u200B的\u200B游戏\n"
     u8"跟不上\u200B的\u200B地点：\u200B你\u200B这边\u200B的\u200B冒险\n"
     u8"已经\u200B和\u200B他\u200B的\u200B不\u200B一样\u200B了。\u200B你\u200B已\u200B回家；\u200B你\u200B的\n"
     u8"猫\u200B留\u200B在\u200B他\u200B的\u200B队伍\u200B里，\u200B由\u200B游戏\u200BAI\n"
     u8"操控。\n"
     u8"\n"
     u8"想\u200B回到\u200B冒险\u200B中，\u200B点\u200B你\u200B家里\u200B的\n"
     u8"任意\u200B一\u200B个\u200B牌子。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"回家\n"
     u8"\n"
     u8"你\u200B这边\u200B的\u200B战斗\u200B与\u200B%s\u200B的\n"
     u8"不\u200B同步\u200B了：\u200B战斗\u200B在\u200B没有\u200B你\u200B的\u200B情况\u200B下\u200B继续，\n"
     u8"所以\u200B你\u200B回家\u200B了。\u200B你\u200B的\u200B猫\u200B留\u200B在\n"
     u8"他\u200B的\u200B队伍\u200B里，\u200B由\u200B游戏\u200BAI\u200B操控。\n"
     u8"\n"
     u8"想\u200B回到\u200B冒险\u200B中，\u200B点\u200B你\u200B家里\u200B的\n"
     u8"任意\u200B一\u200B个\u200B牌子。"},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"你\u200B的\u200B存档\u200B正\u200B处于\u200B冒险\u200B中。\n"
     u8"多人游戏\u200B时，\u200B冒险\u200B要\u200B和\u200B房主\u200B一起\u200B进行：\n"
     u8"回到\u200B菜单\u200B选择\u200B另\u200B一个\u200B存档，\n"
     u8"或者\u200B单人\u200B把\u200B这个\u200B存档\u200B玩完。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"你\u200B的\u200B游戏\u200B正在\u200B加载\u200B%s\u200B的\u200B冒险。"},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%s\u200B重新加载\u200B了\u200B冒险：\n"
     u8"你\u200B也\u200B会\u200B重新加载。"},
    {u8"Rentrer", u8"回家"},
    {u8"Rester", u8"留下"},
    {u8"Ne pas attendre", u8"不等了"},
    {u8"Recharger", u8"重新加载"},
    {u8"J'arrive", u8"我来了"},
    {u8"Pas encore", u8"还没好"},
    {u8"Échanger", u8"交易"},
    {u8"Multijoueur", u8"多人游戏"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"这\u200B是\u200B%s\u200B家\u200B的\u200B管道：\n"
     u8"从\u200B你\u200B自己\u200B家里\n"
     u8"把\u200B猫\u200B送\u200B给\u200BNPC。"},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"NPC\u200B以\u200B%s\u200B的\u200B游戏\u200B为准：\n"
     u8"这个\u200B版本\u200B的\u200B游戏\u200B无法\n"
     u8"把\u200B你\u200B的\u200B猫\u200B送进\u200B管道。"},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"这\u200B是\u200B整个\u200B队伍\u200B的\u200B失败：\n"
     u8"%s\u200B的\u200B猫\u200B也\u200B不\u200B会\u200B回家。"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"加入\u200B%s\n"
     u8"\n"
     u8"你\u200B已经\u200B在\u200B一\u200B个\u200B多人房间\u200B里\u200B了：\n"
     u8"先\u200B退出\u200B房间\u200B（“多人游戏”，\u200B然后\u200B“退出”），\n"
     u8"再\u200B点\u200B一次\u200B“加入游戏”。"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"加入\u200B%s\n"
     u8"\n"
     u8"你\u200B的\u200B游戏\u200B将\u200B回到\u200B主菜单\u200B（你\u200B的\u200B进度\n"
     u8"会\u200B保存，\u200B和\u200B“保存然后退到主菜单”\u200B一样），\n"
     u8"然后\u200B加入\u200B%s\u200B的\u200B房间。\n"
     u8"\n"
     u8"以后\u200B再说：\u200B等\u200B你\u200B回到\u200B主菜单\u200B时\n"
     u8"再\u200B加入。"},
    {u8"Y aller", u8"前往"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"加入\u200B%s\n"
     u8"\n"
     u8"你\u200B回到\u200B主菜单\u200B后，\u200B游戏\n"
     u8"就\u200B会\u200B加入\u200B%s\u200B的\u200B房间\n"
     u8"（按\u200BEsc，\u200B然后\u200B选\u200B“保存然后退到主菜单”）。"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"加入\u200B%s\n"
     u8"\n"
     u8"目前\u200B无法\u200B通过\u200BSteam\u200B连接。\n"
     u8"稍后\u200B再\u200B试，\u200B或者\u200B通过\u200B地址\u200B加入\n"
     u8"（“多人游戏”，\u200B然后\u200B“加入”）。"},
    {u8"%s a annulé le départ.", u8"%s\u200B取消\u200B了\u200B出发。"},
    {u8"réseau local", u8"局域网"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"你\u200B的\u200B猫\u200B已经\u200B跟着\u200B%s\u200B的\u200B队伍\u200B出发\u200B了：\n"
     u8"你\u200B不\u200B在\u200B的\u200B时候，\u200B战斗\u200B中\u200B由\u200B游戏\u200BAI\u200B操控\n"
     u8"它们，\u200B%s\u200B替\u200B它们\u200B做\u200B选择；\u200B冒险\u200B结束\u200B后\n"
     u8"它们\u200B会\u200B回到\u200B你\u200B家。"},
    {u8"Internet", u8"互联网"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s\u200B正在\u200B守家：\u200B这\u200B场\u200B战斗\u200B他\u200B要\n"
     u8"独自\u200B用\u200B自己\u200B的\u200B猫\u200B来\u200B打。\u200B打完\u200B就\u200B回来。"},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s\u200B正在\u200B守家：\u200B每个\u200B人\u200B都\u200B要\n"
     u8"独自\u200B用\u200B自己\u200B的\u200B猫\u200B打\u200B各自\u200B的\u200B战斗。\u200B打完\u200B就\u200B回来。"},
    {u8"%s a fini de défendre sa maison.", u8"%s\u200B守家\u200B结束\u200B了。"},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s\u200B守家\u200B结束\u200B了。\n"
     u8"%s\u200B还在\u200B守家。"},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s\u200B守家\u200B结束\u200B了。\n"
     u8"%s\u200B还在\u200B守家。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"你\u200B在\u200B投票\u200B后\u200B取消\u200B了：\u200B%s\u200B将\n"
     u8"不\u200B带\u200B你\u200B和\u200B你\u200B的\u200B猫\u200B出发。\u200B等\u200B他\u200B到\u200B了\u200B地图\u200B上，\n"
     u8"你\u200B可以\u200B加入\u200B他\u200B的\u200B冒险。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"出发\u200B冒险\n"
     u8"\n"
     u8"这\u200B次\u200B没有\u200B猫能\u200B出发：\n"
     u8"%s\u200B不\u200B去\u200B冒险\u200B了。"},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"战利品\u200B还\u200B没\u200B分配\u200B完：\n"
     u8"等\u200B物品\u200B逐个\u200B展示\u200B完毕。"},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"你\u200B要\u200B去\u200B加入\u200B%s\u200B的\u200B冒险：\n"
     u8"回来\u200B后\u200B再\u200B守家。"},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"由\u200B%s\u200B发起\u200B冒险：\n"
     u8"让\u200B他\u200B点\u200B“出发！”。"},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"多人游戏\n"
     u8"\n"
     u8"创建房间\u200B让\u200B朋友\u200B加入，\n"
     u8"或者\u200B加入\u200B朋友\u200B的\u200B游戏。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"多人房间\n"
     u8"\n"
     u8"[c:red]无法\u200B打开\u200B端口\u200B%u。[/c]\n"
     u8"可能\u200B有\u200B其他\u200B程序\u200B正在\u200B使用\u200B它。\n"
     u8"关闭\u200B房间，\u200B然后\u200B重试。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"多人房间\n"
     u8"\n"
     u8"正在\u200B创建房间……"},
    {u8"%s (hôte)", u8"%s（房主）"},
    {u8"En attente d'un ami...", u8"正在\u200B等待\u200B朋友……"},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Steam\u200B好友\u200B一键\u200B就\u200B能\u200B加入：\u200B在\u200B他们\u200B的\n"
     u8"Steam\u200B好友\u200B列表\u200B里，\u200B对\u200B你\u200B的\u200B名字\u200B选\u200B“加入游戏”。\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"或\u200B通过\u200B地址：\n"
     u8"%s   （%s，\u200B端口\u200B%u）\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"把\u200B这个\u200B地址\u200B告诉\u200B朋友：\n"
     u8"%s   （%s，\u200B端口\u200B%u）\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"地址\u200B已\u200B复制：\u200B用\u200BCtrl+V\u200B粘贴\u200B给\u200B他。\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"在\u200B这\u200B台\u200B电脑\u200B上\u200B找不到\u200B地址\u200B（端口\u200B%u）。\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"多人房间\n"
     u8"\n"
     u8"%s\n"
     u8"玩家\u200B（%d/4）：\n"
     u8"%s\n"
     u8"所有人\u200B到齐\u200B后，\u200B选择\u200B你\u200B的\u200B存档。"},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"加入\u200B游戏\n"
     u8"\n"
     u8"房主\u200B地址：\n"
     u8"%s_\n"
     u8"\n"
     u8"输入\u200B朋友\u200B给\u200B你\u200B的\u200B地址，\n"
     u8"或\u200B用\u200BCtrl+V\u200B粘贴。\u200B按\u200BEnter\u200B确认。\u200B%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"用\u200BSteam\u200B更\u200B简单：\u200B在\u200B你\u200B的\u200B好友\n"
     u8"列表\u200B里，\u200B对\u200B朋友\u200B选\u200B“加入游戏”。"},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"加入\u200B游戏\n"
     u8"\n"
     u8"[c:red]地址\u200B无效：\u200B%s[/c]\n"
     u8"\n"
     u8"例如：\u200B26.123.45.67\n"
     u8"或\u200B使用\u200B其他\u200B端口：\u200B26.123.45.67:27600"},
    {u8"(vide)", u8"（空）"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.", u8"请\u200B确认\u200B你们\u200B的\u200B游戏\u200B和\u200B模组\u200B版本\u200B相同。"},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).", u8"房主\u200B的\u200B游戏\u200B已\u200B满\u200B（最多\u200B4名\u200B玩家）。"},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"你们\u200B的\u200B模组\u200B版本\u200B不同：\n"
     u8"双方\u200B都\u200B更新\u200B到\u200B最新版。"},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"你们\u200B的\u200B游戏\u200B版本\u200B不同：\n"
     u8"双方\u200B都\u200B在\u200BSteam\u200B里\u200B更新\u200BMewgenics。"},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"游戏\u200B文件\u200B不\u200B一致：\n"
     u8"在\u200BSteam\u200B里\u200B验证\u200B文件\u200B完整性\u200B（属性 > 已安装文件）。"},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"mgmp.json\u200B里\u200B的\u200Bnet.control\u200B设置\u200B让\u200B两\u200B名\u200B玩家\n"
     u8"控制\u200B同一\u200B只猫：\u200B把\u200B它\u200B改回\u200B\"auto\"。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"多人房间\n"
     u8"\n"
     u8"[c:red]连接被拒绝。[/c]\n"
     u8"%s\n"
     u8"（%s）"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"多人房间\n"
     u8"\n"
     u8"[c:red]无法\u200B加入\u200B%s：[/c]\n"
     u8"%s。\n"
     u8"退出\u200B房间\u200B后\u200B重试\u200B（或\u200B通过\u200B地址\u200B加入）。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"多人房间\n"
     u8"\n"
     u8"正在连接%s……"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"多人房间\n"
     u8"\n"
     u8"[c:red]无法\u200B连接\u200B%s。[/c]\n"
     u8"检查\u200B地址，\u200B并\u200B确认\u200B朋友\u200B已经\n"
     u8"点\u200B了\u200B“创建房间”。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"多人房间\n"
     u8"\n"
     u8"已断开连接。"},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"存档\u200B画面\u200B会\u200B自动\u200B打开。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"多人房间\n"
     u8"\n"
     u8"正在\u200B连接\u200B%s……\u200B%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"多人房间\n"
     u8"\n"
     u8"[c:red]与\u200B房主\u200B的\u200B连接\u200B已\u200B断开。[/c]\n"
     u8"退出\u200B房间，\u200B然后\u200B重新\u200B加入。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"多人房间\n"
     u8"\n"
     u8"正在\u200B通过\u200BSteam\u200B连接\u200B%s……\n"
     u8"（需要\u200B几秒钟）\u200B%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"多人房间\n"
     u8"\n"
     u8"已\u200B连接，\u200B正在\u200B向\u200B房主\u200B确认\u200B身份……\u200B%s"},
    {u8"%s (toi)", u8"%s（你）"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"多人房间\n"
     u8"\n"
     u8"你\u200B加入\u200B了\u200B%s\u200B的\u200B游戏！\n"
     u8"\n"
     u8"玩家\u200B（%d/4）：\n"
     u8"%s\n"
     u8"正在\u200B等待\u200B%s\u200B的\u200B游戏：\n"
     u8"他\u200B选\u200B好\u200B存档\u200B后，\n"
     u8"存档\u200B画面\u200B会\u200B自动\u200B打开。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"多人房间\n"
     u8"\n"
     u8"你\u200B加入\u200B了\u200B%s\u200B的\u200B游戏！\n"
     u8"\n"
     u8"玩家\u200B（%d/4）：\n"
     u8"%s\n"
     u8"正在\u200B等待\u200B%s\u200B的\u200B游戏：\n"
     u8"他\u200B选\u200B好\u200B存档\u200B后，\n"
     u8"你\u200B就\u200B可以\u200B选择\u200B自己\u200B的\u200B存档。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"多人房间\n"
     u8"\n"
     u8"你\u200B加入\u200B了\u200B%s\u200B的\u200B游戏！\n"
     u8"\n"
     u8"玩家\u200B（%d/4）：\n"
     u8"%s\n"
     u8"选择\u200B你\u200B的\u200B存档：\u200B你\u200B的\u200B家，\u200B你\u200B的\u200B猫。\n"
     u8"NPC\u200B和\u200B剧情\u200B将\u200B沿用\u200B%s\u200B的，\n"
     u8"出发\u200B冒险\u200B时，\u200B你\u200B会\u200B加入\u200B他\u200B的\u200B队伍。"},
    {u8"Héberger", u8"创建房间"},
    {u8"Rejoindre", u8"加入"},
    {u8"Choisir la sauvegarde", u8"选择存档"},
    {u8"Arrêter", u8"关闭房间"},
    {u8"Choisir ta sauvegarde", u8"选择你的存档"},
    {u8"Quitter", u8"退出"},
    {u8"Annuler", u8"取消"},
    {u8"Échanger avec %s", u8"与%s交易"},
    {u8"Donner à qui ?", u8"送给谁？"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"按\u200B上\u200B或\u200B下\u200B选择，\u200B然后\u200B按\u200BEnter。"},
    {u8"Pièces à donner : %lld_", u8"要\u200B赠送\u200B的\u200B钱：\u200B%lld_"},
    {u8"Nourriture à donner : %lld_", u8"要\u200B赠送\u200B的\u200B食物：\u200B%lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"输入\u200B数字\u200B（T：\u200B全部），\u200B然后\u200B按\u200BEnter。"},
    {u8"Échanger avec %s : les objets de ton rangement", u8"与\u200B%s\u200B交易：\u200B你\u200B仓库\u200B里\u200B的\u200B物品"},
    {u8"16 au plus par don : %u choisis.", u8"每\u200B次\u200B最多\u200B赠送\u200B16件：\u200B已\u200B选\u200B%u件。"},
    {u8"16 au plus par don : %u choisi.", u8"每\u200B次\u200B最多\u200B赠送\u200B16件：\u200B已\u200B选\u200B%u件。"},
    {u8"Lecture du rangement...", u8"正在\u200B读取\u200B仓库……"},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"上/下：\u200B选择，\u200B空格：\u200B勾选，\u200B左/右：\u200B数量。\n"
     u8"输入\u200B字母\u200B搜索，\u200B然后\u200B按\u200BEnter。"},
    {u8"Tu n'as rien choisi à donner.", u8"你\u200B没有\u200B选择\u200B要\u200B赠送\u200B的\u200B东西。"},
    {u8"Tu n'as rien à donner pour l'instant.", u8"你\u200B暂时\u200B没有\u200B可\u200B赠送\u200B的\u200B东西。"},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"把钱袋里的%s\n"
     u8"送给%s？"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"把%s\n"
     u8"送给%s？"},
    {u8"Envoi du don...", u8"正在赠送……"},
    {u8"Personne ne peut recevoir maintenant.", u8"现在\u200B没\u200B人\u200B能\u200B接收。"},
    {u8"Personne d'autre n'est dans la partie.", u8"游戏\u200B里\u200B没有\u200B其他\u200B人。"},
    {u8"Valider", u8"确定"},
    {u8"Donner", u8"赠送"},
    {u8"Recommencer", u8"重来"},
    {u8"Fermer", u8"关闭"},
    {u8"Pas d'échange possible ici.", u8"这里\u200B无法\u200B交易。"},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"窗口\u200B已\u200B关闭：\u200B房主\u200B进入\u200B了\u200B一\u200B个\u200B地点。\n"
     u8"你\u200B的\u200B选择\u200B已\u200B保留\u200B（按\u200BE\u200B继续）。"},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"冒险\u200B中\u200B交易\n"
     u8"\n"
     u8"在\u200B地图\u200B上\u200B按\u200BE，\u200B可以\u200B把\u200B钱袋\u200B里\u200B的\u200B钱\n"
     u8"或\u200B食物\u200B送\u200B给\u200B其他\u200B玩家。"},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
