# [Osy] Final project

*Read this in other language: [**english**](https://github.com/Hanisekkus/Osy-Zaverecny_Projekt), [**czech**](https://github.com/Hanisekkus/Osy-Zaverecny_Projekt/blob/master/README.cz.md)*

Final project to succesfull finish subject [Osy] written in C++.

* The project solves IPC problem [readers-writers](https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem).

## Instalation

### C++
* Make sure you have installed GNU C/C++ compiler.
 
  > *Tested on v.9.3.0*

### Clone the project
* In terminal:

   ```bash
   git clone https://github.com/Hanisekkus/Osy-Zaverecny_Projekt.git
   ```

**_.. or_** 
* In browser:

   [**Download**](https://github.com/Hanisekkus/Osy-Zaverecny_Projekt/archive/master.zip)

## Usage

* In terminal goes to the directory with project:

   ```bash
   make
   ```
   
   > *To delete binary files:*
   > **<pre>  make clean</pre>**
   
* After compilation of project:

  ```bash
  ./server_fork [port_number]
  ```
  **_.. or_** 
   ```bash
  ./server_thread [port_number]
  ```
  
  >*For example:*
  >**<pre> ./server_fork 1234</pre>**
  
  
* Open another terminal or terminals in project directory:

  ```bash
  ./clientsSimulation [ip_or_name] [port_number] [number_of_clients](volitelné/optional)
  ```
  
  >* For example:*
  >**<pre>  ./clientsSimulation localhost 1234</pre>**

* Done

## License
[MIT](https://choosealicense.com/licenses/mit/)
