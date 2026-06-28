# 4-osiowe ramię robotyczne z kinematyką odwrotną

Czteroosiowe ramię robotyczne sterowane mikrokontrolerem ATmega328P (Arduino Uno),
z **kinematyką odwrotną liczoną bezpośrednio na mikrokontrolerze** (bez bibliotek
zewnętrznych) oraz autorską aplikacją desktopową do sterowania przez UART.

Cała konstrukcja mechaniczna została zaprojektowana od podstaw w **Autodesk Fusion**
i wykonana technologią **druku 3D**.

<!-- Wstaw tutaj główne zdjęcie ramienia -->
<!-- ![Ramię robotyczne](docs/img/arm_main.jpg) -->

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
- Kinematyka odwrotna liczona w czasie rzeczywistym na ATmega328P, bez bibliotek
  zewnętrznych (prawo cosinusów + zależności trygonometryczne).
- Komunikacja z komputerem przez UART (USB).
- Autorska aplikacja desktopowa do wygodnego wysyłania komend.
- Konstrukcja własna — projekt 3D w Fusion, wydruk na drukarce 3D.

---

## Budowa mechaniczna

| Oś | Funkcja | Serwo |
|----|---------|-------|
| 1  | Obrót względem bazy | serwo 25 kg·cm |
| 2  | Staw ramienia | MG996R |
| 3  | Staw ramienia | MG996R |
| 4  | Staw ramienia / efektor | MG996R |

- **Projekt:** autorski, wykonany w Autodesk Fusion.
- **Wykonanie:** druk 3D.
- Mocny serwomechanizm (25 kg·cm) w bazie przenosi największe obciążenie —
  moment od całej masy ramienia, pozostałe stawy napędzane serwami MG996R.

<!-- Wstaw zdjęcie modelu z Fusion oraz wydrukowanych części -->
<!-- ![Model CAD](docs/img/fusion_model.jpg) -->

---

## Elektronika i zasilanie

- **Sterownik:** Arduino Uno (ATmega328P).
- **Zasilanie:** zewnętrzny zasilacz laboratoryjny (regulowane napięcie/prąd dla serw).
- **Komunikacja:** UART przez USB do komputera.

> **Uwaga dot. zasilania:** serwa, szczególnie pod obciążeniem, pobierają duże prądy
> chwilowe — dlatego użyto osobnego zasilacza laboratoryjnego zamiast zasilania z USB
> Arduino. Masy (GND) zasilacza i Arduino muszą być wspólne.

<!-- Wstaw schemat połączeń / zdjęcie elektroniki -->

---

## Kinematyka odwrotna

Pozycja efektora zadawana jest we współrzędnych, a program oblicza kąty poszczególnych
serw potrzebne do jej osiągnięcia. Obliczenia wykonywane są **bezpośrednio na
ATmega328P**, bez gotowych bibliotek:

- obrót bazy wyznaczany z rzutu pozycji docelowej na płaszczyznę poziomą,
- kąty stawów ramienia liczone w pionowej płaszczyźnie roboczej z użyciem
  **prawa cosinusów** oraz zależności trygonometrycznych,
- wynikowe kąty przeliczane na sygnał PWM sterujący serwami.

<!-- Opcjonalnie: wstaw rysunek/szkic geometrii ramienia z oznaczeniami kątów -->

---

## Protokół komunikacji

Komendy wysyłane są przez UART. Obsługiwane są dwa typy poleceń:

**1. Sterowanie pojedynczym serwem (ID + kąt):**
```
<ID serwa> <kąt>
```
Przykład — ustawienie serwa nr 2 na 90°:
```
2 90
```

**2. Sterowanie pozycją efektora (współrzędne):**
```
<X> <Y> <Z>
```
Program przelicza współrzędne na kąty serw przez kinematykę odwrotną.

> Format komend uzupełnię/doprecyzuję zgodnie z finalną implementacją.

---

## Aplikacja sterująca

Autorska aplikacja desktopowa do sterowania ramieniem przez port szeregowy —
wybór portu COM, wysyłanie komend (tryb bezpośredni i kartezjański).

<!-- DO UZUPEŁNIENIA — dołączę kod aplikacji i szczegóły (język, biblioteki, GUI) -->
<!-- ![Aplikacja](docs/img/app_screenshot.jpg) -->

---

## Uruchomienie

### Wymagania
- Arduino IDE (lub PlatformIO),
- Arduino Uno + kabel USB,
- zewnętrzny zasilacz dla serw,
- aplikacja sterująca (zob. sekcja wyżej).

### Wgranie firmware
1. Otwórz szkic z katalogu `firmware/` w Arduino IDE.
2. Wybierz płytkę **Arduino Uno** i właściwy port.
3. Wgraj szkic na płytkę.

### Sterowanie
1. Podłącz zasilanie serw (wspólna masa z Arduino).
2. Uruchom aplikację sterującą i wybierz port COM Arduino.
3. Wysyłaj komendy zgodnie z [protokołem](#protokół-komunikacji).

---

## Galeria

<!-- Wstaw tutaj zdjęcia i ewentualnie wideo z działania ramienia -->
<!-- ![Ramię w ruchu](docs/img/arm_action.jpg) -->

---

## Plany rozwoju

- [ ] Dodanie chwytaka jako wymiennego narzędzia efektora.
- [ ] Zapis i odtwarzanie sekwencji ruchów.
- [ ] Płynna interpolacja ruchu między pozycjami.
- [ ] Rozbudowa aplikacji o wizualizację pozycji ramienia.

---

## Licencja

<!-- Wybierz licencję, np. MIT — mogę dodać plik LICENSE jeśli chcesz -->
Projekt udostępniony na licencji MIT (do uzupełnienia).
