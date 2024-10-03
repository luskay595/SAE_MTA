Pour modéliser la dynamique des populations humaines \( H(t) \) et zombies \( Z(t) \) en tenant compte des interactions entre ces deux populations, on peut proposer le système suivant d'équations différentielles.

### Hypothèses du modèle :

1. **Diminution de la population humaine** :  
   Le taux de reproduction effectif des humains diminue avec l'augmentation des zombies, car plus il y a de zombies, plus il y a de victimes humaines. Cela implique une diminution de \( H(t) \) proportionnelle à la population de zombies \( Z(t) \).

2. **Augmentation de la population de zombies** :  
   Le taux de reproduction des zombies augmente avec l'augmentation des humains, car les humains infectés deviennent des zombies. Cela implique une augmentation de \( Z(t) \) proportionnelle à la population humaine \( H(t) \).

### Proposition de système d'équations différentielles :

\[
\begin{cases}
H'(t) = rH(t) - \alpha H(t) Z(t), \\
Z'(t) = \beta H(t) Z(t) - \delta Z(t).
\end{cases}
\]

### Explications :

1. **Équation pour la population humaine \( H(t) \)** :
   \[
   H'(t) = rH(t) - \alpha H(t) Z(t).
   \]
   - Le terme \( rH(t) \) modélise la croissance naturelle des humains sans influence des zombies (comme dans l'équation de Malthus).
   - Le terme \( -\alpha H(t) Z(t) \) représente la diminution de la population humaine due aux attaques de zombies, où \( \alpha \) est un paramètre positif qui quantifie l'intensité de ces interactions.

2. **Équation pour la population de zombies \( Z(t) \)** :
   \[
   Z'(t) = \beta H(t) Z(t) - \delta Z(t).
   \]
   - Le terme \( \beta H(t) Z(t) \) représente l'augmentation de la population de zombies causée par l'infection d'humains. Ici, \( \beta \) est un paramètre positif qui quantifie l'efficacité de la transformation des humains en zombies.
   - Le terme \( -\delta Z(t) \) modélise la disparition des zombies (soit par mort naturelle, extermination, ou autre cause), avec un taux de disparition \( \delta \).

### Conclusion :

Le système proposé satisfait les conditions décrites :
- La population humaine diminue en fonction du nombre de zombies présents.
- La population de zombies augmente en fonction du nombre d'humains infectés.
