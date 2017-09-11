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
killall PR && mpirun -np 3 ./PR
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

Roadmap:

- [ ] 1. Veirfy that threaded queues work.

- [ ] 2. Write some kind of manual test to confirm resource acquisition works.

- [ ] 3. Implement simple participants gathering as described in `solution.txt`

- [ ] 3a.    Implement a simple manual test for it

- [ ] 4. Implement aquiering acceptnace as in `solution.txt`

- [ ] 4a.    Implement a simple manual test for it

- [ ] 5. Implement the whole meeting logic 
(participants + resource + accept) in one place.

- [ ] 5a.    Implement a simple manual test for it

- [ ] 6. Implement random resource exchange 

- [ ] 7. Implement acceptor passing logic

Notes:

* Ad. 1 
So previously we had Manager that would put and retrieve 
elements from message queues with one thread. 
This should be done asynchronously, though. 
It's implemented already and compiles, but wasn't tested at runtime.

* Ad. 2 This should require writing some code that would print who sends 
resource to who, so that we can visually investigate that it works.

* Ad. 3 You could take a look at how resource acquisition is done in 
`node.cc : Node::handle(Message msg)`, starting with condition 
`if (msg.word == Words::RESOURCE_REQUEST and resource_count_ > 0)`

* Ad. 4 This is the core of the project. We can have a short discussion 
about how to do it properly

* Ad. 6 Resource should be randomly passed between nodes from time to time.

* Ad. 7 This will be hard as fuck, but it is a must-have, unfortunatelly. 
The process is somewhat described in `solution.txt`, but we will 
think about it more once everything else is done.