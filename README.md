# Installation

To build:

```bash
mkdir -p build
cd build
cmake ..
make 
```

To run:
```bash
cd build
./PR
```

Opis src:		
* `comm.h` -- zawiera klasę Manager. Każdy wątek ma swój Manager object, który zajmuje się komunikajcą po MPI.
            Zawiera kolejki wiadomości odebranych i tych do wysłania. 
            Odbieranie i nadawanie powinno mieć swoje wątki.
                    
* `common.h` -- importy wspólne dla wszystkich, żeby nie trzeba było tyle pisać w każdym pliku

* `config.h` -- zawiera Config który parsujemy z linii poleceń. Określa liczbę nodów itd. Generlanie nuda.

* `message.h` -- określa strukturę wiadomości przesyłanych pomiędzy Node'ami (nie wie nic o MPI).
             Centralnym elementem wiadomośći jest `word`, które określa o czym ta wiadomość jest (request na risors, przekazanie risorsa itd)

* `node.h`   -- zawiera klasę Node, czyli core tego projektu. Node'y komunikują się ze sobą w celu organizacji spotkań, wymiany zasobów itd. 
            Są ułożone w drzewo.
            To tu znajduje się cała logika związana z dogadywaniem się pomięðzy node'ami -- zbieranie risorsów, organizacja spotkan itd.
            
* `utils.h`  -- utilities





