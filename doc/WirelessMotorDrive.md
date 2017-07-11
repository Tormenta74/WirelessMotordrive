Wireless Motordrive
===================
Proyecto para la Actividad Práctica 4 de [Sistemas Empotrados y de Tiempo Real](https://www2.ulpgc.es/index.php?pagina=plan_estudio&ver=pantalla&numPantalla=99&nCodAsignatura=40840&codTitulacion=4008&codPlan=40&codEspecialidad=02), en la ULPGC.

Por Diego Sáinz de Medrano.

Tabla de contenidos
--------------------------
[TOC]
Componentes
------------------
Se han usado las siguientes piezas de hardware:

- RD02 - 12V robot drive [de Robot Electronics][1]
- LCD05 - 16x2 green [de Robot Electronics][2]
Montaje
----------
Operación del robot
--------------------------

```sequence
Note left of Timer2: each 512ms...
Timer2->loop: lcd_update_flag = true
Note right of loop: LCD05 command
loop-->Timer2: lcd_update_flag = false

Note left of Timer2: each 16ms...
Timer2->loop: speed_check_flag = true
Note right of loop: RD02 speed regulations
loop-->Timer2: speed_check_flag = false
```


```flow
st=>start: Loop begin
e=>end: Loop end
op1=>operation: Speed regulation
op2=>operation: Liquid Display update
cond=>condition: Speed check flag?
cond1=>condition: LCD Update flag?

st->cond
cond(yes)->op1->cond1
cond(no)->cond1
cond1(yes)->op2->e
cond1(no)->e
```
Ficheros
-----------

Referencias
---------------


[1]: http://www.robot-electronics.co.uk/rd02-12v-robot-drive.html
[2]: http://www.robot-electronics.co.uk/lcd05-16x2-green.html