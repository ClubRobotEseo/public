#!/bin/sh

echo "Supression des jonctions DISPIC"
rm 4_Balises/Software_Emetteur_US/QS
rm 4_Balises/Software_Recepteur_IR_DSPIC/Project/QS

echo "Supression des jonctions STM32"
rm 1_Propulsion/CurrentSoftware/QS
rm 1_Propulsion/CurrentSoftware/stm32f4xx
rm 2_Strategie/CurrentSoftwareSTM32/QS
rm 2_Strategie/CurrentSoftwareSTM32/stm32f4xx
rm 3_Actionneur/CurrentSoftwareSTM32/QS
rm 3_Actionneur/CurrentSoftwareSTM32/stm32f4xx
rm 4_Balises/Balise_Emettrice/CurrentSoftware/QS
rm 4_Balises/Balise_Emettrice/CurrentSoftware/stm32f4xx
rm 4_Balises/Balise_Receptrice/CurrentSoftware/QS
rm 4_Balises/Balise_Receptrice/CurrentSoftware/stm32f4xx
rm 6_IHM/CurrentSoftware/QS
rm 6_IHM/CurrentSoftware/stm32f4xx
rm 7_BeaconEye/CurrentSoftware/QS
rm 7_BeaconEye/CurrentSoftware/stm32f4xx
rm R_D/HokuyoUTM30LX/Software/QS
rm R_D/HokuyoUTM30LX/Software/stm32f4xx
rm R_D/HokuyoUTM30LX/SoftwareURBCallbackInIT/QS
rm R_D/HokuyoUTM30LX/SoftwareURBCallbackInIT/stm32f4xx
rm R_D/projet_matrice_led/CurrentSoftware/QS
rm R_D/projet_matrice_led/CurrentSoftware/stm32f4xx
rm R_D/carteMosfet/CurrentSoftware/QS
rm R_D/carteMosfet/CurrentSoftware/stm32f4xx
