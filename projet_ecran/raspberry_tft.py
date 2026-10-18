# type: ignore

import math
import time

import board
import digitalio
from adafruit_rgb_display import st7789
from PIL import Image, ImageDraw, ImageFont

# Configuration matérielle
cs_pin = digitalio.DigitalInOut(board.D17)
dc_pin = digitalio.DigitalInOut(board.D25)
reset_pin = digitalio.DigitalInOut(board.D24)
BAUDRATE = 10000000

# Taille de l'écran
SCREEN_WIDTH = 320
SCREEN_HEIGHT = 240

# Initialisation de l'écran ST7789 (oui c'est le TJCTM24028-SPI)
spi = board.SPI()
disp = st7789.ST7789(
    spi,
    rotation=90,
    width=SCREEN_WIDTH,
    height=SCREEN_HEIGHT,
    cs=cs_pin,
    dc=dc_pin,
    rst=reset_pin,
    baudrate=BAUDRATE,
)

# Image vide & police d'écriture
image = Image.new("RGB", (SCREEN_WIDTH, SCREEN_HEIGHT))
draw = ImageDraw.Draw(image)
font = ImageFont.truetype("/home/pi/RobotoMono-VariableFont_wght.ttf", 10)

# Paramètres de la courbe
amplitude = 100
amortissement = 0.01
frequence = 0.1

# Vitesse de défilement (pixels par seconde)
vitesse_animation = 50

# Temps au lancement
temps_debut_programme = time.time()

print("Appuie sur Ctrl+C dans le terminal pour arrêter l'animation.")


def func_to_draw(x, t, amplitude, amortissement, frequence):
    """
    Calcule la position Y d'une onde sinusoïdale amortie.
    Formule: f(x,t) = A * e^(-lambda * x) * sin(omega * t)
    """
    return amplitude * math.exp(-amortissement * x) * math.sin(frequence * t)


try:
    while True:
        # 1. Calcul du vrai temps écoulé (en secondes)
        temps_ecoule_sec = time.time() - temps_debut_programme

        # 2. Conversion du temps en décalage pour la courbe
        decalage = temps_ecoule_sec * vitesse_animation

        # Trace la courbe
        draw.rectangle((0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
                       outline=0, fill=(0, 0, 0))
        draw.line([(0, SCREEN_HEIGHT//2), (SCREEN_WIDTH, SCREEN_HEIGHT//2)],
                  fill=(50, 50, 50), width=1)
        draw.line([(SCREEN_WIDTH//4, 0), (SCREEN_WIDTH//4, SCREEN_HEIGHT)],
                  fill=(50, 50, 50), width=1)

        points_courbe = []
        for x in range(SCREEN_WIDTH):
            t = x - decalage

            y = func_to_draw(x, t, amplitude, amortissement, frequence)
            y_ecran = (SCREEN_HEIGHT // 2) - y
            points_courbe.append((x + SCREEN_WIDTH//4, int(y_ecran)))

        draw.line(points_courbe, fill=(0, 255, 0), width=2)

        # Affichage du titre
        titre = f"Sinus amortie: A={amplitude}, λ={amortissement}, ω={frequence}, t={temps_ecoule_sec:.2f}s"

        left, top, right, bottom = font.getbbox(titre)
        largeur_texte = right - left
        hauteur_texte = bottom - top

        # On centre sur l'écran
        pos_x = (SCREEN_WIDTH - largeur_texte) // 2
        pos_y = 10
        draw.text((pos_x, pos_y), titre, font=font, fill=(255, 255, 255))

        # Car l'écran est inversé
        image_corrigee = image.transpose(Image.FLIP_LEFT_RIGHT)
        disp.image(image_corrigee)

except KeyboardInterrupt:
    draw.rectangle((0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
                   outline=0, fill=(0, 0, 0))
    disp.image(image)
    print("\nAnimation arrêtée.")
