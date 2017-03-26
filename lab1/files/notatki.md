# Uwagi wstępne

Reset przez minicom (^A^F^B) tylko w ekstremalnych przypadkach,
gdy nie ma dostępu - normalnie może się zepsuć płytka

Punkt 4 - opcje:

* http - statyczna strona
* http + php (apache to ostateczność)
* interpreter języka skryptowego + skrypt uruchamiany przy starcie systemu
  wybrane: ruby

# Tryb awaryjny

Odpalenie minicom:

$ minicom -D /dev/ttyUSB0 -o

configure: CTRL+A O
baudrate 115200, hardware flow control off

# Konfiguracja jądra

* na początek to co w poradniku

# Ćwiczenie

1.	tylko zainstalowaliśmy ifplugd (init script też dodaliśmy ale niepotrzebnie)
2.	Hostname systemu:
	System configuration -> System hostname
3.	NTP:
	Target packages -> Networking applications -> ntp -> ntpd + ntpdate
	Dodanie init scriptu:
	- Init script dodany przez overlay
	- $ chmod +x
	- nazwa S48ntpdate aby wykonał się przed S49ntpd
4.	package w dokumentach
5.	users table w dokumentach + ustawienia buildroota
	(root password + users table location)
