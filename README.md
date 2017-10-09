# Introduction
This repo contains source code for a final project in Paralell Programming course. 

It uses OpenMPI to implement a simulated "Resistance Movement", where processes communicate inside a specific structure 
to organize meetings and exchange resources and roles.

The full problem description is available in `problem.txt` (in polish).

To summarize, the task was to create a simple simulation of a "Resistance Movement". 

We assume that some number of processes are arranged in a tree-like structure, where each node
has a parent, some number of children, and some number of neighbours on the same level. 

From time to time the process may want to organise a meeting. To do that it has to accomplish three things:

* gather some number of participants among it's adjacent nodes

* collect a piece of "Resource": a DVD or a magazine

* get acceptation from an "Acceptor" higher in the hierarchy. There is a limited number of Acceptors. 

Acceptors may randomly decide to pass their role to parent, children, 
or someone on the same tree level, but who's not a neighbour.

There is an upper bound on the number of processes that may participates 
in meetings at any given time (in the whole structure, not a in a single meeting!).

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
mpirun -np 16 ./PR
```

# Source Structure

* `ack_queue.h` -- Implementations of Queues used by acceptors to keep track of meeting requests.

* `comm.h` -- Manager class, which abstracts away OpenMPI, and implements asynchronous message queues for each process.
                    
* `common.h` -- Common imports.

* `config.h` -- Configuration with default parametrs and command-line parsing.

* `message.h` -- Processes communicate by sending Message structs. 
                 Contains the struct definition and some handy functions to manipulate it, as well as 
                 Words available to the processes.

* `node.h`   -- Node class, which is the heart of the solution. Core logic lives here.

* `utils.h`  -- General utilities.

# Our solution

Here we briefly describe the solution that was implemented. 

The project was quite fun for us, since it was the first time we 
developed an "application" based on message passing. 

The environment had to be fully asynchronous, with no global state, so it took 
us some time to come up with algorithms that would implement all of the requirements.

## Initial setup
All `Node`s are created in a single thread (`MPI root`), 
and then the `Manager`s communicate to receive serialized `Node`s. This let's us easily set up the tree-like structure, 
as well as distribute resources and acceptor roles in a safe and easy manner.

## Communication protocols

### Abstracting away MPI
Since we wanted to think only about what's important, we've abstracted the `OpenMPI` away, so that we could
 think on a slightly higher level of abstraction. Only `Manager` class knows about `MPI`, and the rest of the
 code uses queues published by the `Manager`.
 
### Message struct
We use a `Message` struct, that contains several pieces of information: a Lamport Timestamp, destination 
(specific process or broadcast), optional payload, unique identifier and a `Word`, which defines how the `Message` should
be handled. 

The most important field in `Message` is the `word`. Each node has a mapping from `Word` to a handler. Some `Words` require
additional payload to be passed with the `Message` (we have `8` `32-bit` `int`s for that)
 
### Sending Messages
After creating a `Message`, the `Node` passes it to its `Manager` in a non-blocking fashion. The `Manager` takes care of safely putting the message
into asynchronous queue, from which it will later fetch it, and send using MPI. 

When a message is created, it's `destination` field is filled with either an `int`, or the `ALL` token, 
which means that each `Node` should consume the message. Then, the message is simply passed to all adjacent nodes.

Each `Node` keeps a cache of seen messages: seen messages are ignored and not handled in any way. Recall that `Message` 
contains a unique identifier in a form of `(sender, serial-number)` pair.

Messages that are neither `broadcast` nor for a specific `Node`, simply get passed further, so that they're guaranteed 
to eventually reach their destination.

We assume queues are `FIFO` queues and are not limited in size.

## Gathering participants
This was the easiest part of the project. 

Each `Node` has a `meeting state`, which takes one of the available values. See code for details.

A `Node` which decides to organize a meeting sets its state to `Organiser`. 

Since we're given an upper bound on the number of processes that may participate in a meeting in the whole structure, 
we're selecting at most `max processes` processes from our neighbours, children and parent, and send an invitation token.

Then we wait for all the responses. If at least `k` processes accept our invitation, we start looking for a `Resource`.

A process that gave us a positive response marks its state as `Waiting`, and it will respond with a word `Deny` to any
other meeting offers.

If we fail to execute the meeting, a `Invite Cancel` is sent to all participants, so that they can unlock their states.

## Gathering resources
There is a limited number of `Resource` items in the structure. Each process can have more than one `Resource`. 

Once a `Node` have gathered enough participants, it asks for `Resource`. Several things may happen next.

* We already have it. In this case we're all good, and we can start looking for `Acceptor`

* We do not have it. In this case we'll send a `Resource Request`, adressed to `ALL` nodes hoping someone will give it to use.

* We have it, but it's locked -- perhaps someone else asked for this. In that case mark it as `Needed`. If the 
requesting process cancels the request -- we consume the resource. Else, we do as above -- ask for it.

It may happen that we receive several responses. In that case, we send a confirmation to the first one, and 
cancellation to the rest. 

If we're receiving a request for resource, again, several things may happen depending on our state. 

If we do not have it, we do not respond. But we put the asking process into a queue -- once we have a resource, we will
answer. This make sure that there is no situation in which the process is waiting for a resource indefinitely. 

Same happens if the resource is available, but locked. 

If it is available and not locked, we send a positive reponse and lock the resource until we get either a confirmation or
a cancellation.

We also check the queue from time to time (e.g. after finishing a meeting), and offer the resource to anyone who 
has requested it while we were using it.

## Getting Acceptance
Before processes can start a meeting, they need to get a permission from a dedicated `Acceptor` process that is 
higher in the hierarchy. 

The problem is that at any given time, in the whole tree, only `p` processes can meet at the same time. 

We solve this problem by using `Request queues` and `Report queues`.

### Each process is an Acceptor
Each process in our tree has all the infrastructure of an `Acceptor`, and each process handles `Request`s and `Report`s.
The only difference between true `Acceptor` and an ordinary process is that ordinary process does not send any messages:
it processes each message it gets, prepares a response, and then instead of sending it, puts it into cache. 

This is necessary if we want to implement acceptor passing later on.

### Requests and Reports
A process will send a request to everyone, hoping that someone will give it permission to meet. 

In the request, it includes it's Lamport timestamp, and number of processes that will take part in the meeting.

Now, each `Acceptor` has a `Request queue`, where each item is a pair of values: `(Request, Report Queue)`. This is a priority queue ordered by request timestamps. 

We'll talk about `Report Queue` in a moment.

If there is no entry with the current `Request` in that `Request Queue`, a new one is created and placed accordingly to 
its timestamp.

Then this entry is modified: an entry is added its `Report Queue`. 

This entry is a `Report` message. This message contains all the information about the `Request` that triggered the `Report`
generation, and additionally information about the `Acceptor`: its level in the hierarchy, and timestamp when the `Request` was
received. 

This `Report` is then sent to all other Nodes.

Upon receiving the `Report` a similar procedure as described above is triggered: we update the right `Report Queue`, by
inserting the `Report`.

`Report queue` is also a priority queue, ordered by the timestamp when the coupled `Request` was received.

### Generating responses
Now, how can we use these queues to give response to a process?

First of all, we have to assume that we now the number of `Acceptor`s in the tree. This is not a very strong assumption, 
since we can always compute this at the beginning of event loop, by sending a single query message from each `Node`.

When an entry in the `Request queue` is modified, a series of checks is triggered:

* Do we have a `Report` from each acceptor for this particular `Request`? If not, ignore it, and wait.

* Are all acceptors lower in the hierarchy than requesting process, and our `Report` has the lowest timestamp? 
if so, send a `Deny` signal.

* Are all the `Requests` that have lower timestamp already deployed? If not, let's wait.
    
* If we sum the number of processes in all the `Request`s with lower timestamps, do we exceed the maximum allowed number?
If so, ignore for now.

* Are we the first (according to timestamp on `Report`) process that is higher in the hierarchy than the requesting 
process, and none of the aforementioned checks was fired? We can send an `Accept`.

* Else, we wait, because someone else will answer the `Request`

Once a process receives an `Accept`, it sends messages to it's invitees, waits for responses and sends a `Fulfilled` 
message, which allows `Acceptors` to delete the entry from queue.

## Acceptor Passing Procedure
It is more tricky that it seems to make sure we do not miss any message when the token is being passed.

For example what happens if the new `Acceptor` ignores a message, beacuse it is not yet an acceptor, and the old acceptor 
ignores it, because at the time it receives it, the `Acceptor` token was already sent to the new process?

To overcome this we implemented a solution where each process is an `Acceptor`, but only some of them are active. 

When two processes decide that they want to exchange acceptor token (in a similar fashion to how resource exchange works),
the old process goes into `Retired` mode, and new one into `Taking Over` mode. 

A unique `Acceptor ID` is now in the new process. Before it handles any new messages, it replaces all `Acceptor ID == -1` 
values (placeholders) with its new `Acceptor ID`.

From now on, for other processes it is indistinguishable as an `Acceptor` from the old one.

Now, what about the `Retired` state? We make sure, that as long as necessary (we keep track of timestamps for each process),
whenever we receive a message, we send to the `Taking Over` acceptor for testing. 

The testing means, that the `Taking Over` process checks in its cache, if it is one of the messages that was seen, but 
ignored due to the fact that the `Taking Over` was inactive at the time of receiving. If so, the generated response 
is sent.