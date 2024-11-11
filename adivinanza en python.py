import random

def adivina_el_numero():
    print("Bienvenido al juego de 'Adivina el Número'")
    print("Estoy pensando en un número entre 1 y 100.")
    
    numero_secreto = random.randint(1, 100)
    intentos = 10  # Número de intentos permitidos
    
    # Comando de adivinanza
    while intentos > 0:
        try:
            adivinanza = int(input(f"Tienes {intentos} intentos restantes. Ingresa tu número: "))
            
            if adivinanza < 1 or adivinanza > 100:
                print("Por favor, ingresa un número entre 1 y 100.")
                continue

            if adivinanza == numero_secreto:
                print(f"¡Felicidades! Adivinaste el número {numero_secreto}. ¡Has ganado!")
                break
            elif adivinanza < numero_secreto:
                print("El número secreto es mayor.")
            else:
                print("El número secreto es menor.")
            
            intentos -= 1
        except ValueError:
            print("Entrada no válida. Por favor, ingresa un número entero.")
    
    if intentos == 0:
        print(f"Lo siento, se te acabaron los intentos. El número secreto era {numero_secreto}. ¡Mejor suerte la próxima vez!")

# Empieza el juego
if __name__ == "__main__":
    adivina_el_numero()
