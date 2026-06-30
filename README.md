# 4-osiowe ramię robotyczne z chwytakiem

Czteroosiowe ramię robotyczne z chwytakiem, sterowane mikrokontrolerem ATmega328P
(Arduino Uno), z **samodzielnie napisaną kinematyką odwrotną** liczoną na
mikrokontrolerze oraz autorską aplikacją desktopową do sterowania przez UART.

Cała konstrukcja mechaniczna została zaprojektowana od podstaw w **Autodesk Fusion**
i wykonana technologią **druku 3D**.

> Algorytm kinematyki odwrotnej napisany samodzielnie (prawo cosinusów + trygonometria).
> Warstwa obsługi (parser komend, presety, sterowanie PCA9685) powstała z pomocą narzędzi LLM.

---

## Spis treści
- [Funkcje](#funkcje)
- [Budowa mechaniczna](#budowa-mechaniczna)
- [Elektronika i zasilanie](#elektronika-i-zasilanie)
- [Kinematyka odwrotna](#kinematyka-odwrotna)
- [Protokół komunikacji](#protokół-komunikacji)
- [Aplikacja sterująca](#aplikacja-sterująca)
- [Uruchomienie](#uruchomienie)
- [Galeria](#galeria)
- [Plany rozwoju](#plany-rozwoju)

---

## Funkcje

- Sterowanie ramieniem w dwóch trybach:
  - **bezpośrednim** — podanie ID serwa i zadanego kąta,
  - **kartezjańskim** — podanie współrzędnych docelowych efektora (end-effector),
    z automatycznym przeliczeniem na kąty serw przez kinematykę odwrotną.
- Samodzielnie napisana kinematyka odwrotna liczona na ATmega328P
  (prawo cosinusów + zależności trygonometryczne).
- Płynny ruch synchroniczny — interpolacja pozycji serw tak, by ruch zakończył się jednocześnie.
- Sterowanie chwytakiem (otwieranie / zamykanie).
- Presety pozycji zapisywane w EEPROM (zachowywane po odłączeniu zasilania).
- Komunikacja z komputerem przez UART (USB).
- Konstrukcja własna — projekt 3D w Fusion, wydruk na drukarce 3D.

---

## Budowa mechaniczna

| Oś | Funkcja | Serwo |
|----|---------|-------|
| 1  | Obrót względem bazy | serwo 25 kg·cm |
| 2  | Staw ramienia (bark) | MG996R |
| 3  | Staw ramienia (łokieć) | MG996R |
| 4  | Nadgarstek / orientacja efektora | MG996R |
| —  | Chwytak | MG996R |

- **Projekt:** autorski, wykonany w Autodesk Fusion.
- **Wykonanie:** druk 3D.
- Mocny serwomechanizm (25 kg·cm) w bazie przenosi największe obciążenie —
  moment od całej masy ramienia, pozostałe stawy napędzane serwami MG996R.
- Długości członów ramienia przyjęte w obliczeniach: L1 = 11 cm, L2 = 13 cm.

---

## Elektronika i zasilanie

- **Mikrokontroler:** Arduino Uno (ATmega328P).
- **Sterownik serw:** PCA9685 (I2C, 50 Hz).
- **Zasilanie serw:** 6 V / 4 A (osobne od Arduino).
- **Komunikacja:** UART przez USB do komputera (9600 baud).

> **Uwaga dot. zasilania:** serwa, szczególnie pod obciążeniem, pobierają duże prądy
> chwilowe — dlatego zasilane są z osobnego źródła 6 V / 4 A, a nie z portu USB Arduino.

---

## Kinematyka odwrotna

Pozycja efektora zadawana jest we współrzędnych, a program oblicza kąty poszczególnych
serw potrzebne do jej osiągnięcia. Obliczenia wykonywane są **bezpośrednio na ATmega328P**:

- obrót bazy wyznaczany z `atan2(y, x)`, czyli z rzutu pozycji docelowej na płaszczyznę poziomą,
- problem sprowadzany do 2D (odległość pozioma `l` oraz odległość do celu `h`),
- kąty stawów ramienia liczone w pionowej płaszczyźnie roboczej z użyciem
  **prawa cosinusów** dla ramienia dwuczłonowego (L1, L2),
- orientacja nadgarstka korygowana względem kątów barku i łokcia,
- wynikowe kąty ograniczane do zakresu 0–180° i przeliczane na sygnał PWM.

Gdy cel leży poza zasięgiem, odległość `h` jest przycinana do `L1 + L2`.

---

## Protokół komunikacji

Komendy wysyłane są przez UART (zakończone znakiem nowej linii).

**1. Sterowanie pojedynczym serwem (ID + kąt)** — 2 cyfry ID + 3 cyfry kąta:
```
IIKKK
```
Przykład — ustawienie serwa nr 2 na 90°:
```
01090
```
> **Uwaga dot. ID:** serwa są numerowane od ID 0.

**2. Sterowanie pozycją efektora (współrzędne):**
```
k x,y,z
```
Program przelicza współrzędne na kąty serw przez kinematykę odwrotną.

**Pozostałe komendy:**

| Komenda | Działanie |
|---------|-----------|
| `o` | otwórz chwytak |
| `z` | zamknij chwytak |
| `off <id>` | wyłącz sygnał (zwolnij moment) na danym serwie |
| `save <n>` | zapisz bieżącą pozycję jako preset `n` (EEPROM) |
| `goto <n>` | przejdź do presetu `n` |
| `stop` | zatrzymaj ruch i pętlę sekwencji |
| `cfg` | wypisz bieżące pozycje wszystkich serw |

---

## Aplikacja sterująca

Autorska aplikacja desktopowa do sterowania ramieniem przez port szeregowy —
wybór portu COM, wysyłanie komend (tryb bezpośredni i kartezjański).

---

## Uruchomienie

### Wymagania
- Arduino IDE (lub PlatformIO),
- biblioteki: `Adafruit_PWMServoDriver`, `Wire`, `EEPROM`,
- Arduino Uno + kabel USB,
- moduł PCA9685,
- zewnętrzny zasilacz serw 6 V / 4 A,
- aplikacja sterująca (zob. sekcja wyżej).

### Wgranie firmware
1. Otwórz szkic z katalogu `firmware/` w Arduino IDE.
2. Doinstaluj bibliotekę `Adafruit PWM Servo Driver Library`.
3. Wybierz płytkę **Arduino Uno** i właściwy port.
4. Wgraj szkic na płytkę.

### Sterowanie
1. Podłącz zasilanie serw (wspólna masa z Arduino).
2. Uruchom aplikację sterującą i wybierz port COM Arduino.
3. Wysyłaj komendy zgodnie z [protokołem](#protokół-komunikacji).

> **Kalibracja:** zakresy PWM serw (`servoMin` / `servoMax`) oraz długości członów
> (L1, L2) są zdefiniowane w kodzie i należy dostroić je do własnej konstrukcji.

---

## Galeria

todo

---

## Plany rozwoju

- [ ] Rozbudowa aplikacji o wizualizację pozycji ramienia.
- [ ] Profil przyspieszania/hamowania ruchu (easing) zamiast interpolacji liniowej.
- [ ] Kalibracja z poziomu aplikacji (bez wgrywania firmware).
- [ ] Wymienne narzędzia na efektorze.

---

## Licencja

Projekt udostępniony na licencji MIT.
