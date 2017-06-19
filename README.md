# ripple-simulator

This simulates avalanche to consensus with a network of randomly-connected
nodes that sparsely, randomly trust each other.

PLEASE ADD YOUR OWN NOTES TO THIS FILE. We'll mirror our collected wisdom
into the Wiki.

1) 8 second worst-case convergence seems possible with sparse trust over a
realistic simulated Internet.

2) Supressing validations, other than over very slow links, seems to do more
harm than good. Every message not sent increases convergence time, resulting
in more messages overall. Supressing validations over slow links may make
some sense, since odds are the node will get the validation from a faster
path first anyway. (To some extent, this happens automatically, unless the
pipe is fat and slow.)

