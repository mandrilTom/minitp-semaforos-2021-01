# SOR-Semáforos-1S-2021

## Mini TP de semáforos

### Alumno: Tomas Eduardo Esteban Mandril
### Legajo: 40142645/2017

En el presente informe se presenta el trabajo realizado.
El codigo fuente simula una competencia entre **4 equipos** de cocina que deben armar **1 sandwich de milanesa** cada uno.
Solo tienen acceso a un salero, una sarten que cocina una milanesa a la vez, y un horno con lugar para dos panes, *el cual se acordo puede compartirse por dos equipos a la vez*.
Por lo tanto, deben coordinar entre ellos el acceso a los mismos.
Asimismo, cada equipo realiza sus tareas de forma paralela, por lo que tambien deben coordinarse entre si.

Para coordinar las tareas *entre equipos*, se utilizaron mutex globales para los elementos a compartir (salero, sarten, horno).
Para las tareas *dentro de un mismo equipo*, se utilizaron semaforos **únicos para cada equipo** que habilitaban la tarea siguiente a una requerida para continuar.
En un momento se penso que los mutex debian ir junto con los semaforos, pero esto hacia que no se compartiesen entre equipos, por lo que se los separo.

El codigo fuente **NO** debia tener en sí mismo el orden de pasos a seguir para realizar la receta, sino que debia levantarse de un archivo.
Para esto se creo un txt con las instrucciones y, mediante fgets, strtok y strcpy, se leian los datos en el archivo.
Los mismos se almacenaban en una estructura construida para que cada equipo pueda acceder a los pasos a seguir.

Tambien se debía, a medida que funcionaba el programa, escribirse el progreso y resultado de la competencia paso a paso en un archivo de salida.
Esto se hizo con dos funciones *realizarAccion* y *accionTerminada*, que utilizar snprintf y fprintf para escribir la informacion de salida.
Para evitar la condicion de carrera, se utilizo un mutex especifico para el manejo del archivo de salida.

Asimismo, para indicar cuál equipo fue el ganador, se utilizó un arreglo que guarda los id de los equipos en el orden que fueron terminando la tarea pedida, y el acceso al mismo fue manejado mediante un mutex particular.
