# [Osy] Závěrečný projekt

*Možné číst v těchto jazycích: [**angličtina**](https://github.com/Hanisekkus/Osy-Zaverecny_Projekt), [**čeština**](https://github.com/Hanisekkus/Osy-Zaverecny_Projekt/blob/master/README.cz.md)*

Závěrečný projekt do předmětu Osy psán v C++.
  
* Projekt řeší IPC problém [spisovatelé-čtenáři](https://cs.qwe.wiki/wiki/Readers%E2%80%93writers_problem).

## Instalace

### C++
* Ujisti se, že máš GNU C/C++ kompilátor.
 
  > *[CZ] Vyzkoušeno na verzi 9.3.0*  

### Naklonuj si projekt
* V terminálu:

   ```bash
   git clone https://github.com/Hanisekkus/Osy-Zaverecny_Projekt.git
   ```

**_.. nebo_** 
* V prohlížeci:

   [**Stáhnout**](https://github.com/Hanisekkus/Osy-Zaverecny_Projekt/archive/master.zip)

## Spuštení

* V terminálu ve složce s projektem:

   ```bash
   make
   ```
   
   > *Pro smazání binárek:*
   > **<pre>  make clean</pre>**
   
* Po kompilaci projektu:

  ```bash
  ./server_fork [port_number]
  ```
  **_.. nebo_** 
   ```bash
  ./server_thread [port_number]
  ```
  
  >*Například:*
  >**<pre> ./server_fork 1234</pre>**
  
  
* Otevři si další terminál/y ve složce s projektem:

  ```bash
  ./clientsSimulation [ip_or_name] [port_number] [number_of_clients](volitelné/optional)
  ```
  
  >*Například:*
  >**<pre>  ./clientsSimulation localhost 1234</pre>**

* Hotovo

## License
[MIT](https://choosealicense.com/licenses/mit/)
