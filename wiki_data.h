#ifndef WIKI_DATA_H
#define WIKI_DATA_H

// Données des articles Wikipedia
// Format: TITRE|MOTS_CLES|CONTENU

const char* wiki_articles_data[] = {
    
    // === INFORMATIQUE ===
    "Ordinateur|computer pc machine informatique|Un ordinateur est une machine electronique programmable capable de traiter des informations. Compose d'un processeur (CPU), de memoire (RAM), de stockage et de peripheriques. Invente dans les annees 1940, il revolutionne le monde moderne.",
    
    "Internet|web reseau mondial connexion|Reseau mondial d'ordinateurs interconnectes permettant l'echange d'informations. Ne en 1969 avec ARPANET, il devient public dans les annees 1990. Protocoles TCP/IP, HTTP, DNS. Plus de 5 milliards d'utilisateurs en 2023.",
    
    "Systeme d'exploitation|OS operating system kernel|Programme principal qui gere les ressources materiel et logiciel d'un ordinateur. Interface entre utilisateur et machine. Exemples : Windows, Linux, macOS, Android. Gere la memoire, processus, fichiers et peripheriques.",
    
    "Programmation|code developpement software|Art de creer des programmes informatiques en ecrivant du code source. Langages : Python, Java, C++, JavaScript. Paradigmes : procedurale, orientee objet, fonctionnelle. Etapes : analyse, conception, codage, test, maintenance.",
    
    "Intelligence Artificielle|IA AI machine learning|Domaine informatique visant a creer des machines capables d'imiter l'intelligence humaine. Branches : apprentissage automatique, vision, langage naturel. Applications : reconnaissance vocale, conduite autonome, jeux, diagnostic medical.",
    
    "Base de donnees|database SQL NoSQL stockage|Systeme organise pour stocker et gerer des informations. Types : relationnelles (SQL), NoSQL, graph. SGBD : MySQL, PostgreSQL, MongoDB. Operations : CREATE, READ, UPDATE, DELETE (CRUD). Indexation pour performances.",
    
    "Cryptographie|chiffrement securite encryption|Science du chiffrement des informations pour assurer confidentialite et integrite. Cles symetriques et asymetriques. Algorithmes : AES, RSA, SHA. Applications : HTTPS, blockchain, signatures numeriques, mots de passe.",
    
    "Algorithme|algorithm tri recherche complexite|Sequence finie d'instructions pour resoudre un probleme. Complexite temporelle et spatiale (notation Big O). Types : tri (quicksort, mergesort), recherche (binaire), graphes (Dijkstra). Base de l'informatique.",
    
    // === SCIENCES ===
    "Physique quantique|quantum mecanique particules|Branche de la physique etudiant le comportement de la matiere a l'echelle atomique. Principes : superposition, intrication, incertitude. Applications : laser, transistor, IRM, ordinateur quantique. Revolution scientifique du 20e siecle.",
    
    "Genetique|ADN genes heredite evolution|Science de l'heredite et variation des organismes vivants. ADN : code genetique, 4 bases (ATCG). Genes : unites hereditaires. Applications : medecine personnalisee, OGM, therapie genique, criminalistique.",
    
    "Chimie organique|carbone molecules medicaments|Etude des composes contenant du carbone. Base de la vie : proteines, glucides, lipides, acides nucleiques. Petrochimie, pharmacie, plastiques. Reactions : substitution, elimination, addition.",
    
    "Astronomie|espace etoiles planetes univers|Science qui etudie les objets celestes et l'univers. Systeme solaire : 8 planetes, asteroides, cometes. Etoiles : fusion nucleaire, supernovas. Galaxies : Voie lactee, Andromede. Big Bang il y a 13,8 milliards d'annees.",
    
    "Neurologie|cerveau neurones systeme nerveux|Science du systeme nerveux et du cerveau. 86 milliards de neurones. Zones : cortex, cervelet, tronc cerebral. Maladies : Alzheimer, Parkinson, epilepsie. Plasticite cerebrale, memoire, conscience.",
    
    "Energie nucleaire|atome fission fusion centrale|Energie liberee par reactions nucleaires. Fission : uranium-235, plutonium. Fusion : deuterium + tritium = helium. Centrales nucleaires : 10% electricite mondiale. Dechets radioactifs, accidents possibles.",
    
    // === HISTOIRE ===
    "Seconde Guerre mondiale|1939 1945 Hitler nazi resistance|Conflit mondial de 1939-1945. Axe (Allemagne, Italie, Japon) vs Allies (France, UK, URSS, USA). Genocides : Shoah, 6 millions de juifs. 70 millions de morts. Bombes atomiques sur Hiroshima et Nagasaki.",
    
    "Revolution francaise|1789 republique droits homme|Revolution politique et sociale en France (1789-1799). Causes : crise economique, inegalites. Evenements : prise Bastille, Declaration droits, Terreur. Fin monarchie absolue, naissance republique moderne.",
    
    "Empire romain|Rome cesar legions antiquite|Etat antique centre sur Rome (753 av. JC - 476 ap. JC). Apogee sous Trajan : de l'Ecosse au Sahara. Legions, droit romain, aqueducs, routes. Christianisation sous Constantin. Chute : invasions barbares.",
    
    "Renaissance|15e 16e siecle art humanisme|Mouvement culturel europeen (15e-16e s.). Redecouvertes antiques, humanisme. Art : Leonard de Vinci, Michel-Ange, Raphael. Sciences : Copernic, Galilee. Imprimerie diffuse le savoir. Centre : Italie puis Europe.",
    
    "Conquete spatiale|espace lune apollo spoutnik|Course technologique USA-URSS (1957-1975). 1957 : Spoutnik, premier satellite. 1961 : Gagarine, premier homme. 1969 : Apollo 11, premiers pas lunaires (Armstrong). Stations spatiales, sondes, telescopes spatiaux.",
    
    // === GEOGRAPHIE ===
    "Rechauffement climatique|climat CO2 temperature effet serre|Augmentation temperature moyenne terrestre depuis 1850. Causes : gaz effet serre (CO2, methane). Consequences : fonte glaces, montee eaux, evenements extremes. Accord Paris 2015 : limiter +1,5°C.",
    
    "Amazonie|foret tropicale biodiversite deforestation|Plus grande foret tropicale mondiale (5,5 millions km²). Poumon planete : 20% oxygene. 400 milliards arbres, 16000 especes. Deforestation : agriculture, elevage. Peuples indigenes menaces.",
    
    "Himalaya|montagne everest nepal tibet|Chaine montagneuse Asie, 2400 km. Everest : 8849m, sommet mondial. Formation : collision plaques Inde-Asie. Glaciers : sources Gange, Yangtsé. Alpinisme extreme, sherpa, mal des montagnes.",
    
    "Sahara|desert sable oasis afrique|Plus grand desert chaud (9 millions km²). Temperature : +50°C jour, -10°C nuit. Dunes, regs, oasis. Peuples nomades : Touaregs, Bedouins. Resources : petrole, gaz, phosphates. Desertification croissante.",
    
    // === LITTERATURE ET ARTS ===
    "Shakespeare|theatre anglais hamlet romeo|William Shakespeare (1564-1616), dramaturge anglais. Oeuvres : Hamlet, Romeo et Juliette, Macbeth, Le Roi Lear. 39 pieces, 154 sonnets. Theatre Globe Londres. Influence majeure litterature mondiale.",
    
    "Musique classique|orchestre symphonie compositeur|Tradition musicale occidentale savante. Periodes : baroque (Bach), classique (Mozart), romantique (Beethoven). Instruments : cordes, vents, percussions. Formes : symphonie, concerto, sonate, opera.",
    
    "Peinture|art toile couleur musee|Art visuel utilisant pigments sur support. Techniques : huile, acrylique, aquarelle. Mouvements : Renaissance, impressionnisme, cubisme, surrealisme. Maitres : Da Vinci, Van Gogh, Picasso, Monet.",
    
    // === SCIENCES HUMAINES ===
    "Psychologie|comportement mental therapie|Science du comportement et processus mentaux. Branches : cognitive, sociale, clinique, developpementale. Therapies : psychanalyse, comportementale, humaniste. Troubles : depression, anxiete, schizophrenie.",
    
    "Economie|marche monnaie banque commerce|Science production, distribution, consommation richesses. Systemes : capitalisme, socialisme, economie mixte. Indicateurs : PIB, inflation, chomage. Marches financiers, banques centrales, mondialisation.",
    
    "Philosophie|pensee sagesse ethique morale|Recherche sagesse, verite, sens existence. Branches : metaphysique, ethique, logique, esthetique. Philosophes : Platon, Aristote, Descartes, Kant, Nietzsche. Questions fondamentales sur l'etre, connaissance, valeurs.",
    
    // === MEDECINE ===
    "Vaccination|immunite virus bacterie prevention|Stimulation systeme immunitaire contre maladies. Types : vivant attenue, inactif, ARN messager. Maladies eradiquees : variole. Calendrier vaccinal, immunite collective, hesitation vaccinale.",
    
    "Cancer|tumeur oncologie chimiotherapie radiotherapie|Proliferation anarchique cellules anormales. Causes : genetique, environnement, infections. Types : carcinomes, sarcomes, leucemies. Traitements : chirurgie, chimio, radiotherapie, immunotherapie.",
    
    "Antibiotiques|bacterie infection resistance penicilline|Medicaments contre infections bacteriennes. Decouverte : Fleming 1928 (penicilline). Types : beta-lactamines, quinolones, macrolides. Resistance bacterienne : usage excessif, evolution naturelle. Alternatives recherchees.",
    
    // === INFORMATIQUE (SUITE) ===
    "Microsoft Excel|tableur spreadsheet microsoft mal|Tableur de Microsoft, faussement considere comme solution universelle. PROBLEMES : formules cryptiques, erreurs silencieuses, corruption donnees, pas de versioning, limites performances. Utilise a tort pour bases donnees. Preferer : Python, R, vraies BDD.",
    
    "Vim|editeur texte terminal commands|Editeur de texte modal ultra-puissant pour terminaux. Modes : normal, insertion, visuel, commande. Courbe apprentissage abrupte mais efficacite maximale. Raccourcis : hjkl, dd, yy, :wq. Rival : Emacs. Adore des developpeurs hardcore.",
    
    "JavaScript|web navigateur programming langage|Langage de programmation pour web, initialement cote client. Syntaxe C-like, typage dynamique. Frameworks : React, Vue, Angular. Node.js cote serveur. Critiques : comportements bizarres, '==' vs '===', hoisting. Omnipresent malgre defauts.",
    
    NULL  // Marqueur de fin
};

#endif // WIKI_DATA_H
