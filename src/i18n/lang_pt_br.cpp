// lang_pt_br.cpp -- Mewgenics Multi : les textes du mod et du lanceur en portugais du Bresil (pt-br).
//
// Une paire par texte : { la cle, c'est-a-dire le texte francais EXACT passe a
// tr() ou trk() dans le code (memes \n, memes espaces, memes marqueurs %s), puis
// sa traduction }. Un texte absent (ou "") reste en francais. Les regles :
// docs/TRADUCTION.md ; tests/test_i18n.cpp les verifie, tools/i18n_cles.py liste
// les cles et ce qui manque.
// Ton : « voce », familier, comme le tutoiement du francais du mod.
#include "mm_lang.h"

namespace mm {

extern const TrPair kTable_pt_br[];
const TrPair kTable_pt_br[] = {
    {u8"mgmp.json n'a pas pu être lu : réglages non enregistrés.",
     u8"Não foi possível ler o mgmp.json: configurações não salvas."},
    {u8"mgmp.json contient des commentaires (// ou /* */), que l'enregistrement effacerait : retire-les, ou change le réglage à la main dans le fichier.",
     u8"O mgmp.json contém comentários (// ou /* */) que seriam apagados ao salvar: remova-os ou altere a configuração manualmente no arquivo."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le (ou remets celui du zip) avant de changer les réglages.",
     u8"O mgmp.json não é um JSON válido: corrija-o (ou restaure o do zip) antes de alterar as configurações."},
    {u8"Impossible d'écrire dans le dossier du mod : réglages non enregistrés.",
     u8"Não foi possível gravar na pasta do mod: configurações não salvas."},
    {u8"mgmp.json n'a pas pu être remplacé (ouvert ailleurs ?) : réglages non enregistrés.",
     u8"Não foi possível substituir o mgmp.json (aberto em outro programa?): configurações não salvas."},
    {u8"Steam est ouvert.", u8"A Steam está aberta."},
    {u8"Sans lui, le jeu se ferme avant que le mod se charge. Ouvre Steam et attends qu'il soit connecté.",
     u8"Sem ela, o jogo fecha antes de o mod carregar. Abra a Steam e espere até que ela se conecte."},
    {u8"mgmp.json manque à côté de Mewgenics\u00A0Multi.exe : enregistre les réglages pour le recréer.",
     u8"O mgmp.json não está ao lado do Mewgenics\u00A0Multi.exe: salve as configurações para recriá-lo."},
    {u8"mgmp.json n'est pas du JSON valide : corrige-le, ou remets celui du zip.",
     u8"O mgmp.json não é um JSON válido: corrija-o ou restaure o do zip."},
    {u8"Réglages lus dans mgmp.json.", u8"Configurações lidas do mgmp.json."},
    {u8"Mewgenics.exe est introuvable (%s) : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe não encontrado (%s): em Configurações, clique em Procurar… e escolha o Mewgenics.exe."},
    {u8"resources.gpak manque à côté de Mewgenics.exe : vérifie les fichiers du jeu dans Steam.",
     u8"O resources.gpak não está ao lado do Mewgenics.exe: verifique os arquivos do jogo na Steam."},
    {u8"Jeu trouvé : %s.", u8"Jogo encontrado: %s."},
    {u8"steam_appid.txt : à vérifier une fois le jeu trouvé.",
     u8"steam_appid.txt: será verificado quando o jogo for encontrado."},
    {u8"Il manque à côté de Mewgenics.exe : sans lui, Steam relance le jeu sans le mod.",
     u8"Não está ao lado do Mewgenics.exe: sem ele, a Steam reinicia o jogo sem o mod."},
    {u8"Celui à côté de Mewgenics.exe ne contient pas 686060 : Steam relancerait le jeu sans le mod.",
     u8"O que está ao lado do Mewgenics.exe não contém 686060: a Steam reiniciaria o jogo sem o mod."},
    {u8"steam_appid.txt est en place à côté du jeu.", u8"steam_appid.txt está no lugar, ao lado do jogo."},
    {u8"Fichiers du mod présents.", u8"Arquivos do mod presentes."},
    {u8"Il manque mgmp.dll et mgmp_loader.exe à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Faltam mgmp.dll e mgmp_loader.exe ao lado do Mewgenics\u00A0Multi.exe: extraia de novo a pasta inteira do zip."},
    {u8"Il manque %s à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau tout le dossier du zip.",
     u8"Falta %s ao lado do Mewgenics\u00A0Multi.exe: extraia de novo a pasta inteira do zip."},
    {u8"Mewgenics Multi, build %s, protocole %u : tout le monde doit avoir le même.",
     u8"Mewgenics Multi, build %s, protocolo %u: todos precisam ter o mesmo."},
    {u8"Le jeu est introuvable : choisis d'abord Mewgenics.exe dans Réglages.",
     u8"Jogo não encontrado: primeiro escolha o Mewgenics.exe em Configurações."},
    {u8"steam_appid.txt n'a pas pu être écrit à côté du jeu : copie-le à la main (LISEZ-MOI, étape 2).",
     u8"Não foi possível gravar o steam_appid.txt ao lado do jogo: copie-o manualmente (LISEZ-MOI, etapa 2)."},
    {u8"Windows n'a pas eu ton accord : steam_appid.txt n'est pas copié. Tu peux réessayer, ou le copier à la main (LISEZ-MOI, étape 2).",
     u8"O Windows não recebeu sua autorização: o steam_appid.txt não foi copiado. Tente de novo ou copie-o manualmente (LISEZ-MOI, etapa 2)."},
    {u8"Mewgenics Multi n'a pas pu se relancer en administrateur : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"O Mewgenics Multi não conseguiu reiniciar como administrador: copie o steam_appid.txt manualmente (LISEZ-MOI, etapa 2)."},
    {u8"La copie en administrateur n'a pas marché : copie steam_appid.txt à la main (LISEZ-MOI, étape 2).",
     u8"A cópia como administrador não funcionou: copie o steam_appid.txt manualmente (LISEZ-MOI, etapa 2)."},
    {u8"Le mod n'a pas pu être chargé : %s", u8"Não foi possível carregar o mod: %s"},
    {u8"mgmp_loader.exe s'est arrêté sans charger le mod. Relance ; si ça recommence, envoie loader.txt.",
     u8"O mgmp_loader.exe parou sem carregar o mod. Tente de novo; se acontecer outra vez, envie o loader.txt."},
    {u8"mgmp_loader.exe s'est arrêté (code %s) sans charger le mod. Cause la plus probable : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"O mgmp_loader.exe parou (código %s) sem carregar o mod. Causa mais provável: Steam fechada ou desconectada. Abra a Steam, espere até que ela se conecte e tente de novo."},
    {u8"Le jeu s'est fermé pendant son démarrage, avant le mod. Cause la plus fréquente : Steam fermé ou déconnecté. Ouvre Steam, attends qu'il soit connecté, puis relance.",
     u8"O jogo fechou durante a inicialização, antes do mod. Causa mais comum: Steam fechada ou desconectada. Abra a Steam, espere até que ela se conecte e tente de novo."},
    {u8"Steam relance le jeu (ça peut prendre une minute ou deux)…",
     u8"A Steam está reiniciando o jogo (pode levar um ou dois minutos)…"},
    {u8"Ouverture du jeu…", u8"Abrindo o jogo…"},
    {u8"Le jeu démarre…", u8"O jogo está iniciando…"},
    {u8"Injection du mod dans le jeu…", u8"Injetando o mod no jogo…"},
    {u8"Le mod est en place, le jeu reprend…", u8"O mod está pronto, o jogo continua…"},
    {u8"Toujours en attente du jeu relancé par Steam… Si Steam vient de s'ouvrir, il te pose peut-être une question.",
     u8"Ainda aguardando o jogo reiniciado pela Steam… Se a Steam acabou de abrir, talvez ela esteja te fazendo uma pergunta."},
    {u8"Jeu relancé trouvé : injection du mod…", u8"Jogo reiniciado encontrado: injetando o mod…"},
    {u8"Le jeu est lancé avec le mod.", u8"O jogo foi iniciado com o mod."},
    {u8"Le mod a tardé à se mettre en place ; le jeu reprend quand même.",
     u8"O mod demorou para ficar pronto; o jogo continua mesmo assim."},
    {u8"Le jeu relancé tarde à démarrer ; injection quand même.",
     u8"O jogo reiniciado está demorando; injetando mesmo assim."},
    {u8"mgmp.dll est introuvable à côté de Mewgenics\u00A0Multi.exe : extrais de nouveau le dossier du mod depuis le zip.",
     u8"mgmp.dll não encontrado ao lado do Mewgenics\u00A0Multi.exe: extraia de novo a pasta do mod do zip."},
    {u8"Mewgenics.exe est introuvable : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"Mewgenics.exe não encontrado: em Configurações, clique em Procurar… e escolha o Mewgenics.exe."},
    {u8"mgmp.json est illisible : corrige-le, ou remets celui du zip.",
     u8"O mgmp.json está ilegível: corrija-o ou restaure o do zip."},
    {u8"Steam a relancé le jeu sans le mod : vérifie steam_appid.txt à côté de Mewgenics.exe (bouton Réparer), puis relance.",
     u8"A Steam reiniciou o jogo sem o mod: verifique o steam_appid.txt ao lado do Mewgenics.exe (botão Reparar) e tente de novo."},
    {u8"Ce jeu n'est pas la version visée par le mod (1.1.21239) : a-t-il été mis à jour ?",
     u8"Este jogo não é a versão para a qual o mod foi feito (1.1.21239): ele foi atualizado?"},
    {u8"Un programme (overlay, antivirus, anti-triche) a empêché le chargement du mod.",
     u8"Um programa (overlay, antivírus, antitrapaça) impediu o carregamento do mod."},
    {u8"Le jeu a mis trop longtemps à démarrer : le mod n'est pas chargé. Relance.",
     u8"O jogo demorou demais para iniciar: o mod não foi carregado. Tente de novo."},
    {u8"L'injection du mod a échoué et le jeu a été fermé. Relance ; si ça recommence, envoie loader.txt.",
     u8"A injeção do mod falhou e o jogo foi fechado. Tente de novo; se acontecer outra vez, envie o loader.txt."},
    {u8"mgmp.json ne dit pas où est le jeu : dans Réglages, clique sur Parcourir… et choisis Mewgenics.exe.",
     u8"O mgmp.json não diz onde está o jogo: em Configurações, clique em Procurar… e escolha o Mewgenics.exe."},
    {u8"build %s · protocole %u", u8"build %s · protocolo %u"},
    {u8"Steam ouvert", u8"Steam aberta"},
    {u8"Steam fermé", u8"Steam fechada"},
    {u8"Réglages (mgmp.json)", u8"Configurações (mgmp.json)"},
    {u8"Jeu trouvé", u8"Jogo encontrado"},
    {u8"Jeu incomplet", u8"Jogo incompleto"},
    {u8"Jeu introuvable", u8"Jogo não encontrado"},
    {u8"Fichiers du mod", u8"Arquivos do mod"},
    {u8"Réparer", u8"Reparar"},
    {u8"Jouer", u8"Jogar"},
    {u8"Règle d'abord les croix rouges.", u8"Resolva primeiro os X vermelhos."},
    {u8"Réglages", u8"Configurações"},
    {u8"Journaux", u8"Registros"},
    {u8"Langue", u8"Idioma"},
    {u8"Le jeu (Mewgenics.exe)", u8"O jogo (Mewgenics.exe)"},
    {u8"Parcourir…", u8"Procurar…"},
    {u8"Mewgenics.exe et resources.gpak sont là.", u8"Mewgenics.exe e resources.gpak estão aqui."},
    {u8"resources.gpak manque à côté : ce n'est pas le bon dossier.",
     u8"Falta o resources.gpak ao lado: esta não é a pasta certa."},
    {u8"Aucun Mewgenics.exe ici : choisis-le avec Parcourir.",
     u8"Nenhum Mewgenics.exe aqui: escolha-o com Procurar."},
    {u8"Jouer ensemble par Steam (liste d'amis)", u8"Jogar junto pela Steam (lista de amigos)"},
    {u8"Décoche si Steam pose problème : il ne reste que l'adresse, dans le menu Multijoueur du jeu.",
     u8"Desmarque se a Steam der problema: só restará a conexão por endereço, no menu Multijogador do jogo."},
    {u8"Enregistrer", u8"Salvar"},
    {u8"Retour", u8"Voltar"},
    {u8"Aucun journal pour l'instant : le mod en écrit un à chaque partie.",
     u8"Nenhum registro por enquanto: o mod grava um a cada partida."},
    {u8"1 journal du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"1 registro do mod, ao lado do Mewgenics\u00A0Multi.exe."},
    {u8"%d journaux du mod, à côté de Mewgenics\u00A0Multi.exe.",
     u8"%d registros do mod, ao lado do Mewgenics\u00A0Multi.exe."},
    {u8"Le dernier : %s", u8"O último: %s"},
    {u8"écrit le %s", u8"gravado em %s"},
    {u8"Ouvrir le dossier", u8"Abrir a pasta"},
    {u8"Copier le dernier journal", u8"Copiar o último registro"},
    {u8"Pour demander de l'aide : copie le dernier journal, puis colle-le dans Discord avec Ctrl+V.",
     u8"Para pedir ajuda: copie o último registro e cole no Discord com Ctrl+V."},
    {u8"Lancement", u8"Iniciando"},
    {u8"Le jeu se lance avec le mod…", u8"O jogo está abrindo com o mod…"},
    {u8"Bonne partie ! Cette fenêtre se ferme toute seule.",
     u8"Bom jogo! Esta janela vai fechar sozinha."},
    {u8"Le détail est dans loader.txt, à côté de Mewgenics\u00A0Multi.exe.",
     u8"Os detalhes estão no loader.txt, ao lado do Mewgenics\u00A0Multi.exe."},
    {u8"Réessayer", u8"Tentar de novo"},
    {u8"Si Steam relance le jeu, ça peut prendre une minute ou deux.",
     u8"Se a Steam reiniciar o jogo, pode levar um ou dois minutos."},
    {u8"Journal copié : colle-le dans Discord avec Ctrl+V.",
     u8"Registro copiado: cole no Discord com Ctrl+V."},
    {u8"Le presse-papiers n'a pas pu être rempli : réessaie, ou ouvre le dossier.",
     u8"Não foi possível copiar para a área de transferência: tente de novo ou abra a pasta."},
    {u8"Où est Mewgenics.exe ?", u8"Onde está o Mewgenics.exe?"},
    {u8"Direct2D est indisponible sur ce PC : Mewgenics Multi ne peut pas s'afficher.",
     u8"O Direct2D não está disponível neste PC: o Mewgenics Multi não pode ser exibido."},
    {u8"La carte graphique n'a pas pu être préparée : Mewgenics Multi ne peut pas s'afficher.",
     u8"Não foi possível preparar a placa de vídeo: o Mewgenics Multi não pode ser exibido."},
    {u8"Chacun sa maison et ses chats,\n"
     u8"l'aventure ensemble !",
     u8"Cada um com sua casa e gatos,\n"
     u8"na aventura, todos juntos!"},
    {u8"(illustrations du jeu introuvables)", u8"(ilustrações do jogo não encontradas)"},
    {u8"Comme le jeu (%s)", u8"Igual ao jogo (%s)"},
    {u8"Comme Windows (%s)", u8"Igual ao Windows (%s)"},
    {u8"Automatique (%s)", u8"Automático (%s)"},
    {u8"Lancement annulé.", u8"Inicialização cancelada."},
    {u8"Lancement du mod…", u8"Iniciando o mod…"},
    {u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.",
     u8"O Windows não conseguiu preparar a inicialização. Abra o Mewgenics Multi de novo."},
    {u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis le zip (ou ton antivirus l'a mis de côté).",
     u8"O mgmp_loader.exe não pôde iniciar: extraia de novo a pasta do mod do zip (ou seu antivírus o colocou em quarentena)."},
    {u8"la connexion Steam avec l'hôte est coupée", u8"a conexão Steam com o anfitrião caiu"},
    {u8"l'hôte ne répond pas par Steam (salon fermé, ou pas le mod ?)",
     u8"o anfitrião não responde pela Steam (lobby fechado ou sem o mod?)"},
    {u8"Steam refuse la connexion", u8"a Steam recusou a conexão"},
    {u8"À %s de choisir", u8"Vez de %s escolher"},
    {u8"À toi de choisir", u8"Sua vez de escolher"},
    {u8"À %s de choisir la capacité à remplacer.", u8"Vez de %s escolher a habilidade a substituir."},
    {u8"Ce chat est à %s", u8"Este gato é de %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Le choix partagé s'est arrêté (erreur du jeu) :\n"
     u8"choisis les colliers pour toute l'équipe.",
     u8"Escolha das coleiras\n"
     u8"\n"
     u8"A escolha compartilhada parou (erro do jogo):\n"
     u8"escolha as coleiras da equipe toda."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"Prêt ! %s sait que tu as fini :\n"
     u8"il validera le départ pour toute l'équipe.",
     u8"Escolha das coleiras\n"
     u8"\n"
     u8"Pronto! %s sabe que você terminou:\n"
     u8"ele vai confirmar a saída da equipe toda."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"C'est %s qui valide le départ :\n"
     u8"dis-lui quand tes chats ont leurs colliers.",
     u8"Escolha das coleiras\n"
     u8"\n"
     u8"Quem confirma a saída é %s:\n"
     u8"avise quando seus gatos estiverem com as coleiras."},
    {u8"%s et %s", u8"%s e %s"},
    {u8"%s, %s", u8"%s, %s"},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisissent encore les colliers de leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Escolha das coleiras\n"
     u8"\n"
     u8"%s ainda estão escolhendo as coleiras dos próprios gatos.\n"
     u8"Clique de novo em \"Travar!\" para partir\n"
     u8"mesmo assim."},
    {u8"Choix des colliers\n"
     u8"\n"
     u8"%s choisit encore les colliers de ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Escolha das coleiras\n"
     u8"\n"
     u8"%s ainda está escolhendo as coleiras dos próprios gatos.\n"
     u8"Clique de novo em \"Travar!\" para partir\n"
     u8"mesmo assim."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Le départ de %s ne s'affiche plus ici.\n"
     u8"Termine ces écrans toi-même : tu le rejoindras\n"
     u8"sur la carte.",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"A saída de %s não aparece mais aqui.\n"
     u8"Termine estas telas sozinho: você vai\n"
     u8"encontrá-lo no mapa."},
    {u8"  %s : parti", u8"  %s: saiu"},
    {u8"  %s : prêt !", u8"  %s: pronto!"},
    {u8"  %s : charge ta partie...", u8"  %s: carregando sua partida..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton équipe charge ta partie pour partir avec toi :\n"
     u8"\n",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Sua equipe está carregando sua partida para ir com você:\n"
     u8"\n"},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Ton rangement n'a pas pu être chargé ici :\n"
     u8"tes chats partent avec les objets qu'ils portent.",
     u8"Escolha dos itens\n"
     u8"\n"
     u8"Não foi possível carregar seu armazenamento aqui:\n"
     u8"seus gatos partem com os itens que já estão usando."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipent encore leurs chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Escolha dos itens\n"
     u8"\n"
     u8"%s ainda equipam os próprios gatos.\n"
     u8"Clique de novo em \"Travar!\" para partir\n"
     u8"mesmo assim."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s équipe encore ses chats.\n"
     u8"Clique de nouveau sur « Valider » pour partir\n"
     u8"quand même.",
     u8"Escolha dos itens\n"
     u8"\n"
     u8"%s ainda equipa os próprios gatos.\n"
     u8"Clique de novo em \"Travar!\" para partir\n"
     u8"mesmo assim."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Prêt ! Tes chats partent avec ces objets :\n"
     u8"%s validera le départ pour toute l'équipe.",
     u8"Escolha dos itens\n"
     u8"\n"
     u8"Pronto! Seus gatos partem com estes itens:\n"
     u8"%s vai confirmar a saída da equipe toda."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"Les objets choisis pour tes chats n'ont pas pu\n"
     u8"être repris dans l'aventure de %s : tes chats\n"
     u8"partent avec ceux qu'ils avaient, ton rangement\n"
     u8"ne change pas.",
     u8"Escolha dos itens\n"
     u8"\n"
     u8"Os itens escolhidos para seus gatos não\n"
     u8"puderam ir para a aventura de %s: seus gatos\n"
     u8"partem com os que tinham, seu armazenamento\n"
     u8"não muda."},
    {u8"Choix des objets\n"
     u8"\n"
     u8"%s a déjà validé le départ : tes chats partent\n"
     u8"avec les objets choisis jusqu'ici, la suite\n"
     u8"ne compte plus.",
     u8"Escolha dos itens\n"
     u8"\n"
     u8"%s já confirmou a saída: seus gatos partem\n"
     u8"com os itens escolhidos até agora, o resto\n"
     u8"não conta mais."},
    {u8"Ta bourse : %s.", u8"Sua bolsa: %s."},
    {u8"%s n'est plus dans la partie.", u8"%s não está mais na partida."},
    {u8"Tu n'as pas de bourse dans cette aventure (tu n'y as pas amené de chat).",
     u8"Você não tem bolsa nesta aventura (não levou nenhum gato)."},
    {u8"Les échanges marchent dans ta maison, ou en aventure à plusieurs avec ta bourse. Pour l'instant : %s.",
     u8"As trocas funcionam na sua casa, ou em aventuras multijogador com sua bolsa. Por enquanto: %s."},
    {u8"pas de partie à plusieurs en cours", u8"nenhuma partida multijogador ativa"},
    {u8"une fonction du jeu manque, les échanges à la maison sont coupés",
     u8"falta uma função do jogo, as trocas em casa estão desativadas"},
    {u8"tu es dans la partie de l'hôte", u8"você está na partida do anfitrião"},
    {u8"tu n'es pas dans ta maison", u8"você não está na sua casa"},
    {u8"ta save n'est pas chargée", u8"seu save não está carregado"},
    {u8"un départ en aventure est en cours", u8"uma aventura está começando"},
    {u8"ta maison est attaquée", u8"sua casa está sob ataque"},
    {u8"ta maison est illisible", u8"erro ao ler sua casa"},
    {u8"ton rangement attend ce que tes chats rapportent de l'aventure de l'hôte (s'ils sont rentrés, recharge ta save : Échap, « Sauvegarder et retourner au menu », puis « Jouer » et ta save)",
     u8"seu armazenamento aguarda o que seus gatos trazem da aventura do anfitrião (se já voltaram, recarregue seu save: Esc, \"Salvar e sair para o menu\", depois \"Jogar\" e seu save)"},
    {u8"des objets de ton rangement attendent encore leur nouveau propriétaire (butin, achats, PNJ)",
     u8"itens do seu armazenamento ainda aguardam o novo dono (saque, compras, NPCs)"},
    {u8"Pas d'échange maintenant : %s.", u8"Sem trocas agora: %s."},
    {u8"Ton don précédent attend encore sa réponse.", u8"Seu presente anterior ainda aguarda resposta."},
    {u8"Ta save n'est pas encore connue du mod : recharge-la depuis le menu.",
     u8"O mod ainda não conhece seu save: recarregue-o pelo menu."},
    {u8"Choisis un joueur de la partie.", u8"Escolha um jogador da partida."},
    {u8"Pas d'échange maintenant : %s n'est pas dans sa maison.",
     u8"Sem trocas agora: %s não está em casa."},
    {u8"Ce montant n'est pas possible.", u8"Esse valor não é válido."},
    {u8"Choisis des pièces, de la nourriture ou des objets à donner.",
     u8"Escolha moedas, comida ou itens para dar."},
    {u8"16 objets au plus par don.", u8"Até 16 itens por presente."},
    {u8"Pas d'objets maintenant : %s.", u8"Sem itens agora: %s."},
    {u8"Ta maison est illisible : pas de don.", u8"Erro ao ler sua casa: sem presente."},
    {u8"Tu n'as pas autant de pièces.", u8"Você não tem tantas moedas."},
    {u8"Tu n'as pas autant de nourriture.", u8"Você não tem tanta comida."},
    {u8"Ton rangement est illisible : pas de don.", u8"Erro ao ler seu armazenamento: sem presente."},
    {u8"Un objet choisi est déjà promis à un don pas encore fini.",
     u8"Um item escolhido já está em um presente ainda não concluído."},
    {u8"Un objet choisi n'est plus dans ton rangement : choisis à nouveau.",
     u8"Um item escolhido não está mais no seu armazenamento: escolha de novo."},
    {u8"Un objet choisi ne peut pas se donner.", u8"Um item escolhido não pode ser dado."},
    {u8"Le don n'a pas pu partir (connexion).", u8"O presente não foi enviado (conexão)."},
    {u8"Don envoyé à %s (%s) : en attente de sa réponse…",
     u8"Presente enviado para %s (%s): aguardando resposta…"},
    {u8"Don demandé à l'hôte : en attente de sa réponse…",
     u8"Presente pedido ao anfitrião: aguardando resposta…"},
    {u8"Pas de don : %s.", u8"Sem presente: %s."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que l'écran des objets sera fermé.",
     u8"%s recebeu seu presente: seus itens sairão do seu armazenamento assim que a tela de itens for fechada."},
    {u8"%s a bien reçu ton don : tes objets quitteront ton rangement dès que possible (après le départ, la défense de ta maison ou le retour de tes chats).",
     u8"%s recebeu seu presente: seus itens sairão do seu armazenamento assim que possível (após a saída, a defesa da sua casa ou a volta dos seus gatos)."},
    {u8"%s a bien reçu ton don : ce que tu as donné quittera ta maison à ton retour dans cette save.",
     u8"%s recebeu seu presente: o que você deu sairá da sua casa quando você voltar a este save."},
    {u8"Pas de réponse d'%s pour ton don : rien n'a quitté ta maison.",
     u8"Sem resposta de %s ao seu presente: nada saiu da sua casa."},
    {u8"Pas de réponse de %s pour ton don : rien n'a quitté ta maison.",
     u8"Sem resposta de %s ao seu presente: nada saiu da sua casa."},
    {u8"Les échanges marchent pendant une partie à plusieurs.",
     u8"As trocas funcionam durante uma partida multijogador."},
    {u8"Dans ta maison : %s.", u8"Na sua casa: %s."},
    {u8"%s n'est pas dans sa maison.", u8"%s não está em casa."},
    {u8"%lld pièces", u8"%lld moedas"},
    {u8"%lld pièce", u8"%lld moeda"},
    {u8"%lld nourritures", u8"%lld de comida"},
    {u8"%lld nourriture", u8"%lld de comida"},
    {u8"rien", u8"nada"},
    {u8"%s n'est pas dans sa maison", u8"%s não está em casa"},
    {u8"une fenêtre ou l'écran des objets est resté ouvert chez %s",
     u8"uma janela ou a tela de itens ficou aberta no jogo de %s"},
    {u8"le jeu d'%s n'a pas pu les ranger", u8"o jogo de %s não pôde guardá-los"},
    {u8"le jeu de %s n'a pas pu les ranger", u8"o jogo de %s não pôde guardá-los"},
    {u8"les dons ne marchent pas chez %s", u8"os presentes não funcionam para %s"},
    {u8"%s n'est plus dans la partie", u8"%s não está mais na partida"},
    {u8"%s a encore des chats dans l'aventure de l'hôte (son rangement attend leur retour)",
     u8"%s ainda tem gatos na aventura do anfitrião (o armazenamento dele aguarda a volta deles)"},
    {u8"raison inconnue", u8"motivo desconhecido"},
    {u8"Cadeau d'%s", u8"Presente de %s"},
    {u8"Cadeau de %s", u8"Presente de %s"},
    {u8"%s t'a donné %s.", u8"%s te deu %s."},
    {u8"C'est dans ta maison, et ta save est à jour.",
     u8"Já está na sua casa, e seu save está atualizado."},
    {u8"C'est dans ta maison.", u8"Já está na sua casa."},
    {u8"Finalement, %s a bien reçu %s : c'est parti de ta maison.",
     u8"No fim, %s recebeu %s: saiu da sua casa."},
    {u8"%s a bien reçu %s : c'est parti de ta maison.", u8"%s recebeu %s: saiu da sua casa."},
    {u8"(1 objet n'a pas pu quitter ton rangement : il est aussi chez toi.)",
     u8"(1 item não saiu do armazenamento: vocês dois o têm.)"},
    {u8"(%u objets n'ont pas pu quitter ton rangement : ils sont aussi chez toi.)",
     u8"(%u itens não saíram do armazenamento: vocês dois os têm.)"},
    {u8"Le don à %s n'est pas passé : %s. Rien n'a quitté ta maison.",
     u8"O presente para %s não foi entregue: %s. Nada saiu da sua casa."},
    {u8"Rien à donner dans ton rangement.", u8"Nada para dar no armazenamento."},
    {u8"Tape des lettres pour chercher un objet.", u8"Digite letras para buscar um item."},
    {u8"Recherche : %s_", u8"Busca: %s_"},
    {u8"Aucun objet ne correspond.", u8"Nenhum item corresponde."},
    {u8"(page %u sur %u)", u8"(página %u de %u)"},
    {u8"Butin : %s.", u8"Saque: %s."},
    {u8"Besoin", u8"Necessidade"},
    {u8"Cupidité", u8"Ganância"},
    {u8"Passer", u8"Passar"},
    {u8"%u s", u8"%u s"},
    {u8"toi", u8"você"},
    {u8"Joueur %u", u8"Jogador %u"},
    {u8"Un objet", u8"Um item"},
    {u8"Partage du butin : objet %u sur %u", u8"Divisão do saque: item %u de %u"},
    {u8"Besoin : %s", u8"Necessidade: %s"},
    {u8"Cupidité : %s", u8"Ganância: %s"},
    {u8"Passer : %s", u8"Passar: %s"},
    {u8"Partage du butin", u8"Divisão do saque"},
    {u8"%u objets du sac à partager entre %s.", u8"%u itens da mochila a dividir entre %s."},
    {u8"%u objet du sac à partager entre %s.", u8"%u item da mochila a dividir entre %s."},
    {u8"Pour chaque objet : Besoin (il te sert), Cupidité (si\n"
     u8"personne n'en a besoin) ou Passer. Un dé de 100 départage.",
     u8"Para cada item: Necessidade (serve para você), Ganância\n"
     u8"(se ninguém precisar) ou Passar. Um dado de 100 lados desempata."},
    {u8"En attente de %s...", u8"Aguardando %s..."},
    {u8"En attente de %s... (%s)", u8"Aguardando %s... (%s)"},
    {u8"Tout le monde est là : ça commence !", u8"Todos estão aqui: vai começar!"},
    {u8"Commencer", u8"Começar"},
    {u8"D'accord", u8"Ok"},
    {u8"Le veux-tu ? (%s)", u8"Você quer este item? (%s)"},
    {u8"Je le veux", u8"Eu quero"},
    {u8"Besoin : il te sert, tu passes avant les autres.\n"
     u8"Cupidité : tu le prends si personne n'en a besoin.\n"
     u8"(%s)",
     u8"Necessidade: serve para você, você tem prioridade.\n"
     u8"Ganância: você fica com ele se ninguém precisar.\n"
     u8"(%s)"},
    {u8"Ton choix : %s.", u8"Sua escolha: %s."},
    {u8"Ton choix est fait.", u8"Sua escolha foi feita."},
    {u8"En attente de %s (%s).", u8"Aguardando %s (%s)."},
    {u8"Le résultat arrive...", u8"O resultado está chegando..."},
    {u8"%s ne sont pas là : leur tour passe.", u8"%s não estão aqui: perdem a vez."},
    {u8"%s n'est pas là : son tour passe.", u8"%s não está aqui: perde a vez."},
    {u8"Personne n'en veut : il reste chez toi.", u8"Ninguém quer: fica com você."},
    {u8"Personne n'en veut : il reste chez %s.", u8"Ninguém quer: fica com %s."},
    {u8"Tu l'emportes (%s) !", u8"Você ganhou (%s)!"},
    {u8"%s l'emporte (%s).", u8"%s ganhou (%s)."},
    {u8"Dés : %s (égalité départagée).", u8"Dados: %s (empate desfeito)."},
    {u8"Dés : %s.", u8"Dados: %s."},
    {u8"Partage du butin terminé", u8"Divisão do saque concluída"},
    {u8"Tu remportes : %s.", u8"Você ganhou: %s."},
    {u8"%s remporte : %s.", u8"%s ganhou: %s."},
    {u8"Remporté, mais resté chez %s : %s.", u8"Você ganhou, mas ficaram com %s: %s."},
    {u8"Personne n'en a voulu (restent chez toi) : %s.", u8"Ninguém quis (ficam com você): %s."},
    {u8"Personne n'en a voulu (restent chez %s) : %s.", u8"Ninguém quis (ficam com %s): %s."},
    {u8"Aucun objet n'a été partagé.", u8"Nenhum item foi dividido."},
    {u8"Ta save est déjà ouverte : la fenêtre suivante\n"
     u8"te dit comment la recharger avec ton butin.",
     u8"Seu save já está aberto: a próxima janela\n"
     u8"explica como recarregá-lo com o seu saque."},
    {u8"Ton butin t'attend dans ton rangement, chez toi.",
     u8"Seu saque espera por você no armazenamento, em casa."},
    {u8"Ce que les autres remportent quitte ton rangement.",
     u8"O que os outros ganham sai do seu armazenamento."},
    {u8"Dr Beanies", u8"Dr. Beanies"},
    {u8"Butch", u8"Butch"},
    {u8"Tink", u8"Tink"},
    {u8"Frank", u8"Frank"},
    {u8"Bébé Jack", u8"Bebê Jack"},
    {u8"Tracy", u8"Tracy"},
    {u8"Broyeur d'organe", u8"Moedor de Órgãos"},
    {u8"Steven", u8"Steven"},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Le chat que %s a donné à %s a rempli\n"
     u8"sa jauge. La récompense sera prise pour lui\n"
     u8"dès que tu seras dans ta maison : ses pièces,\n"
     u8"sa nourriture et ses objets iront à %s,\n"
     u8"ses déblocages à tout le monde.",
     u8"Recompensa pronta\n"
     u8"\n"
     u8"O gato que %s deu a %s encheu\n"
     u8"a barra. A recompensa será coletada\n"
     u8"assim que você estiver em casa: as moedas,\n"
     u8"a comida e os itens irão para %s,\n"
     u8"os desbloqueios para todos."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"La récompense du don de %s n'a pas pu\n"
     u8"lui être envoyée : elle reste chez toi.",
     u8"Recompensa de %s\n"
     u8"\n"
     u8"A recompensa pela doação de %s não pôde\n"
     u8"ser enviada a ele: ela fica com você."},
    {u8"Récompense pour %s\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : %s partent chez lui.\n"
     u8"Les déblocages sont pour tout le monde.",
     u8"Recompensa para %s\n"
     u8"\n"
     u8"%s recompensou o gato que %s\n"
     u8"lhe deu: %s vão para a casa dele.\n"
     u8"Os desbloqueios são para todos."},
    {u8"Récompense pour tout le monde\n"
     u8"\n"
     u8"%s a récompensé le chat que %s\n"
     u8"lui a donné : ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Recompensa para todos\n"
     u8"\n"
     u8"%s recompensou o gato que %s\n"
     u8"lhe deu: os desbloqueios são para\n"
     u8"todos."},
    {u8"%s te remercie pour ton chat : %s\n"
     u8"(ses déblocages sont pour tout le monde).",
     u8"%s agradece pelo seu gato: %s\n"
     u8"(os desbloqueios são para todos)."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s te remercie pour ton chat : %s.\n"
     u8"Ils arrivent dans ta maison au prochain\n"
     u8"chargement de ta save (au retour de\n"
     u8"l'aventure). Ses déblocages sont pour\n"
     u8"tout le monde.",
     u8"Recompensa de %s\n"
     u8"\n"
     u8"%s agradece pelo seu gato: %s.\n"
     u8"Isso chega à sua casa no próximo\n"
     u8"carregamento do seu save (na volta da\n"
     u8"aventura). Os desbloqueios são para\n"
     u8"todos."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s :\n"
     u8"sa récompense, des déblocages, est pour\n"
     u8"tout le monde. Elle arrive dans ta save\n"
     u8"à son prochain chargement.",
     u8"Recompensa de %s\n"
     u8"\n"
     u8"Seu gato encheu a barra, e %s deu\n"
     u8"uma recompensa para todos: desbloqueios.\n"
     u8"Ela chega ao seu save no próximo\n"
     u8"carregamento."},
    {u8"Récompense de %s\n"
     u8"\n"
     u8"%s, qui avait rempli la jauge de %s,\n"
     u8"n'est plus là : la récompense reste chez toi.",
     u8"Recompensa de %s\n"
     u8"\n"
     u8"%s, que tinha enchido a barra de %s,\n"
     u8"não está mais aqui: a recompensa fica com você."},
    {u8"Récompense prête\n"
     u8"\n"
     u8"Ton chat a rempli la jauge de %s !\n"
     u8"%s prendra la récompense pour toi\n"
     u8"dès qu'il sera dans sa maison.",
     u8"Recompensa pronta\n"
     u8"\n"
     u8"%s: barra cheia graças ao seu gato!\n"
     u8"%s vai pegar a recompensa para você\n"
     u8"assim que estiver em casa."},
    {u8"les bourses ne marchent pas dans cette partie", u8"as bolsas não funcionam nesta partida"},
    {u8"rien à donner", u8"nada para dar"},
    {u8"c'est trop d'un coup", u8"é demais de uma vez"},
    {u8"choisis un autre joueur", u8"escolha outro jogador"},
    {u8"ton don précédent attend encore la réponse de l'hôte",
     u8"seu presente anterior ainda aguarda a resposta do anfitrião"},
    {u8"tu n'as pas de bourse dans cette aventure", u8"você não tem bolsa nesta aventura"},
    {u8"envoi impossible (connexion)", u8"envio impossível (conexão)"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"Chacun rapporte sa bourse dans sa maison :",
     u8"Fim da aventura\n"
     u8"\n"
     u8"Cada um leva sua bolsa para casa:"},
    {u8"%s n'est plus là : sa bourse reste chez toi.", u8"%s não está mais aqui: a bolsa fica com você."},
    {u8"%s : rien.", u8"%s: nada."},
    {u8"%s n'a pas pu recevoir sa bourse : elle reste chez toi.",
     u8"%s não pôde receber a bolsa: ela fica com você."},
    {u8"%s : %s.", u8"%s: %s."},
    {u8"Toi : %s.", u8"Você: %s."},
    {u8"Pas encore de réponse de l'hôte pour ton don : regarde ta bourse dans la fenêtre du chemin.",
     u8"Ainda sem resposta do anfitrião ao seu presente: confira sua bolsa na janela do caminho."},
    {u8"Ton don n'est pas passé : %s.", u8"Seu presente não foi entregue: %s."},
    {u8"(et d'autres trouvailles)", u8"(e outros achados)"},
    {u8"pas de bourse pour ce joueur dans cette aventure", u8"esse jogador não tem bolsa nesta aventura"},
    {u8"ta bourse n'en a pas autant", u8"sua bolsa não tem tudo isso"},
    {u8"l'aventure n'est plus suivie", u8"o mod não segue mais a aventura"},
    {u8"%s t'a donné %s : c'est dans ta bourse.", u8"%s te deu %s: está na sua bolsa."},
    {u8"Tu as donné %s à %s : c'est dans sa bourse.", u8"Você deu %s para %s: já está na bolsa."},
    {u8"%s a donné %s à %s.", u8"%s deu %s para %s."},
    {u8"Un chat", u8"Um gato"},
    {u8"%s a trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"%s achou %s e perdeu %s: conta para sua bolsa."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse de %s.",
     u8"%s achou %s e perdeu %s: conta para a bolsa de %s."},
    {u8"%s a trouvé %s et perdu %s : ça compte pour la bourse d'%s.",
     u8"%s achou %s e perdeu %s: conta para a bolsa de %s."},
    {u8"%s a trouvé %s : c'est pour ta bourse.", u8"%s achou %s: vai para sua bolsa."},
    {u8"%s a trouvé %s : c'est pour la bourse de %s.", u8"%s achou %s: vai para a bolsa de %s."},
    {u8"%s a trouvé %s : c'est pour la bourse d'%s.", u8"%s achou %s: vai para a bolsa de %s."},
    {u8"%s a dépensé %s : c'est pris sur ta bourse.", u8"%s gastou %s: sai da sua bolsa."},
    {u8"%s a dépensé %s : c'est pris sur la bourse de %s.", u8"%s gastou %s: sai da bolsa de %s."},
    {u8"%s a dépensé %s : c'est pris sur la bourse d'%s.", u8"%s gastou %s: sai da bolsa de %s."},
    {u8"%s a perdu %s : c'est pris sur ta bourse.", u8"%s perdeu %s: sai da sua bolsa."},
    {u8"%s a perdu %s : c'est pris sur la bourse de %s.", u8"%s perdeu %s: sai da bolsa de %s."},
    {u8"%s a perdu %s : c'est pris sur la bourse d'%s.", u8"%s perdeu %s: sai da bolsa de %s."},
    {u8"%s n'a rien trouvé.", u8"%s não achou nada."},
    {u8"un objet", u8"um item"},
    {u8"Tu as acheté %s pour %s : c'est pris sur ta bourse.",
     u8"Você comprou %s por %s: sai da sua bolsa."},
    {u8"%s a acheté %s pour %s : c'est pris sur sa bourse.",
     u8"%s comprou %s por %s: sai da própria bolsa."},
    {u8"Tu as acheté %s : %s de plus pour ta bourse.", u8"Você comprou %s: %s a mais na sua bolsa."},
    {u8"%s a acheté %s : %s de plus pour sa bourse.", u8"%s comprou %s: %s a mais na própria bolsa."},
    {u8"Tu as acheté %s (gratuit).", u8"Você comprou %s (grátis)."},
    {u8"%s a acheté %s (gratuit).", u8"%s comprou %s (grátis)."},
    {u8"En combat, tes chats ont trouvé %s et perdu %s : ça compte pour ta bourse.",
     u8"No combate, seus gatos acharam %s e perderam %s: conta para sua bolsa."},
    {u8"En combat, tes chats ont trouvé %s : c'est pour ta bourse.",
     u8"No combate, seus gatos acharam %s: vai para sua bolsa."},
    {u8"En combat, tes chats ont perdu %s : c'est pris sur ta bourse.",
     u8"No combate, seus gatos perderam %s: sai da sua bolsa."},
    {u8"En combat, tes chats n'ont rien trouvé.", u8"Seus gatos nada acharam no combate."},
    {u8"En combat, les chats d'%s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"No combate, os gatos de %s acharam %s e perderam %s: conta para a própria bolsa."},
    {u8"En combat, les chats de %s ont trouvé %s et perdu %s : ça compte pour sa bourse.",
     u8"No combate, os gatos de %s acharam %s e perderam %s: conta para a própria bolsa."},
    {u8"En combat, les chats d'%s ont trouvé %s : c'est pour sa bourse.",
     u8"No combate, os gatos de %s acharam %s: vai para a própria bolsa."},
    {u8"En combat, les chats de %s ont trouvé %s : c'est pour sa bourse.",
     u8"No combate, os gatos de %s acharam %s: vai para a própria bolsa."},
    {u8"En combat, les chats d'%s ont perdu %s : c'est pris sur sa bourse.",
     u8"No combate, os gatos de %s perderam %s: sai da própria bolsa."},
    {u8"En combat, les chats de %s ont perdu %s : c'est pris sur sa bourse.",
     u8"No combate, os gatos de %s perderam %s: sai da própria bolsa."},
    {u8"En combat, les chats d'%s n'ont rien trouvé.", u8"No combate, os gatos de %s não acharam nada."},
    {u8"En combat, les chats de %s n'ont rien trouvé.", u8"No combate, os gatos de %s não acharam nada."},
    {u8"Ton sac est ouvert\n"
     u8"\n"
     u8"L'hôte est entré dans une case :\n"
     u8"ferme ton sac pour le suivre.",
     u8"Sua mochila está aberta\n"
     u8"\n"
     u8"O anfitrião entrou em uma sala:\n"
     u8"feche a mochila para segui-lo."},
    {u8"Objets\n"
     u8"\n"
     u8"Ce chat est à %s : seul son propriétaire\n"
     u8"peut changer ses objets.",
     u8"Itens\n"
     u8"\n"
     u8"Este gato é de %s: só o dono\n"
     u8"pode mexer nos itens dele."},
    {u8"Objets\n"
     u8"\n"
     u8"Pendant une case (une boutique, une montée de\n"
     u8"niveau...), seul l'hôte change les objets.\n"
     u8"Équipe tes chats sur la carte.",
     u8"Itens\n"
     u8"\n"
     u8"Dentro de uma sala (uma loja, uma subida de\n"
     u8"nível...), só o anfitrião mexe nos itens.\n"
     u8"Equipe seus gatos no mapa."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte n'est pas encore revenu sur la carte :\n"
     u8"attends-le, ton sac se débloquera tout seul.",
     u8"Itens\n"
     u8"\n"
     u8"O anfitrião ainda não voltou ao mapa:\n"
     u8"espere, sua mochila vai se desbloquear sozinha."},
    {u8"Objets\n"
     u8"\n"
     u8"Ton sac se met à jour avec celui de l'hôte :\n"
     u8"ferme-le, puis rouvre-le dans un instant.",
     u8"Itens\n"
     u8"\n"
     u8"Sua mochila está sincronizando com a do anfitrião:\n"
     u8"feche-a e abra de novo em um instante."},
    {u8"Objets\n"
     u8"\n"
     u8"Un autre joueur a pris cet objet juste avant toi :\n"
     u8"ton changement n'a pas été pris. Ton sac revient\n"
     u8"comme chez l'hôte quand tu le fermes.",
     u8"Itens\n"
     u8"\n"
     u8"Outro jogador pegou este item antes de você:\n"
     u8"sua mudança não valeu. Ao fechar, sua mochila\n"
     u8"volta a ser igual à do anfitrião."},
    {u8"Objets\n"
     u8"\n"
     u8"L'hôte est entré dans une case avant que ton\n"
     u8"dernier changement n'arrive : il est annulé.\n"
     u8"Refais-le sur la carte.",
     u8"Itens\n"
     u8"\n"
     u8"O anfitrião entrou em uma sala antes da sua\n"
     u8"última mudança chegar: ela foi cancelada.\n"
     u8"Refaça no mapa."},
    {u8"Objets\n"
     u8"\n"
     u8"Cet objet n'est plus là : un autre joueur\n"
     u8"l'a pris juste avant toi.",
     u8"Itens\n"
     u8"\n"
     u8"Este item não está mais aqui: outro jogador\n"
     u8"o pegou logo antes de você."},
    {u8"%s n'est plus là : ses achats restent chez toi (%s).",
     u8"%s não está mais aqui: as compras ficam com você (%s)."},
    {u8"%s rapporte ses achats : %s.", u8"%s leva as compras para casa: %s."},
    {u8"Les achats d'%s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Não foi possível enviar as compras de %s: elas ficam com você."},
    {u8"Les achats de %s n'ont pas pu lui être envoyés : ils restent chez toi.",
     u8"Não foi possível enviar as compras de %s: elas ficam com você."},
    {u8"Achats de l'aventure", u8"Compras da aventura"},
    {u8"Boutique", u8"Loja"},
    {u8"%s n'ont pas fini dans cette\n"
     u8"boutique : attends-les (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour eux).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s não terminaram nesta\n"
     u8"loja: espere por eles (depois de sair, você não\n"
     u8"poderá mais comprar para eles).\n"
     u8"Clique de novo para sair mesmo assim."},
    {u8"%s n'a pas fini dans cette\n"
     u8"boutique : attends-le (une fois sorti, tu ne\n"
     u8"pourrais plus acheter pour lui).\n"
     u8"Clique encore pour sortir quand même.",
     u8"%s não terminou nesta\n"
     u8"loja: espere (depois de sair, você não\n"
     u8"poderá mais comprar para ele).\n"
     u8"Clique de novo para sair mesmo assim."},
    {u8"Ton achat n'a pas pu être demandé à l'hôte :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"Não foi possível pedir sua compra ao anfitrião:\n"
     u8"nada foi comprado. Tente de novo em um instante."},
    {u8"L'hôte n'est plus dans cette boutique :\n"
     u8"l'achat ne s'est pas fait.",
     u8"O anfitrião não está mais nesta loja:\n"
     u8"a compra não foi feita."},
    {u8"Cet objet n'est plus à vendre chez l'hôte :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Este item não está mais à venda na loja do anfitrião:\n"
     u8"a compra não foi feita."},
    {u8"Cet objet vient d'être vendu :\n"
     u8"l'achat ne s'est pas fait.",
     u8"Este item acabou de ser vendido:\n"
     u8"a compra não foi feita."},
    {u8"Il coûte %s ; ta bourse en a %lld.", u8"Ele custa %s; sua bolsa tem %lld."},
    {u8"Ta bourse ne suffit pas pour cet objet.", u8"Sua bolsa não tem o suficiente para este item."},
    {u8"(Chacun paie avec sa bourse, toi aussi.)", u8"(Cada um paga com a própria bolsa, você também.)"},
    {u8"(Chacun paie avec sa bourse.)", u8"(Cada um paga com a própria bolsa.)"},
    {u8"Pour l'instant, seul l'hôte achète les\n"
     u8"bonbons et les familiers (avec sa bourse).",
     u8"Por enquanto, só o anfitrião compra\n"
     u8"balas e familiares (com a própria bolsa)."},
    {u8"Le jeu n'a pas pu faire l'achat chez l'hôte.",
     u8"O jogo do anfitrião não conseguiu fazer a compra."},
    {u8"Pas de bourses dans cette aventure :\n"
     u8"seul l'hôte achète.",
     u8"Sem bolsas nesta aventura:\n"
     u8"só o anfitrião compra."},
    {u8"Tu n'as pas amené de chats dans cette\n"
     u8"aventure : tu n'as pas de bourse pour\n"
     u8"acheter.",
     u8"Você não levou gatos nesta\n"
     u8"aventura: não tem bolsa para\n"
     u8"comprar."},
    {u8"Tu n'as pas de bourse dans cette aventure\n"
     u8"(tu n'y as pas amené de chats) : pas d'achat.",
     u8"Você não tem bolsa nesta aventura\n"
     u8"(não levou gatos): sem compras."},
    {u8"L'hôte ne peut pas acheter pour toi en ce\n"
     u8"moment (une fenêtre est ouverte chez lui) :\n"
     u8"rien n'a été acheté. Réessaie dans un instant.",
     u8"O anfitrião não pode comprar para você agora\n"
     u8"(há uma janela aberta no jogo dele):\n"
     u8"nada foi comprado. Tente de novo em um instante."},
    {u8"un bonbon", u8"uma bala"},
    {u8"un familier", u8"um familiar"},
    {u8"un article", u8"um produto"},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentrent chez %s.",
     u8"Fim da aventura\n"
     u8"\n"
     u8"%s voltam para a casa de %s."},
    {u8"Fin de l'aventure\n"
     u8"\n"
     u8"%s rentre chez %s.",
     u8"Fim da aventura\n"
     u8"\n"
     u8"%s volta para a casa de %s."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu partir\n"
     u8"et ta boîte de départ est vide : pas d'aventure\n"
     u8"cette fois. (Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Os gatos dos outros jogadores não puderam partir\n"
     u8"e sua caixa de aventuras está vazia: sem aventura\n"
     u8"desta vez. (O registro do mod, mgmp.log, diz por quê.)"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ta boîte de départ est vide et personne n'amène\n"
     u8"de chat : pas d'aventure cette fois.",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Sua caixa de aventuras está vazia e ninguém leva\n"
     u8"gatos: sem aventura desta vez."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"Les chats des autres joueurs n'ont pas pu\n"
     u8"rejoindre l'équipe : tu pars avec les tiens.\n"
     u8"(Le journal du mod, mgmp.log, dit pourquoi.)",
     u8"Equipe mista\n"
     u8"\n"
     u8"Os gatos dos outros jogadores não puderam\n"
     u8"entrar na equipe: você parte com os seus.\n"
     u8"(O registro do mod, mgmp.log, diz por quê.)"},
    {u8"Retour de l'aventure", u8"Volta da aventura"},
    {u8"%s est de retour à la maison.", u8"%s voltou para casa."},
    {u8"%s sont de retour à la maison.", u8"%s voltaram para casa."},
    {u8"%s a péri pendant l'aventure.", u8"%s morreu durante a aventura."},
    {u8"%s ont péri pendant l'aventure.", u8"%s morreram durante a aventura."},
    {u8"%s ne revient pas de l'aventure.", u8"%s não volta da aventura."},
    {u8"%s ne reviennent pas de l'aventure.", u8"%s não voltam da aventura."},
    {u8"%s a changé depuis le départ :\n"
     u8"son retour d'aventure n'a pas été recopié.",
     u8"%s mudou desde a saída:\n"
     u8"o retorno da aventura não foi copiado."},
    {u8"%s ont changé depuis le départ :\n"
     u8"leur retour d'aventure n'a pas été recopié.",
     u8"%s mudaram desde a saída:\n"
     u8"o retorno da aventura não foi copiado."},
    {u8"Ton rangement a changé depuis le départ :\n"
     u8"les objets équipés au départ n'en ont pas été retirés.",
     u8"Seu armazenamento mudou desde a saída:\n"
     u8"os itens equipados na saída não foram retirados dele."},
    {u8"Ton rangement n'a pas été modifié : les objets\n"
     u8"équipés au départ y sont encore.",
     u8"Seu armazenamento não foi alterado: os itens\n"
     u8"equipados na saída ainda estão lá."},
    {u8"Tu rapportes %s dans ta maison.", u8"Você leva %s para casa."},
    {u8"Les PNJ te remercient de tes chats : %s dans ta maison.",
     u8"Os NPCs agradecem pelos seus gatos: %s na sua casa."},
    {u8"Cadeaux des PNJ", u8"Presentes dos NPCs"},
    {u8"Ton butin est dans ton rangement : %s.", u8"Seu saque está guardado: %s."},
    {u8"Les cadeaux des PNJ sont dans ton rangement : %s.",
     u8"Os presentes dos NPCs estão guardados: %s."},
    {u8"%s, recrutés pendant l'aventure, arrivent dans ta maison.",
     u8"%s, recrutados durante a aventura, chegam à sua casa."},
    {u8"%s, recruté pendant l'aventure, arrive dans ta maison.",
     u8"%s, recrutado durante a aventura, chega à sua casa."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Equipe mista\n"
     u8"\n"
     u8"%s não puderam entrar na equipe:\n"
     u8"nesta aventura, você comanda parte\n"
     u8"dos gatos da equipe."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"pour cette aventure, tu diriges une partie\n"
     u8"des chats de l'équipe.",
     u8"Equipe mista\n"
     u8"\n"
     u8"%s não pôde entrar na equipe:\n"
     u8"nesta aventura, você comanda parte\n"
     u8"dos gatos da equipe."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Equipe mista\n"
     u8"\n"
     u8"%s não puderam entrar na equipe:\n"
     u8"você acompanha sem comandar gatos."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu rejoindre l'équipe :\n"
     u8"tu suis l'aventure sans diriger de chat.",
     u8"Equipe mista\n"
     u8"\n"
     u8"%s não pôde entrar na equipe:\n"
     u8"você acompanha sem comandar gatos."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'ont pas pu partir et restent à la maison.",
     u8"Equipe mista\n"
     u8"\n"
     u8"%s não puderam partir e ficam em casa."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"%s n'a pas pu partir et reste à la maison.",
     u8"Equipe mista\n"
     u8"\n"
     u8"%s não pôde partir e fica em casa."},
    {u8"Tes chats sont rentrés de l'aventure.", u8"Seus gatos voltaram da aventura."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe de %s : ce chat est\n"
     u8"à toi ! Tant que tu n'es pas là, l'IA du jeu\n"
     u8"le joue en combat et %s choisit pour lui,\n"
     u8"et il rentrera chez toi à la fin de l'aventure.",
     u8"Novo gato na equipe\n"
     u8"\n"
     u8"%s entrou na equipe de %s: este gato\n"
     u8"é seu! Até você chegar, a IA do jogo o controla\n"
     u8"em combate e %s escolhe por ele, e ele\n"
     u8"vai para sua casa no fim da aventura."},
    {u8"Nouveaux chats : %s arrivent chez toi.", u8"Novos gatos: %s chegam à sua casa."},
    {u8"Nouveau chat : %s arrive chez toi.", u8"Novo gato: %s chega à sua casa."},
    {u8"La 4e place\n"
     u8"\n"
     u8"À 3 joueurs, l'équipe a une place libre :\n"
     u8"elle est à toi, tu emmènes 2 chats.",
     u8"A 4ª vaga\n"
     u8"\n"
     u8"Com 3 jogadores, sobra uma vaga:\n"
     u8"ela é sua, você leva 2 gatos."},
    {u8"La 4e place\n"
     u8"\n"
     u8"La place libre est déjà prise par %s :\n"
     u8"à 3 joueurs, tu emmènes 1 chat.\n"
     u8"Retire un chat de ta boîte de départ.",
     u8"A 4ª vaga\n"
     u8"\n"
     u8"A vaga livre já foi pega por %s:\n"
     u8"com 3 jogadores, você leva 1 gato.\n"
     u8"Tire um gato da caixa de aventuras."},
    {u8"La 4e place\n"
     u8"\n"
     u8"Un quatrième joueur est là : chacun emmène\n"
     u8"1 chat. Retire un chat de ta boîte de départ.",
     u8"A 4ª vaga\n"
     u8"\n"
     u8"Um quarto jogador chegou: cada um leva\n"
     u8"1 gato. Tire um gato da sua caixa de aventuras."},
    {u8"La 4e place va au premier qui met un 2e chat dans sa boîte.",
     u8"A 4ª vaga fica com quem puser primeiro um 2º gato na caixa."},
    {u8"La 4e place est à %s.", u8"A 4ª vaga é de %s."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Equipe mista\n"
     u8"\n"
     u8"Com %d jogadores, você leva no máximo %d gatos:\n"
     u8"tire %d gatos da caixa de aventuras.%s\n"
     u8"Os outros jogadores completam a equipe com os deles."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chats au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Equipe mista\n"
     u8"\n"
     u8"Com %d jogadores, você leva no máximo %d gatos:\n"
     u8"tire %d gato da caixa de aventuras.%s\n"
     u8"Os outros jogadores completam a equipe com os deles."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chats de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Equipe mista\n"
     u8"\n"
     u8"Com %d jogadores, você leva no máximo %d gato:\n"
     u8"tire %d gatos da caixa de aventuras.%s\n"
     u8"Os outros jogadores completam a equipe com os deles."},
    {u8"Équipe mixte\n"
     u8"\n"
     u8"À %d joueurs, tu emmènes %d chat au maximum :\n"
     u8"retire %d chat de ta boîte de départ.%s\n"
     u8"Les autres joueurs compléteront l'équipe avec les leurs.",
     u8"Equipe mista\n"
     u8"\n"
     u8"Com %d jogadores, você leva no máximo %d gato:\n"
     u8"tire %d gato da caixa de aventuras.%s\n"
     u8"Os outros jogadores completam a equipe com os deles."},
    {u8"Tu n'es pas dans ta maison :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Você não está na sua casa:\n"
     u8"vai jogar com os gatos dos outros."},
    {u8"Ta boîte de départ n'est pas encore débloquée :\n"
     u8"tu joueras avec les chats des autres.",
     u8"Sua caixa de aventuras ainda não foi desbloqueada:\n"
     u8"vai jogar com os gatos dos outros."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en 1 ou 2 dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Sua caixa de aventuras está vazia: vai jogar com os gatos dos outros.\n"
     u8"Para levar os seus: responda \"Ainda não\", coloque 1 ou 2 na caixa\n"
     u8"(do Butch) e clique na sua placa \"Partir!\"."},
    {u8"Ta boîte de départ est vide : tu joueras avec les chats des autres.\n"
     u8"Pour emmener les tiens : « Pas encore », mets-en un dans ta boîte\n"
     u8"(chez Butch), puis clique sur ton panneau « À l'aventure ! ».",
     u8"Sua caixa de aventuras está vazia: vai jogar com os gatos dos outros.\n"
     u8"Para levar os seus: responda \"Ainda não\", coloque um na caixa\n"
     u8"(do Butch) e clique na sua placa \"Partir!\"."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"réponds « Pas encore », retires-en %d,\n"
     u8"puis clique sur ton panneau « À l'aventure ! ».%s",
     u8"Sua caixa de aventuras tem %d gatos (máximo %d):\n"
     u8"responda \"Ainda não\", tire %d\n"
     u8"e clique na sua placa \"Partir!\".%s"},
    {u8"Tes chats partent de ta boîte de départ (chez Butch).",
     u8"Seus gatos partem da sua caixa de aventuras (do Butch)."},
    {u8"Tu emmènes %s.", u8"Você leva %s."},
    {u8"Départ en aventure", u8"Rumo à aventura"},
    {u8"La session n'est pas prête : réessaie dans un instant.",
     u8"A sessão não está pronta: tente de novo em um instante."},
    {u8"La 4e place est en train de t'être attribuée :\n"
     u8"la question revient dans un instant.",
     u8"A 4ª vaga está sendo atribuída a você:\n"
     u8"a pergunta volta em um instante."},
    {u8"Tes chats n'ont pas pu être préparés\n"
     u8"pour l'aventure (voir mgmp.log).",
     u8"Não foi possível preparar seus gatos\n"
     u8"para a aventura (veja mgmp.log)."},
    {u8"%s reviennent d'une aventure, mais leur retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de les faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s voltaram de uma aventura, mas o retorno\n"
     u8"ainda não foi copiado para o seu save: recarregue-o\n"
     u8"antes de mandá-los de novo (\"Recarregar\" vai\n"
     u8"aparecer após esta saída; senão Esc, \"Salvar e\n"
     u8"sair para o menu\", depois \"Jogar\" e seu save)."},
    {u8"%s revient d'une aventure, mais son retour\n"
     u8"n'est pas encore recopié dans ta save : recharge-la\n"
     u8"avant de le faire repartir (« Recharger » te sera\n"
     u8"proposé après ce départ ; sinon Échap, « Sauvegarder\n"
     u8"et retourner au menu », puis « Jouer » et ta save).",
     u8"%s voltou de uma aventura, mas o retorno\n"
     u8"ainda não foi copiado para o seu save: recarregue-o\n"
     u8"antes de mandá-lo de novo (\"Recarregar\" vai\n"
     u8"aparecer após esta saída; senão Esc, \"Salvar e\n"
     u8"sair para o menu\", depois \"Jogar\" e seu save)."},
    {u8"Le retour de %s attend d'être recopié.", u8"Falta copiar o retorno de %s."},
    {u8"L'envoi de ta réponse à l'hôte a échoué :\n"
     u8"réessaie.",
     u8"O envio da sua resposta ao anfitrião falhou:\n"
     u8"tente de novo."},
    {u8"Ta bourse de l'aventure : %s.", u8"Sua bolsa da aventura: %s."},
    {u8"Ta boîte de départ contient %d chats, %d au maximum :\n"
     u8"retires-en %d, puis clique sur ton panneau « À l'aventure ! ».",
     u8"Sua caixa de aventuras tem %d gatos (máximo %d):\n"
     u8"tire %d e clique na sua placa \"Partir!\"."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi !\n"
     u8"Tu le diriges, et il rentrera chez toi\n"
     u8"à la fin de l'aventure.",
     u8"Novo gato na equipe\n"
     u8"\n"
     u8"%s entrou na equipe: este gato é seu!\n"
     u8"Você o comanda. No fim da aventura,\n"
     u8"ele vai para a sua casa."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à toi.",
     u8"Novo gato na equipe\n"
     u8"\n"
     u8"%s entrou na equipe: este gato é seu."},
    {u8"Nouveau chat dans l'équipe\n"
     u8"\n"
     u8"%s rejoint l'équipe : ce chat est à %s.",
     u8"Novo gato na equipe\n"
     u8"\n"
     u8"%s entrou na equipe: o gato é de %s."},
    {u8"Un nouveau chat", u8"Um novo gato"},
    {u8"en bas", u8"embaixo"},
    {u8"en haut", u8"em cima"},
    {u8"à droite", u8"à direita"},
    {u8"à gauche", u8"à esquerda"},
    {u8"au milieu", u8"no meio"},
    {u8"n°%d", u8"nº %d"},
    {u8"%s (%s)", u8"%s (%s)"},
    {u8"[c:red]Ton vote n'a pas été pris : %s.[/c]", u8"[c:red]Seu voto não foi aceito: %s.[/c]"},
    {u8"Choix du chemin\n"
     u8"\n",
     u8"Escolha do caminho\n"
     u8"\n"},
    {u8"Même chemin", u8"Mesmo caminho"},
    {u8"Choisir", u8"Escolher"},
    {u8"Changer", u8"Mudar"},
    {u8"Sortie", u8"Saída"},
    {u8"Retour à la maison", u8"Volta para casa"},
    {u8"Combat", u8"Combate"},
    {u8"Combat difficile", u8"Combate difícil"},
    {u8"Mini-boss", u8"Mini chefe"},
    {u8"Boss", u8"Chefe"},
    {u8"Événement", u8"Evento"},
    {u8"Événement spécial", u8"Evento especial"},
    {u8"Trésor", u8"Tesouro"},
    {u8"Meuble", u8"Móvel"},
    {u8"Nourriture", u8"Comida"},
    {u8"Bonus", u8"Bônus"},
    {u8"Case vide", u8"Sala vazia"},
    {u8"Case", u8"Sala"},
    {u8"l'hôte n'est pas sur la carte", u8"o anfitrião não está no mapa"},
    {u8"vos cartes ne sont pas les mêmes", u8"os mapas de vocês não são iguais"},
    {u8"l'hôte ne peut pas aller sur cette case", u8"o anfitrião não pode ir para essa sala"},
    {u8"le chemin est déjà choisi", u8"o caminho já foi escolhido"},
    {u8"ce tour est fini", u8"esta rodada acabou"},
    {u8"l'hôte ne l'a pas pris", u8"o anfitrião não o aceitou"},
    {u8"pas de vote", u8"sem voto"},
    {u8"absent, ne vote pas", u8"ausente, não vota"},
    {u8"pas encore sur la carte", u8"ainda não está no mapa"},
    {u8"pas encore voté", u8"ainda não votou"},
    {u8"%s (toi) : %s", u8"%s (você): %s"},
    {u8"%s : %s", u8"%s: %s"},
    {u8"En pause : on attend tout le monde sur la carte.", u8"Em pausa: esperando todos no mapa."},
    {u8"À toi de choisir : encore %u s.", u8"Sua vez de escolher: faltam %u s."},
    {u8"En attente des autres : encore %u s.", u8"Aguardando os outros: faltam %u s."},
    {u8"Égalité entre %s :\n"
     u8"le sort choisit %s.",
     u8"Empate entre %s:\n"
     u8"a sorte escolhe %s."},
    {u8"Tout le monde a choisi %s.", u8"Todos escolheram %s."},
    {u8"%s l'emporte.", u8"%s venceu."},
    {u8"%s : prêt", u8"%s: pronto"},
    {u8"%s : défend sa maison", u8"%s: defendendo a casa"},
    {u8"%s : pas encore", u8"%s: ainda não"},
    {u8"Tout le monde est prêt !", u8"Todos estão prontos!"},
    {u8"En attente d'un joueur.", u8"Aguardando um jogador."},
    {u8"En attente de %d joueurs.", u8"Aguardando %d jogadores."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"Tu es prêt à passer la nuit.\n"
     u8"%s\n"
     u8"\n"
     u8"%s",
     u8"Fim do dia\n"
     u8"\n"
     u8"Você está pronto para passar a noite.\n"
     u8"%s\n"
     u8"\n"
     u8"%s"},
    {u8"\n"
     u8"Ton panneau sert au départ en aventure :\n"
     u8"retire tes chats de l'équipe pour finir la journée.",
     u8"\n"
     u8"Sua placa serve para partir em aventura:\n"
     u8"tire seus gatos da equipe para finalizar o dia."},
    {u8"Fin de journée\n"
     u8"\n"
     u8"1 joueur attend pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Fim do dia\n"
     u8"\n"
     u8"1 jogador espera para passar a noite.\n"
     u8"\n"
     u8"%s%s"},
    {u8"Fin de journée\n"
     u8"\n"
     u8"%d joueurs attendent pour passer la nuit.\n"
     u8"\n"
     u8"%s%s",
     u8"Fim do dia\n"
     u8"\n"
     u8"%d jogadores esperam para passar a noite.\n"
     u8"\n"
     u8"%s%s"},
    {u8"1 joueur attend", u8"1 jogador espera"},
    {u8"%d joueurs attendent", u8"%d jogadores esperam"},
    {u8"Passer la nuit", u8"Passar a noite"},
    {u8"Plus tard", u8"Mais tarde"},
    {u8"Pour continuer, appuie sur Échap\n"
     u8"puis sur « Sauvegarder et retourner au menu ».",
     u8"Para continuar, aperte Esc\n"
     u8"e depois \"Salvar e sair para o menu\"."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. « Recharger » :\n"
     u8"le mod passe par le menu et la rouvre.",
     u8"O que espera por você\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Seu save já está aberto: tudo isso só estará\n"
     u8"nele no próximo carregamento. \"Recarregar\":\n"
     u8"o mod passa pelo menu e o reabre."},
    {u8"Ce qui t'attend\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Ta save est déjà ouverte : tout cela n'y sera\n"
     u8"qu'à son prochain chargement. Recharge-la :\n"
     u8"Échap, « Sauvegarder et retourner au menu »,\n"
     u8"puis « Jouer » et ta save.",
     u8"O que espera por você\n"
     u8"\n"
     u8"%s\n"
     u8"\n"
     u8"Seu save já está aberto: tudo isso só estará\n"
     u8"nele no próximo carregamento. Recarregue-o:\n"
     u8"Esc, \"Salvar e sair para o menu\",\n"
     u8"depois \"Jogar\" e seu save."},
    {u8"%s : j'arrive !", u8"%s: já vou!"},
    {u8"%s : réfléchit...", u8"%s: pensando..."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu veux partir à l'aventure.\n"
     u8"Ton équipe répond :\n"
     u8"\n"
     u8"%s",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Você quer partir em aventura.\n"
     u8"Sua equipe responde:\n"
     u8"\n"
     u8"%s"},
    {u8"%s : pas encore là", u8"%s: ainda não chegou"},
    {u8"En attente de ton équipe\n"
     u8"\n"
     u8"%s\n"
     u8"Si tu avances sans l'attendre, l'IA du jeu\n"
     u8"joue ses chats dans les combats jusqu'à\n"
     u8"son arrivée.",
     u8"Aguardando sua equipe\n"
     u8"\n"
     u8"%s\n"
     u8"Se você avançar sem esperar, a IA do jogo\n"
     u8"controla os gatos deles nos combates até\n"
     u8"eles chegarem."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"%s part à l'aventure !\n"
     u8"Chacun emmène ses chats et dirige les siens.\n"
     u8"\n"
     u8"%s",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"%s vai partir em aventura!\n"
     u8"Cada um leva e comanda os próprios gatos.\n"
     u8"\n"
     u8"%s"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Tes chats ne sont pas du voyage : tu suivras\n"
     u8"l'aventure sans diriger de chat.",
     u8"Aventura em andamento\n"
     u8"\n"
     u8"%s está em aventura!\n"
     u8"Seus gatos não vão nesta viagem: você vai\n"
     u8"acompanhar sem comandar gatos."},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure avec tes chats !\n"
     u8"Tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux.\n"
     u8"Rejoins son équipe ?",
     u8"Aventura em andamento\n"
     u8"\n"
     u8"%s está em aventura com seus gatos!\n"
     u8"Até você chegar, a IA do jogo os controla\n"
     u8"em combate e %s escolhe por eles.\n"
     u8"Entrar na equipe dele?"},
    {u8"Aventure en cours\n"
     u8"\n"
     u8"%s est en aventure !\n"
     u8"Rejoins son équipe ?",
     u8"Aventura em andamento\n"
     u8"\n"
     u8"%s está em aventura!\n"
     u8"Entrar na equipe dele?"},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"L'aventure de %s est terminée :\n"
     u8"tu rentres dans ta maison.",
     u8"Sair da aventura\n"
     u8"\n"
     u8"A aventura de %s terminou:\n"
     u8"você volta para sua casa."},
    {u8"Quitter l'aventure\n"
     u8"\n"
     u8"C'est l'aventure de %s : seul lui peut\n"
     u8"l'abandonner. Elle continue sans toi.\n"
     u8"\n"
     u8"Tu rentres dans ta maison ? Pour revenir,\n"
     u8"clique sur un panneau de ta maison.",
     u8"Sair da aventura\n"
     u8"\n"
     u8"A aventura é de %s: só ele pode\n"
     u8"abandoná-la. Ela continua sem você.\n"
     u8"\n"
     u8"Ir para casa? Para voltar à aventura,\n"
     u8"clique em uma placa da sua casa."},
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
     u8"Sair da aventura\n"
     u8"\n"
     u8"A aventura é de %s: só ele pode\n"
     u8"abandoná-la. Ela continua sem você, e seus\n"
     u8"gatos ficam na equipe dele: a IA do jogo\n"
     u8"os controla em combate, e eles voltam para\n"
     u8"sua casa no fim.\n"
     u8"\n"
     u8"Ir para casa? Para voltar à aventura,\n"
     u8"clique em uma placa da sua casa."},
    {u8"Rejoindre l'aventure\n"
     u8"\n"
     u8"Le jeu de %s t'envoie l'aventure en cours.\n"
     u8"S'il vient de faire un choix dans une case\n"
     u8"(événement, montée de niveau), tu arriveras\n"
     u8"à son retour sur la carte.",
     u8"Entrar na aventura\n"
     u8"\n"
     u8"O jogo de %s está enviando a aventura para você.\n"
     u8"Se ele acabou de fazer uma escolha em uma sala\n"
     u8"(evento, subida de nível), você chega\n"
     u8"quando ele voltar ao mapa."},
    {u8"En route pour l'aventure\n"
     u8"\n"
     u8"La partie de %s va se charger :\n"
     u8"tu partiras avec lui.",
     u8"A caminho da aventura\n"
     u8"\n"
     u8"A partida de %s vai carregar:\n"
     u8"você vai partir com ele."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"La connexion avec %s est coupée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Volta para casa\n"
     u8"\n"
     u8"A conexão com %s caiu.\n"
     u8"Você volta para sua casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le départ avec %s n'a pas eu lieu.\n"
     u8"Tu rentres dans ta maison.",
     u8"Volta para casa\n"
     u8"\n"
     u8"A saída com %s não aconteceu.\n"
     u8"Você volta para sua casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"L'aventure de %s est terminée.\n"
     u8"Tu rentres dans ta maison.",
     u8"Volta para casa\n"
     u8"\n"
     u8"A aventura de %s terminou.\n"
     u8"Você volta para sua casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Le combat de %s était fini, mais ta copie\n"
     u8"continuait : elle s'était désynchronisée,\n"
     u8"plus personne ne jouait ce combat. Tu es\n"
     u8"rentré ; tes chats restent dans son équipe.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Volta para casa\n"
     u8"\n"
     u8"O combate de %s tinha acabado, mas sua cópia\n"
     u8"continuava: ela tinha dessincronizado,\n"
     u8"ninguém mais jogava esse combate. Você\n"
     u8"voltou; seus gatos ficam na equipe dele.\n"
     u8"\n"
     u8"Para voltar à aventura, clique em\n"
     u8"uma placa da sua casa."},
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
     u8"Volta para casa\n"
     u8"\n"
     u8"Seu combate acabou, mas o de %s\n"
     u8"continuava: sua cópia tinha\n"
     u8"dessincronizado. Você voltou; seus gatos\n"
     u8"ficam na equipe dele, e a IA do jogo os\n"
     u8"controla até o fim do combate.\n"
     u8"\n"
     u8"Para voltar à aventura, clique em\n"
     u8"uma placa da sua casa."},
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
     u8"Volta para casa\n"
     u8"\n"
     u8"%s entrou em uma sala que seu jogo\n"
     u8"não conseguiu seguir: sua cópia da aventura\n"
     u8"não era mais a mesma. Você voltou; seus\n"
     u8"gatos ficam na equipe dele, e a IA do jogo\n"
     u8"os controla.\n"
     u8"\n"
     u8"Para voltar à aventura, clique em\n"
     u8"uma placa da sua casa."},
    {u8"Retour à la maison\n"
     u8"\n"
     u8"Ta copie du combat s'est désynchronisée de\n"
     u8"celle de %s : le combat continuait sans toi,\n"
     u8"alors tu es rentré. Tes chats restent dans\n"
     u8"son équipe, l'IA du jeu les joue.\n"
     u8"\n"
     u8"Pour revenir dans l'aventure, clique sur\n"
     u8"un panneau de ta maison.",
     u8"Volta para casa\n"
     u8"\n"
     u8"Sua cópia do combate dessincronizou da\n"
     u8"de %s: o combate continuava sem você,\n"
     u8"então você voltou. Seus gatos ficam na\n"
     u8"equipe dele, e a IA do jogo os controla.\n"
     u8"\n"
     u8"Para voltar à aventura, clique em\n"
     u8"uma placa da sua casa."},
    {u8"Ta save est en pleine aventure.\n"
     u8"En multijoueur, les aventures se jouent avec l'hôte :\n"
     u8"retourne au menu et choisis une autre save,\n"
     u8"ou finis celle-ci en solo.",
     u8"Seu save está no meio de uma aventura.\n"
     u8"No multijogador, as aventuras são jogadas com o anfitrião:\n"
     u8"volte ao menu e escolha outro save,\n"
     u8"ou termine este sozinho."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Ton jeu charge l'aventure de %s.",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Carregando a aventura de %s..."},
    {u8"%s a rechargé l'aventure :\n"
     u8"tu la recharges aussi.",
     u8"%s recarregou a aventura:\n"
     u8"você também vai recarregá-la."},
    {u8"Rentrer", u8"Ir para casa"},
    {u8"Rester", u8"Ficar"},
    {u8"Ne pas attendre", u8"Não esperar"},
    {u8"Recharger", u8"Recarregar"},
    {u8"J'arrive", u8"Já vou"},
    {u8"Pas encore", u8"Ainda não"},
    {u8"Échanger", u8"Trocar"},
    {u8"Multijoueur", u8"Multijogador"},
    {u8"Ce tuyau est celui de la maison de %s :\n"
     u8"envoie tes chats aux PNJ depuis\n"
     u8"ta propre maison.",
     u8"Este cano é o da casa de %s:\n"
     u8"mande seus gatos aos NPCs a partir da\n"
     u8"sua própria casa."},
    {u8"Les PNJ suivent la partie de %s :\n"
     u8"envoyer tes chats dans le tuyau n'est\n"
     u8"pas possible avec cette version du jeu.",
     u8"Os NPCs seguem a partida de %s:\n"
     u8"mandar seus gatos pelo cano não é\n"
     u8"possível com esta versão do jogo."},
    {u8"\n"
     u8"\n"
     u8"C'est une défaite pour toute l'équipe :\n"
     u8"les chats de %s ne rentreront pas non plus.",
     u8"\n"
     u8"\n"
     u8"É uma derrota para a equipe toda:\n"
     u8"os gatos de %s também não vão voltar."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Tu es déjà dans un salon multijoueur :\n"
     u8"quitte-le d'abord (Multijoueur, puis « Quitter »),\n"
     u8"puis clique à nouveau sur « Rejoindre la partie ».",
     u8"Juntar-se a %s\n"
     u8"\n"
     u8"Você já está em um lobby multijogador:\n"
     u8"saia dele primeiro (Multijogador, depois \"Sair\"),\n"
     u8"e clique de novo em \"Entrar no jogo\"."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu va revenir au menu principal (ta partie\n"
     u8"est sauvegardée, comme avec « Sauvegarder et retourner au menu »),\n"
     u8"puis rejoindre le salon de %s.\n"
     u8"\n"
     u8"Plus tard : il le rejoindra quand tu iras\n"
     u8"au menu principal.",
     u8"Juntar-se a %s\n"
     u8"\n"
     u8"Seu jogo vai voltar ao menu principal (sua partida\n"
     u8"é salva, como com \"Salvar e sair para o menu\"),\n"
     u8"e depois entrar no lobby de %s.\n"
     u8"\n"
     u8"Mais tarde: seu jogo entra quando você for\n"
     u8"ao menu principal."},
    {u8"Y aller", u8"Ir agora"},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"Ton jeu rejoindra le salon de %s\n"
     u8"dès que tu seras au menu principal\n"
     u8"(Échap, puis « Sauvegarder et retourner au menu »).",
     u8"Juntar-se a %s\n"
     u8"\n"
     u8"Seu jogo vai entrar no lobby de %s\n"
     u8"assim que você for ao menu principal\n"
     u8"(Esc, depois \"Salvar e sair para o menu\")."},
    {u8"Rejoindre %s\n"
     u8"\n"
     u8"La connexion par Steam est impossible pour l'instant.\n"
     u8"Réessaie dans un moment, ou rejoins par adresse\n"
     u8"(Multijoueur, puis « Rejoindre »).",
     u8"Juntar-se a %s\n"
     u8"\n"
     u8"A conexão pela Steam não é possível no momento.\n"
     u8"Tente de novo daqui a pouco ou entre por endereço\n"
     u8"(Multijogador, depois \"Entrar\")."},
    {u8"%s a annulé le départ.", u8"%s cancelou a saída."},
    {u8"réseau local", u8"rede local"},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tes chats sont déjà partis avec l'équipe de %s :\n"
     u8"tant que tu n'es pas là, l'IA du jeu les joue\n"
     u8"en combat et %s choisit pour eux ; ils rentreront\n"
     u8"chez toi à la fin de l'aventure.",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Seus gatos já partiram com a equipe de %s:\n"
     u8"até você chegar, a IA do jogo os controla\n"
     u8"em combate e %s escolhe por eles; eles voltam\n"
     u8"para sua casa no fim da aventura."},
    {u8"Internet", u8"Internet"},
    {u8"%s défend sa maison : un combat qu'il joue\n"
     u8"seul, avec ses chats. Il revient ensuite.",
     u8"%s está defendendo a casa: um combate solo,\n"
     u8"com os gatos dele. Depois ele volta."},
    {u8"%s défendent leur maison : chacun joue\n"
     u8"son combat seul, avec ses chats. Ils reviennent ensuite.",
     u8"%s defendem as próprias casas: cada um joga\n"
     u8"seu combate sozinho, com os próprios gatos. Depois voltam."},
    {u8"%s a fini de défendre sa maison.", u8"%s terminou de defender a casa."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défend la sienne encore.",
     u8"%s terminou de defender a casa.\n"
     u8"%s ainda defende a própria casa."},
    {u8"%s a fini de défendre sa maison.\n"
     u8"%s défendent la leur encore.",
     u8"%s terminou de defender a casa.\n"
     u8"%s ainda defendem as próprias casas."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Tu as annulé après le vote : %s part\n"
     u8"sans toi ni tes chats. Tu pourras rejoindre\n"
     u8"son aventure une fois qu'il sera sur la carte.",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Você cancelou após o voto: %s parte\n"
     u8"sem você e sem seus gatos. Você poderá entrar\n"
     u8"na aventura quando ele estiver no mapa."},
    {u8"Départ en aventure\n"
     u8"\n"
     u8"Aucun chat ne peut partir cette fois :\n"
     u8"%s ne part pas à l'aventure.",
     u8"Rumo à aventura\n"
     u8"\n"
     u8"Nenhum gato pode partir desta vez:\n"
     u8"%s não vai sair em aventura."},
    {u8"Le partage du butin n'est pas fini :\n"
     u8"attends la fin du défilé des objets.",
     u8"A divisão do saque não terminou:\n"
     u8"espere o desfile dos itens acabar."},
    {u8"Tu pars rejoindre l'aventure de %s :\n"
     u8"tu défendras ta maison à ton retour.",
     u8"Você vai entrar na aventura de %s:\n"
     u8"vai defender sua casa quando voltar."},
    {u8"C'est %s qui lance l'aventure :\n"
     u8"demande-lui de cliquer sur « À l'aventure ! ».",
     u8"Quem inicia a aventura é %s:\n"
     u8"peça a ele para clicar em \"Partir!\"."},
    {u8"Multijoueur\n"
     u8"\n"
     u8"Héberge une partie pour que tes amis te rejoignent,\n"
     u8"ou rejoins la partie d'un ami.",
     u8"Multijogador\n"
     u8"\n"
     u8"Hospede uma partida para seus amigos entrarem,\n"
     u8"ou entre na partida de um amigo."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible d'ouvrir le port %u.[/c]\n"
     u8"Un autre programme l'utilise peut-être.\n"
     u8"Arrête le salon, puis réessaie.",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"[c:red]Não foi possível abrir a porta %u.[/c]\n"
     u8"Talvez outro programa esteja usando.\n"
     u8"Encerre o lobby e tente de novo."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Ouverture du salon...",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Abrindo o lobby..."},
    {u8"%s (hôte)", u8"%s (anfitrião)"},
    {u8"En attente d'un ami...", u8"Aguardando um amigo..."},
    {u8"Tes amis Steam te rejoignent en un clic : dans leur\n"
     u8"liste d'amis Steam, « Rejoindre la partie » sur ton nom.\n"
     u8"\n",
     u8"Seus amigos da Steam entram com um clique: na lista\n"
     u8"de amigos da Steam, \"Entrar no jogo\" no seu nome.\n"
     u8"\n"},
    {u8"Ou par adresse :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Ou por endereço:\n"
     u8"%s   (%s, porta %u)\n"
     u8"%s"},
    {u8"Donne cette adresse à ton ami :\n"
     u8"%s   (%s, port %u)\n"
     u8"%s",
     u8"Passe este endereço para seu amigo:\n"
     u8"%s   (%s, porta %u)\n"
     u8"%s"},
    {u8"Elle est déjà copiée : colle-la-lui avec Ctrl+V.\n",
     u8"Já está copiado: cole para seu amigo com Ctrl+V.\n"},
    {u8"Adresse introuvable sur ce PC (port %u).\n",
     u8"Endereço não encontrado neste PC (porta %u).\n"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"%s\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Quand tout le monde est là, choisis ta sauvegarde.",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"%s\n"
     u8"Jogadores (%d/4):\n"
     u8"%s\n"
     u8"Quando todos estiverem aqui, escolha seu save."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"Adresse de l'hôte :\n"
     u8"%s_\n"
     u8"\n"
     u8"Tape l'adresse que ton ami t'a donnée,\n"
     u8"ou colle-la avec Ctrl+V. Entrée pour valider.%s",
     u8"Entrar em uma partida\n"
     u8"\n"
     u8"Endereço do anfitrião:\n"
     u8"%s_\n"
     u8"\n"
     u8"Digite o endereço do seu amigo,\n"
     u8"ou cole com Ctrl+V. Enter para confirmar.%s"},
    {u8"\n"
     u8"\n"
     u8"Par Steam, c'est plus simple : dans ta liste\n"
     u8"d'amis, « Rejoindre la partie » sur ton ami.",
     u8"\n"
     u8"\n"
     u8"Pela Steam é mais simples: na sua lista\n"
     u8"de amigos, \"Entrar no jogo\" no nome do seu amigo."},
    {u8"Rejoindre une partie\n"
     u8"\n"
     u8"[c:red]Adresse invalide : %s[/c]\n"
     u8"\n"
     u8"Exemple : 26.123.45.67\n"
     u8"ou, avec un autre port : 26.123.45.67:27600",
     u8"Entrar em uma partida\n"
     u8"\n"
     u8"[c:red]Endereço inválido: %s[/c]\n"
     u8"\n"
     u8"Exemplo: 26.123.45.67\n"
     u8"ou, com outra porta: 26.123.45.67:27600"},
    {u8"(vide)", u8"(vazio)"},
    {u8"Vérifiez que vous avez la même version du jeu et du mod.",
     u8"Verifiquem se vocês têm a mesma versão do jogo e do mod."},
    {u8"La partie de l'hôte est complète (4 joueurs au plus).",
     u8"A partida do anfitrião está cheia (máximo de 4 jogadores)."},
    {u8"Vous n'avez pas la même version du mod :\n"
     u8"prenez tous les deux la dernière.",
     u8"Vocês não têm a mesma versão do mod:\n"
     u8"os dois devem usar a mais recente."},
    {u8"Vous n'avez pas la même version du jeu :\n"
     u8"mettez Mewgenics à jour tous les deux (Steam).",
     u8"Vocês não têm a mesma versão do jogo:\n"
     u8"os dois devem atualizar o Mewgenics (Steam)."},
    {u8"Les fichiers du jeu ne sont pas les mêmes :\n"
     u8"vérifiez-les dans Steam (Propriétés, Fichiers installés).",
     u8"Os arquivos do jogo não são os mesmos:\n"
     u8"verifiquem na Steam (Propriedades, Arquivos instalados)."},
    {u8"Le réglage net.control de mgmp.json donne le même chat\n"
     u8"aux deux joueurs : remettez-le sur \"auto\".",
     u8"A configuração net.control do mgmp.json dá o mesmo gato\n"
     u8"aos dois jogadores: coloquem de volta em \"auto\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion a été refusée.[/c]\n"
     u8"%s\n"
     u8"(%s)",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"[c:red]A conexão foi recusada.[/c]\n"
     u8"%s\n"
     u8"(%s)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de rejoindre %s :[/c]\n"
     u8"%s.\n"
     u8"Quitte le salon, puis réessaie (ou par adresse).",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"[c:red]Não foi possível se juntar a %s:[/c]\n"
     u8"%s.\n"
     u8"Saia do lobby e tente de novo (ou por endereço)."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Conectando a %s..."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]Impossible de joindre %s.[/c]\n"
     u8"Vérifie l'adresse, et que ton ami a bien\n"
     u8"cliqué sur « Héberger ».",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"[c:red]Não foi possível conectar a %s.[/c]\n"
     u8"Verifique o endereço e se seu amigo\n"
     u8"clicou em \"Hospedar\"."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Déconnecté.",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Desconectado."},
    {u8"\n"
     u8"\n"
     u8"L'écran des sauvegardes s'ouvrira tout seul.",
     u8"\n"
     u8"\n"
     u8"A tela de saves vai abrir sozinha."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s...%s",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Conectando a %s...%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"[c:red]La connexion avec l'hôte est coupée.[/c]\n"
     u8"Quitte le salon, puis rejoins-le à nouveau.",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"[c:red]A conexão com o anfitrião caiu.[/c]\n"
     u8"Saia do lobby e entre nele de novo."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connexion à %s par Steam...\n"
     u8"(quelques secondes)%s",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Conectando a %s pela Steam...\n"
     u8"(alguns segundos)%s"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Connecté, présentation à l'hôte...%s",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Conectado, falando com o anfitrião...%s"},
    {u8"%s (toi)", u8"%s (você)"},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"l'écran des sauvegardes s'ouvrira tout seul.",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Você entrou na partida de %s!\n"
     u8"\n"
     u8"Jogadores (%d/4):\n"
     u8"%s\n"
     u8"Aguardando a partida de %s:\n"
     u8"assim que ele escolher o save dele,\n"
     u8"a tela de saves vai abrir sozinha."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"En attente de la partie de %s :\n"
     u8"dès qu'il aura choisi sa sauvegarde,\n"
     u8"tu pourras choisir la tienne.",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Você entrou na partida de %s!\n"
     u8"\n"
     u8"Jogadores (%d/4):\n"
     u8"%s\n"
     u8"Aguardando a partida de %s:\n"
     u8"assim que ele escolher o save dele,\n"
     u8"você poderá escolher o seu."},
    {u8"Salon multijoueur\n"
     u8"\n"
     u8"Tu as rejoint la partie de %s !\n"
     u8"\n"
     u8"Joueurs (%d/4) :\n"
     u8"%s\n"
     u8"Choisis ta sauvegarde : ta maison, tes chats.\n"
     u8"Les PNJ et l'histoire seront ceux de %s,\n"
     u8"et pour partir à l'aventure, tu rejoindras son équipe.",
     u8"Lobby multijogador\n"
     u8"\n"
     u8"Você entrou na partida de %s!\n"
     u8"\n"
     u8"Jogadores (%d/4):\n"
     u8"%s\n"
     u8"Escolha seu save: sua casa, seus gatos.\n"
     u8"Os NPCs e a história serão os de %s,\n"
     u8"e para partir em aventura, você entra na equipe dele."},
    {u8"Héberger", u8"Hospedar"},
    {u8"Rejoindre", u8"Entrar"},
    {u8"Choisir la sauvegarde", u8"Escolher o save"},
    {u8"Arrêter", u8"Encerrar"},
    {u8"Choisir ta sauvegarde", u8"Escolher seu save"},
    {u8"Quitter", u8"Sair"},
    {u8"Annuler", u8"Cancelar"},
    {u8"Échanger avec %s", u8"Trocar com %s"},
    {u8"Donner à qui ?", u8"Dar para quem?"},
    {u8"Haut ou Bas pour choisir, puis Entrée.", u8"Cima/Baixo: escolher, depois Enter."},
    {u8"Pièces à donner : %lld_", u8"Moedas para dar: %lld_"},
    {u8"Nourriture à donner : %lld_", u8"Comida para dar: %lld_"},
    {u8"Tape un nombre (T : tout), puis Entrée.", u8"Digite um valor (T: tudo), depois Enter."},
    {u8"Échanger avec %s : les objets de ton rangement", u8"Trocar com %s: itens do seu armazenamento"},
    {u8"16 au plus par don : %u choisis.", u8"Até 16 por presente: %u escolhidos."},
    {u8"16 au plus par don : %u choisi.", u8"Até 16 por presente: %u escolhido."},
    {u8"Lecture du rangement...", u8"Lendo o armazenamento..."},
    {u8"Haut/Bas : choisir, Espace : cocher, Gauche/Droite : combien.\n"
     u8"Des lettres pour chercher, puis Entrée.",
     u8"Cima/Baixo: escolher, Espaço: marcar, Esquerda/Direita: quantos.\n"
     u8"Letras para buscar, depois Enter."},
    {u8"Tu n'as rien choisi à donner.", u8"Você não escolheu nada para dar."},
    {u8"Tu n'as rien à donner pour l'instant.", u8"Você não tem nada para dar agora."},
    {u8"Donner %s de ta bourse\n"
     u8"à %s ?",
     u8"Dar %s da sua bolsa\n"
     u8"para %s?"},
    {u8"Donner %s\n"
     u8"à %s ?",
     u8"Dar %s\n"
     u8"para %s?"},
    {u8"Envoi du don...", u8"Enviando o presente..."},
    {u8"Personne ne peut recevoir maintenant.", u8"Ninguém pode receber agora."},
    {u8"Personne d'autre n'est dans la partie.", u8"Não há mais ninguém na partida."},
    {u8"Confirmer", u8"Confirmar"},
    {u8"Donner", u8"Dar"},
    {u8"Recommencer", u8"Recomeçar"},
    {u8"Fermer", u8"Fechar"},
    {u8"Pas d'échange possible ici.", u8"Nenhuma troca possível aqui."},
    {u8"La fenêtre s'est fermée : l'hôte est entré dans une case.\n"
     u8"Ce que tu avais choisi est gardé (E pour reprendre).",
     u8"A janela fechou: o anfitrião entrou em uma sala.\n"
     u8"O que você escolheu foi guardado (E para retomar)."},
    {u8"Échanger en aventure\n"
     u8"\n"
     u8"Sur la carte, appuie sur E pour donner des pièces\n"
     u8"ou de la nourriture de ta bourse à un autre joueur.",
     u8"Trocar na aventura\n"
     u8"\n"
     u8"No mapa, aperte E para dar moedas\n"
     u8"ou comida da sua bolsa a outro jogador."},
    {nullptr, nullptr}   // fin de la table
};

} // namespace mm
