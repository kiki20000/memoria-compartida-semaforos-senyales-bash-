#!/bin/bash

ERR_NOT_NUMBER=1
ERR_OUT_OF_RANGE=2

MIN_HIJOS=2
MAX_HIJOS=12
MIN_PIEZAS=1
MAX_PIEZAS=10


Number(){
	#----------------------------------
	# NO TOCAR - PODEIS USARLA EN CheckArguments() para determinar si un parámetro  es numérico
	# Parámetros: eimo-
	#    $1 : parametro a comprobar
	# Salida:
	#    1  : si es numérico
	#    0  : si no es numérico
	#----------------------------------
	if [ "$1" -eq "$1" ] 2>/dev/null; then
		return 1
	else
		return 0
	fi
}


Sintaxis(){
	#----------------------------------
	# NO TOCAR
	#----------------------------------
	#Debe de mostrar la sintaxis de uso del programa, con la explicación de los parametros.
	echo "-------------------------------------------------------------------------------------------"
	echo "pintar.sh <numHijos> <maxPiezas>"
	echo ""
	echo "Los parametros son obligatorios:"
	echo "numHijos     : Es el número de hijos que se van a lanzar"
	echo "               El valor mínimo para este parámetro es $MIN_HIJOS"
	echo "               El valor máximo para este parámetro es $MAX_HIJOS"
	echo "maxPiezas    : Es el número máximo de piezas que puede asumir un hijo"
	echo "               El valor mínimo para este parámetro es $MIN_PIEZAS"
	echo "               El valor máximo para este parámetro es $MAX_PIEZAS"
	echo "-------------------------------------------------------------------------------------------"
}

