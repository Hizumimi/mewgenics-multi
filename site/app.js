// Mewgenics Multi : langue, traduction, bouton Copier, numéro de la dernière version.
// Chargé dans <head> (sans defer) pour choisir la langue avant le premier affichage.
(function () {
  "use strict";
  var LANGUES = ["fr", "en", "es", "de", "it", "pt-BR", "ru", "ko", "ja", "zh-CN"];
  // Google Fonts à ajouter pour les écritures que les polices de base n'ont pas (le russe est déjà couvert).
  var POLICES = {
    ko: "Jua&family=Gowun+Dodum",
    ja: "Mochiy+Pop+One&family=Kosugi+Maru",
    "zh-CN": "ZCOOL+KuaiLe"
  };
  var CLE = "mm-langue";
  var html = document.documentElement;

  // "PT-br" -> "pt-BR" ; "pt-PT" -> "pt-BR" ; "zh-TW" -> "zh-CN" ; inconnu -> null
  // shortcut: le chinois traditionnel tombe sur zh-CN, à changer si une table zh-TW arrive.
  function trouver(code, prefixe) {
    code = String(code || "").toLowerCase();
    for (var i = 0; i < LANGUES.length; i++) {
      var l = LANGUES[i].toLowerCase();
      if (l === code || (prefixe && l.split("-")[0] === code.split("-")[0])) return LANGUES[i];
    }
    return null;
  }

  function choisir() {
    var l = trouver(new URLSearchParams(location.search).get("lang"));
    if (l) return l;
    try { l = trouver(localStorage.getItem(CLE)); if (l) return l; } catch (e) {}
    var nav = navigator.languages || [navigator.language];
    for (var i = 0; i < nav.length; i++) { l = trouver(nav[i], true); if (l) return l; }
    return "en";
  }

  function appliquer(t) {
    var i, els = document.querySelectorAll("[data-i18n]");
    for (i = 0; i < els.length; i++) if (t[els[i].getAttribute("data-i18n")] != null) els[i].textContent = t[els[i].getAttribute("data-i18n")];
    els = document.querySelectorAll("[data-i18n-html]");
    for (i = 0; i < els.length; i++) if (t[els[i].getAttribute("data-i18n-html")] != null) els[i].innerHTML = t[els[i].getAttribute("data-i18n-html")];
    els = document.querySelectorAll("[data-i18n-attr]");
    for (i = 0; i < els.length; i++) {
      var paires = els[i].getAttribute("data-i18n-attr").split(",");
      for (var j = 0; j < paires.length; j++) {
        var p = paires[j].split(":"), cle = (p[1] || "").trim();
        if (t[cle] != null) els[i].setAttribute(p[0].trim(), t[cle]);
      }
    }
  }

  function pret(fn) {
    if (document.readyState !== "loading") fn();
    else document.addEventListener("DOMContentLoaded", fn);
  }

  // <html lang>, classe lang-xx (polices par écriture) et sélecteur disent la langue vraiment affichée.
  var langue;
  function poser(code) {
    langue = code;
    html.lang = code;
    html.className = (html.className.replace(/\blang-\S+/g, "") + " lang-" + code).trim();
    var choix = document.getElementById("langue");
    if (choix) choix.value = code;
  }

  poser(choisir());

  // Le français est déjà dans la page : on ne charge une table que pour une autre langue.
  // Une clé absente de la table garde le texte français ; une table absente remet toute la page en fr.
  if (langue !== "fr") {
    var voulue = langue;
    html.classList.add("i18n-attente");
    var montrer = function () { html.classList.remove("i18n-attente"); };
    var rater = function () { poser("fr"); montrer(); };
    setTimeout(montrer, 1500);
    var s = document.createElement("script");
    s.src = "i18n/" + voulue + ".js";
    s.onload = function () {
      var t = window.MM_I18N && window.MM_I18N[voulue];
      if (!t) return rater();
      if (POLICES[voulue]) {
        var lien = document.createElement("link");
        lien.rel = "stylesheet";
        lien.href = "https://fonts.googleapis.com/css2?family=" + POLICES[voulue] + "&display=swap";
        document.head.appendChild(lien);
      }
      pret(function () { appliquer(t); montrer(); });
    };
    s.onerror = rater;
    document.head.appendChild(s);
  }

  pret(function () {
    // Sélecteur de langue : on mémorise le choix et on recharge avec ?lang=
    var choix = document.getElementById("langue");
    if (choix) {
      choix.value = langue;
      choix.addEventListener("change", function () {
        try { localStorage.setItem(CLE, choix.value); } catch (e) {}
        var u = new URL(location.href);
        u.searchParams.set("lang", choix.value);
        location.href = u.href;
      });
    }

    // Boutons Copier : presse-papiers, sinon sélection du texte pour Ctrl+C.
    var boutons = document.querySelectorAll("[data-copie]");
    for (var i = 0; i < boutons.length; i++) boutons[i].addEventListener("click", copier);

    function copier(ev) {
      var b = ev.currentTarget, code = document.getElementById(b.getAttribute("data-copie"));
      var texte = code.textContent, etiquette = b.querySelector("span");
      var bravo = function () {
        if (!b.dataset.avant) b.dataset.avant = etiquette.textContent;
        etiquette.textContent = b.getAttribute("data-ok");
        b.classList.add("ok");
        clearTimeout(b._t);
        b._t = setTimeout(function () { etiquette.textContent = b.dataset.avant; b.classList.remove("ok"); }, 2000);
      };
      var selectionner = function () {
        var r = document.createRange();
        r.selectNodeContents(code);
        var sel = getSelection();
        sel.removeAllRanges();
        sel.addRange(r);
        try { if (document.execCommand("copy")) bravo(); } catch (e) {}
      };
      if (navigator.clipboard && window.isSecureContext) navigator.clipboard.writeText(texte).then(bravo, selectionner);
      else selectionner();
    }

    // Numéro de la dernière version (facultatif : la page reste entière si l'appel échoue).
    var boite = document.getElementById("version");
    if (boite && window.fetch) {
      fetch("https://api.github.com/repos/Hizumimi/mewgenics-multi/releases/latest")
        .then(function (r) { if (!r.ok) throw r; return r.json(); })
        .then(function (j) {
          var nom = j && (j.name || j.tag_name);
          if (nom) { document.getElementById("version-nom").textContent = nom; boite.hidden = false; }
        })
        .catch(function () {});
    }
  });
})();
