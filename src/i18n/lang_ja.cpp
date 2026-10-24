// lang_ja.cpp -- Mewgenics Multi : les textes du mod et du lanceur en japonais (ja).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : la forme polie simple des jeux (desu / masu), comme combined.csv. \u200B aux coupures permises.
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_ja[];
const TrPair kTable_ja[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"mgmp.jsonを\u200B読み込めませんでした。\u200B設定は\u200B保存されていません。"},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"mgmp.jsonに\u200Bコメント（//\u200Bまたは\u200B/* */）が\u200Bあり、\u200B保存すると\u200B消えてしまいます。\u200Bコメントを\u200B削除するか、\u200Bファイルを\u200B直接\u200B編集して\u200B設定を\u200B変更してください。"},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"mgmp.jsonが\u200B正しい\u200BJSONでは\u200Bありません。\u200B設定を\u200B変更する\u200B前に\u200B修正してください（または\u200Bzipに\u200B入っていた\u200Bものに\u200B戻してください）。"},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"MODの\u200Bフォルダーに\u200B書き込めません。\u200B設定は\u200B保存されていません。"},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"mgmp.jsonを\u200B置き換えられませんでした（ほかの\u200Bアプリで\u200B開いていませんか？）。\u200B設定は\u200B保存されていません。"},
    {u8"Steam est ouvert.", u8"Steamが\u200B起動しています。"},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Steamが\u200Bないと、\u200BMODが\u200B読み込まれる\u200B前に\u200Bゲームが\u200B終了します。\u200BSteamを\u200B起動して、\u200B接続されるまで\u200B待ってください。"},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"Mewgenics\u00A0Multi.exeと\u200B同じ\u200Bフォルダーに\u200Bmgmp.jsonが\u200Bありません。\u200B設定を\u200B保存すると\u200B作り直されます。"},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"mgmp.jsonが\u200B正しい\u200BJSONでは\u200Bありません。\u200B修正するか、\u200Bzipに\u200B入っていた\u200Bものに\u200B戻してください。"},
    {u8"Réglages lus dans mgmp.json.", u8"mgmp.jsonの\u200B設定を\u200B読み込みました。"},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exeが\u200B見つかりません（%s）。\u200B「設定」で\u200B「参照…」を\u200Bクリックして、\u200BMewgenics.exeを\u200B選んでください。"},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"Mewgenics.exeと\u200B同じ\u200Bフォルダーに\u200Bresources.gpakが\u200Bありません。\u200BSteamで\u200Bゲームファイルの\u200B整合性を\u200B確認してください。"},
    {u8"Jeu trouvé : %s.", u8"ゲームを検出：%s"},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.", u8"steam_appid.txt：\u200Bゲームが\u200B見つかってから\u200B確認します。"},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Mewgenics.exeと\u200B同じ\u200Bフォルダーに\u200Bありません。\u200Bこれが\u200Bないと、\u200BSteamが\u200BMODなしで\u200Bゲームを\u200B再起動してしまいます。"},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"Mewgenics.exeと\u200B同じ\u200Bフォルダーに\u200Bある\u200Bファイルに\u200B686060が\u200B書かれていません。\u200BSteamが\u200BMODなしで\u200Bゲームを\u200B再起動してしまいます。"},
    {u8"steam_appid.txt est en place à côté du jeu.", u8"steam_appid.txtは\u200Bゲームと\u200B同じ\u200Bフォルダーに\u200Bあります。"},
    {u8"Fichiers du mod présents.", u8"MODの\u200Bファイルが\u200Bそろっています。"},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Mewgenics\u00A0Multi.exeと\u200B同じ\u200Bフォルダーに\u200Bmgmp.dllと\u200Bmgmp_loader.exeが\u200Bありません。\u200Bzipから\u200Bフォルダーを\u200Bまるごと\u200B展開し直してください。"},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Mewgenics\u00A0Multi.exeと\u200B同じ\u200Bフォルダーに\u200B%sが\u200Bありません。\u200Bzipから\u200Bフォルダーを\u200Bまるごと\u200B展開し直してください。"},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics\u00A0Multi、\u200Bビルド %s、\u200Bプロトコル %u：\u200B全員が\u200B同じ\u200Bバージョンを\u200B使う\u200B必要が\u200Bあります。"},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"ゲームが\u200B見つかりません。\u200B先に\u200B「設定」で\u200BMewgenics.exeを\u200B選んでください。"},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"ゲームの\u200Bフォルダーに\u200Bsteam_appid.txtを\u200B書き込めませんでした。\u200B手動で\u200Bコピーしてください（LISEZ-MOIの\u200B手順2）。"},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"Windowsの\u200B確認で\u200B許可しなかったため、\u200Bsteam_appid.txtは\u200Bコピーされていません。\u200Bもう一度\u200B試すか、\u200B手動で\u200Bコピーしてください（LISEZ-MOIの\u200B手順2）。"},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"Mewgenics\u00A0Multiを\u200B管理者として\u200B再起動できませんでした。\u200Bsteam_appid.txtを\u200B手動で\u200Bコピーしてください（LISEZ-MOIの\u200B手順2）。"},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"管理者としての\u200Bコピーに\u200B失敗しました。\u200Bsteam_appid.txtを\u200B手動で\u200Bコピーしてください（LISEZ-MOIの\u200B手順2）。"},
    {u8"Le mod n'a pas pu être chargé : %s", u8"MODを\u200B読み込めませんでした：%s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"mgmp_loader.exeが\u200BMODを\u200B読み込まずに\u200B終了しました。\u200Bもう一度\u200B起動してください。\u200Bまた\u200B問題が\u200B起きたら、\u200Bloader.txtを\u200B送ってください。"},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"mgmp_loader.exeが\u200BMODを\u200B読み込まずに\u200B終了しました（コード%s）。\u200B最も\u200B考えられる\u200B原因：\u200BSteamが\u200B起動していないか、\u200B接続されていません。\u200BSteamを\u200B起動し、\u200B接続されるまで\u200B待ってから、\u200Bもう一度\u200B起動してください。"},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"ゲームが\u200B起動中に\u200B終了しました（MODの\u200B読み込み前）。\u200Bよくある\u200B原因：\u200BSteamが\u200B起動していないか、\u200B接続されていません。\u200BSteamを\u200B起動し、\u200B接続されるまで\u200B待ってから、\u200Bもう一度\u200B起動してください。"},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"Steamが\u200Bゲームを\u200B再起動しています（1、2分\u200Bかかる\u200Bことが\u200Bあります）…"},
    {u8"Ouverture du jeu…", u8"ゲームを\u200B起動しています…"},
    {u8"Le jeu démarre…", u8"ゲーム起動中…"},
    {u8"Injection du mod dans le jeu…", u8"ゲームに\u200BMODを\u200B注入しています…"},
    {u8"Le mod est en place, le jeu reprend…", u8"MODの\u200B準備が\u200Bできました。\u200Bゲームを\u200B再開します…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Steamによる\u200Bゲームの\u200B再起動を\u200Bまだ\u200B待っています…\u200BSteamが\u200B起動した\u200Bばかりなら、\u200B何か\u200B確認を\u200B求めているかもしれません。"},
    {u8"Jeu relancé trouvé : injection du mod…", u8"再起動した\u200Bゲームを\u200B検出：\u200BMODを\u200B注入しています…"},
    {u8"Le jeu est lancé avec le mod.", u8"ゲームが\u200BMOD付きで\u200B起動しました。"},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.",
     u8"MODの\u200B準備に\u200B時間が\u200Bかかりましたが、\u200Bゲームを\u200B再開します。"},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.", u8"再起動した\u200Bゲームの\u200B起動が\u200B遅れていますが、\u200Bこのまま\u200B注入します。"},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"Mewgenics\u00A0Multi.exeと\u200B同じ\u200Bフォルダーに\u200Bmgmp.dllが\u200B見つかりません。\u200Bzipから\u200BMODの\u200Bフォルダーを\u200B展開し直してください。"},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exeが\u200B見つかりません。\u200B「設定」で\u200B「参照…」を\u200Bクリックして、\u200BMewgenics.exeを\u200B選んでください。"},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"mgmp.jsonを\u200B読み込めません。\u200B修正するか、\u200Bzipに\u200B入っていた\u200Bものに\u200B戻してください。"},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"Steamが\u200BMODなしで\u200Bゲームを\u200B再起動しました。\u200BMewgenics.exeと\u200B同じ\u200Bフォルダーの\u200Bsteam_appid.txtを\u200B確認してから（「修復」ボタン）、\u200Bもう一度\u200B起動してください。"},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"この\u200Bゲームは\u200BMODの\u200B対象\u200Bバージョン（1.1.21239）では\u200Bありません。\u200Bアップデート\u200Bされましたか？"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"ほかの\u200Bプログラム（オーバーレイ、\u200Bアンチウイルス、\u200Bアンチチートなど）が\u200BMODの\u200B読み込みを\u200Bブロックしました。"},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"ゲームの\u200B起動に\u200B時間が\u200Bかかりすぎたため、\u200BMODは\u200B読み込まれていません。\u200Bもう一度\u200B起動してください。"},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"MODの\u200B注入に\u200B失敗したため、\u200Bゲームを\u200B終了しました。\u200Bもう一度\u200B起動してください。\u200Bまた\u200B問題が\u200B起きたら、\u200Bloader.txtを\u200B送ってください。"},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"mgmp.jsonに\u200Bゲームの\u200B場所が\u200B書かれていません。\u200B「設定」で\u200B「参照…」を\u200Bクリックして、\u200BMewgenics.exeを\u200B選んでください。"},
    {u8"build %s · protocole %u", u8"ビルド %s · プロトコル %u"},
    {u8"Steam ouvert", u8"Steam起動中"},
    {u8"Steam fermé", u8"Steam未起動"},
    {u8"Réglages (mgmp.json)", u8"設定（mgmp.json）"},
    {u8"Jeu trouvé", u8"ゲーム検出"},
    {u8"Jeu incomplet", u8"ゲームファイル不足"},
    {u8"Jeu introuvable", u8"ゲーム未検出"},
    {u8"Fichiers du mod", u8"MODファイル"},
    {u8"Réparer", u8"修復"},
    {u8"Jouer", u8"プレイ"},
    {u8"Règle d'abord les croix rouges.", u8"先に\u200B赤い\u200B×の\u200B項目を\u200B解決してください。"},
    {u8"Réglages", u8"設定"},
    {u8"Journaux", u8"ログ"},
    {u8"Langue", u8"言語"},
    {u8"Le jeu (Mewgenics.exe)", u8"ゲーム（Mewgenics.exe）"},
    {u8"Parcourir…", u8"参照…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exeと\u200Bresources.gpakが\u200Bあります。"},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"同じ\u200Bフォルダーに\u200Bresources.gpakが\u200Bないため、\u200B正しい\u200Bフォルダーでは\u200Bありません。"},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.",
     u8"ここに\u200BMewgenics.exeは\u200Bありません。\u200B「参照…」で\u200B選んでください。"},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Steamで\u200B一緒に\u200B遊ぶ（フレンドリスト）"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Steamで\u200B問題が\u200B起きる\u200Bときは\u200Bチェックを\u200B外してください。\u200Bゲームの\u200Bマルチプレイ\u200Bメニューでは\u200Bアドレスでの\u200B接続だけに\u200Bなります。"},
    {u8"Enregistrer", u8"保存"},
    {u8"Retour", u8"戻る"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"まだ\u200Bログが\u200Bありません。\u200BMODは\u200Bプレイする\u200Bたびに\u200Bログを\u200B1つ\u200B書き出します。"},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.", u8"MODの\u200Bログ：1件（Mewgenics\u00A0Multi.exeと\u200B同じ\u200Bフォルダー）"},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"MODの\u200Bログ：%d件（Mewgenics\u00A0Multi.exeと\u200B同じ\u200Bフォルダー）"},
    {u8"Le dernier : %s", u8"最新：%s"},
    {u8"écrit le %s", u8"更新日時：%s"},
    {u8"Ouvrir le dossier", u8"フォルダーを開く"},
    {u8"Copier le dernier journal", u8"最新のログをコピー"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"助けを\u200B求める\u200Bときは、\u200B最新の\u200Bログを\u200Bコピーして、\u200BDiscordに\u200BCtrl+Vで\u200B貼り付けてください。"},
    {u8"Lancement", u8"起動"},
    {u8"Le jeu se lance avec le mod…", u8"MOD付きで\u200Bゲームを\u200B起動しています…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.", u8"楽しんでください！\u200Bこの\u200Bウィンドウは\u200B自動で\u200B閉じます。"},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"詳細は\u200BMewgenics\u00A0Multi.exeと\u200B同じ\u200Bフォルダーの\u200Bloader.txtに\u200Bあります。"},
    {u8"Réessayer", u8"再試行"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"Steamが\u200Bゲームを\u200B再起動する\u200B場合、\u200B1、2分\u200Bかかる\u200Bことが\u200Bあります。"},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.", u8"ログを\u200Bコピーしました。\u200BDiscordに\u200BCtrl+Vで\u200B貼り付けてください。"},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"クリップボードに\u200Bコピーできませんでした。\u200Bもう一度\u200B試すか、\u200Bフォルダーを\u200B開いてください。"},
    {u8"Où est Mewgenics.exe ?", u8"Mewgenics.exeは\u200Bどこですか？"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"この\u200BPCでは\u200BDirect2Dが\u200B使えないため、\u200BMewgenics\u00A0Multiを\u200B表示できません。"},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"グラフィックカードを\u200B準備できなかったため、\u200BMewgenics\u00A0Multiを\u200B表示できません。"},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"家も\u200Bネコも\u200Bそれぞれ、\n"
     u8"冒険は\u200Bみんなで！"},
    {u8"(illustrations du jeu introuvables)", u8"（ゲームの\u200Bイラストが\u200B見つかりません）"},
    {u8"Comme le jeu (%s)", u8"ゲームと同じ（%s）"},
    {u8"Comme Windows (%s)", u8"Windowsと同じ（%s）"},
    {u8"Automatique (%s)", u8"自動（%s）"},
    {u8"Lancement annulé.", u8"起動を\u200Bキャンセルしました。"},
    {u8"Lancement du mod…", u8"MODを\u200B起動しています…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"Windowsが\u200B起動の\u200B準備に\u200B失敗しました。\u200BMewgenics\u00A0Multiを\u200Bもう一度\u200B起動してください。"},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"mgmp_loader.exeを\u200B起動できませんでした。\u200Bzipから\u200BMODの\u200Bフォルダーを\u200B展開し直してください（アンチウイルスに\u200Bブロックされた\u200B可能性も\u200Bあります）。"},
    {u8"la connexion Steam avec l'hôte est coupée", u8"ホストとの\u200BSteam接続が\u200B切れました"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"ホストが\u200BSteamで\u200B応答しません（ロビーが\u200B閉じているか、\u200BMODが\u200Bない？）"},
    {u8"Steam refuse la connexion", u8"Steamが\u200B接続を\u200B拒否しました"},
    {u8"À %s de choisir", u8"%sが選ぶ番です"},
    {u8"À toi de choisir", u8"あなたが選ぶ番です"},
    {u8"À %s de choisir la capacité à remplacer.", u8"入れ替える\u200Bアビリティは\u200B%sが\u200B選びます。"},
    {u8"Ce chat est à %s", u8"このネコは\u200B%sの\u200Bネコです"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"タグ選び\n"
     u8"\n"
     u8"共有の\u200Bタグ選びが\u200B止まりました（ゲームの\u200Bエラー）。\n"
     u8"チーム\u200B全員の\u200Bタグを\u200B選んでください。"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"タグ選び\n"
     u8"\n"
     u8"準備完了！\u200B%sに\u200B伝えました。\n"
     u8"チーム\u200B全員の\u200B出発は\u200Bホストが\u200B決定します。"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"タグ選び\n"
     u8"\n"
     u8"出発を\u200B決定するのは\u200B%sです。\n"
     u8"ネコに\u200Bタグを\u200B付けたら\u200B伝えてください。"},
    {u8"%s et %s", u8"%sと%s"},
    {u8"%s, %s", u8"%s、%s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"タグ選び\n"
     u8"\n"
     u8"%sが\u200Bまだ\u200Bネコの\u200Bタグを\u200B選んでいます。\n"
     u8"もう一度\u200B「やったる\u200Bニャ！」を\u200B押すと\u200Bそのまま\n"
     u8"出発します。"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"タグ選び\n"
     u8"\n"
     u8"%sが\u200Bまだ\u200Bネコの\u200Bタグを\u200B選んでいます。\n"
     u8"もう一度\u200B「やったる\u200Bニャ！」を\u200B押すと\u200Bそのまま\n"
     u8"出発します。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"冒険に出発\n"
     u8"\n"
     u8"%sの\u200B出発は\u200Bここに\u200B表示されなくなりました。\n"
     u8"この\u200B画面は\u200B自分で\u200B進めてください。\u200Bマップで\n"
     u8"合流できます。"},
    {u8"  %s : parti", u8"  %s：出発済み"},
    {u8"  %s : prêt !", u8"  %s：準備完了！"},
    {u8"  %s : charge ta partie...", u8"  %s：読み込み中..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"冒険に出発\n"
     u8"\n"
     u8"チームが、\u200B一緒に\u200B出発するために\u200Bあなたの\u200Bゲームを\u200B読み込んでいます：\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"アイテム選び\n"
     u8"\n"
     u8"ここでは\u200B倉庫を\u200B読み込めませんでした。\n"
     u8"ネコたちは\u200B今\u200B装備している\u200Bアイテムで\u200B出発します。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"アイテム選び\n"
     u8"\n"
     u8"%sが\u200Bまだ\u200Bネコの\u200B装備を\u200B選んでいます。\n"
     u8"もう一度\u200B「やったる\u200Bニャ！」を\u200B押すと\u200Bそのまま\n"
     u8"出発します。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"アイテム選び\n"
     u8"\n"
     u8"%sが\u200Bまだ\u200Bネコの\u200B装備を\u200B選んでいます。\n"
     u8"もう一度\u200B「やったる\u200Bニャ！」を\u200B押すと\u200Bそのまま\n"
     u8"出発します。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"アイテム選び\n"
     u8"\n"
     u8"準備完了！\u200Bネコたちは\u200Bこの\u200Bアイテムで\u200B出発します。\n"
     u8"チーム\u200B全員の\u200B出発は\u200B%sが\u200B決定します。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"アイテム選び\n"
     u8"\n"
     u8"ネコたちに\u200B選んだ\u200Bアイテムを\u200B%sの\u200B冒険に\n"
     u8"反映できませんでした。\u200Bネコたちは\n"
     u8"元の\u200B装備で\u200B出発し、\u200B倉庫の\u200B中身も\n"
     u8"変わりません。"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"アイテム選び\n"
     u8"\n"
     u8"%sが\u200Bすでに\u200B出発を\u200B決定しました。\u200Bネコたちは\n"
     u8"ここまでに\u200B選んだ\u200Bアイテムで\u200B出発します。\n"
     u8"以降の\u200B変更は\u200B無効です。"},
    {u8"Ta bourse : %s.", u8"あなたの\u200Bサイフ：%s。"},
    {u8"%s n'est plus dans la partie.", u8"%sは\u200Bもう\u200Bゲームに\u200Bいません。"},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"この\u200B冒険では\u200Bサイフが\u200Bありません（ネコを\u200B連れてきていないため）。"},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"トレードは\u200B家の\u200B中か、\u200Bマルチプレイの\u200B冒険中に\u200Bサイフを\u200B使って\u200Bできます。\u200B今は\u200Bできません：%s。"},
    {u8"pas de partie à plusieurs en cours", u8"マルチプレイ中では\u200Bありません"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés",
     u8"ゲームの\u200B機能が\u200B足りないため、\u200B家での\u200Bトレードは\u200B無効です"},
    {u8"tu es dans la partie de l'hôte", u8"ホストの\u200Bゲームに\u200Bいます"},
    {u8"tu n'es pas dans ta maison", u8"自分の\u200B家に\u200Bいません"},
    {u8"ta save n'est pas chargée", u8"セーブデータが\u200B読み込まれていません"},
    {u8"un départ en aventure est en cours", u8"冒険に\u200B出発する\u200Bところです"},
    {u8"ta maison est attaquée", u8"家を\u200B防衛中です"},
    {u8"ta maison est illisible", u8"家の\u200Bデータを\u200B読み込めません"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"倉庫は、\u200Bネコたちが\u200Bホストの\u200B冒険から\u200B持ち帰る\u200Bものを\u200B待っています（帰ってきているなら、\u200Bセーブデータを\u200B再ロードしてください：\u200BEsc、\u200B「セーブして\u200Bメニューへ」、\u200B「プレイ」で\u200Bセーブデータを\u200B選択）"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"倉庫の\u200Bアイテムが\u200Bまだ\u200B新しい\u200B持ち主に\u200B渡っていません（戦利品、\u200B買い物、\u200BNPC）"},
    {u8"Pas d'échange maintenant : %s.", u8"今は\u200Bトレードできません：%s。"},
    {u8"Ton don précédent attend encore sa réponse.", u8"前の\u200Bプレゼントは\u200Bまだ\u200B返事待ちです。"},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"MODが\u200Bまだ\u200Bセーブデータを\u200B認識していません。\u200Bメニューから\u200B再ロードしてください。"},
    {u8"Choisis un joueur de la partie.", u8"ゲーム内の\u200Bプレイヤーを\u200B選んでください。"},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.", u8"今は\u200Bトレードできません：\u200B%sが\u200B自分の\u200B家に\u200Bいません。"},
    {u8"Ce montant n'est pas possible.", u8"その\u200B数は\u200B指定できません。"},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.", u8"渡す\u200Bコイン、\u200Bフード、\u200Bアイテムを\u200B選んでください。"},
    {u8"16 objets au plus par don.", u8"1回に\u200B渡せる\u200Bアイテムは\u200B16個までです。"},
    {u8"Pas d'objets maintenant : %s.", u8"今は\u200Bアイテムを\u200B渡せません：%s。"},
    {u8"Ta maison est illisible : pas de don.", u8"家の\u200Bデータを\u200B読み込めないため、\u200B渡せません。"},
    {u8"Tu n'as pas autant de pièces.", u8"コインが\u200B足りません。"},
    {u8"Tu n'as pas autant de nourriture.", u8"フードが\u200B足りません。"},
    {u8"Ton rangement est illisible : pas de don.", u8"倉庫を\u200B読み込めないため、\u200B渡せません。"},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"選んだ\u200Bアイテムの\u200B1つが、\u200Bまだ\u200B終わっていない\u200Bプレゼントに\u200B使われています。"},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"選んだ\u200Bアイテムの\u200B1つが\u200B倉庫に\u200Bありません。\u200B選び直してください。"},
    {u8"Un objet choisi ne peut pas se donner.", u8"選んだ\u200Bアイテムの\u200B1つは\u200B渡せません。"},
    {u8"Le don n'a pas pu partir (connexion).", u8"プレゼントを\u200B送れませんでした（接続）。"},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…", u8"%sに\u200Bプレゼントを\u200B送りました（%s）。\u200B返事を\u200B待っています…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…", u8"プレゼントの\u200B送信を\u200Bホストに\u200B依頼しました。\u200B返事を\u200B待っています…"},
    {u8"Pas de don : %s.", u8"渡せません：%s。"},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%sが\u200Bプレゼントを\u200B受け取りました。\u200Bアイテムは、\u200Bアイテム画面を\u200B閉じると\u200B倉庫から\u200Bなくなります。"},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%sが\u200Bプレゼントを\u200B受け取りました。\u200Bアイテムは、\u200Bできるだけ\u200B早く\u200B倉庫から\u200Bなくなります（出発、\u200B防衛戦、\u200Bネコの\u200B帰還の\u200Bあと）。"},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%sが\u200Bプレゼントを\u200B受け取りました。\u200B渡した\u200Bものは、\u200Bこの\u200Bセーブデータに\u200B戻ったときに\u200B家から\u200Bなくなります。"},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"%sから\u200Bプレゼントの\u200B返事が\u200Bありません。\u200B家からは\u200B何も\u200Bなくなっていません。"},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"%sから\u200Bプレゼントの\u200B返事が\u200Bありません。\u200B家からは\u200B何も\u200Bなくなっていません。"},
    {u8"Les échanges marchent pendant une partie à plusieurs.", u8"トレードは\u200Bマルチプレイ中に\u200B使えます。"},
    {u8"Dans ta maison : %s.", u8"家にあるもの：%s。"},
    {u8"%s n'est pas dans sa maison.", u8"%sは\u200B自分の\u200B家に\u200Bいません。"},
    {u8"%lld pièces", u8"コイン%lld枚"},
    {u8"%lld pièce", u8"コイン%lld枚"},
    {u8"%lld nourritures", u8"フード%lld個"},
    {u8"%lld nourriture", u8"フード%lld個"},
    {u8"rien", u8"なし"},
    {u8"%s n'est pas dans sa maison", u8"%sが\u200B自分の\u200B家に\u200Bいません"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s", u8"%sの\u200B画面で\u200Bウィンドウか\u200Bアイテム画面が\u200B開いたままです"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"%sの\u200Bゲームが\u200Bアイテムを\u200Bしまえませんでした"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"%sの\u200Bゲームが\u200Bアイテムを\u200Bしまえませんでした"},
    {u8"les dons ne marchent pas chez %s", u8"%sの\u200B環境では\u200Bプレゼントが\u200B使えません"},
    {u8"%s n'est plus dans la partie", u8"%sは\u200Bもう\u200Bゲームに\u200Bいません"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%sの\u200Bネコが\u200Bまだ\u200Bホストの\u200B冒険に\u200Bいます（倉庫は\u200Bネコの\u200B帰りを\u200B待っています）"},
    {u8"raison inconnue", u8"理由不明"},
    {u8"Cadeau d'%s", u8"%sからの\u200Bプレゼント"},
    {u8"Cadeau de %s", u8"%sからの\u200Bプレゼント"},
    {u8"%s t'a donné %s.", u8"%sから\u200B%sを\u200Bもらいました。"},
    {u8"C'est dans ta maison, et ta save est à jour.", u8"家に\u200B届きました。\u200Bセーブデータも\u200B更新済みです。"},
    {u8"C'est dans ta maison.", u8"家に\u200B届きました。"},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"最終的に、\u200B%sが\u200B%sを\u200B受け取りました。\u200B家から\u200B送り出されました。"},
    {u8"%s a bien reçu %s : c'est parti de ta maison.", u8"%sが\u200B%sを\u200B受け取りました。\u200B家から\u200B送り出されました。"},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"（アイテム1個を\u200B倉庫から\u200B出せなかったため、\u200Bあなたの\u200B家にも\u200B残っています）"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"（アイテム%u個を\u200B倉庫から\u200B出せなかったため、\u200Bあなたの\u200B家にも\u200B残っています）"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"%sへの\u200Bプレゼントは\u200B届きませんでした：%s。\u200B家からは\u200B何も\u200Bなくなっていません。"},
    {u8"Rien à donner dans ton rangement.", u8"倉庫に\u200B渡せる\u200Bものが\u200Bありません。"},
    {u8"Tape des lettres pour chercher un objet.", u8"文字を\u200B入力して\u200Bアイテムを\u200B検索できます。"},
    {u8"Recherche : %s_", u8"検索：%s_"},
    {u8"Aucun objet ne correspond.", u8"一致する\u200Bアイテムが\u200Bありません。"},
    {u8"(page %u sur %u)", u8"（%u/%uページ）"},
    {u8"Butin : %s.", u8"戦利品：%s。"},
    {u8"Besoin", u8"必要"},
    {u8"Cupidité", u8"欲張り"},
    {u8"Passer", u8"パス"},
    {u8"%u s", u8"%u秒"},
    {u8"toi", u8"あなた"},
    {u8"Joueur %u", u8"プレイヤー%u"},
    {u8"Un objet", u8"アイテム"},
    {u8"Partage du butin : objet %u sur %u", u8"戦利品の分配：アイテム %u/%u"},
    {u8"Besoin : %s", u8"必要：%s"},
    {u8"Cupidité : %s", u8"欲張り：%s"},
    {u8"Passer : %s", u8"パス：%s"},
    {u8"Partage du butin", u8"戦利品の分配"},
    {u8"%u objets du sac à partager entre %s.", u8"リュックの\u200Bアイテム%u個を\u200B%sで\u200B分配します。"},
    {u8"%u objet du sac à partager entre %s.", u8"リュックの\u200Bアイテム%u個を\u200B%sで\u200B分配します。"},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"アイテムごとに\u200B「必要」（使う）、\u200B「欲張り」（誰も\u200B必要と\u200Bしないなら）\n"
     u8"または\u200B「パス」を\u200B選びます。\u200B100面ダイスで\u200B決着します。"},
    {u8"En attente de %s...", u8"%sを\u200B待っています..."},
    {u8"En attente de %s... (%s)", u8"%sを\u200B待っています...（%s）"},
    {u8"Tout le monde est là : ça commence !", u8"全員\u200Bそろいました。\u200B始めます！"},
    {u8"Commencer", u8"開始"},
    {u8"D'accord", u8"ＯＫ"},
    {u8"Le veux-tu ? (%s)", u8"欲しいですか？（%s）"},
    {u8"Je le veux", u8"欲しい！"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"必要：\u200B使う\u200Bアイテム。\u200Bほかの\u200B人より\u200B優先されます。\n"
     u8"欲張り：\u200B誰も\u200B必要と\u200Bしなければ\u200Bもらえます。\n"
     u8"（%s）"},
    {u8"Ton choix : %s.", u8"あなたの選択：%s。"},
    {u8"Ton choix est fait.", u8"選択しました。"},
    {u8"En attente de %s (%s).", u8"%sを\u200B待っています（%s）。"},
    {u8"Le résultat arrive...", u8"まもなく\u200B結果が\u200B出ます..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%sが\u200Bいないため、\u200B順番を\u200B飛ばします。"},
    {u8"%s n'est pas là : son tour passe.", u8"%sが\u200Bいないため、\u200B順番を\u200B飛ばします。"},
    {u8"Personne n'en veut : il reste chez toi.", u8"希望者が\u200Bいないため、\u200Bあなたの\u200B手元に\u200B残ります。"},
    {u8"Personne n'en veut : il reste chez %s.", u8"希望者が\u200Bいないため、\u200B%sの\u200B手元に\u200B残ります。"},
    {u8"Tu l'emportes (%s) !", u8"あなたが\u200B獲得しました（%s）！"},
    {u8"%s l'emporte (%s).", u8"%sが\u200B獲得しました（%s）。"},
    {u8"Dés : %s (égalité départagée).", u8"ダイス：%s（同点のため\u200B振り直し）。"},
    {u8"Dés : %s.", u8"ダイス：%s。"},
    {u8"Partage du butin terminé", u8"戦利品の分配が完了"},
    {u8"Tu remportes : %s.", u8"あなたの獲得：%s。"},
    {u8"%s remporte : %s.", u8"%sの獲得：%s。"},
    {u8"Remporté, mais resté chez %s : %s.", u8"獲得（%sの\u200B手元に\u200B残っています）：%s。"},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"希望者なし（あなたの\u200B手元に\u200B残ります）：%s。"},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"希望者なし（%sの\u200B手元に\u200B残ります）：%s。"},
    {u8"Aucun objet n'a été partagé.", u8"分配された\u200Bアイテムは\u200Bありません。"},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"セーブデータは\u200Bすでに\u200B開いています。\u200B次の\u200Bウィンドウで、\n"
     u8"戦利品ごと\u200B再ロードする\u200B方法を\u200B説明します。"},
    {u8"Ton butin t'attend dans ton rangement, chez toi.", u8"戦利品は\u200B家の\u200B倉庫で\u200Bあなたを\u200B待っています。"},
    {u8"Ce que les autres remportent quitte ton rangement.", u8"ほかの\u200B人が\u200B獲得した\u200Bものは、\u200Bあなたの\u200B倉庫から\u200Bなくなります。"},
    {u8"Dr Beanies", u8"Dr. ビーニーズ"},
    {u8"Butch", u8"ブッチ"},
    {u8"Tink", u8"ティンク"},
    {u8"Frank", u8"フランク"},
    {u8"Bébé Jack", u8"ジャック"},
    {u8"Tracy", u8"トレイシー"},
    {u8"Broyeur d'organe", u8"死体\u200Bひろい"},
    {u8"Steven", u8"スティーブン"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"報酬の準備完了\n"
     u8"\n"
     u8"%sが\u200B%sに\u200B贈った\u200Bネコが\u200Bゲージを\n"
     u8"満たしました。\u200Bあなたが\u200B家に\u200B戻りしだい、\n"
     u8"その人の\u200B代わりに\u200B報酬を\u200B受け取ります。\u200Bコイン、\n"
     u8"フード、\u200Bアイテムは\u200B%sへ、\n"
     u8"アンロックは\u200B全員の\u200Bものに\u200Bなります。"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"%sからの報酬\n"
     u8"\n"
     u8"%sが\u200B贈った\u200Bネコの\u200B報酬を\u200B本人に\u200B送れませんでした。\n"
     u8"報酬は\u200Bあなたの\u200B手元に\u200B残ります。"},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"%sへの報酬\n"
     u8"\n"
     u8"%sが、\u200B%sの\u200B贈った\u200Bネコに\n"
     u8"報酬を\u200Bくれました。\u200B%sは\u200Bその人に\u200B送られます。\n"
     u8"アンロックは\u200B全員の\u200Bものです。"},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"全員への報酬\n"
     u8"\n"
     u8"%sが、\u200B%sの\u200B贈った\u200Bネコに\n"
     u8"報酬を\u200Bくれました。\u200Bアンロックは\n"
     u8"全員の\u200Bものです。"},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%sが\u200Bネコの\u200Bお礼を\u200Bくれました：%s\n"
     u8"（アンロックは\u200B全員の\u200Bものです）"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"%sからの報酬\n"
     u8"\n"
     u8"%sが\u200Bネコの\u200Bお礼を\u200Bくれました：%s。\n"
     u8"次に\u200Bセーブデータを\u200B読み込んだとき\n"
     u8"（冒険から\u200B戻ったとき）、\u200B家に\u200B届きます。\n"
     u8"アンロックは\u200B全員の\n"
     u8"ものです。"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"%sからの報酬\n"
     u8"\n"
     u8"あなたの\u200Bネコが\u200B%sの\u200Bゲージを\u200B満たしました。\n"
     u8"報酬は\u200Bアンロックで、\u200B全員の\u200Bものです。\n"
     u8"セーブデータを\u200B次に\u200B読み込んだとき、\n"
     u8"届きます。"},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"%sからの報酬\n"
     u8"\n"
     u8"%sは\u200B%sの\u200Bゲージを\u200B満たしましたが、\n"
     u8"もう\u200Bいません。\u200B報酬は\u200Bあなたの\u200B手元に\u200B残ります。"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"報酬の準備完了\n"
     u8"\n"
     u8"あなたの\u200Bネコが\u200B%sの\u200Bゲージを\u200B満たしました！\n"
     u8"%sが\u200B家に\u200B戻りしだい、\n"
     u8"あなたの\u200B代わりに\u200B報酬を\u200B受け取ります。"},
    {u8"les bourses ne marchent pas dans cette partie", u8"この\u200Bゲームでは\u200Bサイフが\u200B使えません"},
    {u8"rien à donner", u8"渡すものが\u200Bありません"},
    {u8"c'est trop d'un coup", u8"一度に\u200B渡せる\u200B量を\u200B超えています"},
    {u8"choisis un autre joueur", u8"ほかの\u200Bプレイヤーを\u200B選んでください"},
    {u8"ton don précédent attend encore la réponse de l'hôte", u8"前の\u200Bプレゼントが\u200Bまだ\u200Bホストの\u200B返事待ちです"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"この\u200B冒険では\u200Bサイフが\u200Bありません"},
    {u8"envoi impossible (connexion)", u8"送信できません（接続）"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"冒険終了\n"
     u8"\n"
     u8"それぞれ\u200B自分の\u200Bサイフを\u200B家に\u200B持ち帰ります："},
    {u8"%s n'est plus là : sa bourse reste chez toi.", u8"%sは\u200Bもう\u200Bいません。\u200Bその\u200Bサイフは\u200Bあなたの\u200B手元に\u200B残ります。"},
    {u8"%s : rien.", u8"%s：なし。"},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.", u8"%sに\u200Bサイフを\u200B渡せませんでした。\u200Bあなたの\u200B手元に\u200B残ります。"},
    {u8"%s : %s.", u8"%s：%s。"},
    {u8"Toi : %s.", u8"あなた：%s。"},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"プレゼントへの\u200Bホストの\u200B返事が\u200Bまだ\u200Bありません。\u200B道選びの\u200Bウィンドウで\u200Bサイフを\u200B確認してください。"},
    {u8"Ton don n'est pas passé : %s.", u8"プレゼントは\u200B届きませんでした：%s。"},
    {u8"(et d'autres trouvailles)", u8"（ほかにも\u200B見つけた\u200Bものが\u200Bあります）"},
    {u8"pas de bourse pour ce joueur dans cette aventure", u8"この\u200B冒険では、\u200Bこの\u200Bプレイヤーの\u200Bサイフが\u200Bありません"},
    {u8"ta bourse n'en a pas autant", u8"サイフの\u200B中身が\u200B足りません"},
    {u8"l'aventure n'est plus suivie", u8"冒険の\u200B同期が\u200B切れています"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%sから\u200B%sを\u200Bもらい、\u200Bサイフに\u200B入りました。"},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"%sを\u200B%sに\u200B渡しました。\u200B相手の\u200Bサイフに\u200B入っています。"},
    {u8"%s a donné %s à %s.", u8"%sが\u200B%sを\u200B%sに\u200B渡しました。"},
    {u8"Un chat", u8"あるネコ"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%sが\u200B%sを\u200B見つけ、\u200B%sを\u200B失いました：\u200Bあなたの\u200Bサイフに\u200B反映されます。"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%sが\u200B%sを\u200B見つけ、\u200B%sを\u200B失いました：\u200B%sの\u200Bサイフに\u200B反映されます。"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%sが\u200B%sを\u200B見つけ、\u200B%sを\u200B失いました：\u200B%sの\u200Bサイフに\u200B反映されます。"},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%sが\u200B%sを\u200B発見：\u200Bあなたの\u200Bサイフに\u200B入ります。"},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%sが\u200B%sを\u200B発見：\u200B%sの\u200Bサイフに\u200B入ります。"},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%sが\u200B%sを\u200B発見：\u200B%sの\u200Bサイフに\u200B入ります。"},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%sが\u200B%sを\u200B使いました：\u200Bあなたの\u200Bサイフから\u200B引かれます。"},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.", u8"%sが\u200B%sを\u200B使いました：\u200B%sの\u200Bサイフから\u200B引かれます。"},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.", u8"%sが\u200B%sを\u200B使いました：\u200B%sの\u200Bサイフから\u200B引かれます。"},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%sが\u200B%sを\u200B失いました：\u200Bあなたの\u200Bサイフから\u200B引かれます。"},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%sが\u200B%sを\u200B失いました：\u200B%sの\u200Bサイフから\u200B引かれます。"},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%sが\u200B%sを\u200B失いました：\u200B%sの\u200Bサイフから\u200B引かれます。"},
    {u8"%s n'a rien trouvé.", u8"%sは\u200B何も\u200B見つけませんでした。"},
    {u8"un objet", u8"アイテム"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.", u8"%sを\u200B%sで\u200B買いました：\u200Bサイフから\u200B引かれます。"},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.", u8"%sが\u200B%sを\u200B%sで\u200B買いました：\u200B本人の\u200Bサイフから\u200B引かれます。"},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"%sを\u200B買いました：\u200Bサイフに\u200B%sが\u200B入ります。"},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%sが\u200B%sを\u200B買いました：\u200B本人の\u200Bサイフに\u200B%sが\u200B入ります。"},
    {u8"Tu as acheté %s (gratuit).", u8"%sを\u200B買いました（無料）。"},
    {u8"%s a acheté %s (gratuit).", u8"%sが\u200B%sを\u200B買いました（無料）。"},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"バトルで、\u200Bあなたの\u200Bネコが\u200B%sを\u200B見つけ、\u200B%sを\u200B失いました：\u200Bサイフに\u200B反映されます。"},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"バトルで、\u200Bあなたの\u200Bネコが\u200B%sを\u200B見つけました：\u200Bサイフに\u200B入ります。"},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"バトルで、\u200Bあなたの\u200Bネコが\u200B%sを\u200B失いました：\u200Bサイフから\u200B引かれます。"},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"バトルでは、\u200Bあなたの\u200Bネコの\u200B収穫は\u200Bなし。"},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"バトルで、\u200B%sの\u200Bネコが\u200B%sを\u200B見つけ、\u200B%sを\u200B失いました：\u200B本人の\u200Bサイフに\u200B反映されます。"},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"バトルで、\u200B%sの\u200Bネコが\u200B%sを\u200B見つけ、\u200B%sを\u200B失いました：\u200B本人の\u200Bサイフに\u200B反映されます。"},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"バトルで、\u200B%sの\u200Bネコが\u200B%sを\u200B見つけました：\u200B本人の\u200Bサイフに\u200B入ります。"},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"バトルで、\u200B%sの\u200Bネコが\u200B%sを\u200B見つけました：\u200B本人の\u200Bサイフに\u200B入ります。"},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"バトルで、\u200B%sの\u200Bネコが\u200B%sを\u200B失いました：\u200B本人の\u200Bサイフから\u200B引かれます。"},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"バトルで、\u200B%sの\u200Bネコが\u200B%sを\u200B失いました：\u200B本人の\u200Bサイフから\u200B引かれます。"},
    {u8"En combat, les chats d'%s n'ont rien trouvé.", u8"バトルでは、\u200B%sの\u200Bネコの\u200B収穫は\u200Bなし。"},
    {u8"En combat, les chats de %s n'ont rien trouvé.", u8"バトルでは、\u200B%sの\u200Bネコの\u200B収穫は\u200Bなし。"},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"リュックが開いています\n"
     u8"\n"
     u8"ホストが\u200Bマスに\u200B入りました。\n"
     u8"リュックを\u200B閉じて\u200B追いかけてください。"},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"アイテム\n"
     u8"\n"
     u8"このネコは\u200B%sの\u200Bネコです。\u200B持ち主しか\n"
     u8"アイテムを\u200B変更できません。"},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"アイテム\n"
     u8"\n"
     u8"マスの\u200B中（ショップ、\u200Bレベルアップ\u200Bなど）では、\n"
     u8"アイテムを\u200B変更できるのは\u200Bホストだけです。\n"
     u8"ネコの\u200B装備は\u200Bマップで\u200B行ってください。"},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"アイテム\n"
     u8"\n"
     u8"ホストが\u200Bまだ\u200Bマップに\u200B戻っていません。\n"
     u8"待っていれば、\u200Bリュックは\u200B自動で\u200B使えるように\u200Bなります。"},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"アイテム\n"
     u8"\n"
     u8"リュックを\u200Bホストの\u200Bものと\u200B同期しています。\n"
     u8"一度\u200B閉じて、\u200B少し\u200Bしてから\u200B開き直してください。"},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"アイテム\n"
     u8"\n"
     u8"ほかの\u200Bプレイヤーが\u200B直前に\u200Bこの\u200Bアイテムを\u200B取ったため、\n"
     u8"変更は\u200B反映されませんでした。\u200Bリュックは\u200B閉じると\n"
     u8"ホストと\u200B同じ\u200B状態に\u200B戻ります。"},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"アイテム\n"
     u8"\n"
     u8"最後の\u200B変更が\u200B届く\u200B前に\u200Bホストが\u200Bマスに\n"
     u8"入ったため、\u200B変更は\u200B取り消されました。\n"
     u8"マップで\u200Bやり直してください。"},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"アイテム\n"
     u8"\n"
     u8"この\u200Bアイテムは\u200Bもう\u200Bありません。\n"
     u8"ほかの\u200Bプレイヤーが\u200B直前に\u200B取りました。"},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).",
     u8"%sは\u200Bもう\u200Bいません。\u200B買った\u200Bものは\u200Bあなたの\u200B手元に\u200B残ります（%s）。"},
    {u8"%s rapporte ses achats : %s.", u8"%sが\u200B買った\u200Bものを\u200B持ち帰ります：%s。"},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%sが\u200B買った\u200Bものを\u200B送れませんでした。\u200Bあなたの\u200B手元に\u200B残ります。"},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"%sが\u200B買った\u200Bものを\u200B送れませんでした。\u200Bあなたの\u200B手元に\u200B残ります。"},
    {u8"Achats de l'aventure", u8"冒険中の買い物"},
    {u8"Boutique", u8"ショップ"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%sが\u200Bまだ\u200Bこの\u200Bショップで\n"
     u8"買い物中です。\u200B待ってください（出ると、\u200B代わりに\n"
     u8"買い物が\u200Bできなくなります）。\n"
     u8"もう一度\u200Bクリックすると、\u200Bそのまま\u200B出ます。"},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%sが\u200Bまだ\u200Bこの\u200Bショップで\n"
     u8"買い物中です。\u200B待ってください（出ると、\u200B代わりに\n"
     u8"買い物が\u200Bできなくなります）。\n"
     u8"もう一度\u200Bクリックすると、\u200Bそのまま\u200B出ます。"},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"ホストに\u200B購入を\u200B依頼できませんでした。\n"
     u8"何も\u200B買っていません。\u200B少し\u200Bしてから\u200Bもう一度\u200B試してください。"},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"ホストは\u200Bもう\u200Bこの\u200Bショップに\u200Bいません。\n"
     u8"購入は\u200Bできませんでした。"},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"この\u200Bアイテムは\u200Bもう\u200Bホストの\u200Bショップに\u200Bありません。\n"
     u8"購入は\u200Bできませんでした。"},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"この\u200Bアイテムは\u200B売れて\u200Bしまいました。\n"
     u8"購入は\u200Bできませんでした。"},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"価格は%s、\u200Bサイフには\u200B%lld枚しか\u200Bありません。"},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"この\u200Bアイテムを\u200B買うには\u200Bサイフの\u200B中身が\u200B足りません。"},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"（あなたも\u200Bみんなも\u200B自分の\u200Bサイフで\u200B払います）"},
    {u8"(Chacun paie avec sa bourse.)", u8"（それぞれ\u200B自分の\u200Bサイフで\u200B払います）"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"今のところ、\u200Bアメと\u200Bオトモを\u200B買えるのは\n"
     u8"ホストだけです（本人の\u200Bサイフで）。"},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.", u8"ホストの\u200Bゲームで\u200B購入できませんでした。"},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"この\u200B冒険では\u200Bサイフが\u200Bありません。\n"
     u8"買えるのは\u200Bホストだけです。"},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"この\u200B冒険には\u200Bネコを\u200B連れてきていないため、\n"
     u8"買い物に\u200B使う\u200Bサイフが\n"
     u8"ありません。"},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"この\u200B冒険では\u200Bサイフが\u200Bありません\n"
     u8"（ネコを\u200B連れてきていないため）。\u200B購入できません。"},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"今は\u200Bホストが\u200Bあなたの\u200B代わりに\u200B買えません\n"
     u8"（ホストの\u200B画面で\u200Bウィンドウが\u200B開いています）。\n"
     u8"何も\u200B買っていません。\u200B少し\u200Bしてから\u200Bもう一度\u200B試してください。"},
    {u8"un bonbon", u8"アメ"},
    {u8"un familier", u8"オトモ"},
    {u8"un article", u8"商品"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"冒険終了\n"
     u8"\n"
     u8"%sは\u200B%sの\u200B家に\u200B帰ります。"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"冒険終了\n"
     u8"\n"
     u8"%sは\u200B%sの\u200B家に\u200B帰ります。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"冒険に出発\n"
     u8"\n"
     u8"ほかの\u200Bプレイヤーの\u200Bネコが\u200B出発できず、\n"
     u8"冒険ボックスも\u200B空なので、\u200B今回の\u200B冒険は\u200Bありません。\n"
     u8"（理由は\u200BMODの\u200Bログ、\u200Bmgmp.logに\u200Bあります）"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"冒険に出発\n"
     u8"\n"
     u8"冒険ボックスが\u200B空で、\u200B誰も\u200Bネコを\u200B連れてこないため、\n"
     u8"今回の\u200B冒険は\u200Bありません。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"混成チーム\n"
     u8"\n"
     u8"ほかの\u200Bプレイヤーの\u200Bネコが\u200Bチームに\n"
     u8"加われませんでした。\u200B自分の\u200Bネコだけで\u200B出発します。\n"
     u8"（理由は\u200BMODの\u200Bログ、\u200Bmgmp.logに\u200Bあります）"},
    {u8"Retour de l'aventure", u8"冒険から帰還"},
    {u8"%s est de retour à la maison.", u8"%sが\u200B家に\u200B帰ってきました。"},
    {u8"%s sont de retour à la maison.", u8"%sが\u200B家に\u200B帰ってきました。"},
    {u8"%s a péri pendant l'aventure.", u8"%sは\u200B冒険中に\u200B命を\u200B落としました。"},
    {u8"%s ont péri pendant l'aventure.", u8"%sは\u200B冒険中に\u200B命を\u200B落としました。"},
    {u8"%s ne revient pas de l'aventure.", u8"%sは\u200B冒険から\u200B戻りません。"},
    {u8"%s ne reviennent pas de l'aventure.", u8"%sは\u200B冒険から\u200B戻りません。"},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%sは\u200B出発後に\u200Bデータが\u200B変わったため、\n"
     u8"冒険の\u200B結果を\u200B反映できませんでした。"},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%sは\u200B出発後に\u200Bデータが\u200B変わったため、\n"
     u8"冒険の\u200B結果を\u200B反映できませんでした。"},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"出発後に\u200B倉庫が\u200B変わったため、\n"
     u8"出発時に\u200B装備した\u200Bアイテムは\u200B倉庫から\u200B取り除かれていません。"},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"倉庫は\u200B変更されていません。\u200B出発時に\n"
     u8"装備した\u200Bアイテムは\u200Bまだ\u200B倉庫に\u200Bあります。"},
    {u8"Tu rapportes %s dans ta maison.", u8"%sを\u200B家に\u200B持ち帰りました。"},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.", u8"NPCから\u200Bネコの\u200Bお礼：\u200B%sが\u200B家に\u200B届きました。"},
    {u8"Cadeaux des PNJ", u8"NPCからの\u200Bプレゼント"},
    {u8"Ton butin est dans ton rangement : %s.", u8"戦利品は\u200B倉庫に\u200Bあります：%s。"},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.", u8"NPCからの\u200Bプレゼントは\u200B倉庫に\u200Bあります：%s。"},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.", u8"冒険中に\u200B仲間に\u200Bなった\u200B%sが\u200B家に\u200Bやってきます。"},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.", u8"冒険中に\u200B仲間に\u200Bなった\u200B%sが\u200B家に\u200Bやってきます。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"混成チーム\n"
     u8"\n"
     u8"%sが\u200Bチームに\u200B加われませんでした。\n"
     u8"この\u200B冒険では、\u200Bチームの\u200Bネコの\u200B一部を\n"
     u8"あなたが\u200B操作します。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"混成チーム\n"
     u8"\n"
     u8"%sが\u200Bチームに\u200B加われませんでした。\n"
     u8"この\u200B冒険では、\u200Bチームの\u200Bネコの\u200B一部を\n"
     u8"あなたが\u200B操作します。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"混成チーム\n"
     u8"\n"
     u8"%sが\u200Bチームに\u200B加われませんでした。\n"
     u8"あなたは\u200Bネコを\u200B操作せずに\u200B冒険に\u200B同行します。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"混成チーム\n"
     u8"\n"
     u8"%sが\u200Bチームに\u200B加われませんでした。\n"
     u8"あなたは\u200Bネコを\u200B操作せずに\u200B冒険に\u200B同行します。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"混成チーム\n"
     u8"\n"
     u8"%sは\u200B出発できず、\u200B家に\u200B残ります。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"混成チーム\n"
     u8"\n"
     u8"%sは\u200B出発できず、\u200B家に\u200B残ります。"},
    {u8"Tes chats sont rentrés de l'aventure.", u8"あなたの\u200Bネコが\u200B冒険から\u200B帰ってきました。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"チームに新しい仲間\n"
     u8"\n"
     u8"%sが\u200B%sの\u200Bチームに\u200B加わりました。\n"
     u8"このネコは\u200Bあなたの\u200Bネコです！\u200Bあなたが\u200Bいない\u200B間は\n"
     u8"ゲームの\u200BAIが\u200Bバトルを\u200B行い、\u200B%sが\u200B代わりに\u200B選びます。\n"
     u8"冒険が\u200B終わると、\u200Bあなたの\u200B家に\u200B帰ってきます。"},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"新しい\u200Bネコ：\u200B%sが\u200Bあなたの\u200B家に\u200B来ます。"},
    {u8"Nouveau chat : %s arrive chez toi.", u8"新しい\u200Bネコ：\u200B%sが\u200Bあなたの\u200B家に\u200B来ます。"},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"4番目の枠\n"
     u8"\n"
     u8"3人プレイでは、\u200B空き枠が\u200B1つ\u200Bあります。\n"
     u8"あなたの\u200B枠です。\u200Bネコを\u200B2匹\u200B連れていけます。"},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"4番目の枠\n"
     u8"\n"
     u8"空き枠は\u200Bすでに\u200B%sが\u200B使っています。\n"
     u8"3人プレイでは、\u200Bネコは\u200B1匹です。\n"
     u8"冒険ボックスから\u200Bネコを\u200B1匹\u200B外してください。"},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"4番目の枠\n"
     u8"\n"
     u8"4人目の\u200Bプレイヤーが\u200B来ました：\u200Bそれぞれ\n"
     u8"ネコ1匹です。\u200B冒険ボックスから\u200Bネコを\u200B1匹\u200B外してください。"},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"4番目の\u200B枠は、\u200B冒険ボックスに\u200B2匹目の\u200Bネコを\u200B入れた\u200B最初の\u200B人の\u200Bものです。"},
    {u8"La 4e place est à %s.", u8"4番目の\u200B枠は\u200B%sの\u200Bものです。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混成チーム\n"
     u8"\n"
     u8"%d人プレイでは、\u200B連れていける\u200Bネコは\u200B最大%d匹です。\n"
     u8"冒険ボックスから\u200Bネコを\u200B%d匹\u200B外してください。%s\n"
     u8"残りは\u200Bほかの\u200Bプレイヤーの\u200Bネコで\u200B埋まります。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混成チーム\n"
     u8"\n"
     u8"%d人プレイでは、\u200B連れていける\u200Bネコは\u200B最大%d匹です。\n"
     u8"冒険ボックスから\u200Bネコを\u200B%d匹\u200B外してください。%s\n"
     u8"残りは\u200Bほかの\u200Bプレイヤーの\u200Bネコで\u200B埋まります。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混成チーム\n"
     u8"\n"
     u8"%d人プレイでは、\u200B連れていける\u200Bネコは\u200B最大%d匹です。\n"
     u8"冒険ボックスから\u200Bネコを\u200B%d匹\u200B外してください。%s\n"
     u8"残りは\u200Bほかの\u200Bプレイヤーの\u200Bネコで\u200B埋まります。"},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"混成チーム\n"
     u8"\n"
     u8"%d人プレイでは、\u200B連れていける\u200Bネコは\u200B最大%d匹です。\n"
     u8"冒険ボックスから\u200Bネコを\u200B%d匹\u200B外してください。%s\n"
     u8"残りは\u200Bほかの\u200Bプレイヤーの\u200Bネコで\u200B埋まります。"},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"自分の\u200B家に\u200Bいないため、\n"
     u8"ほかの\u200Bプレイヤーの\u200Bネコで\u200B遊びます。"},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"冒険ボックスが\u200Bまだ\u200Bアンロックされていないため、\n"
     u8"ほかの\u200Bプレイヤーの\u200Bネコで\u200B遊びます。"},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"冒険ボックスが\u200B空なので、\u200Bほかの\u200Bプレイヤーの\u200Bネコで\u200B遊びます。\n"
     u8"自分の\u200Bネコを\u200B連れていくなら：\u200B「まだです」を\u200B選び、\u200Bネコを\u200B1、2匹\u200B冒険ボックスに\u200B入れて\n"
     u8"（ブッチの\u200Bところ）、\u200B自分の\u200B「出発！」の\u200B看板を\u200Bクリックしてください。"},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"冒険ボックスが\u200B空なので、\u200Bほかの\u200Bプレイヤーの\u200Bネコで\u200B遊びます。\n"
     u8"自分の\u200Bネコを\u200B連れていくなら：\u200B「まだです」を\u200B選び、\u200Bネコを\u200B1匹\u200B冒険ボックスに\u200B入れて\n"
     u8"（ブッチの\u200Bところ）、\u200B自分の\u200B「出発！」の\u200B看板を\u200Bクリックしてください。"},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"冒険ボックスに\u200Bネコが\u200B%d匹\u200Bいます（最大%d匹）。\n"
     u8"「まだです」と\u200B答えて\u200B%d匹\u200B外し、\n"
     u8"自分の\u200B「出発！」の\u200B看板を\u200Bクリックしてください。%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).", u8"あなたの\u200Bネコは\u200B冒険ボックス（ブッチの\u200Bところ）から\u200B出発します。"},
    {u8"Tu emmènes %s.", u8"%sを\u200B連れていきます。"},
    {u8"Départ en aventure", u8"冒険に出発"},
    {u8"La session n'est pas prête : réessaie dans un instant.", u8"セッションの\u200B準備中です。\u200B少し\u200B待って\u200Bもう一度\u200B試してください。"},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"4番目の\u200B枠を\u200Bあなたに\u200B割り当てています。\n"
     u8"まもなく\u200Bもう一度\u200B確認が\u200B表示されます。"},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"冒険に\u200B向けた\u200Bネコの\u200B準備に\n"
     u8"失敗しました（mgmp.logを\u200B参照）。"},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%sは\u200B冒険から\u200B戻りましたが、\u200Bその\u200B結果が\n"
     u8"まだ\u200Bセーブデータに\u200B反映されていません。\u200B再出発の\u200B前に\n"
     u8"再ロードしてください（この\u200B出発の\u200Bあとに\u200B「再ロード」が\n"
     u8"表示されます。\u200Bまたは\u200BEsc、\u200B「セーブして\u200Bメニューへ」、\n"
     u8"「プレイ」で\u200Bセーブデータを\u200B選択）。"},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%sは\u200B冒険から\u200B戻りましたが、\u200Bその\u200B結果が\n"
     u8"まだ\u200Bセーブデータに\u200B反映されていません。\u200B再出発の\u200B前に\n"
     u8"再ロードしてください（この\u200B出発の\u200Bあとに\u200B「再ロード」が\n"
     u8"表示されます。\u200Bまたは\u200BEsc、\u200B「セーブして\u200Bメニューへ」、\n"
     u8"「プレイ」で\u200Bセーブデータを\u200B選択）。"},
    {u8"Le retour de %s attend d'être recopié.", u8"%sの\u200B冒険の\u200B結果は\u200Bまだ\u200B反映待ちです。"},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"ホストへの\u200B返事を\u200B送れませんでした。\u200Bもう一度\n"
     u8"試してください"},
    {u8"Ta bourse de l'aventure : %s.", u8"あなたの\u200B冒険の\u200Bサイフ：%s。"},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"冒険ボックスに\u200Bネコが\u200B%d匹\u200Bいます（最大%d匹）。\n"
     u8"%d匹\u200B外してから、\u200B自分の\u200B「出発！」の\u200B看板を\u200Bクリックしてください。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"チームに新しい仲間\n"
     u8"\n"
     u8"%sが\u200B仲間に\u200Bなりました。\u200Bあなたの\u200Bネコです！\n"
     u8"あなたが\u200B操作し、\u200B冒険が\u200B終わると\n"
     u8"あなたの\u200B家に\u200B帰ってきます。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"チームに新しい仲間\n"
     u8"\n"
     u8"%sが\u200B仲間に\u200Bなりました。\u200Bあなたの\u200Bネコです。"},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"チームに新しい仲間\n"
     u8"\n"
     u8"%sが\u200B仲間に\u200Bなりました。\u200B%sの\u200Bネコです。"},
    {u8"Un nouveau chat", u8"新しいネコ"},
    {u8"en bas", u8"下"},
    {u8"en haut", u8"上"},
    {u8"à droite", u8"右"},
    {u8"à gauche", u8"左"},
    {u8"au milieu", u8"中央"},
    {u8"n°%d", u8"%d番"},
    {u8"%s (%s)", u8"%s（%s）"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]あなたの\u200B選択は\u200B反映されませんでした：%s。[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"道選び\n"
     u8"\n"},
    {u8"Même chemin", u8"同じ道"},
    {u8"Choisir", u8"選ぶ"},
    {u8"Changer", u8"変更"},
    {u8"Sortie", u8"出口"},
    {u8"Retour à la maison", u8"帰宅"},
    {u8"Combat", u8"バトル"},
    {u8"Combat difficile", u8"強敵バトル"},
    {u8"Mini-boss", u8"ミニボス"},
    {u8"Boss", u8"ボス"},
    {u8"Événement", u8"イベント"},
    {u8"Événement spécial", u8"特別イベント"},
    {u8"Trésor", u8"お宝"},
    {u8"Meuble", u8"家具"},
    {u8"Nourriture", u8"フード"},
    {u8"Bonus", u8"ボーナス"},
    {u8"Case vide", u8"空きマス"},
    {u8"Case", u8"マス"},
    {u8"l'hôte n'est pas sur la carte", u8"ホストが\u200Bマップに\u200Bいません"},
    {u8"vos cartes ne sont pas les mêmes", u8"マップが\u200B一致しません"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"ホストは\u200Bその\u200Bマスに\u200B行けません"},
    {u8"le chemin est déjà choisi", u8"道は\u200Bもう\u200B決まっています"},
    {u8"ce tour est fini", u8"この\u200B回は\u200B終わりました"},
    {u8"l'hôte ne l'a pas pris", u8"ホストが\u200B受け付けませんでした"},
    {u8"pas de vote", u8"選択なし"},
    {u8"absent, ne vote pas", u8"不在のため\u200B選択なし"},
    {u8"pas encore sur la carte", u8"まだ\u200Bマップに\u200Bいません"},
    {u8"pas encore voté", u8"まだ\u200B選んでいません"},
    {u8"%s (toi) : %s", u8"%s（あなた）：%s"},
    {u8"%s : %s", u8"%s：%s"},
    {u8"En pause : on attend tout le monde sur la carte.", u8"一時停止中：\u200B全員が\u200Bマップに\u200Bそろうのを\u200B待っています。"},
    {u8"À toi de choisir : encore %u s.", u8"選んでください：\u200B残り%u秒。"},
    {u8"En attente des autres : encore %u s.", u8"ほかの\u200B人を\u200B待っています：\u200B残り%u秒。"},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"%sが\u200B同数です。\n"
     u8"くじ引きで\u200B%sに\u200B決まりました。"},
    {u8"Tout le monde a choisi %s.", u8"全員が\u200B%sを\u200B選びました。"},
    {u8"%s l'emporte.", u8"%sに\u200B決まりました。"},
    {u8"%s : prêt", u8"%s：準備完了"},
    {u8"%s : défend sa maison", u8"%s：家を防衛中"},
    {u8"%s : pas encore", u8"%s：まだです"},
    {u8"Tout le monde est prêt !", u8"全員\u200B準備完了！"},
    {u8"En attente d'un joueur.", u8"プレイヤー1人を\u200B待っています。"},
    {u8"En attente de %d joueurs.", u8"プレイヤー%d人を\u200B待っています。"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"1日を\u200B終える\n"
     u8"\n"
     u8"夜を\u200B迎える\u200B準備が\u200Bできました。\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"あなたの\u200B看板は\u200B今\u200B「出発！」です。\n"
     u8"1日を\u200B終えるには、\u200Bチームから\u200Bネコを\u200B外してください。"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"1日を\u200B終える\n"
     u8"\n"
     u8"1人が\u200B夜を\u200B迎えるのを\u200B待っています。\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"1日を\u200B終える\n"
     u8"\n"
     u8"%d人が\u200B夜を\u200B迎えるのを\u200B待っています。\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1人が待っています"},
    {u8"%d joueurs attendent", u8"%d人が待っています"},
    {u8"Passer la nuit", u8"夜を迎える"},
    {u8"Plus tard", u8"あとで"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"続けるには、\u200BEscを\u200B押して\n"
     u8"「セーブして\u200Bメニューへ」を\u200B選んでください。"},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"受け取るもの\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"セーブデータは\u200Bすでに\u200B開いているため、\u200Bこれらは\n"
     u8"次に\u200B読み込んだときに\u200B反映されます。\u200B「再ロード」：\n"
     u8"MODが\u200Bメニュー経由で\u200B開き直します。"},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"受け取るもの\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"セーブデータは\u200Bすでに\u200B開いているため、\u200Bこれらは\n"
     u8"次に\u200B読み込んだときに\u200B反映されます。\n"
     u8"再ロード：Esc、\u200B「セーブして\u200Bメニューへ」、\n"
     u8"「プレイ」で\u200Bセーブデータを\u200B選択。"},
    {u8"%s : j'arrive !", u8"%s：行きます！"},
    {u8"%s : réfléchit...", u8"%s：考え中..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"冒険に出発\n"
     u8"\n"
     u8"冒険への\u200B出発を\u200B呼びかけました。\n"
     u8"チームの\u200B返事：\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s：まだ\u200B来ていません"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"チームを待っています\n"
     u8"\n"
     u8"%s\n"
     u8"待たずに\u200B進むと、\u200Bまだ\u200B来ていない\u200B人の\u200Bネコは\n"
     u8"到着するまで\u200Bバトルで\u200Bゲームの\u200BAIが\n"
     u8"操作します。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"冒険に出発\n"
     u8"\n"
     u8"%sが\u200B冒険に\u200B出発します！\n"
     u8"それぞれ\u200B自分の\u200Bネコを\u200B連れていき、\u200B操作します。\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"冒険中\n"
     u8"\n"
     u8"%sが\u200B冒険中です！\n"
     u8"あなたの\u200Bネコは\u200B参加していないので、\n"
     u8"あなたは\u200Bネコを\u200B操作せずに\u200B冒険に\u200B同行します。"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"冒険中\n"
     u8"\n"
     u8"%sが\u200Bあなたの\u200Bネコと\u200B冒険中です！\n"
     u8"あなたが\u200Bいない\u200B間は、\u200Bバトルを\u200Bゲームの\u200BAIが\n"
     u8"行い、\u200B%sが\u200B代わりに\u200B選びます。\n"
     u8"チームに\u200B参加しますか？"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"冒険中\n"
     u8"\n"
     u8"%sが\u200B冒険中です！\n"
     u8"チームに\u200B参加しますか？"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"冒険から抜ける\n"
     u8"\n"
     u8"%sの\u200B冒険は\u200B終わりました。\n"
     u8"あなたは\u200B家に\u200B帰ります。"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"冒険から抜ける\n"
     u8"\n"
     u8"これは\u200B%sの\u200B冒険です。\u200Bやめられるのは\n"
     u8"本人だけです。\u200B冒険は\u200Bあなた\u200B抜きで\u200B続きます。\n"
     u8"\n"
     u8"家に\u200B帰りますか？\u200B戻るときは、\n"
     u8"家の\u200B看板を\u200Bクリックしてください。"},
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
     u8"冒険から抜ける\n"
     u8"\n"
     u8"これは\u200B%sの\u200B冒険です。\u200Bやめられるのは\n"
     u8"本人だけです。\u200B冒険は\u200Bあなた\u200B抜きで\u200B続き、\n"
     u8"あなたの\u200Bネコは\u200Bチームに\u200B残ります。\u200Bバトルは\n"
     u8"ゲームの\u200BAIが\u200B操作し、\u200B最後に\n"
     u8"家に\u200B帰ってきます。\n"
     u8"\n"
     u8"家に\u200B帰りますか？\u200B戻るときは、\n"
     u8"家の\u200B看板を\u200Bクリックしてください。"},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"冒険に参加\n"
     u8"\n"
     u8"%sの\u200Bゲームから\u200B進行中の\u200B冒険を\u200B受信中です。\n"
     u8"その人が\u200Bマスで\u200B選択を\u200Bした\u200B直後なら\n"
     u8"（イベント、\u200Bレベルアップ）、\u200Bマップに\u200B戻った\n"
     u8"ときに\u200B合流します。"},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"冒険へ出発\n"
     u8"\n"
     u8"%sの\u200Bゲームを\u200B読み込みます。\n"
     u8"一緒に\u200B出発します。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"帰宅\n"
     u8"\n"
     u8"%sとの\u200B接続が\u200B切れました。\n"
     u8"家に\u200B帰ります。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"帰宅\n"
     u8"\n"
     u8"%sとの\u200B出発は\u200B取りやめに\u200Bなりました。\n"
     u8"家に\u200B帰ります。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"帰宅\n"
     u8"\n"
     u8"%sの\u200B冒険は\u200B終わりました。\n"
     u8"家に\u200B帰ります。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"帰宅\n"
     u8"\n"
     u8"%sの\u200Bバトルは\u200B終わっていましたが、\n"
     u8"あなたの\u200Bバトルは\u200B同期が\u200Bずれたまま\u200B続いていました。\n"
     u8"誰も\u200Bプレイしていないため、\u200Bあなたは\n"
     u8"家に\u200B戻りました。\u200Bネコは\u200Bチームに\u200B残ります。\n"
     u8"\n"
     u8"冒険に\u200B戻るときは、\n"
     u8"家の\u200B看板を\u200Bクリックしてください。"},
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
     u8"帰宅\n"
     u8"\n"
     u8"あなたの\u200Bバトルは\u200B終わりましたが、\u200B%sの\n"
     u8"バトルは\u200B続いていました：\n"
     u8"同期が\u200Bずれていたため、\u200Bあなたは\u200B家に\n"
     u8"戻りました。\u200Bネコは\u200Bチームに\u200B残り、\u200Bバトルが\n"
     u8"終わるまで\u200Bゲームの\u200BAIが\u200B操作します。\n"
     u8"\n"
     u8"冒険に\u200B戻るときは、\n"
     u8"家の\u200B看板を\u200Bクリックしてください。"},
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
     u8"帰宅\n"
     u8"\n"
     u8"%sが\u200B入った\u200Bマスに、\u200Bあなたの\u200Bゲームが\n"
     u8"ついていけませんでした。\u200B冒険の\u200B状態が\n"
     u8"食い違ったため、\u200Bあなたは\u200B家に\u200B戻りました。\n"
     u8"ネコは\u200Bチームに\u200B残り、\u200Bゲームの\u200BAIが\n"
     u8"操作します。\n"
     u8"\n"
     u8"冒険に\u200B戻るときは、\n"
     u8"家の\u200B看板を\u200Bクリックしてください。"},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"帰宅\n"
     u8"\n"
     u8"あなたの\u200Bバトルが\u200B%sの\u200Bバトルと\n"
     u8"ずれてしまい、\u200Bあなた\u200B抜きで\u200B続いていたため、\n"
     u8"家に\u200B戻りました。\u200Bネコは\u200Bチームに\u200B残り、\n"
     u8"ゲームの\u200BAIが\u200B操作します。\n"
     u8"\n"
     u8"冒険に\u200B戻るときは、\n"
     u8"家の\u200B看板を\u200Bクリックしてください。"},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"この\u200Bセーブデータは\u200B冒険の\u200B途中です。\n"
     u8"マルチプレイでは、\u200B冒険は\u200Bホストと\u200B一緒に\u200B行います。\n"
     u8"メニューに\u200B戻って\u200B別の\u200Bセーブデータを\u200B選ぶか、\n"
     u8"ソロで\u200B最後まで\u200B遊んでください。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"冒険に出発\n"
     u8"\n"
     u8"%sの\u200B冒険を\u200B読み込んでいます。"},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%sが\u200B冒険を\u200B再ロードしました。\n"
     u8"あなたも\u200B再ロードします。"},
    {u8"Rentrer", u8"家に帰る"},
    {u8"Rester", u8"残る"},
    {u8"Ne pas attendre", u8"待たずに進む"},
    {u8"Recharger", u8"再ロード"},
    {u8"J'arrive", u8"行きます！"},
    {u8"Pas encore", u8"まだです"},
    {u8"Échanger", u8"トレード"},
    {u8"Multijoueur", u8"マルチプレイ"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"これは\u200B%sの\u200B家の\u200Bパイプです。\n"
     u8"ネコを\u200BNPCに\u200B送るときは、\n"
     u8"自分の\u200B家から\u200B送ってください。"},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"NPCは\u200B%sの\u200Bゲームの\u200Bものです。\n"
     u8"この\u200Bバージョンの\u200Bゲームでは、\u200Bネコを\n"
     u8"パイプに\u200B送ることは\u200Bできません。"},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"チーム\u200B全員の\u200B負けに\u200Bなります。\n"
     u8"%sの\u200Bネコも\u200B帰ってきません。"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"%sに参加\n"
     u8"\n"
     u8"すでに\u200Bマルチプレイロビーに\u200Bいます。\n"
     u8"先に\u200B退出して（マルチプレイ→「退出」）、\n"
     u8"もう一度\u200B「ゲームに\u200B参加」を\u200Bクリックしてください。"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"%sに参加\n"
     u8"\n"
     u8"ゲームは\u200Bいったん\u200Bメインメニューに\u200B戻ります\n"
     u8"（「セーブして\u200Bメニューへ」と\u200B同じく\u200Bセーブされます）。\n"
     u8"そのあと\u200B%sの\u200Bロビーに\u200B参加します。\n"
     u8"\n"
     u8"「あとで」：\u200Bメインメニューに\u200B戻ったときに\n"
     u8"参加します。"},
    {u8"Y aller", u8"行く"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"%sに参加\n"
     u8"\n"
     u8"メインメニューに\u200B戻りしだい、\u200B%sの\n"
     u8"ロビーに\u200B参加します。\n"
     u8"（Esc→「セーブして\u200Bメニューへ」）"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"%sに参加\n"
     u8"\n"
     u8"今は\u200BSteamで\u200B接続できません。\n"
     u8"しばらく\u200Bしてから\u200B再試行するか、\u200Bアドレスで\u200B参加してください\n"
     u8"（マルチプレイ→「参加する」）。"},
    {u8"%s a annulé le départ.", u8"%sが\u200B出発を\u200B取りやめました。"},
    {u8"réseau local", u8"LAN"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"冒険に出発\n"
     u8"\n"
     u8"あなたの\u200Bネコは\u200Bすでに\u200B%sの\u200Bチームと\u200B出発しました。\n"
     u8"あなたが\u200Bいない\u200B間は、\u200Bバトルを\u200Bゲームの\u200BAIが\n"
     u8"行い、\u200B%sが\u200B代わりに\u200B選びます。\u200B冒険が\n"
     u8"終わると\u200Bあなたの\u200B家に\u200B帰ってきます。"},
    {u8"Internet", u8"インターネット"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%sは\u200B家を\u200B防衛中です：\u200B自分の\u200Bネコだけで\n"
     u8"戦う\u200Bバトルです。\u200B終わると\u200B戻ってきます。"},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%sは\u200B家を\u200B防衛中です：\u200Bそれぞれ\n"
     u8"自分の\u200Bネコだけで\u200Bバトルを\u200Bします。\u200B終わると\u200B戻ってきます。"},
    {u8"%s a fini de défendre sa maison.", u8"%sが\u200B家の\u200B防衛を\u200B終えました。"},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%sが\u200B家の\u200B防衛を\u200B終えました。\n"
     u8"%sは\u200Bまだ\u200B防衛中です。"},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%sが\u200B家の\u200B防衛を\u200B終えました。\n"
     u8"%sは\u200Bまだ\u200B防衛中です。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"冒険に出発\n"
     u8"\n"
     u8"返事の\u200Bあとで\u200B取りやめたため、\u200B%sは\n"
     u8"あなたも\u200Bネコも\u200B連れずに\u200B出発します。\n"
     u8"その人が\u200Bマップに\u200B着いたら、\u200B冒険に\u200B参加できます。"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"冒険に出発\n"
     u8"\n"
     u8"今回は\u200B出発できる\u200Bネコが\u200Bいません。\n"
     u8"%sは\u200B冒険に\u200B出発しません。"},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"戦利品の\u200B分配が\u200B終わっていません。\n"
     u8"最後の\u200Bアイテムまで\u200B待ってください。"},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"%sの\u200B冒険に\u200B参加します。\n"
     u8"家の\u200B防衛は\u200B戻ってから\u200B行います。"},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"冒険を\u200B始めるのは\u200B%sです。\n"
     u8"「出発！」を\u200Bクリックしてもらってください。"},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"マルチプレイ\n"
     u8"\n"
     u8"ゲームを\u200Bホストして\u200Bフレンドに\u200B参加してもらうか、\n"
     u8"フレンドの\u200Bゲームに\u200B参加します。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"[c:red]ポート%uを\u200B開けません。[/c]\n"
     u8"ほかの\u200Bプログラムが\u200B使っている\u200B可能性が\u200Bあります。\n"
     u8"ロビーを\u200B閉じて、\u200B再試行してください。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"ロビーを\u200B開いています..."},
    {u8"%s (hôte)", u8"%s（ホスト）"},
    {u8"En attente d'un ami...", u8"フレンドを\u200B待っています..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Steamの\u200Bフレンドは\u200Bワンクリックで\u200B参加できます：\n"
     u8"フレンドリストで\u200Bあなたの\u200B名前の\u200B「ゲームに\u200B参加」を\u200B選びます。\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"またはアドレスで：\n"
     u8"%s   （%s、ポート%u）\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"この\u200Bアドレスを\u200Bフレンドに\u200B伝えてください：\n"
     u8"%s   （%s、ポート%u）\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"コピー済みです：\u200BCtrl+Vで\u200B貼り付けて\u200B送ってください。\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"この\u200BPCの\u200Bアドレスが\u200B見つかりません（ポート%u）。\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"%s\n"
     u8"プレイヤー（%d/4）：\n"
     u8"%s\n"
     u8"全員\u200Bそろったら、\u200Bセーブデータを\u200B選んでください。"},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"ゲームに参加\n"
     u8"\n"
     u8"ホストのアドレス：\n"
     u8"%s_\n"
     u8"\n"
     u8"フレンドに\u200B教わった\u200Bアドレスを\u200B入力するか、\n"
     u8"Ctrl+Vで\u200B貼り付けて、\u200BEnterで\u200B決定します。%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Steamなら\u200Bもっと\u200B簡単です：\u200Bフレンドリストで\n"
     u8"フレンドの\u200B「ゲームに\u200B参加」を\u200B選びます。"},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"ゲームに参加\n"
     u8"\n"
     u8"[c:red]無効な\u200Bアドレス：%s[/c]\n"
     u8"\n"
     u8"例：26.123.45.67\n"
     u8"別の\u200Bポートの\u200B場合：26.123.45.67:27600"},
    {u8"(vide)", u8"（空）"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.", u8"ゲームと\u200BMODの\u200Bバージョンが\u200B同じか\u200B確認してください。"},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).", u8"ホストの\u200Bゲームは\u200B満員です（最大4人）。"},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"MODの\u200Bバージョンが\u200B違います：\n"
     u8"2人とも\u200B最新版に\u200Bしてください。"},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"ゲームの\u200Bバージョンが\u200B違います：\n"
     u8"2人とも\u200BMewgenicsを\u200Bアップデートしてください（Steam）。"},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"ゲームの\u200Bファイルが\u200B一致しません：\n"
     u8"Steamで\u200B確認してください（プロパティ→\u200Bインストール済みファイル）。"},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"mgmp.jsonの\u200Bnet.control\u200B設定で、\u200B2人に\u200B同じ\u200Bネコが\n"
     u8"割り当てられています：\u200B\"auto\"に\u200B戻してください。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"[c:red]接続を\u200B拒否されました。[/c]\n"
     u8"%s\n"
     u8"（%s）"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"[c:red]%sに\u200B参加できません：[/c]\n"
     u8"%s。\n"
     u8"ロビーから\u200B退出し、\u200B再試行してください（または\u200Bアドレスで）。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"%sに\u200B接続中..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"[c:red]%sに\u200B接続できません。[/c]\n"
     u8"アドレスと、\u200Bフレンドが\u200B「ホストする」を\n"
     u8"クリックしたか\u200B確認してください。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"切断されました。"},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"セーブデータの\u200B画面が\u200B自動で\u200B開きます。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"%sに\u200B接続中...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"[c:red]ホストとの\u200B接続が\u200B切れました。[/c]\n"
     u8"ロビーから\u200B退出して、\u200Bもう一度\u200B参加してください。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"Steamで\u200B%sに\u200B接続中...\n"
     u8"（数秒かかります）%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"接続完了。\u200Bホストと\u200B通信中...%s"},
    {u8"%s (toi)", u8"%s（あなた）"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"%sの\u200Bゲームに\u200B参加しました！\n"
     u8"\n"
     u8"プレイヤー（%d/4）：\n"
     u8"%s\n"
     u8"%sの\u200Bゲームを\u200B待っています：\n"
     u8"その人が\u200Bセーブデータを\u200B選ぶと、\n"
     u8"セーブデータの\u200B画面が\u200B自動で\u200B開きます。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"%sの\u200Bゲームに\u200B参加しました！\n"
     u8"\n"
     u8"プレイヤー（%d/4）：\n"
     u8"%s\n"
     u8"%sの\u200Bゲームを\u200B待っています：\n"
     u8"その人が\u200Bセーブデータを\u200B選ぶと、\n"
     u8"あなたも\u200B自分の\u200Bセーブデータを\u200B選べます。"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"マルチプレイロビー\n"
     u8"\n"
     u8"%sの\u200Bゲームに\u200B参加しました！\n"
     u8"\n"
     u8"プレイヤー（%d/4）：\n"
     u8"%s\n"
     u8"セーブデータを\u200B選んでください：\u200Bあなたの\u200B家と\u200Bネコです。\n"
     u8"NPCと\u200Bストーリーは\u200B%sの\u200Bものになり、\n"
     u8"冒険には\u200Bその人の\u200Bチームに\u200B参加して\u200B出発します。"},
    {u8"Héberger", u8"ホストする"},
    {u8"Rejoindre", u8"参加する"},
    {u8"Choisir la sauvegarde", u8"セーブデータを選ぶ"},
    {u8"Arrêter", u8"閉じる"},
    {u8"Choisir ta sauvegarde", u8"セーブデータを選ぶ"},
    {u8"Quitter", u8"退出"},
    {u8"Annuler", u8"キャンセル"},
    {u8"Échanger avec %s", u8"%sとトレード"},
    {u8"Donner à qui ?", u8"誰に渡しますか？"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"上下キーで\u200B選んで、\u200BEnterで\u200B決定します。"},
    {u8"Pièces à donner : %lld_", u8"渡すコイン：%lld_"},
    {u8"Nourriture à donner : %lld_", u8"渡すフード：%lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"数を\u200B入力し（T：全部）、\u200BEnterで\u200B決定します。"},
    {u8"Échanger avec %s : les objets de ton rangement", u8"%sとトレード：\u200B倉庫の\u200Bアイテム"},
    {u8"16 au plus par don : %u choisis.", u8"1回16個まで：\u200B%u個\u200B選択中。"},
    {u8"16 au plus par don : %u choisi.", u8"1回16個まで：\u200B%u個\u200B選択中。"},
    {u8"Lecture du rangement...", u8"倉庫を\u200B読み込み中..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"上下：選択、\u200Bスペース：\u200Bチェック、\u200B左右：\u200B個数\n"
     u8"文字入力で\u200B検索、\u200BEnterで\u200B決定。"},
    {u8"Tu n'as rien choisi à donner.", u8"渡す\u200Bものを\u200B選んでいません。"},
    {u8"Tu n'as rien à donner pour l'instant.", u8"今は\u200B渡せる\u200Bものが\u200Bありません。"},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"サイフから\u200B%sを\u200B%sに\n"
     u8"渡しますか？"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"%sを\u200B%sに\n"
     u8"渡しますか？"},
    {u8"Envoi du don...", u8"プレゼントを送信中..."},
    {u8"Personne ne peut recevoir maintenant.", u8"今は\u200B誰も\u200B受け取れません。"},
    {u8"Personne d'autre n'est dans la partie.", u8"ゲームに\u200Bほかの\u200Bプレイヤーが\u200Bいません。"},
    {u8"Confirmer", u8"決定"},
    {u8"Donner", u8"渡す"},
    {u8"Recommencer", u8"やり直す"},
    {u8"Fermer", u8"閉じる"},
    {u8"Pas d'échange possible ici.", u8"ここでは\u200Bトレードできません。"},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"ホストが\u200Bマスに\u200B入ったため、\u200Bウィンドウを\u200B閉じました。\n"
     u8"選んだ\u200B内容は\u200B残っています（Eで\u200B再開）。"},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"冒険中のトレード\n"
     u8"\n"
     u8"マップで\u200BEキーを\u200B押すと、\u200Bサイフの\u200Bコインや\n"
     u8"フードを\u200Bほかの\u200Bプレイヤーに\u200B渡せます。"},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