StrLen(){
	#----------------------------------
	# NO TOCAR
	#----------------------------------
	#Devuelve la longitud de la cadena que se pasa por parametro
	str=$1
	echo ${#str}
}


ToNumber(){
	#----------------------------------
	# NO TOCAR
	#----------------------------------
	#Convierte una cadena de caracteres a numero.
	#No hace compobaciones todos los caracteres deben ser numeros. Se adminte el . decimal
	str=$1
	echo $(bc -l <<<"${str}")
}

GetFileName(){
	#----------------------------------
	# NO TOCAR
	#----------------------------------
	#Devuelve el nombre del fichero en función del número pasado, teniendo en cuenta de poner un 0
	#delante de los digitos 0 al 9
	child=$1
	if [ $(StrLen $child) -eq 1 ]; then
		#Componemos el nombre del fichero antenponiendo un 0 al valor de $child
		fichero="Child0${child}Data.txt"
	else
		#Componemos el nombre del fichero utilizando simplemente $child
		fichero="Child${child}Data.txt"
	fi
	echo $fichero	
}

MostrarResultados(){
	#----------------------------------
	# NO TOCAR
	#----------------------------------
	echo ""
	echo "Informe ---------------------"
	echo "Numero de hijos $NUM_HIJOS"
	echo "Maximo de piezas por hijo $NUM_PIEZAS"

	totDelay=0
	for i in `seq 0 $((NUM_HIJOS-1))`;
	do
		echo "Hijo $i:"
		echo "  NumPiezas: ${piezasHijos[$i]} Delay:${delays[$i]}"
		totDelay=$(echo $totDelay+${delays[$i]} | bc)
	done

	echo "Total Delays: $totDelay"
	echo "-----------------------------"
}



#***************************************************************************
# Completar las siguientes funciones según el enunciado
#***************************************************************************

CheckArguments(){
#----------------------------------
# BEGIN TODO
#----------------------------------
	#Comprobamos que el número de parametros es 2
	if [ $# -ne 2 ]; then

		Sintaxis
		exit 0

	fi

	#Comprobamos que el parámetro numHijos es un número

	if Number $1 ; then
		echo no es num
		Sintaxis
		exit 0

	fi


	#Comprobamos el rango del parámetro numHijos
	if [ $1 -lt $MIN_HIJOS ] || [ $1 -gt $MAX_HIJOS ]; then

		Sintaxis
		exit 0

	fi

	#Comprobamos que el parámetro numPiezas es un número

	if Number $2 ; then

		Sintaxis
		exit 0

	fi
	#Comprobamos el rango del parámetro numPiezas
	if [ $2 -lt $MIN_PIEZAS ] || [ $2 -gt $MAX_PIEZAS ]; then

		Sintaxis
		exit 0

	fi

#----------------------------------
# END TODO
#----------------------------------
}


LoadPiezas(){
	# Asignamos a la variable num hijos el parametro pasado
	numHijos=$1

	#Iteramos desde 0 hasta numHijos-1 
	#En el interior del bucle 
	#  Asignamos a la variable fichero en nombre del fichero a tratar en cada iteración.
	#  Los nombres de fichero son ChildNNData.txt donde NN es el numero de hijo desde 00 hasta 11 son validos.
	#  Podeis usar la funcion GetFileName para obtener el nombre del fichero correcto para el hijo $i
	#  Una vez en la variable fichero tengamos el nombre del fichero, leemos la primera linea del mismo. 
	#  La primera linea contiene el numero de piezas con formato 0%d, es decir 01, 02, 03... etc.
	#  Si usamos ToNumber podremos pasar por ejemplo el 05 (caracteres) a un 5 (numérico)
	#  Guardar los numeros de piezas de cada hijo en un array llamado piezasHijos que sera usado en la funcion MostrarResultados
#----------------------------------
# BEGIN TODO
#----------------------------------
	#Iteramos de 0 a numHijos-1
	for i in `seq 0 $((numHijos-1))`; 
	do
		#Obtenemos el nombre del fichero para el hijo $i
		fichero=`GetFileName $i`

		#Leemos el número de piezas del fichero. Ojo, aquí lee un string. Primera linea del mismo.
		read line < $fichero

		#Asignamos el numero de piezas al elemento $i del array piezasHijos. Ojo, se asigna un numérico.
		
		variableaux=`ToNumber $line`

		piezasHijos[$i]=$variableaux


	done 
#----------------------------------
# END TODO
#----------------------------------
}

LoadDelays(){
	#Iteramos desde 0 hasta numHijos-1 
	#En el interior del bucle 
	#   Obtenemos el nombre del fichero para el hijo $i
#----------------------------------
# BEGIN TODO
#----------------------------------
	#Iteramos de 0 a numHijos-1
	for i in `seq 0 $((numHijos-1))`;
	do
		#Obtenemos el nombre del fichero usando la funcion GetFileName
		fichero= GetFileName $i

		#En la variable delay obtenemos el total de delay del hijo $i 
		#usando la funcion ReadDelay a la que pasamos el nombre del fichero
		delay=$(ReadDelay $fichero)

		#Almacenamos el delay del hijo en el array delays, en la posición $i
		delays[$i]=$delay
	done
#----------------------------------
# END TODO
#----------------------------------
}

ReadDelay(){
	# Recibe como parametro el nombre del fichero a leer
	# Lee los delays que el hijo tiene en cada pieza.
	# La primera linea del fichero tiene el numero de Piezas, que leemos para saber cuantas lineas mas tenemos que leer.
	# Las siguientes lineas (una por cada pieza) tienen tres valores separados por ; con el formato:  tiempoPrevisto; tiempoUtilizado; delay
	# Es decir el ultimo campo tiene el delay de la pieza.
	# Obtenemos el delay de cada pieza y los sumamos para devolver el delay total 
	#Asignamos el parametro de 
	fichero=$1

#----------------------------------
# BEGIN TODO
#----------------------------------
	#Definimos un bloque anónimo { ... } al que le redireccionamos el fichero de entrada para que el comando read lea de éste.
	{
		#inicializamos a cero el contador total de delay
		childDelay=0

		#Leemos la primera linea del fichero que es el numero de piezas en la variable nPiezas
		read nPiezas 

		#iteramos desde 1 hasta nPiezas
		for i in `seq 1 $nPiezas`;
        do
        	#Leemos una linea del fichero
        	read -r variable 

        	#Obtenemos el tercer campo de la linea (separador es ;), que corresponde al delay
            strDelay= echo $variable | cut -d';' -f 3

            #El delay leeido es una cadana de caracteres que respresenta un numero flotante
            #La pasamos a Numérico con la funcíon ToNumber
            numDelay= ToNumber $strDelay

            #incrementamos el valor de childDelay con el delay leido. Tener en cuenta que es un numero flotante (con decimales)
            childDelay=$(echo $childDelay+$numDelay | bc)

        done   
    } < $fichero
#----------------------------------
# END TODO
#----------------------------------
    #Devolvemos el delay total del hijo.
	echo "$childDelay"
}


#----------------------------------------------
# MAIN
#----------------------------------------------

#----------------------------------
# BEGIN TODO
#----------------------------------
	#Comprobar los argumentos llamando a CheckArguments apropiadamente
	CheckArguments $*
#----------------------------------
# END TODO
#----------------------------------

#Una vez comprobado que todo correcto asignamos los parametros a variables
NUM_HIJOS=$1
NUM_PIEZAS=$2

#----------------------------------
# BEGIN TODO
#----------------------------------
	#Llamamos al ejecutable pintar (o pintarprofesor) pasándole el numero de hijos y el numero de piezas por hijo.
	./pintar $NUM_HIJOS $NUM_PIEZAS
#----------------------------------
# END TODO
#----------------------------------


#Utilizamos la funcion LoadPiezas pasándole el numero de hijos. Esta funcion dejará el array piezasHijo cargado correctamente.
LoadPiezas $NUM_HIJOS


#Llamamos a la funcion LoadDelays que nos va a dejar cargado el array delays correctamente.
LoadDelays

MostrarResultados

#----------------------------------
# FIN
#----------------------------------