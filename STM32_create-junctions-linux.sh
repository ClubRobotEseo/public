#!/bin/sh

echo "Jonction pour les codes dsPIC30F"
cd 4_Balises/Software_Emetteur_US
ln -s ../../STM32/QS  QS
cd -

cd 4_Balises/Software_Recepteur_IR_DSPIC/Project
ln -s ../../STM32/QS  QS
cd -

echo "Jonctions pour les codes STM32"
cd 1_Propulsion/CurrentSoftware
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd 2_Strategie/CurrentSoftwareSTM32
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd 3_Actionneur/CurrentSoftwareSTM32
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd 4_Balises/Balise_Emettrice/CurrentSoftware
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd 4_Balises/Balise_Receptrice/CurrentSoftware
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd 6_IHM/CurrentSoftware
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd 7_BeaconEye/CurrentSoftware
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd R_D/HokuyoUTM30LX/Software
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd R_D/HokuyoUTM30LX/SoftwareURBCallbackInIT
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd R_D/projet_matrice_led/CurrentSoftware
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -

cd R_D/carteMosfet/CurrentSoftware
ln -s ../../STM32/QS  QS
ln -s ../../STM32/stm32f4xx  stm32f4xx
cd -