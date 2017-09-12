# Project ripple-simulator

This project is a fork of the Ripple Network Consensus Protocol [simulator](https://github.com/ripple/simulator).

The original codes simulates avalanche to consensus with a network of randomly-connected
nodes that sparsely, randomly trust each other.

This fork tides up the code in different files to make it easier and cleaner to read.
Besides that, the main motivation for this fork is to separate the network creation from the simulation run.

By allowing to run a parametrized network, this project will be useful to analyse how different network topologies impact the consensus process.

## Compiling

Before compiling, it is required to have the following libraries installed in the system:

* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

* [sparsehash](https://github.com/sparsehash/sparsehash) 

It is required to clone or download the project and install it. If using Ubuntu, for simplicity run this instead:

```
sudo apt-get install libjsoncpp-dev libsparsehash-dev
```

After having the libraries installed in the system just run:

```
make
```

## Running

The make command produces to executable files. To do a simple, random JSON file generator, simply run:

```
./gen outputFile.json
```

This will produce the output of the original simulator network into the `outputFile.json` file.

In order to run the simulation for any custom file (named `inputFile.json`), just run:

```
./sim inputFile.json
```

All the test files are located in the `json/` subfolder.


## About the JSON file

Here is an example JSON file showing the structure used for the simulation. More nodes and links should be provided, this is just an example.

```json
{
	"numNodes": 11,
	"unlThresh": 2,
	"nodes": [{
		"nodeId": 0,
		"vote": 1,
		"latency": 401,
		"uniqueNodeList": [4, 1, 3, 5, 2]
	}, {
		"nodeId": 1,
		"vote": -1,
		"latency": 74,
		"uniqueNodeList": [4, 0, 3, 5, 2]
	} 

	],
	"links": [{
		"from": 0,
		"to": 1,
		"latency": 228
	}, {
		"from": 1,
		"to": 9,
		"latency": 110
	}
	
	]
}
```

The first attribute is **numNodes** which should be consistent with the **nodes** list attribute size. There is no consistency check.

The **unlThresh** attribute is the amount of nodes in the UNL to consider consensus. This should be change to a percentage value from 0 to 100 in the future.

Then there are two lists: **nodes** and **links**.

Each **node** in the list has a **nodeId** that has to go from **0** until **(numNodes - 1)**. The **vote** attribute has to be either **1** (loyal or consistent) or **-1** (faulty or dissident). Every **nodeId** in the **uniqueNodeList** must exist. The same goes for the **to** and **from** attributes for each **link**. The consistency checks have not been implemented yet.

Each node transmits the messages through the network using the links and they vote on transactions regarding the votes of trusted nodes in their UNL (unique node list).

## Related project

In order to generate JSON files with more flexibility, another auxiliary project [graph-builder](https://github.com/sebastiandagostino/graph-builder) was created in Java. All the test files in the `json/` folder where generated with that tool (and prettyfied with [JSONLint](https://jsonlint.com/))

## Built With

* [jsoncpp](https://github.com/open-source-parsers/jsoncpp) - Used to parse JSON streams

* [sparsehash](https://github.com/sparsehash/sparsehash) - Used for efficent hashmaps usage

## Author

* **Sebastian D'Agostino** - *Code Refactoring*

See the list of the [original contributors](https://github.com/ripple/simulator/graphs/contributors).

## Notes from the original authors regarding the simulation run

1) 8 second worst-case convergence seems possible with sparse trust over a
realistic simulated Internet.

2) Supressing validations, other than over very slow links, seems to do more
harm than good. Every message not sent increases convergence time, resulting
in more messages overall. Supressing validations over slow links may make
some sense, since odds are the node will get the validation from a faster
path first anyway. (To some extent, this happens automatically, unless the
pipe is fat and slow.)
