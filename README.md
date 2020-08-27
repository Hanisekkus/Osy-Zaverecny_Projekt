# [Osy] Závěrečný projekt/Final project

[CZ] Závěrečný projekt do předmětu Osy psán v C++.
  
* [CZ] Projekt řeší IPC problém [spisovatelé-čtenáři](https://cs.qwe.wiki/wiki/Readers%E2%80%93writers_problem).

[ENG] Final project to succesfull finish subject [Osy] written in C++.

* [ENG] The project solves IPC problem [readers-writers](https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem).

## Instalace/Instalation

### C++
* [CZ] Ujisti se, že máš GNU C/C++ kompilátor.
* [ENG] Make sure you have installed GNU C/C++ compiler.
 
  > *[CZ] Vyzkoušeno na verzi 9.3.0*
  
  > *[ENG] Tested on v.9.3.0*

### Naklonuj si projekt/Clone the project
* [CZ] V terminálu:<br />
[ENG] In terminal:

   ```bash
   git clone https://github.com/Hanisekkus/Osy-Zaverecny_Projekt.git
   ```

**_.. nebo/or_** 
* [CZ] V prohlížeci:<br />
[ENG] In browser:

   [**Stáhnout/Download**](https://github.com/Hanisekkus/Osy-Zaverecny_Projekt/archive/master.zip)

## Spuštení/Usage

* [CZ] V terminálu ve složce s projektem:<br />
[ENG] In terminal goes to the directory with project:

   ```bash
   make
   ```
   
   > *[CZ] Pro smazání binárek:<br />*
   > *[ENG] For delete binary files:*
   > **<pre>  make clean</pre>**
   
* [CZ] Po kompilaci projektu:<br />
[ENG] After compilation of project:

  ```bash
  ./server_fork [port_number]
  ```
  **_.. nebo/or_** 
   ```bash
  ./server_thread [port_number]
  ```
  
  >*[CZ] Například:<br />*
  >*[ENG] For example:<br />*
  >**<pre> ./server_fork 1234</pre>**
  
  
* [CZ] Otevři si další terminál/y ve složce s projektem:<br />
[ENG] Open another terminal or terminals in project directory:

  ```bash
  ./clientsSimulation [ip_or_name] [port_number] [number_of_clients](volitelné/optional)
  ```
  
  >*[CZ] Například:<br />*
  >*[ENG] For example:*
  >**<pre>  ./clientsSimulation localhost 1234</pre>**

* Hotovo/Done

## License
[MIT](https://choosealicense.com/licenses/mit/)
