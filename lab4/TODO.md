opis device tree

$ dtc -I dtb -O dts -o plik.dts plik.dtb

zmiana

 sec
 audio
-	status = "disabled"
+	status = "okay"

$ dtc -O dtb -I dts -o plik.dtb plik.dts

+ zrobić skrypt uruchomieniowy
